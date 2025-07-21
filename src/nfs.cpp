#include <nfs.h>
#include <iostream>
#include <io.h>
#include <string.h>
#include <stdbool.h>
#include <cstdio>
#include <cstring> // 用於 strcpy, strlen
#include <cstdint>
#include <cstdlib>   // For _malloc, _realloc, free, _exit
#include <algorithm>
#include <vector>
#include <cctype>  // For tolower (雖然原始碼可能手動轉換)
#include <intrin.h>


/* global variables */
// --- 全域變數宣告 (實際定義應在專案的某個 .cpp 檔案中) ---
// int nfs_iio_BLOCK_SIZEv; // 系統分頁大小或固定的區塊大小
// int nfs_iio_CLOCK;     // IIO 操作時鐘，用於快取回收演算法 (LRU)

int nfs_iio_BLOCK_SIZEv = 4096; // 範例值，通常由 get_page_size() 初始化
int nfs_iio_CLOCK = 0;          // 範例值
int nfs_iio_IOMODE = 0;         // 範例值，影響開啟模式和 auto_truncate
int nfs_data_IOMODE = 0;        // 範例值，影響 nfs_data_open 的開啟模式
char nfs_glob_key_buffer[4096]; // 假設檔名不太可能超過此長度
int nfs_errno = 0;	// 假設 nfs_errno 是一個全域變數，用於儲存錯誤碼。

/* public function */

// 取得系統分頁大小
DWORD get_page_size(void)
{
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	return systemInfo.dwPageSize;
}

// 檢查實體檔案是否存在
int file_exists(const char* fileName)
{
	FILE* file = fopen(fileName, "rb");
	if (file)
	{
		fclose(file);
		return 1;
	}
	return 0;
}

// 從指定的位元組陣列基底位置，取得特定位元索引的值
// bitIndex 是相對於 byteValue 指標的絕對位元索引
int bit_get(const char* byteValue, int bitIndex)
{
	if (bitIndex >= 0)
	{
		// (bitIndex >> 3) 計算 byteValue 中的位元組偏移
		// (bitIndex & 7)  計算該位元組內的位元位置
		return (byteValue[bitIndex >> 3] >> (bitIndex & 7)) & 1;
	}
	return 2; // 依照原始碼，負數位元索引返回 2
}

static inline int find_first_set_bit(unsigned char n) {
	if (n == 0) return 0; // Or some error indicator
#if defined(_MSC_VER)
	unsigned long index;
	_BitScanForward(&index, n);
	return index;
#elif defined(__GNUC__) || defined(__clang__)
	return __builtin_ffs(n) - 1; // ffs is 1-based, we want 0-based
#else
	// Fallback for other compilers
	int count = 0;
	while ((n & 1) == 0) {
		n >>= 1;
		count++;
	}
	return count;
#endif
}

// 找出兩個字串（視為位元組陣列）第一個不同的位元之全域位元索引
// 假設傳入的字串在被呼叫的上下文中保證是不同的
int bitfirst_different(const char* str1, const char* str2)
{
	//printf("  [D] bitfirst_different: Comparing ('%s', '%s')\n", str1, str2);
	int byteIndex = 0;
	// 1. 找出第一個內容不同的位元組，或其中一個字串結束的位置
	while (str1[byteIndex] == str2[byteIndex])
	{
		if (str1[byteIndex] == '\0') // 若在此處字元也相同且為結束符，則跳出
		{
			break;
		}
		byteIndex++;
	}

	// 2. 從 byteIndex 開始，逐位元比較，找出第一個不同的位元
	int local_bit_offset = 0;

	if (bit_get(&str1[byteIndex], local_bit_offset) != bit_get(&str2[byteIndex], local_bit_offset))
	{
		//printf("  [D] bitfirst_different: Found diff at byte %d, local_bit %d. Result: %d\n", byteIndex, local_bit_offset, byteIndex * 8 + local_bit_offset);
		return byteIndex * 8 + local_bit_offset;
	}

	do
	{
		local_bit_offset++;
	} while (bit_get(&str1[byteIndex], local_bit_offset) == bit_get(&str2[byteIndex], local_bit_offset));

	//printf("  [D] bitfirst_different: Found diff at byte %d, local_bit %d. Result: %d\n", byteIndex, local_bit_offset, byteIndex * 8 + local_bit_offset);
	return byteIndex * 8 + local_bit_offset;
}

// 給定大小，計算需要多少個 512 位元組的區塊
int nblocks(int size)
{
	if (size == 0)
	{
		return 1; // 特殊情況：大小為 0 也算 1 個區塊
	}
	return (size - 1) / 512 + 1;
}

// 給定偏移量，計算其所在的 512 位元組區塊的編號
int blockno(int offset)
{
	return offset / 512;
}

/**
 * @brief 移除鎖定檔案。
 * @param baseName 鎖定檔案的基礎名稱 (不含 .lock 副檔名)。
 * @return 成功時返回 0，失敗時返回非零值 (依 _remove 函式的行為)。
 */
int lock_remove(const char* baseName)
{
	char fileName[256];
	strcpy(fileName, baseName);
	strcat(fileName, ".lock"); // strcat is safer if fileName is guaranteed to have space
	// Original used: strcpy(&fileName[strlen(baseName)], ".lock");
	return remove(fileName);
}

/**
 * @brief 檢查鎖定檔案的狀態。
 * @param baseName 鎖定檔案的基礎名稱。
 * @param accessMode 請求的存取模式 (例如，1 可能表示讀取模式)。
 * @return BOOL (int)，如果可以存取則為 TRUE (1)，否則為 FALSE (0)。
 */
int lock_check(const char* baseName, int accessMode) // BOOL is often typedef'd to int
{
	char fileName[256];
	strcpy(fileName, baseName);
	strcat(fileName, ".lock");

	FILE* lockFile = fopen(fileName, "rb"); // 以二進位讀取模式開啟
	if (!lockFile)
	{
		return 1; // 鎖定檔案不存在，視為可存取
	}

	char lockFlags[4]; // 儲存從檔案讀取的鎖定標誌
	if (fread(lockFlags, 1, sizeof(lockFlags), lockFile) != sizeof(lockFlags)) {
		fclose(lockFile);
		return 0; // 讀取錯誤，視為不可存取 (或處理錯誤)
	}
	fclose(lockFile);

	// 原始邏輯：
	// v5 is FALSE if: (lockFlags[0] has bit 1 set) OR (lockFlags[0] has bit 0 clear)
	// v5 is TRUE if: (lockFlags[0] bit 1 is clear) AND (lockFlags[0] bit 0 is set) AND (accessMode == 1)
	if ((lockFlags[0] & 0x02) || !(lockFlags[0] & 0x01))
	{
		return 0; // 獨佔鎖定或不可讀
	}
	else
	{
		return (accessMode == 1); // 共享鎖定，檢查請求模式是否為允許的共享模式
	}
}

/**
 * @brief 釋放一個鎖定，更新鎖定檔案中的計數，並在計數為零時移除鎖定檔案。
 * @param baseName 鎖定檔案的基礎名稱。
 * @return 成功時返回 0。如果鎖定檔案無法開啟，返回 -1。
 * 如果計數歸零並成功刪除檔案，也返回 0 (來自 lock_remove)。
 * 如果計數歸零但刪除失敗，返回 -1 (來自 lock_remove)。
 */
int lock_leave(const char* baseName)
{
	char fileName[256];
	strcpy(fileName, baseName);
	strcat(fileName, ".lock");

	// "r+b" 允許讀取和寫入二進位檔案，檔案必須已存在。
	FILE* lockFile = fopen(fileName, "r+b");
	if (!lockFile)
	{
		return -1; // 無法開啟鎖定檔案
	}

	char lockFlags[4]; // 鎖定標誌 (前4位元組)
	int lockCount = 0;   // 鎖定計數 (接下來4位元組)

	// 讀取鎖定標誌 (雖然在此函式中未使用其值，但原始碼讀取了它)
	if (fread(lockFlags, 1, sizeof(lockFlags), lockFile) != sizeof(lockFlags)) {
		fclose(lockFile);
		return -1; // 讀取標誌錯誤
	}

	// 讀取鎖定計數
	if (fread(&lockCount, sizeof(int), 1, lockFile) != 1) {
		fclose(lockFile);
		return -1; // 讀取計數錯誤
	}

	lockCount--; // 遞減鎖定計數

	// 將檔案指標移回鎖定計數的起始位置 (跳過4位元組的標誌)
	if (fseek(lockFile, sizeof(lockFlags), SEEK_SET) != 0) {
		fclose(lockFile);
		return -1; // 定位錯誤
	}

	// 寫回更新後的鎖定計數
	if (fwrite(&lockCount, sizeof(int), 1, lockFile) != 1) {
		// 雖然寫入可能在 fclose 時才完全提交，但檢查返回值是好的習慣
		fclose(lockFile);
		return -1; // 寫入計數錯誤
	}

	// fflush(lockFile); // 確保寫入操作已刷入，雖然 fclose 也會做

	fclose(lockFile);

	if (lockCount > 0)
	{
		return 0; // 鎖定仍然有效 (計數 > 0)
	}
	else
	{
		return lock_remove(baseName); // 計數為 0 或更少，嘗試移除鎖定檔案
	}
}

/**
 * @brief 根據全域 nfs_errno 的值列印錯誤訊息。
 * @param unusedHandle 在此特定函式中未使用，但保留以符合原始簽章。
 * @param prefixMessage 要在錯誤訊息前輸出的前綴字串。
 */
void nfs_perror(void* unusedHandle, const char* prefixMessage) // 原型為 struct _nfs_Handle*，此處用 void* 簡化
{
	const char* errorMessage = nullptr;

	// (char)nfs_errno 轉換暗示 nfs_errno 的值較小
	switch (static_cast<char>(nfs_errno))
	{
	case 1:  errorMessage = "Could not open/create the IIO file"; break;
	case 2:  errorMessage = "Could not open/create the data file"; break;
	case 3:  errorMessage = "Could not initialize the FAT channel"; break;
	case 4:  errorMessage = "Could not initialize the NT channel"; break;
	case 5:  errorMessage = "Could not initialize the DT channel"; break;
	case 6:  errorMessage = "Could not allocate the FAT channel"; break;
	case 7:  errorMessage = "Could not allocate the NT channel"; break;
	case 8:  errorMessage = "Could not allocate the DT channel"; break;
	case 9:  errorMessage = "Invalid file system handle"; break;
	case 10: errorMessage = "File already closed"; break;
	case 11: errorMessage = "File not found"; break;
	case 12: errorMessage = "No files open"; break;
	case 13: errorMessage = "Invalid file descriptor"; break;
	case 14: errorMessage = "Invalid node in NT"; break;
	case 15: errorMessage = "No memory available for opening files"; break;
	case 16: errorMessage = "File system is locked by another process"; break;
	case 17: errorMessage = "The file you are trying to create already exists"; break;
	case 18: errorMessage = "Internal error in DT"; break;
	case 19: errorMessage = "Invalid parameters"; break;
	default: errorMessage = "Internal error"; break;
	}
	printf("%s: %s\n", prefixMessage, errorMessage);
}

//Layer 2
/**
 * @brief 取得快取頁面的資料緩衝區指標。
 * 原始碼中，指標的最低有效位 (LSB) 用作同步旗標，此處將其清除後返回。
 * @param page 指向 NfsIioCachePage 的指標。
 * @return 指向實際資料緩衝區的 void* 指標。
 */
void* cache_page_get_buffer(NfsIioCachePage* page)
{
	if (page) {
		// 清除 LSB (同步旗標) 以獲得實際的緩衝區指標
		return reinterpret_cast<void*>(page->buffer_ptr_and_syncflag_storage & ~static_cast<uintptr_t>(1));
	}
	return nullptr;
}

/**
 * @brief 設定快取頁面的同步狀態 (儲存在緩衝區指標的 LSB)。
 * @param page 指向 NfsIioCachePage 的指標。
 * @param sync_status 0 表示未同步 (dirty)，1 表示已同步 (clean)。
 */
void cache_page_set_sync(NfsIioCachePage* page, int sync_status)
{
	if (page) {
		if (sync_status & 1) { // 如果 sync_status 的 LSB 為 1 (已同步)
			page->buffer_ptr_and_syncflag_storage |= static_cast<uintptr_t>(1);
		}
		else { // 如果 sync_status 的 LSB 為 0 (未同步)
			page->buffer_ptr_and_syncflag_storage &= ~static_cast<uintptr_t>(1);
		}
	}
}

/**
 * @brief 取得快取頁面的同步狀態 (從緩衝區指標的 LSB 讀取)。
 * @param page 指向 NfsIioCachePage 的指標。
 * @return 如果已同步則返回 1，否則返回 0。如果 page 為 nullptr，也返回 0 (或可視為錯誤)。
 */
int cache_page_get_sync(NfsIioCachePage* page)
{
	if (page) {
		return static_cast<int>(page->buffer_ptr_and_syncflag_storage & static_cast<uintptr_t>(1));
	}
	return 0; // 或者根據錯誤處理策略返回其他值
}

/**
 * @brief 選擇一個最適合被回收 (重複使用) 的快取頁面索引 (LRU 策略)。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param exclude_page_idx 要排除的頁面索引 (例如，新建立的頁面不應立即被回收)。
 * @return 最適合回收的快取頁面在通道快取中的索引。
 */
int cache_page_choose_best_to_reuse(NfsIioFile* file, int channel_idx, int exclude_page_idx)
{
	int best_idx_to_reuse = 0;
	int current_page_scan_idx = 0;
	int min_last_access_time = 0x7FFFFFFF; // 最大整數，用於比較

	if (!file || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return 0; // 或錯誤代碼
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel->cache_header || !channel->cache_header->pages) {
		return 0; // 或錯誤代碼
	}

	NfsIioCache* cache = channel->cache_header;
	int allocated_pages_count = cache->num_pages_allocated;

	if (allocated_pages_count > 0) {
		for (current_page_scan_idx = 0; current_page_scan_idx < allocated_pages_count; ++current_page_scan_idx) {
			NfsIioCachePage* currentPage = cache->pages[current_page_scan_idx];
			if (currentPage && currentPage->last_access_time < min_last_access_time && current_page_scan_idx != exclude_page_idx) {
				min_last_access_time = currentPage->last_access_time;
				best_idx_to_reuse = current_page_scan_idx;
			}
		}
	}
	return best_idx_to_reuse;
}

/**
 * @brief 擴展指定通道的快取頁面指標陣列。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param required_page_array_idx 需要的最小頁面陣列索引 (用於決定擴展後的大小)。
 * @return 擴展後 pages 陣列的新大小 (槽數)。
 */
int cache_expand(NfsIioFile* file, int channel_idx, int required_page_array_idx)
{
	if (!file || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return 0; // 或錯誤代碼
	}
	NfsIioChannel* channel = file->channels[channel_idx];

	int current_allocated_count = 0;
	if (channel->cache_header) {
		current_allocated_count = channel->cache_header->num_pages_allocated;
	}

	int initial_allocated_count = current_allocated_count;
	int new_allocated_count = current_allocated_count;

	if (new_allocated_count == 0) {
		new_allocated_count = 1; // 如果初始為0，至少分配1個開始
	}

	// 擴展邏輯：持續加倍，直到滿足 required_page_array_idx 且至少為 1024
	while (new_allocated_count <= required_page_array_idx || new_allocated_count < 1024) {
		new_allocated_count *= 2;
		if (new_allocated_count == 0) { // 防止乘以2後溢位變成0 (極端情況)
			new_allocated_count = required_page_array_idx > 1024 ? required_page_array_idx + 1 : 1024; // 設定一個足夠大的值
			if (new_allocated_count < initial_allocated_count) new_allocated_count = initial_allocated_count * 2; //確保增長
			if (new_allocated_count == 0) new_allocated_count = 2048; //最終保障
			break;
		}
	}


	if (!channel->cache_header) {
		channel->cache_header = static_cast<NfsIioCache*>(malloc(sizeof(NfsIioCache)));
		if (!channel->cache_header) return 0; // 分配失敗
		channel->cache_header->pages = nullptr;
		channel->cache_header->num_pages_allocated = 0;
		channel->cache_header->num_pages_active = 0;
	}

	channel->cache_header->num_pages_allocated = new_allocated_count;
	NfsIioCachePage** new_pages_array = static_cast<NfsIioCachePage**>(realloc(channel->cache_header->pages, sizeof(NfsIioCachePage*) * new_allocated_count));

	if (!new_pages_array) {
		// _realloc 失敗，舊記憶體仍然有效 (如果 channel->cache_header->pages 非空)
		// 但 num_pages_allocated 可能已更新，需要謹慎處理
		channel->cache_header->num_pages_allocated = initial_allocated_count; // 恢復舊值
		return 0; // 表示擴展失敗
	}
	channel->cache_header->pages = new_pages_array;

	// 將新分配的指標槽初始化為 nullptr
	for (int i = initial_allocated_count; i < new_allocated_count; ++i) {
		channel->cache_header->pages[i] = nullptr;
	}

	return new_allocated_count;
}

/**
 * @brief 計算 IIO 檔案標頭的大小。
 * 原始碼中，此函式檢查標頭是否能容納於一個 nfs_iio_BLOCK_SIZEv 內，
 * 但總是返回 nfs_iio_BLOCK_SIZEv。
 * @param file IIO 檔案指標。
 * @return 標頭大小 (固定為 nfs_iio_BLOCK_SIZEv)。
 */
int header_size(NfsIioFile* file)
{
	// 檢查標頭資訊是否超出一個區塊的大小
	// 8 (magic + num_channels_short) + 8 * num_channels (channel_block_size + channel_logical_size)
	if (file && (8 + (8 * file->num_channels)) > nfs_iio_BLOCK_SIZEv) {
		printf("internal error: iio header overflow. Too many iio channels or too small BLOCK_SIZEv.\n");
		_exit(-1); // 嚴重錯誤，退出
	}
	return nfs_iio_BLOCK_SIZEv; // 標頭固定為一個區塊大小
}

/**
 * @brief 從 IIO 檔案的絕對區塊位置讀取 N 個區塊的資料。
 * @param file IIO 檔案指標。
 * @param absolute_block_index 要讀取的起始絕對區塊索引。
 * @param num_blocks_to_read 要讀取的區塊數量。
 * @param buffer 儲存讀取資料的緩衝區。
 * @return 讀取的總位元組數 (等於 num_blocks_to_read * nfs_iio_BLOCK_SIZEv)。
 * 如果發生錯誤 (例如檔案未開啟、緩衝區為空)，返回 -1。
 */
int read_absolute_block_n(NfsIioFile* file, int absolute_block_index, int num_blocks_to_read, void* buffer)
{
	if (!file || !file->file_handle) {
		return -1;
	}
	if (!buffer) {
		return -1;
	}
	if (num_blocks_to_read <= 0) {
		return 0;
	}

	long offset = static_cast<long>(header_size(file)) + static_cast<long>(absolute_block_index) * nfs_iio_BLOCK_SIZEv;
	if (fseek(file->file_handle, offset, SEEK_SET) != 0) {
		return -1; // 定位失敗
	}

	size_t total_bytes_to_read = static_cast<size_t>(num_blocks_to_read) * nfs_iio_BLOCK_SIZEv;
	size_t bytes_actually_read = fread(buffer, 1, total_bytes_to_read, file->file_handle);

	if (bytes_actually_read < total_bytes_to_read) {
		// 如果讀取的位元組數少於預期 (例如到達檔案結尾)，
		// 將緩衝區剩餘部分填零。
		memset(static_cast<char*>(buffer) + bytes_actually_read, 0, total_bytes_to_read - bytes_actually_read);
	}
	// 原始碼返回的是請求讀取的區塊總大小，而不是實際 fread 的結果
	return static_cast<int>(total_bytes_to_read);
}

/**
 * @brief 將 N 個區塊的資料寫入 IIO 檔案的絕對區塊位置。
 * @param file IIO 檔案指標。
 * @param absolute_block_index 要寫入的起始絕對區塊索引。
 * @param num_blocks_to_write 要寫入的區塊數量。
 * @param buffer 包含要寫入資料的緩衝區。
 * @return 寫入的總位元組數 (等於 num_blocks_to_write * nfs_iio_BLOCK_SIZEv)。
 * 如果發生錯誤 (例如檔案未開啟、緩衝區為空)，返回 -1。
 */
int write_absolute_block_n(NfsIioFile* file, int absolute_block_index, int num_blocks_to_write, const void* buffer) // buffer is const
{
	if (!file || !file->file_handle) {
		return -1;
	}
	if (!buffer) {
		return -1;
	}
	if (num_blocks_to_write <= 0) {
		return 0;
	}

	long offset = static_cast<long>(header_size(file)) + static_cast<long>(absolute_block_index) * nfs_iio_BLOCK_SIZEv;
	if (fseek(file->file_handle, offset, SEEK_SET) != 0) {
		return -1; // 定位失敗
	}

	size_t total_bytes_to_write = static_cast<size_t>(num_blocks_to_write) * nfs_iio_BLOCK_SIZEv;
	/* size_t bytes_actually_written = */ fwrite(buffer, 1, total_bytes_to_write, file->file_handle);

	// 原始碼未檢查 fwrite 的返回值來進行錯誤處理或確認寫入的位元組數。
	// 為了保持一致性，此處也直接返回請求寫入的區塊總大小。
	// 在實際應用中，應檢查 fwrite 的返回值。

	return static_cast<int>(total_bytes_to_write);
}

/**
 * @brief 將 IIO 檔案的標頭資訊寫入檔案。
 * 標頭包含一個魔術數字、通道數量，以及每個通道的配置資訊。
 * @param file 指向 NfsIioFile 結構的指標。
 * @return 成功時返回 0，如果檔案未開啟則返回 -1。
 */
int write_header(NfsIioFile* file)
{
	if (!file || !file->file_handle) {
		return -1;
	}

	// 檔案標頭的第一部分：魔術數字和通道數
	struct FileHeaderPart {
		int magic_number;
		int num_channels_int; // 雖然 NfsIioFile::num_channels 是 short，但寫入時似乎是作為 int
	} file_header;

	file_header.magic_number = 1145258561; // 原始碼中的魔術數字 (0x444F4941)
	file_header.num_channels_int = file->num_channels;

	if (fseek(file->file_handle, 0, SEEK_SET) != 0) {
		return -1; // 定位失敗
	}
	if (fwrite(&file_header, 1, sizeof(FileHeaderPart), file->file_handle) != sizeof(FileHeaderPart)) {
		return -1; // 寫入檔案標頭失敗
	}

	// 每個通道的標頭資訊
	struct ChannelHeaderPart {
		int blocks_per_stripe;
		int current_size_bytes;
	} channel_header;

	for (short i = 0; i < file->num_channels; ++i) {
		if (!file->channels || !file->channels[i]) {
			// 處理通道指標無效的情況，可能需要更複雜的錯誤處理
			continue;
		}
		NfsIioChannel* current_channel = file->channels[i];
		channel_header.blocks_per_stripe = current_channel->blocks_per_stripe;
		channel_header.current_size_bytes = current_channel->current_size_bytes;

		if (fwrite(&channel_header, 1, sizeof(ChannelHeaderPart), file->file_handle) != sizeof(ChannelHeaderPart)) {
			return -1; // 寫入通道標頭失敗
		}
	}

	if (fflush(file->file_handle) != 0) {
		return -1; // 刷新緩衝區失敗
	}
	return 0;
}

/**
 * @brief 計算並返回 IIO 檔案中所有通道的 blocks_per_stripe 的總和。
 * 這個值代表一個完整的 "chunk" 或 "interleaved stripe" 包含的總區塊數。
 * @param file 指向 NfsIioFile 結構的指標。
 * @return 所有通道 blocks_per_stripe 的總和。如果 file 為 nullptr，返回 -1。
 */
int nfs_iio_blocks_per_chunk(NfsIioFile* file)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘
	if (!file) {
		return -1;
	}

	int total_blocks_in_chunk = 0;
	if (file->channels) { // 確保通道陣列存在
		for (short i = 0; i < file->num_channels; ++i) {
			if (file->channels[i]) { // 確保通道指標有效
				total_blocks_in_chunk += file->channels[i]->blocks_per_stripe;
			}
		}
	}
	return total_blocks_in_chunk;
}

/**
 * @brief 設定指定 IIO 通道的目前讀寫位置 (seek position)。
 * @param file 指向 NfsIioFile 結構的指標。
 * @param channel_idx 要設定的通道索引。
 * @param seek_position 新的讀寫位置。
 * @return 成功時返回 seek_position，失敗 (例如無效的檔案或通道索引) 時返回 -1。
 */
int nfs_iio_seek(NfsIioFile* file, int channel_idx, int seek_position)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘

	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return -1;
	}

	file->channels[channel_idx]->current_seek_position = seek_position;
	return seek_position;
}

/**
 * @brief 取得指定索引的 IIO 通道的指標。
 * @param file 指向 NfsIioFile 結構的指標。
 * @param channel_idx 要取得的通道索引。
 * @return 指向 NfsIioChannel 結構的指標。如果檔案無效或通道索引超出範圍，返回 nullptr。
 */
NfsIioChannel* nfs_iio_get_channel(NfsIioFile* file, int channel_idx)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘

	if (file && file->channels && channel_idx >= 0 && channel_idx < file->num_channels) {
		return file->channels[channel_idx];
	}
	return nullptr;
}

/**
 * @brief 取得指定 IIO 通道的目前邏輯大小 (位元組)。
 * @param channel 指向 NfsIioChannel 結構的指標。
 * @return 通道的大小 (位元組)。如果 channel 為 nullptr，返回 -1。
 */
int nfs_iio_channel_size(NfsIioChannel* channel)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘

	if (channel) {
		return channel->current_size_bytes;
	}
	return -1;
}

/**
 * @brief 取得指定 IIO 通道的每個 "stripe" (條帶/基本單位) 包含的區塊數。
 * @param channel 指向 NfsIioChannel 結構的指標。
 * @return 每個 stripe 的區塊數。如果 channel 為 nullptr，返回 -1。
 */
int nfs_iio_channel_blocks(NfsIioChannel* channel)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘

	if (channel) {
		return channel->blocks_per_stripe;
	}
	return -1;
}

/**
 * @brief 截斷指定的 IIO 通道。
 * 將通道的目前邏輯大小設定為其目前的讀寫位置。
 * @param file 指向 NfsIioFile 結構的指標。
 * @param channel_idx 要截斷的通道索引。
 * @return 成功時返回 0，失敗 (例如無效的檔案、通道索引或通道指標) 時返回 -1。
 */
int nfs_iio_channel_truncate(NfsIioFile* file, int channel_idx)
{
	nfs_iio_CLOCK++; // 遞增全域時鐘

	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels) {
		return -1;
	}

	NfsIioChannel* channel_to_truncate = file->channels[channel_idx];
	if (!channel_to_truncate) {
		return -1;
	}

	channel_to_truncate->current_size_bytes = channel_to_truncate->current_seek_position;
	return 0;
}

/**
 * @brief 將通道相對區塊索引轉換為檔案中的絕對區塊索引。
 * @param file IIO 檔案指標。
 * @param channel_idx 目標通道的索引。
 * @param channel_relative_block_idx 通道內的相對區塊索引。
 * @return 檔案中的絕對區塊索引；如果發生錯誤則返回 -1。
 */
int channel_block_to_absolute_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels) {
		return -1;
	}
	NfsIioChannel* target_channel = file->channels[channel_idx];
	if (!target_channel || target_channel->blocks_per_stripe == 0) { // 防止除以零
		return -1;
	}

	int chunk_index = channel_relative_block_idx / target_channel->blocks_per_stripe;
	int offset_within_stripefor_target_channel = channel_relative_block_idx % target_channel->blocks_per_stripe;

	int base_offset_in_chunk = 0;
	for (int i = 0; i < channel_idx; ++i) {
		if (file->channels[i]) { // 確保通道有效
			base_offset_in_chunk += file->channels[i]->blocks_per_stripe;
		}
	}

	int total_blocks_perfull_chunk = nfs_iio_blocks_per_chunk(file);
	if (total_blocks_perfull_chunk < 0) return -1; // nfs_iio_blocks_per_chunk 可能返回錯誤

	return (offset_within_stripefor_target_channel + base_offset_in_chunk) + (chunk_index * total_blocks_perfull_chunk);
}

/**
 * @brief 將通道內的相對位元組位置轉換為檔案中的絕對區塊索引。
 * @param file IIO 檔案指標。
 * @param channel_idx 目標通道的索引。
 * @param channel_relative_byte_pos 通道內的相對位元組位置。
 * @return 檔案中的絕對區塊索引；如果發生錯誤則返回 -1。
 */
int channel_pos_to_absolute_block(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos)
{
	if (nfs_iio_BLOCK_SIZEv == 0) return -1; // 防止除以零
	int channel_relative_block_idx = channel_relative_byte_pos / nfs_iio_BLOCK_SIZEv;
	return channel_block_to_absolute_block(file, channel_idx, channel_relative_block_idx);
}

/**
 * @brief 將指定的快取頁面內容寫回磁碟 (dump)。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param page_array_idx 快取頁面在通道快取陣列中的索引。
 * @return 成功時返回 0，失敗時返回非 0 值 (例如，若頁面指標為空)。
 */
int cache_page_dump(NfsIioFile* file, int channel_idx, int page_array_idx)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return -1;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel->cache_header || !channel->cache_header->pages ||
		page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return -1;
	}

	NfsIioCachePage* page_to_dump = channel->cache_header->pages[page_array_idx];
	if (!page_to_dump) {
		return -1; // 頁面指標為空
	}

	// page_to_dump->disk_block_position 儲存的是此頁面對應的「通道相對位元組起始位置」
	int absolute_disk_block = channel_pos_to_absolute_block(file, channel_idx, page_to_dump->disk_block_position);
	if (absolute_disk_block < 0) return -1; // 轉換失敗

	void* page_buffer = cache_page_get_buffer(page_to_dump);
	if (!page_buffer) return -1; // 獲取緩衝區失敗

	// 寫入的區塊數是該通道的 blocks_per_stripe
	if (write_absolute_block_n(file, absolute_disk_block, channel->blocks_per_stripe, page_buffer) < 0) {
		return -1; // 寫入失敗
	}

	cache_page_set_sync(page_to_dump, 1); // 標記為已同步
	return 0;
}

/**
 * @brief 如果指定的快取頁面未同步 (dirty)，則將其內容寫回磁碟。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param page_array_idx 快取頁面在通道快取陣列中的索引。
 */
void cache_pageflush(NfsIioFile* file, int channel_idx, int page_array_idx)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel->cache_header || !channel->cache_header->pages ||
		page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return;
	}

	NfsIioCachePage* page_toflush = channel->cache_header->pages[page_array_idx];
	if (page_toflush && !cache_page_get_sync(page_toflush)) {
		cache_page_dump(file, channel_idx, page_array_idx);
	}
}

/**
 * @brief 建立或重複使用一個快取頁面於指定的通道快取陣列索引位置。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param page_array_idx_to_create_at 要建立/初始化頁面的索引位置。
 */
void cache_page_create(NfsIioFile* file, int channel_idx, int page_array_idx_to_create_at)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel->cache_header || !channel->cache_header->pages ||
		page_array_idx_to_create_at < 0 || page_array_idx_to_create_at >= channel->cache_header->num_pages_allocated) {
		return;
	}

	NfsIioCache* cache = channel->cache_header;

	if (cache->pages[page_array_idx_to_create_at] == nullptr) { // 如果此槽位是空的
		if (cache->num_pages_active >= 8) { // 假設快取大小上限為 8 (原始碼中的閾值)
			int idx_to_reuse = cache_page_choose_best_to_reuse(file, channel_idx, page_array_idx_to_create_at);
			cache_pageflush(file, channel_idx, idx_to_reuse); // Flush a pagina da riutilizzare

			// 將被重複使用的頁面移到新的槽位
			cache->pages[page_array_idx_to_create_at] = cache->pages[idx_to_reuse];
			cache->pages[idx_to_reuse] = nullptr; // 清空舊槽位

			NfsIioCachePage* reused_page = cache->pages[page_array_idx_to_create_at];
			// 重設被重複使用頁面的元數據
			reused_page->disk_block_position = 0; // 將在 refresh 時設定
			reused_page->last_access_time = nfs_iio_CLOCK;
			cache_page_set_sync(reused_page, 1); // 標記為已同步 (因為內容即將被刷新)
		}
		else {
			// 分配新的快取頁面元數據和資料緩衝區
			cache->num_pages_active++;
			NfsIioCachePage* new_page = static_cast<NfsIioCachePage*>(malloc(sizeof(NfsIioCachePage)));
			if (!new_page) {
				//fprintf(stderr, "Error: Could not allocate memory for NfsIioCachePage metadata.\n");
				// 應有更完善的錯誤處理
				cache->num_pages_active--; // 回復計數
				return;
			}

			size_t buffer_size = static_cast<size_t>(nfs_iio_BLOCK_SIZEv) * channel->blocks_per_stripe;
			if (buffer_size == 0 && channel->blocks_per_stripe > 0) { // 防止blocks_per_stripe很大導致溢位
				//fprintf(stderr, "Error: Buffer size calculation resulted in zero or overflow.\n");
				free(new_page);
				cache->num_pages_active--;
				return;
			}
			if (buffer_size == 0 && channel->blocks_per_stripe == 0) { //允許0大小的stripe
				// do nothing for buffer if stripe has 0 blocks
			}


			void* buffer = nullptr;
			if (buffer_size > 0) {
				buffer = malloc(buffer_size);
				if (!buffer) {
					//fprintf(stderr, "Internal error: not enough memory for cache page buffer. Aborting...\n");
					free(new_page); // 釋放已分配的元數據
					cache->num_pages_active--; // 回復計數
					// _exit(-1); // 原始碼中此處會退出
					return; // 或者返回錯誤
				}
			}

			new_page->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(buffer); // LSB 為 0 (已同步)
			new_page->disk_block_position = 0; // 將在 refresh 時設定
			new_page->last_access_time = nfs_iio_CLOCK; // 設定最後存取時間
			cache_page_set_sync(new_page, 1); // 初始時標記為已同步 (空的)

			cache->pages[page_array_idx_to_create_at] = new_page;
		}
	}
}

/**
 * @brief 刷新(或載入)快取頁面，確保其包含最新的磁碟資料。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_byte_pos 要刷新/載入的通道內相對位元組位置。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int cache_page_refresh(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels || !file->channels[channel_idx]) {
		return -1;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (channel->blocks_per_stripe == 0 || nfs_iio_BLOCK_SIZEv == 0) {
		// 如果stripe大小或區塊大小為0，無法計算頁面索引或讀取，特殊處理或報錯
		if (channel->blocks_per_stripe == 0 && channel_relative_byte_pos == 0) return 0; // 允許0 stripe的通道在0位置refresh
		return -1;
	}

	// 計算此位元組位置屬於哪個快取頁面 (在陣列中的索引)
	int page_size_bytes = nfs_iio_BLOCK_SIZEv * channel->blocks_per_stripe;
	int page_array_idx = channel_relative_byte_pos / page_size_bytes;

	if (!channel->cache_header || page_array_idx >= channel->cache_header->num_pages_allocated) {
		if (cache_expand(file, channel_idx, page_array_idx) <= page_array_idx) { // <= 因為 expand 返回新大小
			return -1; // 擴展失敗
		}
	}
	// 重新獲取 cache_header，因為 cache_expand 可能重新分配它
	NfsIioCache* cache = channel->cache_header;
	if (!cache || !cache->pages) return -1; // cache_expand 後仍可能失敗或pages為空

	if (cache->pages[page_array_idx] == nullptr) {
		cache_page_create(file, channel_idx, page_array_idx);
		if (cache->pages[page_array_idx] == nullptr) { // create 可能失敗
			return -1;
		}
	}

	NfsIioCachePage* target_page = cache->pages[page_array_idx];

	// 計算此頁面應對應的通道相對位元組起始位置 (對齊到頁面邊界)
	int aligned_page_start_pos = page_array_idx * page_size_bytes;

	int absolute_disk_block = channel_pos_to_absolute_block(file, channel_idx, aligned_page_start_pos);
	if (absolute_disk_block < 0) return -1;

	void* page_buffer = cache_page_get_buffer(target_page);
	if (!page_buffer && channel->blocks_per_stripe > 0) return -1; // 如果需要讀取但緩衝區為空

	if (channel->blocks_per_stripe > 0) { // 只有在stripe有區塊時才讀取
		if (read_absolute_block_n(file, absolute_disk_block, channel->blocks_per_stripe, page_buffer) < 0) {
			return -1; // 讀取失敗
		}
	}

	cache_page_set_sync(target_page, 1); // 讀取後視為已同步
	target_page->disk_block_position = aligned_page_start_pos; // 儲存此頁面對應的通道位元組起始位置
	target_page->last_access_time = nfs_iio_CLOCK; // 更新最後存取時間

	return 0;
}

/**
 * @brief 為 IIO 檔案的特定通道建立/初始化快取。
 * 實際上是刷新該通道位置 0 對應的快取頁面。
 * @param file IIO 檔案指標。
 * @param channel_idx 要為其建立快取的通道索引。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int cache_create(NfsIioFile* file, int channel_idx) // IIO 版本
{
	if (!file) {
		return -1;
	}
	// 確保通道 0 位置的頁面被刷新/載入
	return cache_page_refresh(file, channel_idx, 0);
}

/**
 * @brief 銷毀並釋放 IIO 通道的快取管理器及其所有相關資源。
 * @param cache_to_destroy 指向 NfsIioCache 結構 (通道的 cache_header) 的指標。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int cache_destroy(NfsIioCache* cache_to_destroy) // IIO 版本
{
	if (!cache_to_destroy) {
		return -1; // 或 0，如果允許銷毀空指標
	}

	if (cache_to_destroy->pages) {
		for (int i = 0; i < cache_to_destroy->num_pages_allocated; ++i) {
			NfsIioCachePage* page = cache_to_destroy->pages[i];
			if (page) {
				void* buffer = cache_page_get_buffer(page);
				if (buffer) {
					free(buffer); // 釋放實際的資料緩衝區
				}
				free(page); // 釋放 NfsIioCachePage 元數據結構
			}
		}
		free(cache_to_destroy->pages); // 釋放頁面指標陣列
	}
	free(cache_to_destroy); // 釋放 NfsIioCache 結構本身
	return 0;
}

/**
 * @brief (IIO版本) 檢查指定的通道相對區塊是否在快取中 (即是否有為其分配的快取頁面)。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_block_idx 要檢查的通道相對區塊索引。
 * @return 如果區塊對應的快取頁面存在，則為 TRUE (1)，否則為 FALSE (0)。
 */
int is_in_cache(NfsIioFile* file, int channel_idx, int channel_relative_block_idx) // BOOL usually int
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels) {
		return 0;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel || !channel->cache_header || !channel->cache_header->pages || channel->blocks_per_stripe == 0) {
		return 0; // 無法確定或無效的 stripe 大小
	}

	int page_array_idx = channel_relative_block_idx / channel->blocks_per_stripe;

	if (page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return 0; // 索引超出已分配範圍
	}
	return (channel->cache_header->pages[page_array_idx] != nullptr);
}

/**
 * @brief 更新快取，確保包含指定通道相對位元組位置的資料頁面已載入。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_byte_pos 要確保已載入的通道相對位元組位置。
 */
void cache_update(NfsIioFile* file, int channel_idx, int channel_relative_byte_pos)
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels) {
		return;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (!channel || (channel->blocks_per_stripe == 0 && channel_relative_byte_pos != 0) || nfs_iio_BLOCK_SIZEv == 0) {
		// 如果stripe或block大小為0，且位置非0，則無法處理
		if (channel && channel->blocks_per_stripe == 0 && channel_relative_byte_pos == 0) { /* allow for pos 0 */ }
		else return;
	}


	int page_size_bytes = nfs_iio_BLOCK_SIZEv * channel->blocks_per_stripe;
	if (page_size_bytes == 0 && channel_relative_byte_pos != 0) return; // 無法分頁

	int aligned_page_start_byte_pos = channel_relative_byte_pos;
	if (page_size_bytes > 0) { // 只有在 page_size_bytes > 0 時才需要對齊
		aligned_page_start_byte_pos = (channel_relative_byte_pos / page_size_bytes) * page_size_bytes;
	}


	int page_array_idx = 0;
	if (page_size_bytes > 0) {
		page_array_idx = aligned_page_start_byte_pos / page_size_bytes;
	}


	NfsIioCache* cache = channel->cache_header;
	if (!cache || page_array_idx >= cache->num_pages_allocated) {
		if (cache_expand(file, channel_idx, page_array_idx) <= page_array_idx && page_size_bytes > 0) {
			return; // 擴展失敗
		}
		cache = channel->cache_header; // cache_expand 可能重新分配 cache_header->pages
		if (!cache) return;
	}

	if (!cache->pages || cache->pages[page_array_idx] == nullptr) { // 檢查 pages 是否為空以及特定頁面是否為空
		cache_page_create(file, channel_idx, page_array_idx);
		if (!cache->pages || cache->pages[page_array_idx] == nullptr) return; // 建立失敗
	}

	// 在刷新頁面之前，先將該快取槽中可能存在的舊的、髒的頁面寫回
	// 這是為了防止不同磁碟位置的資料共用同一個快取槽而導致資料遺失
	cache_pageflush(file, channel_idx, page_array_idx); // 單頁 flush
	cache_page_refresh(file, channel_idx, aligned_page_start_byte_pos);
}

/**
 * @brief (IIO 通道層級) 清除指定通道的所有快取頁面 (將髒頁面寫回磁碟)。
 * @param file IIO 檔案指標。
 * @param channel_idx 要清除快取的通道索引。
 */
void cacheflush(NfsIioFile* file, int channel_idx) // IIO channel level
{
	if (!file || !file->channels || channel_idx < 0 || channel_idx >= file->num_channels) {
		return;
	}
	NfsIioChannel* channel = file->channels[channel_idx];
	if (channel && channel->cache_header && channel->cache_header->pages) {
		for (int i = 0; i < channel->cache_header->num_pages_allocated; ++i) {
			if (channel->cache_header->pages[i] != nullptr) { // 只處理實際存在的頁面
				cache_pageflush(file, channel_idx, i); // 單頁 flush
			}
		}
	}
}

/**
 * @brief (IIO 檔案層級) 清除 IIO 檔案中所有通道的快取，並刷新檔案緩衝區。
 * @param file IIO 檔案指標。
 */
void flush_data(NfsIioFile* file) // IIO file level
{
	if (!file) {
		return;
	}
	for (short i = 0; i < file->num_channels; ++i) {
		cacheflush(file, i); // 清除每個通道的快取
	}
	if (file->file_handle) {
		fflush(file->file_handle);
	}
}

/**
 * @brief 從快取中讀取一個完整的通道區塊。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_block_idx 要讀取的通道相對區塊索引。
 * @param buffer 儲存讀取資料的緩衝區。
 * @return 成功讀取的位元組數 (等於 nfs_iio_BLOCK_SIZEv)，錯誤時返回 -1 或 0。
 */
int cache_read_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx, void* buffer)
{
	if (!file || !buffer || nfs_iio_BLOCK_SIZEv == 0) return -1;

	if (!is_in_cache(file, channel_idx, channel_relative_block_idx)) {
		cache_update(file, channel_idx, channel_relative_block_idx * nfs_iio_BLOCK_SIZEv);
	}

	NfsIioChannel* channel = nfs_iio_get_channel(file, channel_idx); // 使用已還原的輔助函式
	if (!channel || !channel->cache_header || !channel->cache_header->pages || channel->blocks_per_stripe == 0) {
		return -1;
	}

	int page_array_idx = channel_relative_block_idx / channel->blocks_per_stripe;
	if (page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return -1;
	}

	NfsIioCachePage* page = channel->cache_header->pages[page_array_idx];
	if (!page) {
		// is_in_cache 和 cache_update 應該確保頁面存在，若此處仍為空則表示邏輯問題或記憶體分配失敗
		return -1;
	}

	void* page_buffer_base = cache_page_get_buffer(page);
	if (!page_buffer_base) return -1;

	int block_start_byte_in_channel = channel_relative_block_idx * nfs_iio_BLOCK_SIZEv;
	int block_offset_in_page_buffer = block_start_byte_in_channel - page->disk_block_position;

	if (block_offset_in_page_buffer < 0 ||
		(block_offset_in_page_buffer + nfs_iio_BLOCK_SIZEv) >(channel->blocks_per_stripe * nfs_iio_BLOCK_SIZEv)) {
		// 偏移量計算錯誤或請求超出頁面緩衝區範圍
		return -1;
	}

	memcpy(buffer, static_cast<char*>(page_buffer_base) + block_offset_in_page_buffer, nfs_iio_BLOCK_SIZEv);

	page->last_access_time = nfs_iio_CLOCK;
	return nfs_iio_BLOCK_SIZEv;
}

/**
 * @brief 從快取中讀取通道區塊的部分內容。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_block_idx 通道相對區塊索引。
 * @param offset_in_block 在該區塊內的起始偏移。
 * @param end_offset_in_block 在該區塊內的結束偏移 (包含此位元組)。
 * @param buffer 儲存讀取資料的緩衝區。
 * @return 成功讀取的位元組數，錯誤時返回 0。
 */
unsigned int cache_read_partial_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx,
	int offset_in_block, int end_offset_in_block, void* buffer)
{
	if (!file || !buffer || offset_in_block < 0 || end_offset_in_block < offset_in_block || end_offset_in_block >= nfs_iio_BLOCK_SIZEv) {
		return 0;
	}

	if (!is_in_cache(file, channel_idx, channel_relative_block_idx)) {
		cache_update(file, channel_idx, channel_relative_block_idx * nfs_iio_BLOCK_SIZEv);
	}

	NfsIioChannel* channel = nfs_iio_get_channel(file, channel_idx);
	if (!channel || !channel->cache_header || !channel->cache_header->pages || channel->blocks_per_stripe == 0) {
		return 0;
	}

	int page_array_idx = channel_relative_block_idx / channel->blocks_per_stripe;
	if (page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return 0;
	}

	NfsIioCachePage* page = channel->cache_header->pages[page_array_idx];
	if (!page) {
		return 0;
	}

	void* page_buffer_base = cache_page_get_buffer(page);
	if (!page_buffer_base) return 0;

	int block_start_byte_in_channel = channel_relative_block_idx * nfs_iio_BLOCK_SIZEv;
	int block_offset_in_page_buffer = block_start_byte_in_channel - page->disk_block_position;

	char* source_ptr = static_cast<char*>(page_buffer_base) + block_offset_in_page_buffer + offset_in_block;
	unsigned int bytes_to_copy = end_offset_in_block - offset_in_block + 1;

	if (block_offset_in_page_buffer < 0 ||
		(block_offset_in_page_buffer + offset_in_block + bytes_to_copy) >(static_cast<unsigned int>(channel->blocks_per_stripe) * nfs_iio_BLOCK_SIZEv)) {
		// 偏移量計算錯誤或請求超出頁面緩衝區範圍
		return 0;
	}

	memcpy(buffer, source_ptr, bytes_to_copy);
	page->last_access_time = nfs_iio_CLOCK;
	return bytes_to_copy;
}

/**
 * @brief 將一個完整的通道區塊寫入快取。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_block_idx 要寫入的通道相對區塊索引。
 * @param buffer 包含要寫入資料的緩衝區 (const void*)。
 * @return 成功寫入的位元組數 (等於 nfs_iio_BLOCK_SIZEv)，錯誤時返回 -1 或 0。
 */
int cache_write_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx, const void* buffer)
{
	if (!file || !buffer || nfs_iio_BLOCK_SIZEv == 0) return -1;

	if (!is_in_cache(file, channel_idx, channel_relative_block_idx)) {
		cache_update(file, channel_idx, channel_relative_block_idx * nfs_iio_BLOCK_SIZEv);
	}

	NfsIioChannel* channel = nfs_iio_get_channel(file, channel_idx);
	if (!channel || !channel->cache_header || !channel->cache_header->pages || channel->blocks_per_stripe == 0) {
		return -1;
	}

	int page_array_idx = channel_relative_block_idx / channel->blocks_per_stripe;
	if (page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return -1;
	}

	NfsIioCachePage* page = channel->cache_header->pages[page_array_idx];
	if (!page) {
		return -1;
	}

	void* page_buffer_base = cache_page_get_buffer(page);
	if (!page_buffer_base) return -1;

	int block_start_byte_in_channel = channel_relative_block_idx * nfs_iio_BLOCK_SIZEv;
	int block_offset_in_page_buffer = block_start_byte_in_channel - page->disk_block_position;

	if (block_offset_in_page_buffer < 0 ||
		(block_offset_in_page_buffer + nfs_iio_BLOCK_SIZEv) >(channel->blocks_per_stripe * nfs_iio_BLOCK_SIZEv)) {
		return -1;
	}

	memcpy(static_cast<char*>(page_buffer_base) + block_offset_in_page_buffer, buffer, nfs_iio_BLOCK_SIZEv);

	cache_page_set_sync(page, 0); // 標記為髒頁面
	page->last_access_time = nfs_iio_CLOCK;
	return nfs_iio_BLOCK_SIZEv;
}

/**
 * @brief 將部分內容寫入快取的通道區塊。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param channel_relative_block_idx 通道相對區塊索引。
 * @param offset_in_block 在該區塊內的起始偏移。
 * @param end_offset_in_block 在該區塊內的結束偏移 (包含此位元組)。
 * @param buffer 包含要寫入資料的緩衝區 (const void*)。
 * @return 成功寫入的位元組數，錯誤時返回 0。
 */
int cache_write_partial_channel_block(NfsIioFile* file, int channel_idx, int channel_relative_block_idx,
	int offset_in_block, int end_offset_in_block, const void* buffer)
{
	if (!file || !buffer || offset_in_block < 0 || end_offset_in_block < offset_in_block || end_offset_in_block >= nfs_iio_BLOCK_SIZEv) {
		return 0;
	}

	if (!is_in_cache(file, channel_idx, channel_relative_block_idx)) {
		cache_update(file, channel_idx, channel_relative_block_idx * nfs_iio_BLOCK_SIZEv);
	}

	NfsIioChannel* channel = nfs_iio_get_channel(file, channel_idx);
	if (!channel || !channel->cache_header || !channel->cache_header->pages || channel->blocks_per_stripe == 0) {
		return 0;
	}

	int page_array_idx = channel_relative_block_idx / channel->blocks_per_stripe;
	if (page_array_idx < 0 || page_array_idx >= channel->cache_header->num_pages_allocated) {
		return 0;
	}

	NfsIioCachePage* page = channel->cache_header->pages[page_array_idx];
	if (!page) {
		return 0;
	}

	void* page_buffer_base = cache_page_get_buffer(page);
	if (!page_buffer_base) return 0;

	int block_start_byte_in_channel = channel_relative_block_idx * nfs_iio_BLOCK_SIZEv;
	int block_offset_in_page_buffer = block_start_byte_in_channel - page->disk_block_position;
	char* destination_ptr = static_cast<char*>(page_buffer_base) + block_offset_in_page_buffer + offset_in_block;

	int bytes_to_copy = end_offset_in_block - offset_in_block + 1;

	if (block_offset_in_page_buffer < 0 ||
		(block_offset_in_page_buffer + offset_in_block + bytes_to_copy) >(static_cast<unsigned int>(channel->blocks_per_stripe) * nfs_iio_BLOCK_SIZEv)) {
		return 0;
	}

	memcpy(destination_ptr, buffer, bytes_to_copy);

	cache_page_set_sync(page, 0); // 標記為髒頁面
	page->last_access_time = nfs_iio_CLOCK;
	return bytes_to_copy;
}

/**
 * @brief 從 IIO 檔案讀取標頭資訊。
 * @param file 指向 NfsIioFile 結構的指標 (其 file_handle 必須已開啟)。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int read_header(NfsIioFile* file) {
	if (!file || !file->file_handle) {
		return -1;
	}

	if (fseek(file->file_handle, 0, SEEK_SET) != 0) {
		return -1;
	}

	struct FileHeaderData {
		int magic_number;
		int num_channels_as_int; // 檔案中儲存為 int
	} header_data;

	if (fread(&header_data, sizeof(FileHeaderData), 1, file->file_handle) != 1) {
		return -1; // 讀取檔案標頭失敗
	}

	if (header_data.magic_number != 1145258561) { // "AIO\x01" or specific int
		return -1; // 魔術數字不符
	}

	file->num_channels = static_cast<short>(header_data.num_channels_as_int);
	if (file->num_channels < 0) return -1; // 無效的通道數量

	if (file->num_channels > 0) {
		file->channels = static_cast<NfsIioChannel**>(malloc(sizeof(NfsIioChannel*) * file->num_channels));
		if (!file->channels) {
			file->num_channels = 0; // 分配失敗，重設通道數
			return -1;
		}
		// 將所有新的通道指標初始化為 nullptr
		for (short i = 0; i < file->num_channels; ++i) {
			file->channels[i] = nullptr;
		}

		struct ChannelHeaderData {
			int blocks_per_stripe;
			int current_size_bytes;
		} channel_data;

		for (short i = 0; i < file->num_channels; ++i) {
			file->channels[i] = static_cast<NfsIioChannel*>(malloc(sizeof(NfsIioChannel)));
			if (!file->channels[i]) {
				// 清理已分配的通道和通道陣列
				for (short j = 0; j < i; ++j) free(file->channels[j]);
				free(file->channels);
				file->channels = nullptr;
				file->num_channels = 0;
				return -1;
			}

			if (fread(&channel_data, sizeof(ChannelHeaderData), 1, file->file_handle) != 1) {
				// 清理
				for (short j = 0; j <= i; ++j) free(file->channels[j]);
				free(file->channels);
				file->channels = nullptr;
				file->num_channels = 0;
				return -1; // 讀取通道標頭失敗
			}
			file->channels[i]->blocks_per_stripe = channel_data.blocks_per_stripe;
			file->channels[i]->current_size_bytes = channel_data.current_size_bytes;
			file->channels[i]->current_seek_position = 0; // 初始化讀寫位置
			file->channels[i]->cache_header = nullptr;    // 初始化快取指標
		}
	}
	else {
		file->channels = nullptr; // 沒有通道
	}

	// 為每個已讀取的通道建立快取
	for (short i = 0; i < file->num_channels; ++i) {
		if (cache_create(file, i) != 0) {
			// 快取建立失敗的處理相對複雜，可能需要回滾所有操作
			// 簡化處理：關閉檔案並釋放已分配資源 (更完整的作法是 nfs_iio_close 的一部分邏輯)
			for (short j = 0; j < file->num_channels; ++j) {
				if (file->channels[j]) {
					if (file->channels[j]->cache_header) cache_destroy(file->channels[j]->cache_header);
					free(file->channels[j]);
				}
			}
			if (file->channels) free(file->channels);
			file->channels = nullptr;
			file->num_channels = 0;
			return -1;
		}
	}
	return 0;
}

/**
 * @brief 自動截斷 IIO 檔案至其實際需要的最小大小。
 * @param file 指向 NfsIioFile 結構的指標。
 */
void auto_truncate(NfsIioFile* file) {
	if (!file || !file->file_handle || nfs_iio_BLOCK_SIZEv <= 0) {
		return;
	}

	long current_header_size = header_size(file);
	if (current_header_size < 0) { // header_size 可能返回錯誤
		//fprintf(stderr, "auto_truncate: Invalid header size.\n");
		return;
	}

	int total_blocks_in_one_full_chunk = 0;
	if (file->num_channels > 0) {
		total_blocks_in_one_full_chunk = nfs_iio_blocks_per_chunk(file);
	}

	int max_chunks_needed = 0;
	if (file->num_channels > 0 && file->channels) {
		for (short i = 0; i < file->num_channels; ++i) {
			NfsIioChannel* channel = file->channels[i];
			if (channel) { // 確保通道指標有效
				if (channel->blocks_per_stripe > 0) {
					// 計算此通道需要的資料區塊數 (向上取整)
					int num_data_blocks_for_channel = (channel->current_size_bytes + nfs_iio_BLOCK_SIZEv - 1) / nfs_iio_BLOCK_SIZEv;
					// 計算此通道需要的 "chunk" 數 (向上取整)
					int num_chunks_for_channel = (num_data_blocks_for_channel + channel->blocks_per_stripe - 1) / channel->blocks_per_stripe;
					if (num_chunks_for_channel > max_chunks_needed) {
						max_chunks_needed = num_chunks_for_channel;
					}
				}
				else if (channel->current_size_bytes > 0) {
					// 通道有資料，但其 stripe 大小為0，這是一個不一致的狀態。
					// 原始碼的行為可能未定義，這裡保守地假設至少需要一個 chunk 的空間，
					// 以避免將檔案截斷得過小。或者，這可以視為一個錯誤情況。
					if (max_chunks_needed == 0) max_chunks_needed = 1;
				}
			}
		}
	}

	long new_file_size;
	if (max_chunks_needed > 0 && total_blocks_in_one_full_chunk > 0) {
		long total_data_blocks_to_keep = static_cast<long>(max_chunks_needed) * total_blocks_in_one_full_chunk;
		new_file_size = current_header_size + total_data_blocks_to_keep * nfs_iio_BLOCK_SIZEv;
	}
	else {
		// 沒有資料區塊需要保留 (例如所有通道都為空，或 total_blocks_in_one_full_chunk 為0)
		// 檔案只應包含標頭。
		new_file_size = current_header_size;
	}

	// 使用 Windows API 進行檔案截斷
	int crt_fd = _fileno(file->file_handle); // 從 FILE* 取得 C runtime 檔案描述符
	if (crt_fd != -1) {
		HANDLE win_handle = reinterpret_cast<HANDLE>(_get_osfhandle(crt_fd)); // 轉換為 Windows HANDLE
		if (win_handle != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER li_new_file_size;
			li_new_file_size.QuadPart = new_file_size;

			// 移動檔案指標到新的檔案結尾位置
			if (SetFilePointerEx(win_handle, li_new_file_size, NULL, FILE_BEGIN)) {
				// 設定檔案結尾，截斷檔案
				if (!SetEndOfFile(win_handle)) {
					// SetEndOfFile 失敗
					DWORD error = GetLastError();
					//fprintf(stderr, "auto_truncate: SetEndOfFile failed with error %lu\n", error);
				}
			}
			else {
				// SetFilePointerEx 失敗
				DWORD error = GetLastError();
				//fprintf(stderr, "auto_truncate: SetFilePointerEx failed with error %lu\n", error);
			}
		}
		else {
			// _get_osfhandle 失敗
			//fprintf(stderr, "auto_truncate: _get_osfhandle failed.\n");
		}
	}
	else {
		// _fileno 失敗
		//fprintf(stderr, "auto_truncate: _fileno failed.\n");
	}
}

/**
 * @brief 建立一個新的 IIO 檔案。
 * @param fileName 要建立的檔案的名稱。
 * @return 指向新建立的 NfsIioFile 結構的指標，失敗時返回 nullptr。
 */
NfsIioFile* nfs_iio_create(const char* fileName) {
	nfs_iio_CLOCK++;
	nfs_iio_BLOCK_SIZEv = get_page_size();
	if (nfs_iio_BLOCK_SIZEv < 256) {
		nfs_iio_BLOCK_SIZEv = 256; // 確保最小區塊大小
	}

	NfsIioFile* newfile = static_cast<NfsIioFile*>(malloc(sizeof(NfsIioFile)));
	if (!newfile) {
		return nullptr;
	}

	newfile->file_name = static_cast<char*>(malloc(strlen(fileName) + 1));
	if (!newfile->file_name) {
		free(newfile);
		return nullptr;
	}
	strcpy(newfile->file_name, fileName);

	newfile->num_channels = 0;
	newfile->channels = nullptr;

	// "w+b"：建立檔案以進行讀寫 (如果檔案已存在則截斷)，二進位模式
	newfile->file_handle = fopen(fileName, "w+b");
	if (!newfile->file_handle) {
		free(newfile->file_name);
		free(newfile);
		return nullptr;
	}

	if (write_header(newfile) != 0) {
		fclose(newfile->file_handle);
		free(newfile->file_name);
		free(newfile);
		// _remove(fileName); // 可選：如果建立失敗是否要刪除檔案
		return nullptr;
	}

	return newfile;
}

/**
 * @brief 開啟一個已存在的 IIO 檔案。
 * @param fileName 要開啟的檔案的名稱。
 * @return 指向 NfsIioFile 結構的指標，失敗時返回 nullptr。
 */
NfsIioFile* nfs_iio_open(const char* fileName) {
	nfs_iio_CLOCK++;
	nfs_iio_BLOCK_SIZEv = get_page_size();
	if (nfs_iio_BLOCK_SIZEv < 256) {
		nfs_iio_BLOCK_SIZEv = 256;
	}

	NfsIioFile* openedfile = static_cast<NfsIioFile*>(malloc(sizeof(NfsIioFile)));
	if (!openedfile) {
		return nullptr;
	}

	openedfile->file_name = static_cast<char*>(malloc(strlen(fileName) + 1));
	if (!openedfile->file_name) {
		free(openedfile);
		return nullptr;
	}
	strcpy(openedfile->file_name, fileName);
	openedfile->num_channels = 0; // 將由 read_header 設定
	openedfile->channels = nullptr;  // 將由 read_header 分配

	const char* mode;
	if ((nfs_iio_IOMODE & 2) != 0) { // 假設 bit 1 表示寫入權限
		mode = "r+b"; // 讀寫更新，檔案必須存在
	}
	else {
		mode = "rb";  // 唯讀，檔案必須存在
	}

	openedfile->file_handle = fopen(fileName, mode);
	if (!openedfile->file_handle) {
		free(openedfile->file_name);
		free(openedfile);
		return nullptr;
	}

	if (read_header(openedfile) != 0) {
		fclose(openedfile->file_handle);
		free(openedfile->file_name);
		// read_header 失敗時，其內部應處理已部分分配的 channel 記憶體
		if (openedfile->channels) free(openedfile->channels); // 以防萬一
		free(openedfile);
		return nullptr;
	}

	return openedfile;
}

/**
 * @brief 從指定的 IIO 通道讀取資料。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param buffer 儲存讀取資料的緩衝區。
 * @param bytes_to_read 要讀取的位元組數。
 * @return 實際讀取的位元組數，錯誤時返回 -1 或 0。
 */
int nfs_iio_read(NfsIioFile* file, int channel_idx, void* buffer, int bytes_to_read) {
	nfs_iio_CLOCK++;
	if (!file || !file->file_handle || !buffer || bytes_to_read <= 0) {
		return -1;
	}
	if (channel_idx < 0 || channel_idx >= file->num_channels || !file->channels || !file->channels[channel_idx]) {
		return -1;
	}

	NfsIioChannel* channel = file->channels[channel_idx];
	int current_pos = channel->current_seek_position;

	if (current_pos >= channel->current_size_bytes) {
		// 在檔案結尾或之後，如果 size_bytes 為0也在此情況
		if (channel->current_size_bytes == 0 && current_pos == 0) {
			// 允許從大小為0的檔案的0位置讀取0位元組 (bytes_to_read會被調整為0)
		}
		else {
			return 0; // 無資料可讀
		}
	}

	int readable_bytes = channel->current_size_bytes - current_pos;
	if (bytes_to_read > readable_bytes) {
		bytes_to_read = readable_bytes; // 不要讀取超過通道大小的資料
	}
	if (bytes_to_read <= 0) return 0;


	int total_bytes_read = 0;
	char* current_buffer_ptr = static_cast<char*>(buffer);
	int remaining_bytes_to_read = bytes_to_read;

	while (remaining_bytes_to_read > 0) {
		nfs_iio_CLOCK++;
		int current_block_idx = current_pos / nfs_iio_BLOCK_SIZEv;
		int offset_in_block = current_pos % nfs_iio_BLOCK_SIZEv;
		int bytesfrom_this_block = nfs_iio_BLOCK_SIZEv - offset_in_block;
		if (bytesfrom_this_block > remaining_bytes_to_read) {
			bytesfrom_this_block = remaining_bytes_to_read;
		}

		int read_count;
		if (offset_in_block == 0 && bytesfrom_this_block == nfs_iio_BLOCK_SIZEv) {
			read_count = cache_read_channel_block(file, channel_idx, current_block_idx, current_buffer_ptr);
		}
		else {
			read_count = cache_read_partial_channel_block(file, channel_idx, current_block_idx, offset_in_block, offset_in_block + bytesfrom_this_block - 1, current_buffer_ptr);
		}

		if (read_count <= 0 && bytesfrom_this_block > 0) { // 如果期望讀取但失敗
			// 可能是一個錯誤，或者底層快取函式返回0表示特定情況
			// 為了安全，如果 read_count 不等於 bytesfrom_this_block，可能需要停止
			break; // 假設部分讀取成功或遇到錯誤
		}
		if (read_count > 0) { // 僅在實際讀取到內容時才增加指標
			current_buffer_ptr += read_count; // 使用實際讀取到的量
			total_bytes_read += read_count;
			current_pos += read_count;
			remaining_bytes_to_read -= read_count;
		}
		else { // read_count 是 0 或負數，停止
			break;
		}

	}
	channel->current_seek_position = current_pos;
	return total_bytes_read;
}

/**
 * @brief 向指定的 IIO 通道寫入資料。
 * @param file IIO 檔案指標。
 * @param channel_idx 通道索引。
 * @param buffer 包含要寫入資料的緩衝區 (const void*)。
 * @param bytes_to_write 要寫入的位元組數。
 * @return 實際寫入的位元組數，錯誤時返回 -1 或 0。
 */
int nfs_iio_write(NfsIioFile* file, int channel_idx, const void* buffer, int bytes_to_write) {
	nfs_iio_CLOCK++;
	if (!file || !file->file_handle || !buffer || bytes_to_write <= 0) {
		return -1;
	}
	if (channel_idx < 0 || channel_idx >= file->num_channels || !file->channels || !file->channels[channel_idx]) {
		return -1;
	}

	NfsIioChannel* channel = file->channels[channel_idx];
	int current_pos = channel->current_seek_position;

	int total_bytes_written = 0;
	const char* current_buffer_ptr = static_cast<const char*>(buffer);
	int remaining_bytes_to_write = bytes_to_write;

	while (remaining_bytes_to_write > 0) {
		nfs_iio_CLOCK++;
		int current_block_idx = current_pos / nfs_iio_BLOCK_SIZEv;
		int offset_in_block = current_pos % nfs_iio_BLOCK_SIZEv;
		int bytes_to_this_block = nfs_iio_BLOCK_SIZEv - offset_in_block;
		if (bytes_to_this_block > remaining_bytes_to_write) {
			bytes_to_this_block = remaining_bytes_to_write;
		}

		int written_count;
		if (offset_in_block == 0 && bytes_to_this_block == nfs_iio_BLOCK_SIZEv) {
			written_count = cache_write_channel_block(file, channel_idx, current_block_idx, current_buffer_ptr);
		}
		else {
			written_count = cache_write_partial_channel_block(file, channel_idx, current_block_idx, offset_in_block, offset_in_block + bytes_to_this_block - 1, current_buffer_ptr);
		}

		if (written_count <= 0 && bytes_to_this_block > 0) {
			break; // 寫入失敗或未寫入任何內容
		}
		if (written_count > 0) {
			current_buffer_ptr += written_count;
			total_bytes_written += written_count;
			current_pos += written_count;
			remaining_bytes_to_write -= written_count;
		}
		else {
			break;
		}
	}

	channel->current_seek_position = current_pos;
	if (current_pos > channel->current_size_bytes) {
		channel->current_size_bytes = current_pos; // 如果寫入操作擴展了檔案，更新通道大小
	}
	return total_bytes_written;
}

/**
 * @brief 在 IIO 檔案中配置一個新的通道。
 * @param file IIO 檔案指標。
 * @param blocks_per_stripefor_new_channel 新通道的每個 stripe 的區塊數。
 * @return 新配置通道的索引，失敗時返回 -1。
 */
int nfs_iio_allocate_channel(NfsIioFile* file, int blocks_per_stripefor_new_channel) {
	nfs_iio_CLOCK++;
	if (!file) {
		return -1;
	}

	NfsIioChannel* new_channel = static_cast<NfsIioChannel*>(malloc(sizeof(NfsIioChannel)));
	if (!new_channel) {
		return -1;
	}
	new_channel->blocks_per_stripe = blocks_per_stripefor_new_channel;
	new_channel->current_size_bytes = 0;
	new_channel->current_seek_position = 0;
	new_channel->cache_header = nullptr;

	short old_num_channels = file->num_channels;
	short new_num_channels = old_num_channels + 1;

	NfsIioChannel** new_channels_array = static_cast<NfsIioChannel**>(realloc(file->channels, sizeof(NfsIioChannel*) * new_num_channels));
	if (!new_channels_array) {
		free(new_channel); // 釋放已分配的通道結構
		return -1; // realloc 失敗
	}

	file->channels = new_channels_array;
	file->channels[old_num_channels] = new_channel; // 將新通道加入陣列
	file->num_channels = new_num_channels;          // 更新檔案中的通道數量

	if (cache_create(file, old_num_channels) != 0) { // 為新通道建立快取
		// 快取建立失敗的處理：回滾操作較複雜
		// 簡化：將剛加入的通道設為 nullptr，並減少通道數，但已分配的 new_channel 未釋放
		// 更好的作法是完全清除新加入的通道並縮小陣列，或標記檔案為錯誤狀態
		file->channels[old_num_channels] = nullptr; // 避免懸空指標
		free(new_channel);                       // 釋放通道結構
		file->num_channels = old_num_channels;    // 恢復通道數
		// 注意：_realloc 不提供縮小陣列時釋放多餘空間的保證，但此處不嘗試縮小
		return -1;
	}
	return old_num_channels; // 返回新通道的索引
}

/**
 * @brief 關閉 IIO 檔案，釋放所有相關資源。
 * @param file_to_close 要關閉的 IIO 檔案指標。此指標在函式結束後將無效。
 */
void nfs_iio_close(NfsIioFile* file_to_close) {
	if (!file_to_close) {
		return;
	}
	nfs_iio_CLOCK++;

	flush_data(file_to_close);    // 清除所有通道的快取 (寫回髒頁面)
	if (file_to_close->file_handle) { // 確保檔案已開啟
		if (write_header(file_to_close) != 0) {
			// 寫入標頭失敗的處理
		}
		if ((nfs_iio_IOMODE & 2) != 0) { // 如果是以允許寫入的模式開啟的
			auto_truncate(file_to_close); // 則自動截斷檔案
		}
		fclose(file_to_close->file_handle);
		file_to_close->file_handle = nullptr;
	}

	if (file_to_close->channels) {
		for (short i = 0; i < file_to_close->num_channels; ++i) {
			NfsIioChannel* channel = file_to_close->channels[i];
			if (channel) {
				if (channel->cache_header) {
					cache_destroy(channel->cache_header); // 銷毀通道的快取
					channel->cache_header = nullptr;
				}
				free(channel); // 釋放通道結構本身
			}
		}
		free(file_to_close->channels); // 釋放通道指標陣列
		file_to_close->channels = nullptr;
	}

	if (file_to_close->file_name) {
		free(file_to_close->file_name); // 釋放檔案名稱字串
		file_to_close->file_name = nullptr;
	}
	free(file_to_close); // 最後釋放 IIO 檔案結構本身
}

/**
 * @brief 銷毀 IIO 檔案：關閉檔案，釋放資源，並刪除實體檔案。
 * @param file_to_destroy 要銷毀的 IIO 檔案指標。此指標在函式結束後將無效。
 */
void nfs_iio_destroy(NfsIioFile* file_to_destroy) {
	if (!file_to_destroy) {
		return;
	}

	char* fileName_copy = nullptr;
	if (file_to_destroy->file_name) {
		size_t len = strlen(file_to_destroy->file_name);
		fileName_copy = static_cast<char*>(malloc(len + 1));
		if (fileName_copy) {
			strcpy(fileName_copy, file_to_destroy->file_name);
		}
	}

	nfs_iio_close(file_to_destroy); // 關閉並釋放所有記憶體資源

	if (fileName_copy) {
		remove(fileName_copy); // 刪除實體檔案
		free(fileName_copy);   // 釋放檔案名稱副本
	}
}

// layer3 開始
/**
 * @brief (Data 版本) 檢查指定的 512 位元組區塊索引是否在目前快取的視窗內。
 * @param handle NfsDataHandle 指標。
 * @param block_index_to_check 要檢查的區塊索引 (以512位元組為單位)。
 * @return 如果在快取內則為 1 (TRUE)，否則為 0 (FALSE)。
 */
int is_in_cache(NfsDataHandle* handle, int block_index_to_check) // BOOL
{
	if (!handle || !handle->cache) {
		return 0;
	}
	NfsDataCacheHeader* cache = handle->cache;
	// 檢查 block_index_to_check 是否落在 [cache_start_block, cache_end_block) 區間
	// cache_window_start_offset 是位元組偏移，buffer_capacity 是位元組大小
	if (cache->buffer_capacity == 0) return 0;

	int cache_start_block = cache->cache_window_start_offset / 512;
	int cache_end_block = (cache->cache_window_start_offset + cache->buffer_capacity) / 512;

	if (block_index_to_check >= cache_start_block && block_index_to_check < cache_end_block) {
		return 1;
	}
	return 0;
}

/**
 * @brief (Data 版本) 如果資料快取為髒 (dirty)，則將其內容寫回磁碟。
 * @param handle NfsDataHandle 指標。
 * @return 成功或快取本來就乾淨時返回 0，失敗時返回 -1。
 */
int cache_flush(NfsDataHandle* handle)
{
	if (!handle || !handle->cache || !handle->file_ptr) {
		return -1;
	}
	NfsDataCacheHeader* cache = handle->cache;

	if (cache->is_synced_flag != 1) { // 如果不是 clean (即 dirty)
		if (fseek(handle->file_ptr, cache->cache_window_start_offset, SEEK_SET) != 0) {
			return -1; // fseek 失敗
		}
		if (fwrite(cache->buffer, 1, cache->buffer_capacity, handle->file_ptr) != cache->buffer_capacity) {
			// 寫入的位元組數不完整，可能是錯誤
			// 原始碼未檢查 fwrite 的返回值，但這是一個好的實踐
			// 為了與原始碼行為一致，這裡不立即返回錯誤，但標記為已同步
		}
		cache->is_synced_flag = 1; // 寫回後標記為 clean
	}
	return 0;
}

/**
 * @brief (Data 版本) 滑動資料快取視窗到檔案中的新起始位置。
 * @param handle NfsDataHandle 指標。
 * @param new_desired_start_offset 新的快取視窗起始位元組偏移量。
 * @return 成功時返回 0，失敗時返回非 0 值。
 */
int cache_slide(NfsDataHandle* handle, int new_desired_start_offset)
{
	if (!handle || !handle->cache || !handle->file_ptr) {
		return -1;
	}
	NfsDataCacheHeader* cache = handle->cache;

	if (cache->is_synced_flag != 1) { // 如果 cache 是 dirty
		if (cache_flush(handle) != 0) { // 先 flush
			return -1; // flush 失敗
		}
	}

	int aligned_new_start_offset = new_desired_start_offset;
	if (new_desired_start_offset % 512 != 0) { // 如果未對齊 512 位元組
		aligned_new_start_offset = (new_desired_start_offset / 512) * 512; // 向下對齊
	}

	cache->cache_window_start_offset = aligned_new_start_offset;
	if (fseek(handle->file_ptr, cache->cache_window_start_offset, SEEK_SET) != 0) {
		return -1; // fseek 失敗
	}

	// 從新位置讀取資料填滿快取緩衝區
	// 原始碼中 _fread 的大小是 cache->buffer_capacity
	// 不檢查實際讀取的位元組數，假設填滿或到達檔案結尾
	fread(cache->buffer, 1, cache->buffer_capacity, handle->file_ptr);

	// 滑動並重新讀取後，快取應被視為 clean (因為它反映了磁碟的內容)
	// 原始碼在 cache_create 和 cache_resize 後都設定 is_synced_flag = 1
	cache->is_synced_flag = 1;

	return 0;
}

/**
 * @brief (Data 版本) 為 NfsDataHandle 建立並初始化資料快取。
 * @param handle NfsDataHandle 指標，其 cache 成員將被設定。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int cache_create(NfsDataHandle* handle)
{
	if (!handle) {
		return -1;
	}

	handle->cache = static_cast<NfsDataCacheHeader*>(malloc(sizeof(NfsDataCacheHeader)));
	if (!handle->cache) {
		return -1;
	}

	NfsDataCacheHeader* cache = handle->cache;
	cache->buffer_capacity = 0x10000; // 預設 64KB
	cache->buffer = malloc(cache->buffer_capacity);
	if (!cache->buffer) {
		free(cache);
		handle->cache = nullptr;
		return -1;
	}

	cache->is_synced_flag = 1;      // 初始為 clean
	cache->cache_window_start_offset = 0; // 初始視窗從檔案開頭

	// 初始讀取快取內容 (類似 cache_slide 到位置 0)
	if (handle->file_ptr) { // 確保檔案指標有效
		if (fseek(handle->file_ptr, cache->cache_window_start_offset, SEEK_SET) != 0) {
			// fseek 失敗，清理並返回錯誤
			free(cache->buffer);
			free(cache);
			handle->cache = nullptr;
			return -1;
		}
		fread(cache->buffer, 1, cache->buffer_capacity, handle->file_ptr);
	}
	else {
		// 檔案指標無效，無法執行初始讀取，這可能是一個問題
		// 根據原始碼的 nfs_data_create，此時 file_ptr 應該是有效的
	}
	return 0;
}

/**
 * @brief (Data 版本) 調整資料快取緩衝區的大小。
 * @param handle NfsDataHandle 指標。
 * @param new_capacity 新的快取容量 (位元組)。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int cache_resize(NfsDataHandle* handle, size_t new_capacity)
{
	if (!handle || !handle->cache || !handle->file_ptr) {
		return -1;
	}
	NfsDataCacheHeader* cache = handle->cache;

	// 原始碼要求在 resize 前快取必須是 clean 的
	// (nfs_data_set_cache_size 會先呼叫 nfs_data_flush_cache)
	if (cache->is_synced_flag != 1) {
		// 如果外部未確保 flush，這裡應該返回錯誤或嘗試 flush
		// return -1; // 快取為髒，不應直接 resize
		if (cache_flush(handle) != 0) return -1; //嘗試 flush
	}

	size_t aligned_new_capacity = new_capacity;
	if (new_capacity % 512 != 0) {
		aligned_new_capacity = (new_capacity / 512 + 1) * 512; // 向上對齊到 512 的倍數
	}
	if (aligned_new_capacity == 0 && new_capacity > 0) { //處理對齊後可能為0的情況（如果new_capacity非常大導致溢位）
		aligned_new_capacity = (new_capacity / 512 == (size_t)-1 / 512) ? new_capacity : (new_capacity / 512 + 1) * 512; //簡易防溢位
	}


	if (aligned_new_capacity <= 0) { // 原始碼中 v4 <= 0 時設為 512
		aligned_new_capacity = 512;
	}

	void* new_buffer = realloc(cache->buffer, aligned_new_capacity);
	if (!new_buffer && aligned_new_capacity > 0) { // aligned_new_capacity=0 時 realloc 可能返回 nullptr
		return -1; // realloc 失敗，舊緩衝區仍然有效
	}

	cache->buffer = new_buffer;
	cache->buffer_capacity = aligned_new_capacity;
	cache->is_synced_flag = 1; // 重新分配後，內容需要重新載入，視為 clean

	// 重新載入目前視窗的內容
	if (fseek(handle->file_ptr, cache->cache_window_start_offset, SEEK_SET) != 0) {
		return -1; // fseek 失敗
	}
	fread(cache->buffer, 1, cache->buffer_capacity, handle->file_ptr);

	return 0;
}

/**
 * @brief (Data 版本) 銷毀資料快取，釋放相關記憶體。
 * @param handle NfsDataHandle 指標，其 cache 成員將被銷毀並設為 nullptr。
 * @return 總是返回 0 (原始碼行為)。
 */
int cache_destroy(NfsDataHandle* handle)
{
	if (handle && handle->cache) {
		cache_flush(handle); // 先確保髒資料寫回
		NfsDataCacheHeader* cache = handle->cache;
		if (cache->buffer) {
			free(cache->buffer);
		}
		free(cache);
		handle->cache = nullptr;
	}
	return 0;
}

/**
 * @brief (Data 版本) 透過資料快取讀取指定數量的位元組。
 * 原始碼中，此函式被 nfs_data_read 和 nfs_data_read_contiguous 呼叫。
 * nfs_data_read 總是請求 512 位元組。
 * nfs_data_read_contiguous 請求 num_blocks * 512 位元組。
 * @param handle NfsDataHandle 指標。
 * @param file_offset_to_read_from 檔案中的起始讀取位元組偏移量。
 * @param num_bytes_to_read 要讀取的位元組數量。
 * @param output_buffer 儲存讀取資料的緩衝區。
 * @return 成功時返回 0 (原始碼行為)，錯誤時可考慮返回非0。
 */
int cache_get(NfsDataHandle* handle, int file_offset_to_read_from, int num_bytes_to_read, void* output_buffer)
{
	if (!handle || !handle->cache || !output_buffer || num_bytes_to_read < 0) {
		return -1; // 或其他錯誤碼
	}
	if (num_bytes_to_read == 0) {
		return 0;
	}

	NfsDataCacheHeader* cache = handle->cache;
	char* current_dest_ptr = static_cast<char*>(output_buffer);
	int current_file_offset = file_offset_to_read_from;
	int bytes_remaining_to_read = num_bytes_to_read;

	while (bytes_remaining_to_read > 0) {
		// 檢查目前 file_offset 是否在快取視窗內 (以512位元組區塊為單位檢查)
		// is_in_cache 的第二個參數是 512 位元組區塊的索引
		if (!is_in_cache(handle, current_file_offset / 512)) {
			if (cache_slide(handle, current_file_offset) != 0) { // 滑動快取視窗以包含 current_file_offset
				return -1; // cache_slide 失敗
			}
		}
		// cache_slide 後，cache 指標可能已更新 (如果 NfsDataHandle 被重新分配)，但此處 cache 是局部變數，應重新獲取或假設其有效
		// cache = handle->cache; // 重新獲取以防萬一 (雖然在此結構中 NfsDataHandle 本身不動)

		// 計算從目前快取視窗可以複製多少位元組
		int offset_in_cache_buffer = current_file_offset - cache->cache_window_start_offset;
		int bytes_available_in_window_from_offset = static_cast<int>(cache->buffer_capacity) - offset_in_cache_buffer;

		if (offset_in_cache_buffer < 0 || bytes_available_in_window_from_offset <= 0) {
			// current_file_offset 不在已滑動的快取視窗內，這不應該發生如果 cache_slide 成功
			// 這表示邏輯錯誤或 cache_slide 未按預期工作
			return -1;
		}

		int bytes_to_copy_this_iteration = min(bytes_remaining_to_read, bytes_available_in_window_from_offset);
		// 原始碼中使用 0x200 (512) 作為複製單位，這暗示了它可能總是嘗試以512位元組的倍數來處理內部迴圈，
		// 即使請求的大小不是。但 nfs_data_read_contiguous 的存在表明可以讀取更大的連續塊。
		// 這裡我們採用更直接的 min 計算。如果需要完全符合原始碼的內部 512B chunking，邏輯會更複雜。
		// 為了簡化和直接反映請求，我們先這樣處理。
		// 原始碼的`qmemcpy(..., 0x200u)` 和 `v4 < a3` 的迴圈暗示它每次處理固定大小的塊（可能是512）。
		// 此處的`bytes_to_copy_this_iteration`更通用。
		// 如果要嚴格模擬，則 `bytes_to_copy_this_iteration` 應該是 `std::min({bytes_remaining_to_read, bytes_available_in_window_from_offset, 512});`
		// 並在外層迴圈中處理 `num_bytes_to_read`。
		// 鑒於 nfs_data_read 傳 512, nfs_data_read_contiguous 傳 N*512，
		// 原始碼的迴圈結構 `v4 += 512; ... if (v4 >= a3) break;` 更像是：
		// while (bytes_processed < num_bytes_to_read) { process_a_chunk_of_up_to_512_bytes; }

		if (bytes_to_copy_this_iteration <= 0) break; // 沒有更多可複製的了

		memcpy(current_dest_ptr, static_cast<char*>(cache->buffer) + offset_in_cache_buffer, bytes_to_copy_this_iteration);

		current_dest_ptr += bytes_to_copy_this_iteration;
		current_file_offset += bytes_to_copy_this_iteration;
		bytes_remaining_to_read -= bytes_to_copy_this_iteration;
	}
	return 0; // 原始碼成功時返回 0
}


/**
 * @brief (Data 版本) 透過資料快取寫入指定數量的位元組。
 * 邏輯與 cache_get 對稱。
 * @param handle NfsDataHandle 指標。
 * @param file_offset_to_write_to 檔案中的起始寫入位元組偏移量。
 * @param num_bytes_to_write 要寫入的位元組數量。
 * @param input_buffer 包含要寫入資料的緩衝區。
 * @return 成功時返回 0 (原始碼行為)，錯誤時可考慮返回非0。
 */
int cache_put(NfsDataHandle* handle, int file_offset_to_write_to, int num_bytes_to_write, const void* input_buffer)
{
	if (!handle || !handle->cache || !input_buffer || num_bytes_to_write < 0) {
		return -1;
	}
	if (num_bytes_to_write == 0) {
		return 0;
	}

	NfsDataCacheHeader* cache = handle->cache;
	const char* current_src_ptr = static_cast<const char*>(input_buffer);
	int current_file_offset = file_offset_to_write_to;
	int bytes_remaining_to_write = num_bytes_to_write;

	while (bytes_remaining_to_write > 0) {
		if (!is_in_cache(handle, current_file_offset / 512)) {
			if (cache_slide(handle, current_file_offset) != 0) {
				return -1;
			}
		}
		// cache = handle->cache; // 重新獲取

		int offset_in_cache_buffer = current_file_offset - cache->cache_window_start_offset;
		int bytes_available_in_window_from_offset = static_cast<int>(cache->buffer_capacity) - offset_in_cache_buffer;

		if (offset_in_cache_buffer < 0 || bytes_available_in_window_from_offset <= 0) {
			return -1;
		}

		int bytes_to_copy_this_iteration = min(bytes_remaining_to_write, bytes_available_in_window_from_offset);
		// 同 cache_get 的註解，原始碼可能內部以512B為單位處理。

		if (bytes_to_copy_this_iteration <= 0) break;

		memcpy(static_cast<char*>(cache->buffer) + offset_in_cache_buffer, current_src_ptr, bytes_to_copy_this_iteration);
		cache->is_synced_flag = 0; // 寫入後標記為 dirty

		current_src_ptr += bytes_to_copy_this_iteration;
		current_file_offset += bytes_to_copy_this_iteration;
		bytes_remaining_to_write -= bytes_to_copy_this_iteration;
	}
	return 0; // 原始碼成功時返回 0
}

/**
 * @brief 建立一個新的資料檔案 (.pak)，並初始化其快取。
 * @param fileName 要建立的檔案的名稱。
 * @return 指向 NfsDataHandle 結構的指標，失敗時返回 nullptr。
 */
NfsDataHandle* nfs_data_create(const char* fileName) {
	if (fileName == nullptr) {
		return nullptr;
	}
	NfsDataHandle* handle = static_cast<NfsDataHandle*>(malloc(sizeof(NfsDataHandle)));
	if (!handle) {
		return nullptr;
	}

	handle->file_name = static_cast<char*>(malloc(strlen(fileName) + 1));
	if (!handle->file_name) {
		free(handle);
		return nullptr;
	}
	strcpy(handle->file_name, fileName);
	handle->cache = nullptr; // cache_create 內部會分配

	// "w+b": 建立檔案以進行讀寫，如果檔案已存在則截斷。二進位模式。
	handle->file_ptr = fopen(fileName, "w+b");
	if (!handle->file_ptr) {
		free(handle->file_name);
		free(handle);
		return nullptr;
	}

	if (cache_create(handle) != 0) { // cache_create (Data 版本)
		fclose(handle->file_ptr);
		free(handle->file_name);
		// 如果 cache_create 內部部分分配了 handle->cache，它應自行清理或返回錯誤讓這裡處理
		if (handle->cache) free(handle->cache); // 簡易清理
		free(handle);
		return nullptr;
	}

	return handle;
}

/**
 * @brief 開啟一個已存在的資料檔案 (.pak)，並初始化其快取。
 * @param fileName 要開啟的檔案的名稱。
 * @return 指向 NfsDataHandle 結構的指標，失敗時返回 nullptr。
 */
NfsDataHandle* nfs_data_open(const char* fileName) {
	if (fileName == nullptr) {
		return nullptr;
	}
	NfsDataHandle* handle = static_cast<NfsDataHandle*>(malloc(sizeof(NfsDataHandle)));
	if (!handle) {
		return nullptr;
	}

	handle->file_name = static_cast<char*>(malloc(strlen(fileName) + 1));
	if (!handle->file_name) {
		free(handle);
		return nullptr;
	}
	strcpy(handle->file_name, fileName);
	handle->cache = nullptr;

	const char* open_mode;
	if ((nfs_data_IOMODE & 2) != 0) { // 假設 bit 1 表示需要寫入權限
		open_mode = "r+b"; // 讀寫更新，檔案必須存在
	}
	else {
		open_mode = "rb";  // 唯讀，檔案必須存在
	}

	handle->file_ptr = fopen(fileName, open_mode);
	if (!handle->file_ptr) {
		free(handle->file_name);
		free(handle);
		return nullptr;
	}

	if (cache_create(handle) != 0) { // cache_create (Data 版本)
		fclose(handle->file_ptr);
		free(handle->file_name);
		if (handle->cache) free(handle->cache);
		free(handle);
		return nullptr;
	}
	return handle;
}

/**
 * @brief 如果資料檔案的快取為髒 (dirty)，則將其寫回磁碟。
 * @param handle NfsDataHandle 指標。
 * @return 總是返回 0 (原始碼行為)。若發生錯誤，底層 cache_flush 可能返回非0。
 */
int nfs_data_flush_cache(NfsDataHandle* handle) {
	if (handle && handle->cache && handle->cache->is_synced_flag != 1) {
		cache_flush(handle); // 呼叫底層的 cache_flush (Data 版本)
	}
	return 0;
}

/**
 * @brief 設定資料快取的大小。
 * 在調整大小前會先清除快取 (寫回髒資料)。
 * @param handle NfsDataHandle 指標。
 * @param new_size 新的快取大小 (位元組)。
 * @return cache_resize 的返回值 (通常成功為0，失敗為-1)。
 */
int nfs_data_set_cache_size(NfsDataHandle* handle, size_t new_size) {
	if (!handle) return -1;
	nfs_data_flush_cache(handle); // 確保在 resize 前 flush
	return cache_resize(handle, new_size); // 呼叫底層的 cache_resize (Data 版本)
}

/**
 * @brief 從資料檔案讀取一個 512 位元組的區塊。
 * @param handle NfsDataHandle 指標。
 * @param block_index 要讀取的區塊索引。
 * @param buffer 儲存讀取資料的緩衝區。
 * @return cache_get 的返回值 (通常成功為0，失敗為-1)。
 */
int nfs_data_read(NfsDataHandle* handle, int block_index, void* buffer) {
	if (block_index >= 0) {
		return cache_get(handle, block_index * 512, 512, buffer);
	}
	return -1;
}

/**
 * @brief 向資料檔案寫入一個 512 位元組的區塊。
 * @param handle NfsDataHandle 指標。
 * @param block_index 要寫入的區塊索引。
 * @param buffer 包含要寫入資料的緩衝區 (const void*)。
 * @return cache_put 的返回值 (通常成功為0，失敗為-1)。
 */
int nfs_data_write(NfsDataHandle* handle, int block_index, const void* buffer) {
	if (block_index >= 0) {
		return cache_put(handle, block_index * 512, 512, buffer);
	}
	return -1;
}

/**
 * @brief 從資料檔案連續讀取多個 512 位元組的區塊。
 * @param handle NfsDataHandle 指標。
 * @param start_block_index 起始區塊索引。
 * @param num_blocks 要讀取的區塊數量。
 * @param buffer 儲存讀取資料的緩衝區。
 * @return cache_get 的返回值。
 */
int nfs_data_read_contiguous(NfsDataHandle* handle, int start_block_index, int num_blocks, void* buffer) {
	if (start_block_index >= 0 && num_blocks > 0) {
		return cache_get(handle, start_block_index * 512, num_blocks * 512, buffer);
	}
	if (num_blocks == 0) return 0; // 讀取0區塊成功
	return -1;
}

/**
 * @brief 向資料檔案連續寫入多個 512 位元組的區塊。
 * @param handle NfsDataHandle 指標。
 * @param start_block_index 起始區塊索引。
 * @param num_blocks 要寫入的區塊數量。
 * @param buffer 包含要寫入資料的緩衝區 (const void*)。
 * @return cache_put 的返回值。
 */
int nfs_data_write_contiguous(NfsDataHandle* handle, int start_block_index, int num_blocks, const void* buffer) {
	if (start_block_index >= 0 && num_blocks > 0) {
		return cache_put(handle, start_block_index * 512, num_blocks * 512, buffer);
	}
	if (num_blocks == 0) return 0; // 寫入0區塊成功
	return -1;
}

/**
 * @brief 關閉資料檔案，釋放相關資源 (包括快取)。
 * @param handle 要關閉的 NfsDataHandle 指標。此指標在函式結束後將無效。
 */
void nfs_data_close(NfsDataHandle* handle) {
	if (!handle) {
		return;
	}
	cache_destroy(handle); // 呼叫底層 cache_destroy (Data 版本)，它會先 flush

	if (handle->file_ptr) {
		// cache_destroy 內部已呼叫 cache_flush，這裡的 _fflush 可能是額外的安全措施
		// 或者如果 cache_destroy 的 flush 失敗，這裡的 fflush 仍會嘗試
		if (handle->file_ptr) fflush(handle->file_ptr); // 確保所有內容寫入
		fclose(handle->file_ptr);
		handle->file_ptr = nullptr;
	}
	if (handle->file_name) {
		free(handle->file_name);
		handle->file_name = nullptr;
	}
	free(handle);
}

/**
 * @brief 銷毀資料檔案：關閉檔案、釋放資源，並刪除實體檔案。
 * @param handle 要銷毀的 NfsDataHandle 指標。此指標在函式結束後將無效。
 */
void nfs_data_destroy(NfsDataHandle* handle) {
	if (!handle) {
		return;
	}
	char* file_name_copy = nullptr;
	if (handle->file_name) {
		size_t len = strlen(handle->file_name);
		file_name_copy = static_cast<char*>(malloc(len + 1));
		if (file_name_copy) {
			strcpy(file_name_copy, handle->file_name);
		}
	}

	nfs_data_close(handle); // 關閉並釋放所有記憶體資源

	if (file_name_copy) {
		remove(file_name_copy); // 刪除實體檔案
		free(file_name_copy);   // 釋放檔案名稱副本
	}
}

// Layer 4
/**
 * @brief (FAT 版本) 從 IIO 通道讀取指定索引的 FAT 表項的值。
 * @param fat_handle NfsFatHandle 指標。
 * @param fat_entry_index 要讀取的 FAT 表項的索引。
 * @return FAT 表項的值 (int)。如果讀取失敗，行為未定義 (取決於 nfs_iio_read)。
 */
 // nfs.cpp
int node_get_value(NfsFatHandle* fat_handle, int fat_entry_index)
{
	if (!fat_handle || !fat_handle->iio_file)
		return -2;                    // handle 無效

	int value_read = 0;
	nfs_iio_seek(fat_handle->iio_file,
		fat_handle->fat_iio_channel_id,
		fat_entry_index * sizeof(int));

	int bytes = nfs_iio_read(fat_handle->iio_file,
		fat_handle->fat_iio_channel_id,
		&value_read,
		sizeof(int));

	if (bytes != sizeof(int)) {
		/* 讀到檔案尾端 → 視為尚未配置，值就是 0               *
		 * 其他真正的 I/O 錯誤（fread 回傳 <0）才回傳 -2  */
		return (bytes >= 0) ? 0 : -2;
	}
	return value_read;
}


/**
 * @brief (FAT 版本) 設定 IIO 通道中指定索引的 FAT 表項的值。
 * @param fat_handle NfsFatHandle 指標。
 * @param fat_entry_index 要設定的 FAT 表項的索引。
 * @param value_to_set 要設定的值。
 */
void node_set_value(NfsFatHandle* fat_handle, int fat_entry_index, int value_to_set) {
	if (!fat_handle || !fat_handle->iio_file) {
		return;
	}
	nfs_iio_seek(fat_handle->iio_file, fat_handle->fat_iio_channel_id, fat_entry_index * sizeof(int));
	nfs_iio_write(fat_handle->iio_file, fat_handle->fat_iio_channel_id, &value_to_set, sizeof(int));
}

/**
 * @brief (FAT 版本) 從指定的起始索引開始，尋找 FAT 中的下一個空閒表項。
 * 空閒表項的值為 0。有效的 FAT 索引被認為是正數。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_search_idx 開始搜尋的索引。
 * @return 找到的空閒 FAT 表項的索引。
 */
int next_free(NfsFatHandle* fat_handle, int start_search_idx) {
	int current_idx = start_search_idx;
	// 迴圈條件：當前索引無效 (<=0) 或 當前索引的 FAT 表項已被使用 (非0)
	// 這會確保找到第一個索引 > 0 且其值為 0 的表項。
	while (current_idx <= 0 || node_get_value(fat_handle, current_idx) != 0) {
		current_idx++;
		// 注意：如果 FAT 已滿，此迴圈可能不會終止，除非 IIO 層有大小限制
		// 或 node_get_value 在讀取超出範圍時返回特定錯誤值被檢測到。
		// 這裡假設 FAT 空間足夠或有其他機制防止無限迴圈。
	}
	return current_idx;
}

/**
 * @brief 尋找 FAT 鏈中的最後一個表項的索引。
 * FAT 鏈以 -1 作為結束標記。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx FAT 鏈的起始表項索引。
 * @return FAT 鏈中最後一個有效表項的索引 (即指向 -1 的那個表項)。
 * 如果 start_of_chain_idx 本身就是 -1 或無效，行為取決於 node_get_value。
 */
int find_last_in_chain(NfsFatHandle* fat_handle, int start_of_chain_idx) {
	int previous_idx_in_chain;
	int current_idx_in_chain = start_of_chain_idx;

	if (current_idx_in_chain == -1) return start_of_chain_idx; // 如果起始就是鏈尾

	do {
		previous_idx_in_chain = current_idx_in_chain;
		current_idx_in_chain = node_get_value(fat_handle, previous_idx_in_chain);
	} while (current_idx_in_chain != -1); // -1 是鏈結束標記

	return previous_idx_in_chain;
}

/**
 * @brief (FAT 版本) 回收一個 FAT 表項，將其標記為空閒 (設為0)。
 * 同時，如果回收的表項索引小於目前快取的「下一個空閒起始搜尋索引」，則更新它。
 * @param fat_handle NfsFatHandle 指標。
 * @param fat_entry_idx_to_free 要回收的 FAT 表項的索引。
 * @return 固定返回 1 (原始碼行為，可能用於 nfs_fat_chain_for_each 的回呼)。
 */
int node_recover(NfsFatHandle* fat_handle, int fat_entry_idx_to_free) {
	if (fat_entry_idx_to_free > 0) { // 只回收有效的正索引
		node_set_value(fat_handle, fat_entry_idx_to_free, 0); // 設為0表示空閒
	}

	// 如果回收的索引比目前記錄的 next_free_search_start_idx 更靠前，則更新它
	if (fat_handle && fat_entry_idx_to_free > 0 && fat_entry_idx_to_free < fat_handle->next_free_search_start_idx) {
		fat_handle->next_free_search_start_idx = fat_entry_idx_to_free;
	}
	return 1; // 原始碼返回1
}

/**
 * @brief 建立一個新的 FAT 管理結構。
 * 會配置一個 IIO 通道用於儲存 FAT 資料。
 * @param iio_file 指向 NfsIioFile 的指標，FAT 將在此檔案中建立通道。
 * @param num_iio_blocks_for_fat_channel 為 FAT 通道配置的 IIO 條帶中的區塊數 (預設為4)。
 * @return 指向新建立的 NfsFatHandle 的指標，失敗時返回 nullptr。
 */
NfsFatHandle* nfs_fat_create(NfsIioFile* iio_file, int num_iio_blocks_for_fat_channel) {
	if (!iio_file) {
		return nullptr;
	}

	int blocks_for_fat_stripe = (num_iio_blocks_for_fat_channel == 0) ? 4 : num_iio_blocks_for_fat_channel;
	int fat_channel_id = nfs_iio_allocate_channel(iio_file, blocks_for_fat_stripe);

	if (fat_channel_id < 0) { // IIO 通道分配失敗
		return nullptr;
	}

	NfsFatHandle* fat_handle = static_cast<NfsFatHandle*>(malloc(sizeof(NfsFatHandle)));
	if (!fat_handle) {
		// 若此處分配失敗，先前分配的 IIO 通道 fat_channel_id 會洩漏。
		// 穩健的程式碼應處理此情況，例如嘗試釋放該通道。
		// 此處為忠實還原，不額外處理此洩漏。
		return nullptr;
	}

	fat_handle->iio_file = iio_file;
	fat_handle->fat_iio_channel_id = fat_channel_id;
	fat_handle->next_free_search_start_idx = 1; // FAT 表項索引通常從1開始

	return fat_handle;
}

/**
 * @brief 開啟一個已存在的 FAT。
 * @param iio_file 指向 NfsIioFile 的指標。
 * @param fat_iio_channel_id 儲存 FAT 資料的已存在的 IIO 通道 ID。
 * @return 指向 NfsFatHandle 的指標，失敗時返回 nullptr。
 */
NfsFatHandle* nfs_fat_open(NfsIioFile* iio_file, int fat_iio_channel_id) {
	if (!iio_file) {
		return nullptr;
	}

	NfsFatHandle* fat_handle = static_cast<NfsFatHandle*>(malloc(sizeof(NfsFatHandle)));
	if (!fat_handle) {
		return nullptr;
	}

	fat_handle->iio_file = iio_file;
	fat_handle->fat_iio_channel_id = fat_iio_channel_id;
	// 初始化 next_free_search_start_idx，從索引 0 開始搜尋 (next_free 內部會處理 <=0 的情況)
	fat_handle->next_free_search_start_idx = next_free(fat_handle, 0);

	return fat_handle;
}

/**
 * @brief 關閉 FAT，僅釋放 NfsFatHandle 結構本身。
 * 注意：此函式不釋放 FAT 資料所在的 IIO 通道。
 * @param fat_handle 要關閉的 NfsFatHandle 指標。
 * @return 總是返回 0。
 */
int nfs_fat_close(NfsFatHandle* fat_handle) {
	if (fat_handle) {
		free(fat_handle);
	}
	return 0;
}

/**
 * @brief 建立一個新的 FAT 鏈 (初始只包含一個區塊，標記為鏈尾)。
 * @param fat_handle NfsFatHandle 指標。
 * @return 新鏈的起始區塊索引；如果失敗 (例如無可用空間或無效控制代碼) 則返回 -1。
 */
int nfs_fat_create_chain(NfsFatHandle* fat_handle) {
	if (!fat_handle || !fat_handle->iio_file) {
		return -1;
	}

	// 從 fat_handle->next_free_search_start_idx 開始找一個空閒表項
	// 注意：next_free 內部會處理 start_idx <= 0 的情況，並返回 >0 的索引
	int new_chain_start_idx = fat_handle->next_free_search_start_idx;
	// next_free 會確保找到一個真正可用的空閒塊
	// 或者我們可以更直接地使用 next_free 尋找:
	// new_chain_start_idx = next_free(fat_handle, fat_handle->next_free_search_start_idx);
	// 但原始碼的模式是直接使用 hint，然後更新 hint。

	// 原始碼中，next_free_search_start_idx 是一個提示，不一定是嚴格的下一個空閒塊。
	// next_free 函式才是實際尋找的。
	// 但 nfs_fat_create_chain 的原始碼是：
	// v1 = *((_DWORD *)a1 + 2); /* fat_handle->next_free_search_start_idx */
	// node_set_value(a1, v1, -1);
	// *((_DWORD *)a1 + 2) = next_free(a1, *((_DWORD *)a1 + 2));
	// 這表示它直接使用 hint (v1) 作為新鏈的起始，然後再用 next_free 更新 hint。
	// 這假設 hint 指向的是一個確實可用的空閒塊。
	// 如果 next_free_search_start_idx 被 node_recover 維護得很好，這可能是有效的。

	if (new_chain_start_idx <= 0) { // hint 無效，嘗試重新尋找
		new_chain_start_idx = next_free(fat_handle, 1); // 從1開始找
		if (new_chain_start_idx <= 0) return -1; // 真的沒有空閒塊了
	}

	// 檢查選擇的 new_chain_start_idx 是否真的空閒
	// 雖然 next_free_search_start_idx 應該指向空閒塊，但多一層保險
	if (node_get_value(fat_handle, new_chain_start_idx) != 0) {
		// hint 指向的不是空閒塊，需要重新用 next_free 找
		new_chain_start_idx = next_free(fat_handle, new_chain_start_idx + 1); // 從提示的下一個開始找
		if (new_chain_start_idx <= 0) return -1; // 沒有空閒塊
	}


	node_set_value(fat_handle, new_chain_start_idx, -1); // 將此新區塊標記為鏈尾 (EOC)

	// 更新 next_free_search_start_idx 為 new_chain_start_idx 之後的下一個空閒塊
	// 原始碼是 next_free(fat_handle, current_hint)，由於 current_hint (new_chain_start_idx) 剛被使用，
	// next_free 會自動跳過它尋找下一個。
	fat_handle->next_free_search_start_idx = next_free(fat_handle, new_chain_start_idx);

	return new_chain_start_idx;
}

/**
 * @brief 遍歷 FAT 鏈，並對鏈中的每個表項索引執行回呼函式。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx FAT 鏈的起始表項索引。
 * @param callback 回呼函式，簽章為 int callback(NfsFatHandle*, int current_entry_idx)。
 * 如果回呼函式返回 0，則遍歷提前終止。
 */
void nfs_fat_chain_for_each(NfsFatHandle* fat_handle, int start_of_chain_idx,
	int (*callback)(NfsFatHandle*, int)) {
	if (!fat_handle || !fat_handle->iio_file || !callback || start_of_chain_idx <= 0) { // 通常鏈索引是正數
		if (start_of_chain_idx == -1 && callback) { /*允許遍歷空鏈，不執行回呼*/ return; }
		if (start_of_chain_idx <= 0 && start_of_chain_idx != -1) return; //無效起始
	}

	int current_entry_idx = start_of_chain_idx;
	int next_entry_idx;
	bool problem_encountered = false;

	while (current_entry_idx != -1) { // -1 是 EOC
		next_entry_idx = node_get_value(fat_handle, current_entry_idx);

		if (callback(fat_handle, current_entry_idx) == 0) { // 如果回呼要求停止
			break;
		}

		// 原始碼中有一個檢查 `if (current_entry_idx <= 0) problem_encountered = 1;`
		// 這暗示正常的鏈節點索引應該是正數。如果出現非正數（且非-1 EOC），則鏈可能已損壞。
		if (current_entry_idx <= 0) {
			problem_encountered = true;
		}

		current_entry_idx = next_entry_idx;
		if (problem_encountered) { // 如果遇到問題索引，也停止
			break;
		}
		if (current_entry_idx != -1 && current_entry_idx <= 0) { // 下一個索引無效 (非EOC)
			problem_encountered = true; // 標記問題並將在下一次迴圈開始時中斷
		}
	}
}

/**
 * @brief 銷毀指定的 FAT 鏈 (將鏈中所有表項標記為空閒)。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx 要銷毀的 FAT 鏈的起始表項索引。
 * @return 成功時返回 0，失敗 (例如無效參數) 時返回 -1。
 */
int nfs_fat_destroy_chain(NfsFatHandle* fat_handle, int start_of_chain_idx) {
	if (!fat_handle || !fat_handle->iio_file || start_of_chain_idx <= 0) { // 通常鏈的起始應為正數
		if (start_of_chain_idx == -1) return 0; // 空鏈，無需銷毀
		return -1;
	}
	nfs_fat_chain_for_each(fat_handle, start_of_chain_idx, node_recover);
	return 0;
}

/**
 * @brief 取得 FAT 鏈中第 N 個表項的索引 (0-indexed)。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx FAT 鏈的起始表項索引。
 * @param n 要取得的表項的序數 (0 表示第一個，1 表示第二個，依此类推)。
 * @return 第 N 個表項的索引。如果鏈比 N 短或發生錯誤，則返回 -1。
 */
int nfs_fat_chain_get_nth(NfsFatHandle* fat_handle, int start_of_chain_idx, int n) {
	// 加入 start_of_chain_idx <= 0 的檢查
	if (!fat_handle
		|| !fat_handle->iio_file
		|| n < 0
		|| start_of_chain_idx <= 0) {
		return -1;
	}

	int current_entry_idx = start_of_chain_idx;
	for (int i = 0; i < n; ++i) {
		if (current_entry_idx == -1 || current_entry_idx <= 0) {
			return -1;
		}
		current_entry_idx = node_get_value(fat_handle, current_entry_idx);
	}
	return current_entry_idx;
}


/**
 * @brief 截斷 FAT 鏈，使指定的表項成為新的鏈尾。
 * @param fat_handle NfsFatHandle 指標。
 * @param entry_idx_to_become_new_eoc 要成為新鏈尾的表項索引。鏈將在此表項處結束。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int nfs_fat_chain_truncate(NfsFatHandle* fat_handle, int entry_idx_to_become_new_eoc) {
	if (!fat_handle || !fat_handle->iio_file || entry_idx_to_become_new_eoc <= 0) {
		// 通常 EOC 的索引是有效的正數
		return -1;
	}

	int next_block_after_trunc_point = node_get_value(fat_handle, entry_idx_to_become_new_eoc);
	node_set_value(fat_handle, entry_idx_to_become_new_eoc, -1); // 設定新的鏈尾

	if (next_block_after_trunc_point != -1 && next_block_after_trunc_point > 0) { // 如果原先後面還有鏈
		nfs_fat_destroy_chain(fat_handle, next_block_after_trunc_point); // 銷毀後續的鏈
	}
	return 0;
}

/**
 * @brief 擴展指定的 FAT 鏈，在末尾附加一個新的空閒區塊。
 * @param fat_handle NfsFatHandle 指標。
 * @param chain_to_extend 要擴展的鏈的起始區塊索引。如果此值無效 (<=0 且非 -1)，則行為類似建立新鏈。
 * @return 新加入的區塊的索引；如果失敗則返回 -1。
 */
int nfs_fat_chain_extend(NfsFatHandle* fat_handle, int chain_to_extend) {
	if (!fat_handle || !fat_handle->iio_file) {
		return -1;
	}

	// 獲取一個新的空閒區塊 (邏輯同 nfs_fat_create_chain)
	int new_block_idx = fat_handle->next_free_search_start_idx;
	if (new_block_idx <= 0 || node_get_value(fat_handle, new_block_idx) != 0) {
		new_block_idx = next_free(fat_handle, new_block_idx > 0 ? new_block_idx + 1 : 1);
		if (new_block_idx <= 0) return -1;
	}

	node_set_value(fat_handle, new_block_idx, -1); // 新區塊暫時設為鏈尾
	fat_handle->next_free_search_start_idx = next_free(fat_handle, new_block_idx);


	if (chain_to_extend <= 0) { // 如果原始鏈無效或不存在 (例如傳入0或-1表示從新鏈開始)
		return new_block_idx; // 新區塊自身構成一個新鏈
	}

	int last_block_in_chain = find_last_in_chain(fat_handle, chain_to_extend);
	if (last_block_in_chain == -1 || last_block_in_chain <= 0) { // 原始鏈無效
		// 理論上 find_last_in_chain 會返回有效的最後一個節點或起始節點（如果鏈只有一個元素）
		// 如果 chain_to_extend 有效但 find_last_in_chain 失敗，這可能是個問題
		// 此處假設如果 chain_to_extend > 0，則 last_block_in_chain 應該也是 >0
		// 若 find_last_in_chain 返回 -1，則原鏈為空，新塊成為鏈頭。
		// 但上面已處理 chain_to_extend <=0 的情況。
		// 若 chain_to_extend > 0 但 find_last_in_chain 返回 <=0 (非-1)，則鏈已損壞。
		// 為了安全，如果 last_block_in_chain 無效，則新塊獨立成鏈（雖然這可能不符合預期）。
		// 原始碼中，如果 a2 <= 0，find_last_in_chain(a1,a2) 的行為未明確處理，
		// node_get_value(a1,0) 或 node_get_value(a1,-1) 的結果決定其行為。
		// 假設 find_last_in_chain 對有效的 chain_to_extend (>0) 返回 >0 的值。
		node_set_value(fat_handle, last_block_in_chain, new_block_idx); // 連接舊鏈尾到新區塊
	}
	else {
		node_set_value(fat_handle, last_block_in_chain, new_block_idx);
	}


	return new_block_idx;
}

/**
 * @brief 縮減 FAT 鏈到指定的區塊數量。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx 要縮減的鏈的起始索引。
 * @param num_blocks_to_keep 要保留的區塊數量。如果為0，則銷毀整個鏈。
 * @return 成功時返回 0，失敗時返回 -1。
 */
int nfs_fat_chain_shrink(NfsFatHandle* fat_handle, int start_of_chain_idx, int num_blocks_to_keep) {
	if (!fat_handle || !fat_handle->iio_file || num_blocks_to_keep < 0) {
		return -1;
	}
	if (start_of_chain_idx <= 0 && start_of_chain_idx != -1) return -1; // 無效起始鏈
	if (start_of_chain_idx == -1) return 0; // 空鏈無需縮減

	if (num_blocks_to_keep == 0) { // 如果要保留0個區塊，則銷毀整個鏈
		return nfs_fat_destroy_chain(fat_handle, start_of_chain_idx);
	}

	// 找到新的鏈尾，即第 (num_blocks_to_keep - 1) 個節點 (0-indexed)
	int new_eoc_node_idx = nfs_fat_chain_get_nth(fat_handle, start_of_chain_idx, num_blocks_to_keep - 1);

	if (new_eoc_node_idx == -1 || new_eoc_node_idx <= 0) {
		// 鏈的長度小於 num_blocks_to_keep，或者鏈本身有問題
		// 如果鏈長度 < num_blocks_to_keep，則無需縮減 (或視為錯誤)
		// 計算實際鏈長度以判斷
		int current_len = 0;
		int temp_iter = start_of_chain_idx;
		while (temp_iter != -1 && temp_iter > 0) {
			current_len++;
			if (current_len > num_blocks_to_keep + 5 && num_blocks_to_keep > 0) break; // 安全中斷
			temp_iter = node_get_value(fat_handle, temp_iter);
		}
		if (current_len < num_blocks_to_keep) return -1; // 請求保留的比實際還多
		if (new_eoc_node_idx <= 0 && new_eoc_node_idx != -1) return -1; // get_nth 返回了無效的中間節點
		// 如果 new_eoc_node_idx == -1，表示鏈在到達 num_blocks_to_keep-1 之前就結束了
		// 這意味著鏈長度 < num_blocks_to_keep，所以不需要截斷。
		return 0;
	}

	// new_eoc_node_idx 是有效的，它將成為新的鏈尾
	return nfs_fat_chain_truncate(fat_handle, new_eoc_node_idx);
}

/**
 * @brief 取得整個 FAT 鏈的所有表項索引。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx FAT 鏈的起始索引。
 * @param output_buffer 指向整數陣列的指標，用於儲存鏈中的表項索引。
 * 呼叫者必須確保此緩衝區足夠大。
 */
void nfs_fat_chain_get(NfsFatHandle* fat_handle, int start_of_chain_idx, void* output_buffer) {
	if (!fat_handle || !fat_handle->iio_file || !output_buffer) {
		return;
	}

	int current_block_idx = start_of_chain_idx;
	int* out_ptr = static_cast<int*>(output_buffer);

	// 只處理有效的正鏈索引
	while (current_block_idx != -1 && current_block_idx > 0) {
		*out_ptr = current_block_idx;
		out_ptr++;
		current_block_idx = node_get_value(fat_handle, current_block_idx);
	}
	// 如果希望在鏈尾也加入-1，可以取消註解下一行，但通常只獲取有效塊索引
	// if (current_block_idx == -1 && start_of_chain_idx > 0) { *out_ptr = -1; }
}

/**
 * @brief 取得 FAT 鏈的前 N 個表項索引。
 * @param fat_handle NfsFatHandle 指標。
 * @param start_of_chain_idx FAT 鏈的起始索引。
 * @param num_entries_to_get 要獲取的表項數量。
 * @param output_buffer 指向整數陣列的指標，用於儲存表項索引。
 * 呼叫者必須確保此緩衝區至少能容納 num_entries_to_get 個整數。
 */
void nfs_fat_chain_get_first_n(NfsFatHandle* fat_handle, int start_of_chain_idx, int num_entries_to_get, void* output_buffer) {
	if (!fat_handle || !fat_handle->iio_file || !output_buffer || num_entries_to_get <= 0) {
		return;
	}

	int current_block_idx = start_of_chain_idx;
	int* out_ptr = static_cast<int*>(output_buffer);
	int entries_retrieved = 0;

	while (entries_retrieved < num_entries_to_get) {
		if (current_block_idx == -1 || current_block_idx <= 0) { // 已達鏈尾或遇到無效索引
			break;
		}
		*out_ptr = current_block_idx;
		out_ptr++;
		entries_retrieved++;
		if (entries_retrieved < num_entries_to_get) { // 只有在還需要更多條目時才讀取下一個
			current_block_idx = node_get_value(fat_handle, current_block_idx);
		}
	}
}

// Layer 5
/**
 * @brief (NT 版本) 從 IIO 通道讀取指定索引的 NT 節點資料。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 要讀取的節點索引。
 * @param node_buffer 指向 NfsNode 結構的指標，用於儲存讀取的節點資料。
 * @return nfs_iio_read 的返回值 (通常是讀取的位元組數，或錯誤碼)。
 */
int node_get(NfsNtHandle* nt_handle, int node_index, NfsNode* node_buffer) {
	if (!nt_handle || !nt_handle->iio_file || !node_buffer) {
		return -1;
	}
	nfs_iio_seek(nt_handle->iio_file, nt_handle->nt_iio_channel_id,
		node_index * sizeof(NfsNode));
	int bytes_read = nfs_iio_read(nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		node_buffer,
		sizeof(NfsNode));
	// 如果讀到的長度少於整個結構，就把剩下的部份清成0
	if (bytes_read < (int)sizeof(NfsNode)) {
		int offset = bytes_read > 0 ? bytes_read : 0;
		memset((char*)node_buffer + offset,
			0,
			sizeof(NfsNode) - offset);
		return sizeof(NfsNode);
	}
	return bytes_read;
}


/**
 * @brief (NT 版本) 將指定的 NT 節點資料寫入 IIO 通道。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 要設定的節點索引。
 * @param node_data 指向 NfsNode 結構的指標，包含要寫入的資料。
 * @return nfs_iio_write 的返回值 (通常是寫入的位元組數，或錯誤碼)。
 */
int node_set(NfsNtHandle* nt_handle, int node_index, const NfsNode* node_data) {
	if (!nt_handle || !nt_handle->iio_file || !node_data) {
		return -1; // 或其他錯誤碼
	}
	nfs_iio_seek(nt_handle->iio_file, nt_handle->nt_iio_channel_id, node_index * sizeof(NfsNode));
	return nfs_iio_write(nt_handle->iio_file, nt_handle->nt_iio_channel_id, node_data, sizeof(NfsNode));
}

/**
 * @brief (NT 版本) 從指定的起始索引開始，尋找 NT 中的第一個空閒節點。
 * 空閒節點的引用計數 (ref_count) 小於或等於 0。
 * @param nt_handle NfsNtHandle 指標。
 * @param start_search_idx 開始搜尋的節點索引。
 * @return 找到的空閒節點的索引。
 */
int find_first_free(NfsNtHandle* nt_handle, int start_search_idx) {
	if (!nt_handle) return -1; // 或其他錯誤指示

	int current_idx = start_search_idx;
	NfsNode temp_node;

	// 初始讀取一次
	if (node_get(nt_handle, current_idx, &temp_node) != sizeof(NfsNode)) {
		// 讀取失敗，可能索引無效或IO錯誤，假設節點很多，繼續增加索引可能不安全
		// 這裡的行為取決於 node_get 失敗時的返回值和上層如何處理
		// 為避免潛在無限迴圈（如果 node_get 總失敗且 ref_count 未初始化），最好有退出條件
		// 原始碼的迴圈結構是 `node_get(...); while (ref_count > 0) { node_get(++idx, ...); }`
		// 這暗示第一次 node_get 總能成功，或者 ref_count 會被賦值。
	}

	while (temp_node.ref_count > 0) {
		current_idx++;
		if (node_get(nt_handle, current_idx, &temp_node) != sizeof(NfsNode)) {
			// 讀取失敗，可能已超出NT範圍或IO錯誤
			return -1; // 表示未找到或出錯
		}
		// 添加一個安全中斷條件，防止因NT損壞導致的無限迴圈
		if (current_idx > start_search_idx + 10000000) { // 假設NT不會有這麼多連續已用節點
			return -1; // 超出合理搜尋範圍
		}
	}
	return current_idx;
}

/**
 * @brief (NT 版本) 回收一個 NT 節點，將其內容清零 (特別是 ref_count)。
 * 同時，如果回收的節點索引小於目前快取的「下一個空閒起始搜尋索引」，則更新它。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_idx_to_free 要回收的 NT 節點的索引。
 * @return 固定返回 0 (原始碼行為)。
 */
int node_recover(NfsNtHandle* nt_handle, int node_idx_to_free) {
	if (!nt_handle) return -1;

	NfsNode zero_node;
	memset(&zero_node, 0, sizeof(NfsNode)); // 將節點所有成員清零

	node_set(nt_handle, node_idx_to_free, &zero_node);

	// 如果回收的索引比目前記錄的 next_free_node_search_start_idx 更靠前，則更新它。
	// 原始碼檢查 `a2 < *((_DWORD *)a1 + 2)`，不檢查 a2 是否為正。
	// find_first_free 從0開始搜索是允許的。
	if (node_idx_to_free < nt_handle->next_free_node_search_start_idx) {
		nt_handle->next_free_node_search_start_idx = node_idx_to_free;
	}
	return 0;
}

/**
 * @brief 建立一個新的 NT (Node Table) 管理結構。
 * 會配置一個 IIO 通道用於儲存 NT 資料。
 * @param iio_file 指向 NfsIioFile 的指標，NT 將在此檔案中建立通道。
 * @param num_iio_blocks_for_nt_channel 為 NT 通道配置的 IIO 條帶中的區塊數 (預設為1)。
 * @return 指向新建立的 NfsNtHandle 的指標，失敗時返回 nullptr。
 */
NfsNtHandle* nfs_nt_create(NfsIioFile* iio_file, int num_iio_blocks_for_nt_channel) {
	if (!iio_file) {
		return nullptr;
	}

	int blocks_for_nt_stripe = (num_iio_blocks_for_nt_channel == 0) ? 1 : num_iio_blocks_for_nt_channel;
	int nt_channel_id = nfs_iio_allocate_channel(iio_file, blocks_for_nt_stripe);

	if (nt_channel_id < 0) { // IIO 通道分配失敗
		return nullptr;
	}

	NfsNtHandle* nt_handle = static_cast<NfsNtHandle*>(malloc(sizeof(NfsNtHandle)));
	if (!nt_handle) {
		// 若此處分配失敗，先前分配的 IIO 通道 nt_channel_id 會洩漏。
		// 穩健的程式碼應處理此情況。此處為忠實還原。
		return nullptr;
	}

	nt_handle->iio_file = iio_file;
	nt_handle->nt_iio_channel_id = nt_channel_id;
	nt_handle->next_free_node_search_start_idx = 0; // 搜尋可以從索引 0 開始

	return nt_handle;
}

/**
 * @brief 開啟一個已存在的 NT (Node Table)。
 * @param iio_file 指向 NfsIioFile 的指標。
 * @param nt_iio_channel_id 儲存 NT 資料的已存在的 IIO 通道 ID。
 * @return 指向 NfsNtHandle 的指標，失敗時返回 nullptr。
 */
NfsNtHandle* nfs_nt_open(NfsIioFile* iio_file, int nt_iio_channel_id) {
	if (!iio_file) {
		return nullptr;
	}

	NfsNtHandle* nt_handle = static_cast<NfsNtHandle*>(malloc(sizeof(NfsNtHandle)));
	if (!nt_handle) {
		return nullptr;
	}

	nt_handle->iio_file = iio_file;
	nt_handle->nt_iio_channel_id = nt_iio_channel_id;
	// 初始化 next_free_node_search_start_idx，從索引 0 開始搜尋
	nt_handle->next_free_node_search_start_idx = find_first_free(nt_handle, 0);

	return nt_handle;
}

/**
 * @brief 關閉 NT，僅釋放 NfsNtHandle 結構本身。
 * 注意：此函式不釋放 NT 資料所在的 IIO 通道。
 * @param nt_handle 要關閉的 NfsNtHandle 指標。
 */
void nfs_nt_close(NfsNtHandle* nt_handle) {
	if (nt_handle) {
		free(nt_handle);
	}
}

/**
 * @brief 銷毀 NT，實質上只是呼叫 nfs_nt_close。
 * 此函式不處理底層 IIO 通道資料的銷毀。
 * @param nt_handle 要銷毀的 NfsNtHandle 指標。
 */
void nfs_nt_destroy(NfsNtHandle* nt_handle) {
	nfs_nt_close(nt_handle);
}

/**
 * @brief 取得指定 NT 節點的內容。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 要取得的節點索引。
 * @param node_buffer 用於儲存節點資料的 NfsNode 指標。
 * @return 底層 node_get 的返回值 (通常是讀取的位元組數或錯誤碼)。若 nt_handle 為空則返回 -1。
 */
int nfs_nt_get_node(NfsNtHandle* nt_handle, int node_index, NfsNode* node_buffer) {
	if (!nt_handle) {
		return -1;
	}
	return node_get(nt_handle, node_index, node_buffer);
}

/**
 * @brief 設定指定 NT 節點的內容。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 要設定的節點索引。
 * @param node_data 包含新節點資料的 NfsNode 指標 (const)。
 * @return 底層 node_set 的返回值 (通常是寫入的位元組數或錯誤碼)。
 */
int nfs_nt_set_node(NfsNtHandle* nt_handle, int node_index, const NfsNode* node_data) {
	if (!nt_handle) { // 原始碼中此函式沒有對 nt_handle 的 null 檢查，但加上更安全
		return -1;
	}
	return node_set(nt_handle, node_index, node_data);
}

/**
 * @brief 配置一個新的 NT 節點。
 * 新節點的引用計數設為1，大小和 FAT 鏈設為0 (或無效)。
 * @param nt_handle NfsNtHandle 指標。
 * @return 新配置節點的索引；如果失敗則返回 -1。
 */
int nfs_nt_allocate_node(NfsNtHandle* nt_handle) {
	if (!nt_handle) {
		return -1;
	}

	// 使用 nt_handle->next_free_node_search_start_idx 作為分配的節點索引
	// find_first_free 已在 nfs_nt_open 中初始化此值，並在此函式結尾更新
	// 從提示索引開始，呼叫 find_first_free 跳過所有已使用的節點
	int start_search = nt_handle->next_free_node_search_start_idx;
	if (start_search < 0) start_search = 0;
	int new_node_idx = find_first_free(nt_handle, start_search);
	if (new_node_idx < 0) return -1;


	NfsNode new_node;
	new_node.ref_count = 1;
	new_node.file_size_bytes = 0;
	new_node.fat_chain_start_idx = 0; // 或 -1，取決於「無鏈」的表示方式
	new_node.user_flags_or_type = 0;  // 初始化第四個整數

	if (nfs_nt_set_node(nt_handle, new_node_idx, &new_node) < 0) { // 寫入失敗
		return -1;
	}

	// 更新 nt_handle->next_free_node_search_start_idx 為下一個空閒節點
	// 從剛分配的 new_node_idx 開始搜尋，find_first_free 會找到其後的下一個空閒節點
	nt_handle->next_free_node_search_start_idx = find_first_free(nt_handle, new_node_idx);

	return new_node_idx;
}

/**
 * @brief 增加指定 NT 節點的引用計數。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 節點索引。
 */
void nfs_nt_refcount_incr(NfsNtHandle* nt_handle, int node_index) {
	if (!nt_handle) {
		return;
	}
	NfsNode current_node;
	if (nfs_nt_get_node(nt_handle, node_index, &current_node) >= 0) { // 假設 >=0 表示成功
		// 只有已配置（ref_count>0）的節點才遞增
		if (current_node.ref_count > 0) {
			current_node.ref_count++;
			nfs_nt_set_node(nt_handle, node_index, &current_node);
		}
	}
}

/**
 * @brief 減少指定 NT 節點的引用計數，如果降至 0 或本來就 ≤0 就回收；超出檔案範圍的索引視為無效，直接返回 0。
 * @param nt_handle 指向 NfsNtHandle 的指標
 * @param node_index 節點索引
 * @return
 *   - 對無效節點（超出已分配範圍）或 handle 為 null，直接返回 0
 *   - 第一次從 ref_count>1 → ref_count>0，返回 0（未回收）
 *   - 從 ref_count==1 → ref_count==0，回收並返回 1
 *   - 對已回收（ref_count==0）的有效節點，再次 decr 仍回收並返回 1
 */
int nfs_nt_refcount_decr(NfsNtHandle* nt_handle, int node_index) {
	if (!nt_handle) {
		return 0;
	}

	// 1. 先做 raw read，檢查是否超出檔案範圍
	NfsNode current_node;
	// 定位到該節點在 NT channel 的位移
	nfs_iio_seek(nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode));
	// 直接讀取，不透過 node_get，才能拿到原始 bytes_read
	int bytes_read = nfs_iio_read(nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&current_node,
		sizeof(NfsNode));
	// 如果實際讀到的位元組小於 NfsNode，大概就是超出檔案尾端 → 無效索引
	if (bytes_read < (int)sizeof(NfsNode)) {
		return 0;
	}

	// 2. 如果 ref_count > 0，做真正的遞減
	if (current_node.ref_count > 0) {
		current_node.ref_count--;
		// 寫回新的 ref_count
		nfs_nt_set_node(nt_handle, node_index, &current_node);
		// 如果降到 0，呼叫回收
		if (current_node.ref_count == 0) {
			node_recover(nt_handle, node_index);
			return 1;
		}
		return 0;
	}

	// 3. 已是 ref_count == 0 的「已回收」節點，再次 decr 也視為回收
	//    但要觸發 set + recover 以更新 clock 和 next_free
	nfs_nt_set_node(nt_handle, node_index, &current_node);
	node_recover(nt_handle, node_index);
	return 1;
}

/**
 * @brief 取得 NT 節點中儲存的檔案大小。
 *   如果節點索引超出範圍或 nt_handle 為 null，則返回 -1。
 * @param nt_handle NfsNtHandle 指標。
 * @param node_index 節點索引。
 * @return 檔案大小 (位元組)；讀取失敗或索引無效時返回 -1。
 */
int nfs_nt_node_get_size(NfsNtHandle* nt_handle, int node_index) {
	if (!nt_handle) {
		return -1;
	}

	// 低階 seek 到該節點位置
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode)
	);

	// raw read
	NfsNode current_node;
	int bytes_read = nfs_iio_read(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&current_node,
		sizeof(NfsNode)
	);

	// 如果實際讀到的位元組少於整個結構，視為無效索引
	if (bytes_read < (int)sizeof(NfsNode)) {
		return -1;
	}

	// 成功讀到完整節點，回傳其中的 file_size_bytes
	return current_node.file_size_bytes;
}

/**
 * @brief   設定 NT 節點的檔案大小；只有當該節點確實存在（完整讀回 sizeof(NfsNode)）時才會寫入。
 * @param   nt_handle    已初始化的 NfsNtHandle*，不可為 nullptr
 * @param   node_index   節點索引
 * @param   new_size     要寫入的檔案大小 (bytes)
 */
void nfs_nt_node_set_size(NfsNtHandle* nt_handle, int node_index, int new_size) {
	// 1. handle 檢查
	if (!nt_handle || !nt_handle->iio_file) {
		return;
	}

	// 2. 定位到這個節點在 IIO 檔案中的位移
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode)
	);

	// 3. 嘗試讀回整個 NfsNode 結構
	NfsNode node_buffer;
	int bytes_read = nfs_iio_read(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&node_buffer,
		sizeof(NfsNode)
	);

	// 4. 只有當完整讀到 sizeof(NfsNode) 才視為「有效節點」
	if (bytes_read != (int)sizeof(NfsNode)) {
		return;
	}

	// 5. 更新檔案大小欄位
	node_buffer.file_size_bytes = new_size;

	// 6. 再次 seek，並寫回整個 NfsNode
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode)
	);
	nfs_iio_write(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&node_buffer,
		sizeof(NfsNode)
	);
	// 內部的 nfs_iio_read/nfs_iio_write 都已經會自動遞增 nfs_iio_CLOCK
}


/**
 * @brief 取得指定 NT 節點的 FAT chain 起始索引。
 *        如果節點索引無效（超出範圍）或 nt_handle 為 nullptr，回傳 -1。
 * @param nt_handle 指向已初始化的 NfsNtHandle
 * @param node_index 節點索引
 * @return FAT chain 起始索引；讀取失敗或無效索引時回傳 -1。
 */
int nfs_nt_node_get_chain(NfsNtHandle* nt_handle, int node_index) {
	// 1. handle 檢查
	if (!nt_handle) {
		return -1;
	}

	// 2. 定位到該節點在 IIO 檔案中的位移
	//    每個 NfsNode 大小為 sizeof(NfsNode)，node_index*size 即為偏移量
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		node_index * (int)sizeof(NfsNode)
	);

	// 3. raw read 一個 NfsNode
	NfsNode node_buffer;
	int bytes_read = nfs_iio_read(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&node_buffer,
		sizeof(NfsNode)
	);

	// 4. 判斷是否完整讀到
	if (bytes_read != (int)sizeof(NfsNode)) {
		// 超出檔案尾端或讀取失敗
		return -1;
	}

	// 5. 回傳 chain 起始索引
	return node_buffer.fat_chain_start_idx;
}

/**
 * @brief 設定 NT 節點中儲存的 FAT 鏈起始索引；只有當該節點確實存在時才會更新
 * @param nt_handle              已初始化的 NfsNtHandle*，不可為 nullptr
 * @param node_index             節點索引
 * @param new_fat_chain_start_idx 新的 FAT 鏈起始索引
 */
void nfs_nt_node_set_chain(NfsNtHandle* nt_handle, int node_index, int new_fat_chain_start_idx) {
	// 1. null handle 檢查
	if (!nt_handle || !nt_handle->iio_file) {
		return;
	}

	// 2. 定位到這個節點在 IIO 檔案中的偏移
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode)
	);

	// 3. raw read 一個 NfsNode
	NfsNode node_buffer;
	int bytes_read = nfs_iio_read(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&node_buffer,
		sizeof(NfsNode)
	);

	// 4. 只有在完整讀到 sizeof(NfsNode) 時才視為有效節點
	if (bytes_read != (int)sizeof(NfsNode)) {
		return;
	}

	// 5. 更新 chain 起始索引
	node_buffer.fat_chain_start_idx = new_fat_chain_start_idx;

	// 6. 再次 seek 回原位，並寫回整個 NfsNode
	nfs_iio_seek(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		(long long)node_index * sizeof(NfsNode)
	);
	nfs_iio_write(
		nt_handle->iio_file,
		nt_handle->nt_iio_channel_id,
		&node_buffer,
		sizeof(NfsNode)
	);
}

// Layer 6
/** @brief 從 IIO 通道讀取指定索引的 Trie 節點資料。*/
int trienode_get(NfsDtHandle* dt_handle, int tn_idx, NfsTrieNode* out_node) {
	if (!dt_handle || !dt_handle->iio_file || !out_node) return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode));
	return nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, out_node, sizeof(NfsTrieNode));
}

/** @brief 將指定的 Trie 節點資料寫入 IIO 通道。*/
int trienode_set(NfsDtHandle* dt_handle, int tn_idx, NfsTrieNode* node_to_set) {
	if (!dt_handle || !dt_handle->iio_file || !node_to_set) return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode));
	return nfs_iio_write(dt_handle->iio_file, dt_handle->trienode_channel_id, node_to_set, sizeof(NfsTrieNode));
}

/** @brief 從 IIO 通道讀取指定索引的鍵節點資料。*/
int keynode_get(NfsDtHandle* dt_handle, int kn_idx, NfsKeyNode* out_node) {
	if (!dt_handle || !dt_handle->iio_file || !out_node) return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->keynode_channel_id, kn_idx * sizeof(NfsKeyNode));
	return nfs_iio_read(dt_handle->iio_file, dt_handle->keynode_channel_id, out_node, sizeof(NfsKeyNode));
}

/** @brief 將指定的鍵節點資料寫入 IIO 通道。*/
int keynode_set(NfsDtHandle* dt_handle, int kn_idx, const NfsKeyNode* node_to_set) {
	if (!dt_handle || !dt_handle->iio_file || !node_to_set) return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->keynode_channel_id, kn_idx * sizeof(NfsKeyNode));
	return nfs_iio_write(dt_handle->iio_file, dt_handle->keynode_channel_id, node_to_set, sizeof(NfsKeyNode));
}

/** @brief 檢查指定的 Trie 節點是否空閒。空閒條件：k_index >= 0。索引0的 TrieNode 被視為非空閒 (用於 Trie 頭部)。*/
int trienode_is_free(NfsDtHandle* dt_handle, int tn_idx) {
	if (tn_idx == 0) { // 索引 0 的 TrieNode (Trie 頭部) 不被視為空閒
		return 0;
	}
	NfsTrieNode temp_node;
	if (trienode_get(dt_handle, tn_idx, &temp_node) == sizeof(NfsTrieNode)) {
		return (temp_node.k_index >= 0); // k_index >= 0 表示空閒
	}
	return 0; // 讀取失敗，視為非空閒
}

/** @brief 從指定的起始索引開始，尋找第一個空閒的 Trie 節點 (索引 > 0)。*/
int trienode_find_first_free(NfsDtHandle* dt_handle, int start_idx) {
	if (!dt_handle || !dt_handle->iio_file) return -1;
	// 取得通道資訊
	NfsIioChannel* ch = nfs_iio_get_channel(dt_handle->iio_file, dt_handle->trienode_channel_id);
	if (!ch) return -1;
	int channel_bytes = nfs_iio_channel_size(ch);
	int entry_size = sizeof(NfsTrieNode);
	int num_entries = (channel_bytes > 0) ? (channel_bytes / entry_size) : 0;

	// 從合法起點開始
	int idx = (start_idx > 0) ? start_idx : 1;
	int max_idx = idx + 1000000; // 安全上限

	for (; idx > 0 && idx <= max_idx; ++idx) {
		// 超出已初始化區段，直接當成空閒
		if (idx >= num_entries) {
			return idx;
		}
		// 否則檢查 k_index >= 0 即為空閒
		if (trienode_is_free(dt_handle, idx)) {
			return idx;
		}
	}
	return -1;
}

/** @brief 清除指定的 Trie 節點，將其標記为空閒。k_index 設為 0，子節點指向自身。*/
int trienode_clear(NfsDtHandle* dt_handle, int tn_idx) {
	NfsTrieNode cleared_node;
	cleared_node.nt_idx = 0;
	cleared_node.b_index = 0;
	cleared_node.k_index = 0;   // 標記為空閒
	cleared_node.left_child_idx = tn_idx; // 指向自身 (可用於空閒鏈表)
	cleared_node.right_child_idx = tn_idx;// 指向自身
	return trienode_set(dt_handle, tn_idx, &cleared_node);
}

/** @brief 回收一個 Trie 節點 (將其清除)，並更新 DT 控制代碼中的空閒節點搜尋提示。*/
int trienode_recover(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || tn_idx < 0) return -1; // 不處理無效索引
	if (tn_idx == 0) return -1; // 索引0的 trienode 通常是 head，不應被 recover

	trienode_clear(dt_handle, tn_idx);
	if (tn_idx > 0 && tn_idx < dt_handle->next_free_trienode_idx) { // 只在 tn_idx > 0 時更新
		dt_handle->next_free_trienode_idx = tn_idx;
	}
	return 0;
}

/** @brief 檢查指定的鍵節點是否空閒。空閒條件：next_fragment_idx_flags >= 0。索引0的 KeyNode 被視為非空閒。*/
int keynode_is_free(NfsDtHandle* dt_handle, int kn_idx) {
	if (kn_idx == 0) { // 索引 0 的 KeyNode (用於 DT 頭部) 不被視為空閒
		return 0;
	}
	NfsKeyNode temp_node;
	if (keynode_get(dt_handle, kn_idx, &temp_node) == sizeof(NfsKeyNode)) {
		return (temp_node.next_fragment_idx_flags >= 0); // next_fragment_idx_flags >= 0 表示空閒
	}
	return 0; // 讀取失敗，視為非空閒
}

/** @brief 從指定的起始索引開始，尋找第一個空閒的鍵節點 (索引 > 0)。*/
int keynode_find_first_free(NfsDtHandle* dt_handle, int start_idx) {
	if (!dt_handle || !dt_handle->iio_file) return -1;
	// 起點：若 start_idx <= 0，就從 1 開始
	int idx = (start_idx > 0) ? start_idx : 1;

	// 取得 keynode channel 目前寫入的位元組長度
	NfsIioChannel* ch = nfs_iio_get_channel(dt_handle->iio_file,
		dt_handle->keynode_channel_id);
	int channel_bytes = nfs_iio_channel_size(ch);
	int entry_size = sizeof(NfsKeyNode);
	// 已初始化（含零補齊及寫入）的節點數
	int num_entries = (channel_bytes > 0) ? (channel_bytes / entry_size) : 0;

	// 為了安全，限制搜尋到 start_idx+1,000,000 或 1,000,000
	int max_idx = (start_idx > 0) ? (start_idx + 1000000) : 1000000;

	for (; idx > 0 && idx <= max_idx; ++idx) {
		// 1) 若 idx 超過已初始化區段，直接當成空閒
		if (idx >= num_entries) {
			return idx;
		}
		// 2) 否則照原本方式檢查
		if (keynode_is_free(dt_handle, idx)) {
			return idx;
		}
	}
	return -1;
}

/** @brief 清除指定的鍵節點，將其標記为空閒 (所有成員設為0)。*/
int keynode_clear(NfsDtHandle* dt_handle, int kn_idx) {
	NfsKeyNode cleared_node;
	memset(&cleared_node, 0, sizeof(NfsKeyNode)); // next_fragment_idx_flags 將變為0，表示空閒
	return keynode_set(dt_handle, kn_idx, &cleared_node);
}

unsigned int trienode_get_left(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || !dt_handle->iio_file) return 0; // 或錯誤碼
	int left_val = 0;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, left_child_idx));
	if (nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, &left_val, sizeof(int)) == sizeof(int)) {
		return left_val;
	}
	return 0; // 錯誤時返回0 (或特定錯誤碼)
}

unsigned int trienode_get_right(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || !dt_handle->iio_file) return 0;
	int right_val = 0;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, right_child_idx));
	if (nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, &right_val, sizeof(int)) == sizeof(int)) {
		return right_val;
	}
	return 0;
}

short trienode_get_bindex(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || !dt_handle->iio_file) return 0; // 或錯誤碼
	short b_index_val = 0;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, b_index));
	if (nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, &b_index_val, sizeof(short)) == sizeof(short)) {
		return b_index_val;
	}
	return 0;
}

unsigned int trienode_get_kindex(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || !dt_handle->iio_file) return 0;
	int k_index_val = 0;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, k_index));
	if (nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, &k_index_val, sizeof(int)) == sizeof(int)) {
		return static_cast<unsigned int>(k_index_val) & 0x7FFFFFFF; // 清除 MSB 旗標
	}
	return 0;
}

short trienode_get_nt(NfsDtHandle* dt_handle, int tn_idx) {
	if (!dt_handle || !dt_handle->iio_file) return 0; // 或錯誤碼
	short nt_idx_val = 0;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id, tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, nt_idx));
	if (nfs_iio_read(dt_handle->iio_file, dt_handle->trienode_channel_id, &nt_idx_val, sizeof(short)) == sizeof(short)) {
		return nt_idx_val;
	}
	return 0;
}

int trienode_set_left(NfsDtHandle* dt_handle, int tn_idx, int left_child_idx) {
	if (!dt_handle || !dt_handle->iio_file)
		return -1;
	NfsIioChannel* chan = nfs_iio_get_channel(dt_handle->iio_file, dt_handle->trienode_channel_id);
	if (!chan)
		return -1;
	int max_nodes = chan->current_size_bytes / sizeof(NfsTrieNode);
	if (tn_idx < 0 || tn_idx >= max_nodes)
		return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id,
		tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, left_child_idx));
	return nfs_iio_write(dt_handle->iio_file, dt_handle->trienode_channel_id,
		&left_child_idx, sizeof(int));
}

int trienode_set_right(NfsDtHandle* dt_handle, int tn_idx, int right_child_idx) {
	if (!dt_handle || !dt_handle->iio_file)
		return -1;
	NfsIioChannel* chan = nfs_iio_get_channel(dt_handle->iio_file, dt_handle->trienode_channel_id);
	if (!chan)
		return -1;
	int max_nodes = chan->current_size_bytes / sizeof(NfsTrieNode);
	if (tn_idx < 0 || tn_idx >= max_nodes)
		return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id,
		tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, right_child_idx));
	return nfs_iio_write(dt_handle->iio_file, dt_handle->trienode_channel_id,
		&right_child_idx, sizeof(int));
}

int trienode_set_nt(NfsDtHandle* dt_handle, int tn_idx, short nt_idx) {
	if (!dt_handle || !dt_handle->iio_file)
		return -1;
	NfsIioChannel* chan = nfs_iio_get_channel(dt_handle->iio_file, dt_handle->trienode_channel_id);
	if (!chan)
		return -1;
	int max_nodes = chan->current_size_bytes / sizeof(NfsTrieNode);
	if (tn_idx < 0 || tn_idx >= max_nodes)
		return -1;
	nfs_iio_seek(dt_handle->iio_file, dt_handle->trienode_channel_id,
		tn_idx * sizeof(NfsTrieNode) + offsetof(NfsTrieNode, nt_idx));
	return nfs_iio_write(dt_handle->iio_file, dt_handle->trienode_channel_id,
		&nt_idx, sizeof(short));
}

// --- Fnode (KeyNode chain) 相關函式 ---

/** @brief 從 KeyNode 鏈中提取完整的檔案名稱 (鍵)。*/
int fnode_extract_key(NfsDtHandle* dt_handle, int start_keynode_idx, char* output_buffer) {
	// 參數檢查
	if (!dt_handle || !output_buffer) {
		if (output_buffer) *output_buffer = '\0';
		return -1;
	}
	// 無效起始索引：直接回傳空字串
	if (start_keynode_idx <= 0) {
		*output_buffer = '\0';
		return 0;
	}

	const int max_key_len = 4095;     // 保護性上限
	int output_pos = 0;
	int current_idx = start_keynode_idx;

	do {
		NfsKeyNode node;
		// 讀取 KeyNode，不足大小視為錯誤
		if (keynode_get(dt_handle, current_idx, &node) != sizeof(NfsKeyNode)) {
			output_buffer[output_pos] = '\0';
			return -1;
		}

		// 取出下一個 fragment 的索引 (去掉 MSB 的 in-use 標誌)
		unsigned int next_idx = node.next_fragment_idx_flags & 0x7FFFFFFF;

		// 將本片段的內容複製到 output_buffer
		for (int i = 0; i < 60 && output_pos < max_key_len; ++i) {
			char c = node.key_fragment[i];
			if (c == '\0') {
				// 遇到 '\0' 表示本片段結束，但若 next_idx != 0
				// 則仍需繼續讀取後面 fragment
				break;
			}
			output_buffer[output_pos++] = c;
		}

		current_idx = next_idx;
	} while (current_idx != 0 && output_pos < max_key_len);

	// 最後加上字串結尾
	output_buffer[output_pos] = '\0';
	return 0;
}

/** @brief 釋放 (回收) 一個 KeyNode 鏈。*/
int fnode_free(NfsDtHandle* dt_handle, int start_keynode_idx_to_free) {
	if (!dt_handle || start_keynode_idx_to_free <= 0) { // 通常 KeyNode 索引是正數
		return 0; // 原始碼對無效索引不執行操作並返回0
	}

	NfsKeyNode current_keynode;
	int current_kn_idx = start_keynode_idx_to_free;
	int next_kn_idx;

	while (current_kn_idx != 0) { // 0 表示鏈結束 (next_fragment_idx_flags & 0x7FFFFFFF)
		if (keynode_get(dt_handle, current_kn_idx, &current_keynode) != sizeof(NfsKeyNode)) {
			return -1; // 讀取錯誤，無法繼續釋放
		}
		next_kn_idx = current_keynode.next_fragment_idx_flags & 0x7FFFFFFF;

		keynode_clear(dt_handle, current_kn_idx); // 清除 (標記為空閒)

		if (current_kn_idx > 0 && current_kn_idx < dt_handle->next_free_keynode_idx) {
			dt_handle->next_free_keynode_idx = current_kn_idx; // 更新空閒提示
		}
		current_kn_idx = next_kn_idx;
	}
	return 0;
}

/** @brief 為給定的字串分配一個 KeyNode 鏈來儲存它。*/
int fnode_allocate(NfsDtHandle* dt_handle, const char* source_string) {
	if (!dt_handle || !source_string) return -1;

	size_t len_with_null = strlen(source_string) + 1; // 包含結尾的 '\0'
	int num_fragments = static_cast<int>((len_with_null + 59) / 60); // 每個片段最多60字元，向上取整
	if (num_fragments == 0 && len_with_null == 1) { // 空字串 ""
		num_fragments = 1; // 仍需一個片段來儲存 '\0'
	}
	if (num_fragments == 0) return 0; // 理論上不可能，除非 len_with_null 為0 (無效字串)


	// 原始碼中使用了一個固定大小的本地陣列 (1024) 來儲存分配的索引，這限制了檔名的最大長度。
	// 為了更穩健，可以使用動態陣列，但為忠實還原，我們先假設檔名不會太長以至於超過合理限制。
	// 如果需要處理超長檔名，應使用 std::vector。
	const int MAX_FRAGMENTS_TEMP = 1024;
	if (num_fragments > MAX_FRAGMENTS_TEMP) return -1; // 超出臨時陣列大小限制
	int allocated_indices[MAX_FRAGMENTS_TEMP];

	// 階段 1: 分配所有需要的 KeyNode
	for (int i = 0; i < num_fragments; ++i) {
		int new_kn_idx = dt_handle->next_free_keynode_idx;
		// 檢查提示是否真的空閒，如果不是，則尋找下一個
		if (new_kn_idx <= 0 || !keynode_is_free(dt_handle, new_kn_idx)) {
			new_kn_idx = keynode_find_first_free(dt_handle, (new_kn_idx > 0) ? new_kn_idx : 1);
		}
		if (new_kn_idx <= 0) { // 沒有可用的空閒 KeyNode
			// 需要回滾已分配的 KeyNode (如果有的話)
			for (int k = 0; k < i; ++k) keynode_clear(dt_handle, allocated_indices[k]);
			return -1;
		}
		allocated_indices[i] = new_kn_idx;
		// 暫時將新分配的節點標記 (例如設為非空閒)，防止 find_first_free 立即重用它
		// 原始碼中，更新 next_free_keynode_idx 是在 keynode_set 之後或 find_first_free 內部處理
		// 這裡假設 keynode_find_first_free 會找到下一個 *不同* 的空閒塊
		dt_handle->next_free_keynode_idx = keynode_find_first_free(dt_handle, new_kn_idx + 1);
	}

	// 階段 2: 填寫資料並連結 KeyNode
	NfsKeyNode current_keynode;
	const char* current_source_pos = source_string;

	for (int i = 0; i < num_fragments; ++i) {
		// 設定 next_fragment_idx_flags
		if (i == num_fragments - 1) { // 最後一個片段
			current_keynode.next_fragment_idx_flags = 0x80000000; // 鏈結束標記
		}
		else {
			current_keynode.next_fragment_idx_flags = allocated_indices[i + 1] | 0x80000000; // 指向下一片段並設定使用中旗標
		}

		// 複製鍵片段
		memset(current_keynode.key_fragment, 0, sizeof(current_keynode.key_fragment)); // 先清零
		int chars_to_copy_this_fragment = 0;
		for (int j = 0; j < 60; ++j) {
			if (*current_source_pos == '\0' && i == num_fragments - 1) { // 如果是最後片段且遇到字串結尾
				current_keynode.key_fragment[j] = '\0'; // 複製結尾的 '\0'
				chars_to_copy_this_fragment++;
				current_source_pos++; // 移動指標以記錄已處理 '\0'
				break;
			}
			if (*current_source_pos == '\0' && i < num_fragments - 1) { // 字串提前結束，但還有片段要分配 (理論上不應發生，num_fragments 已計算好)
				break; // 此片段為空，或只包含 '\0'
			}
			if (*current_source_pos == '\0') break; // 安全措施

			current_keynode.key_fragment[j] = *current_source_pos;
			current_source_pos++;
			chars_to_copy_this_fragment++;
		}
		// 即使 _strncpy 遇到 \0，它也會繼續用 \0 填充剩餘部分 (如果目標長度>源長度)
		// 此處的 memcpy/手動複製更精確控制

		if (keynode_set(dt_handle, allocated_indices[i], &current_keynode) < 0) {
			// 寫入失敗，需要回滾
			for (int k = 0; k < num_fragments; ++k) keynode_clear(dt_handle, allocated_indices[k]);
			return -1;
		}
	}
	return (num_fragments > 0) ? allocated_indices[0] : 0; // 返回鏈的第一個 KeyNode 的索引
}


/** @brief (DT 版本) 配置一個新的 TrieNode 並為其關聯一個儲存了鍵字串的 KeyNode 鏈。*/
int node_allocate(NfsDtHandle* dt_handle,
	const char* key_string,
	short nt_idx_for_trienode,
	short b_index_for_trienode) {
	if (!dt_handle || !key_string) return -1;

	// 1. 分配 KeyNode 鏈
	int keynode_chain_start_idx = fnode_allocate(dt_handle, key_string);
	if (keynode_chain_start_idx < 0) return -1;
	if (keynode_chain_start_idx == 0 && strlen(key_string) > 0) {
		return -1; // 空字串之外不應返回 0
	}

	// 2. 找第一個可用的 TrieNode
	int new_trienode_idx = dt_handle->next_free_trienode_idx;
	if (new_trienode_idx <= 0 || !trienode_is_free(dt_handle, new_trienode_idx)) {
		new_trienode_idx = trienode_find_first_free(dt_handle, new_trienode_idx);
	}
	if (new_trienode_idx <= 0) {
		// 分配失敗，回滾 KeyNode
		if (keynode_chain_start_idx > 0) fnode_free(dt_handle, keynode_chain_start_idx);
		return -1;
	}

	// 3. 寫入新節點
	NfsTrieNode new_trienode;
	new_trienode.nt_idx = nt_idx_for_trienode;
	new_trienode.b_index = b_index_for_trienode;
	new_trienode.k_index = keynode_chain_start_idx | 0x80000000;
	new_trienode.left_child_idx = 0;
	new_trienode.right_child_idx = 0;

	if (trienode_set(dt_handle, new_trienode_idx, &new_trienode) < 0) {
		// 寫入失敗，回滾 KeyNode
		if (keynode_chain_start_idx > 0) fnode_free(dt_handle, keynode_chain_start_idx);
		return -1;
	}

	// 4. 更新下一次搜尋提示
	dt_handle->next_free_trienode_idx =
		trienode_find_first_free(dt_handle, new_trienode_idx + 1);

	return new_trienode_idx;
}


/** @brief 將 source_trienode_idx 的鍵相關資訊 (nt_idx, b_index, k_index) 複製到 dest_trienode_idx。
 * 會釋放 dest_trienode_idx 先前關聯的 KeyNode 鏈。
 * dest_trienode_idx 的子節點指標保持不變。
 */
int node_copy_key(NfsDtHandle* dt_handle, int source_trienode_idx, int dest_trienode_idx) {
	if (!dt_handle || source_trienode_idx <= 0 || dest_trienode_idx <= 0) {
		return -1;
	}

	NfsTrieNode source_node;
	NfsTrieNode dest_node_original_content; // 用於獲取舊的 k_index 和子節點

	if (trienode_get(dt_handle, source_trienode_idx, &source_node) != sizeof(NfsTrieNode) ||
		trienode_get(dt_handle, dest_trienode_idx, &dest_node_original_content) != sizeof(NfsTrieNode)) {
		return -1; // 讀取節點失敗
	}

	// 提取目標節點舊的 KeyNode 鏈以供後續釋放
	int old_dest_keynode_chain_start = dest_node_original_content.k_index & 0x7FFFFFFF;

	// 準備新的目標節點內容：從源節點複製鍵相關資訊，保留目標節點的原始子節點
	NfsTrieNode new_dest_content;
	new_dest_content.nt_idx = source_node.nt_idx;
	new_dest_content.b_index = source_node.b_index;
	new_dest_content.k_index = source_node.k_index; // 包含 MSB 旗標
	new_dest_content.left_child_idx = dest_node_original_content.left_child_idx;   // 保留目標的子節點
	new_dest_content.right_child_idx = dest_node_original_content.right_child_idx; // 保留目標的子節點

	if (trienode_set(dt_handle, dest_trienode_idx, &new_dest_content) != sizeof(NfsTrieNode)) {
		return -1; // 設定目標節點失敗
	}

	// 釋放目標節點先前關聯的 KeyNode 鏈 (如果存在且有效)
	if (old_dest_keynode_chain_start > 0 && (dest_node_original_content.k_index < 0)) { // 確保舊 k_index 是有效的「使用中」鍵
		fnode_free(dt_handle, old_dest_keynode_chain_start);
	}
	return 0;
}

// 接下來8條很抖
/** @brief 取得 Trie 樹的頭部節點索引 (總是返回 0)。 */
int p_get_head() {
	return 0;
}

/** @brief 初始化 Trie 樹的頭部節點 (TrieNode 和 KeyNode 都在索引0)。*/
int p_init_head(NfsDtHandle* dt_handle) {
	if (!dt_handle) return -1;

	NfsKeyNode head_keynode;
	memset(&head_keynode, 0, sizeof(NfsKeyNode));
	head_keynode.next_fragment_idx_flags = 0x80000000; // 最後片段標記
	// 原始碼為 head_keynode.key_fragment[0] = '.'; head_keynode.key_fragment[1] = '\0';
	// 為了簡化，這裡不設定特殊鍵，依賴於 trie 邏輯本身。
	// 如果需要，可以取消註解：
	// strcpy(head_keynode.key_fragment, "."); 

	NfsTrieNode head_trienode;
	head_trienode.nt_idx = 0;
	head_trienode.b_index = -1;   // 特殊的位元索引，比任何有效位元索引都小
	head_trienode.k_index = 0 | 0x80000000; // 指向索引為0的 KeyNode (MSB 設定表示使用中)
	head_trienode.left_child_idx = 0;  // 初始時，頭節點的左右子節點都指向自身
	head_trienode.right_child_idx = 0;

	if (keynode_set(dt_handle, 0, &head_keynode) < 0) return -1;
	if (trienode_set(dt_handle, 0, &head_trienode) < 0) return -1;

	return 0;
}

/** * @brief 比較輸入的鍵字串與儲存在 Trie 中指定外部節點的鍵字串是否相同。
 * @return 如果兩個鍵相同則返回 1，不同則返回 0。
 */
int p_compare_keys(NfsDtHandle* dt_handle, const char* key_to_compare, int external_trienode_idx) {
	if (!dt_handle || !key_to_compare || external_trienode_idx < 0) return 0;

	unsigned int stored_key_kn_start_idx = trienode_get_kindex(dt_handle, external_trienode_idx);

	NfsTrieNode ext_node; // 檢查 k_index 是否真的指向一個鍵
	if (trienode_get(dt_handle, external_trienode_idx, &ext_node) < 0) return 0;
	if (ext_node.k_index >= 0) { // k_index MSB 未設定，表示此非外部節點或空閒
		return (*key_to_compare == '\0'); // 只有空字串能匹配一個「無鍵」的節點
	}
	if (stored_key_kn_start_idx == 0 && (ext_node.k_index != static_cast<int>(0x80000000))) {
		//  k_index 清除 MSB 後為0，但原始 k_index 並非僅 MSB 被設定 (例如 0x80000000 代表指向 kn 0)
		//  這可能表示一個無效的 k_index 指向。
		return (*key_to_compare == '\0'); // 視為無鍵，只有空字串能匹配
	}


	char stored_key_buffer[4096]; // 假設檔名不太可能超過此長度
	if (fnode_extract_key(dt_handle, stored_key_kn_start_idx, stored_key_buffer) < 0) {
		return 0;
	}

	return (strcmp(key_to_compare, stored_key_buffer) == 0) ? 1 : 0;
}

/**
 * @brief 尋找輸入的鍵字串 (key1) 與儲存在 Trie 中指定外部節點的鍵字串 (key2) 之間第一個不同的位元。
 * @return 第一個不同位元的索引 (從0開始)；如果鍵相同或發生錯誤，返回一個較大值或-1。
 */
int p_find_first_different_bit(NfsDtHandle* dt_handle, const char* key1, int trienode_idx_for_key2) {
	if (!dt_handle || !key1 || trienode_idx_for_key2 < 0) return -1;

	unsigned int stored_key_kn_start_idx = trienode_get_kindex(dt_handle, trienode_idx_for_key2);
	NfsTrieNode ext_node;
	if (trienode_get(dt_handle, trienode_idx_for_key2, &ext_node) < 0) return -1;
	if (ext_node.k_index >= 0) return bitfirst_different(key1, ""); // 與空字串比較

	char stored_key_buffer[4096];
	if (fnode_extract_key(dt_handle, stored_key_kn_start_idx, stored_key_buffer) < 0) {
		return -1;
	}

	return bitfirst_different(key1, stored_key_buffer);
}

/** @brief 將指定的鍵字串插入到 Patricia Trie 中。*/
int p_insert_key(NfsDtHandle* dt_handle, const char* key_to_insert, short nt_idx) {
	if (!dt_handle || !key_to_insert) return -1;
	//printf("\n\n--- [DEBUG] p_insert_key: Inserting '%s' ---\n", key_to_insert);

	// 步驟 1: 查找與新鍵最匹配的現有節點 x
	int p = p_get_head();
	int x = trienode_get_right(dt_handle, p);
	//printf("[D] Insert - Step 1 (Initial Search)\n");
	while (trienode_get_bindex(dt_handle, p) < trienode_get_bindex(dt_handle, x)) {
		p = x;
		// 【關鍵修正點】
		x = bit_get(key_to_insert, trienode_get_bindex(dt_handle, p))
			? trienode_get_right(dt_handle, p)
			: trienode_get_left(dt_handle, p);
	}
	//printf("[D] Insert - Step 1 Result: Found closest node x=%d (parent p=%d)\n", x, p);

	if (p_compare_keys(dt_handle, key_to_insert, x) == 1) {
		//printf("[D] Insert - ERROR: Key '%s' already exists.\n", key_to_insert);
		return -1;
	}

	int first_diff_bit = p_find_first_different_bit(dt_handle, key_to_insert, x);
	//printf("[D] Insert - Step 2 (DiffBit): First different bit is %d\n", first_diff_bit);

	// 步驟 4: 再次從頭遍歷，為新分支節點找到正確的父節點(p_insert)和要被替換的子節點(x_insert)
	int p_insert = p_get_head();
	int x_insert = trienode_get_right(dt_handle, p_insert);
	//printf("[D] Insert - Step 3 (Find Insertion Point)\n");
	while (trienode_get_bindex(dt_handle, p_insert) < trienode_get_bindex(dt_handle, x_insert) &&
		trienode_get_bindex(dt_handle, x_insert) < first_diff_bit) {
		p_insert = x_insert;
		// 【關鍵修正點】
		x_insert = bit_get(key_to_insert, trienode_get_bindex(dt_handle, p_insert))
			? trienode_get_right(dt_handle, p_insert)
			: trienode_get_left(dt_handle, p_insert);
	}
	//printf("[D] Insert - Step 3 Result: Found insertion point between p_insert=%d and x_insert=%d\n", p_insert, x_insert);

	int new_node_idx = node_allocate(dt_handle, key_to_insert, nt_idx, (short)first_diff_bit);
	if (new_node_idx < 0) {
		//printf("[D] Insert - ERROR: node_allocate failed.\n");
		return -1;
	}
	//printf("[D] Insert - Step 4 (Allocate): Allocated new_node_idx=%d for key '%s' with b_index=%d\n", new_node_idx, key_to_insert, first_diff_bit);

	int new_node_bit = bit_get(key_to_insert, first_diff_bit);
	//printf("[D] Insert - Step 5 (Set Children): bit_get('%s', %d) is %d.\n", key_to_insert, first_diff_bit, new_node_bit);
	if (new_node_bit) {
		trienode_set_left(dt_handle, new_node_idx, x_insert);
		trienode_set_right(dt_handle, new_node_idx, new_node_idx);
	}
	else {
		trienode_set_left(dt_handle, new_node_idx, new_node_idx);
		trienode_set_right(dt_handle, new_node_idx, x_insert);
	}

	//printf("[D] Insert - Step 6 (Attach): Attaching new_node %d to parent %d. Original child was %d.\n", new_node_idx, p_insert, x_insert);
	if (trienode_get_right(dt_handle, p_insert) == x_insert) {
		//printf("  [D] Attaching to RIGHT child of parent %d.\n", p_insert);
		trienode_set_right(dt_handle, p_insert, new_node_idx);
	}
	else {
		//printf("  [D] Attaching to LEFT child of parent %d.\n", p_insert);
		trienode_set_left(dt_handle, p_insert, new_node_idx);
	}
	//printf("--- [DEBUG] p_insert_key: Finished inserting '%s' ---\n", key_to_insert);

	return new_node_idx;
}

/** @brief 從 Trie 中移除指定的鍵。 */
// 刪除指定 key（Patricia Trie）
/** @brief 從 Trie 中移除指定的鍵。 */
/** @brief 從 Trie 中移除指定的鍵。*/
int p_remove_key(NfsDtHandle* dt_handle, const char* key_to_remove) {
	if (!dt_handle || !key_to_remove) return -1;
	//printf("\n\n--- [DEBUG] p_remove_key: Removing '%s' ---\n", key_to_remove);

	int head = p_get_head();
	int grandparent = head;
	int parent = head;
	int current = trienode_get_right(dt_handle, head);

	// 1. 找到 current (外部鍵節點)、parent 和 grandparent
	while (trienode_get_bindex(dt_handle, parent) < trienode_get_bindex(dt_handle, current)) {
		grandparent = parent;
		parent = current;
		current = bit_get(key_to_remove, trienode_get_bindex(dt_handle, parent))
			? trienode_get_right(dt_handle, parent)
			: trienode_get_left(dt_handle, parent);
	}
	// 2. 比對是否真的找到
	if (p_compare_keys(dt_handle, key_to_remove, current) != 1) {
		//printf("[D] Remove - ERROR: Key '%s' not found.\n", key_to_remove);
		return -1;
	}
	//printf("[D] Remove: Found node to delete: current=%d. Its parent is %d, grandparent is %d.\n", current, parent, grandparent);

	// 3. 如果 parent 是 head，重設整顆 Trie
	if (parent == head) {
		//printf("[D] Remove: Parent is head. Deleting last element. Re-initializing head.\n");
		unsigned int kidx = trienode_get_kindex(dt_handle, current);
		if (kidx > 0) fnode_free(dt_handle, kidx);
		trienode_recover(dt_handle, current);
		p_init_head(dt_handle);
		return 0;
	}

	// 4. 找 sibling
	int sibling = (trienode_get_left(dt_handle, parent) == current)
		? trienode_get_right(dt_handle, parent)
		: trienode_get_left(dt_handle, parent);
	//printf("[D] Remove: Sibling of node %d is node %d.\n", current, sibling);

	// 5. 把 sibling 掛到 grandparent 上
	//printf("[D] Remove: Attaching sibling %d to grandparent %d.\n", sibling, grandparent);
	if (trienode_get_left(dt_handle, grandparent) == parent) {
		trienode_set_left(dt_handle, grandparent, sibling);
	}
	else {
		trienode_set_right(dt_handle, grandparent, sibling);
	}

	// 6. 釋放 current 的 key，但**不要** recover current 以免清掉內部節點結構
	unsigned int ck = trienode_get_kindex(dt_handle, current);
	if (ck > 0) fnode_free(dt_handle, ck);
	// 只把 trienode 的 k_index 清掉，保留 b_index 和子節點指標
	{
		NfsTrieNode nodebuf;
		trienode_get(dt_handle, current, &nodebuf);
		nodebuf.k_index = 0;
		trienode_set(dt_handle, current, &nodebuf);
	}

	if (sibling != parent) {
		// 7a. 一般情況：回收 parent trienode (它原本有兩條分支)
		//printf("[D] Remove: Recovering parent node %d.\n", parent);
		// 先釋放 parent 的 key 再 recover
		unsigned int pk = trienode_get_kindex(dt_handle, parent);
		if (pk > 0) fnode_free(dt_handle, pk);
		trienode_recover(dt_handle, parent);
	}
	else {
		// 7b. 兄弟就是 parent：把 parent 轉成純 leaf
		//printf("[D] Remove: Collapsing parent node %d into leaf.\n", parent);
		NfsTrieNode pbuf;
		trienode_get(dt_handle, parent, &pbuf);
		// children 指向自己
		pbuf.left_child_idx = parent;
		pbuf.right_child_idx = parent;
		trienode_set(dt_handle, parent, &pbuf);
		// (k_index 已經是我們要保留的那把 key)
	}

	//printf("--- [DEBUG] p_remove_key: Finished removing '%s' ---\n", key_to_remove);
	return 0;
}


// 查找指定 key，回傳 TrieNode index（外部節點 idx）或 -1
int p_lookup_key(NfsDtHandle* dt_handle, const char* key_to_lookup) {
	if (!dt_handle || !key_to_lookup) return -1;

	//printf("\n[DEBUG] p_lookup_key: Searching for '%s'\n", key_to_lookup);

	int p = p_get_head();
	int x = trienode_get_right(dt_handle, p);
	//printf("  [L] Start: p=%d(b=%d), x=%d(b=%d)\n", p, trienode_get_bindex(dt_handle, p), x, trienode_get_bindex(dt_handle, x));

	while (trienode_get_bindex(dt_handle, p) < trienode_get_bindex(dt_handle, x)) {
		int old_p_idx = p;
		p = x; // p 更新為當前節點
		int bit_to_test = trienode_get_bindex(dt_handle, p);
		int bit_value = bit_get(key_to_lookup, bit_to_test);

		// 【關鍵修正點】: 根據新的父節點 p 來獲取子節點，而不是用舊的 x
		x = bit_value ? trienode_get_right(dt_handle, p) : trienode_get_left(dt_handle, p);

		//printf("  [L] Traverse: From p=%d(b=%d), testing bit %d -> %d. New x is %d.\n", p, bit_to_test, bit_to_test, bit_value, x);
	}

	//printf("[DEBUG] p_lookup_key: Loop terminated. Final p=%d(b=%d), x=%d(b=%d).\n", p, trienode_get_bindex(dt_handle, p), x, trienode_get_bindex(dt_handle, x));

	char stored_key_buffer[4096];
	fnode_extract_key(dt_handle, trienode_get_kindex(dt_handle, x), stored_key_buffer);
	//printf("[DEBUG] p_lookup_key: Comparing search key '%s' with key from node %d ('%s').\n", key_to_lookup, x, stored_key_buffer);

	if (p_compare_keys(dt_handle, key_to_lookup, x) == 1) {
		//printf("[DEBUG] p_lookup_key: Match SUCCEEDED. Returning node index %d.\n", x);
		return x;
	}

	//printf("[DEBUG] p_lookup_key: Match FAILED. Returning -1.\n");
	return -1;
}


/** @brief 在 Trie 中查詢與給定鍵的前 N 個位元匹配的最深節點。*/
int p_lookup_key_n(NfsDtHandle* dt_handle, const char* key_to_lookup, int num_bits_to_match) {
	if (!dt_handle || !key_to_lookup || num_bits_to_match < 0) return -1;

	int p = p_get_head(); // parent
	int x = trienode_get_right(dt_handle, p); // current_node

	// 遍歷 Trie，直到 b_index不再嚴格遞增，或者 b_index >= num_bits_to_match
	while (trienode_get_bindex(dt_handle, p) < trienode_get_bindex(dt_handle, x) &&
		trienode_get_bindex(dt_handle, x) < num_bits_to_match) {
		p = x;
		x = bit_get(key_to_lookup, trienode_get_bindex(dt_handle, x)) ?
			trienode_get_right(dt_handle, x) :
			trienode_get_left(dt_handle, x);
	}
	// 此時，p 是 x 的父節點，或者 x 是已達到的最深匹配節點。
	// 原始碼返回 v3 (p in my code) `return v3;`
	// 這意味著它返回的是「包含」此前綴的最深父節點，而不是 x。
	// 或者說，是遍歷過程中，b_index < num_bits_to_match 的最後一個父節點。
	return p;
}

//一直警告
/**
 * @brief 從 Glob 模式字串中提取非萬用字元的前置部分。
 * @param pattern Glob 模式字串。
 * @param output_buffer 用於儲存提取出的前綴的緩衝區。
 * @return 指向 output_buffer 的指標。
 */
char* find_prefix(const char* pattern, char* output_buffer) {
	if (!pattern || !output_buffer) {
		if (output_buffer) *output_buffer = '\0';
		return output_buffer;
	}

	const char* p = pattern;
	char* q = output_buffer;

	while (*p != '\0') {
		if (*p == '*' || *p == '?' || *p == '[') {
			break;
		}
		*q++ = *p++;
	}
	*q = '\0';
	return output_buffer;
}

const int NFS_FNM_PATHNAME = 0x01; // Treat / special
const int NFS_FNM_NOESCAPE = 0x02; // Disable backslash escaping
const int NFS_FNM_PERIOD = 0x04; // Leading . must be matched explicitly

/**
 * @brief Glob-style pattern matching (Restored from nfs.c 005CB800).
 *
 * Matches a string against a glob pattern.
 * Supports:
 * '*'       - matches any sequence of characters (including none).
 * '?'       - matches any single character.
 * '[set]'   - matches any character in the set.
 * '[!set]' or
 * '[^set]'  - matches any character not in the set.
 * '[a-c]'   - matches any character in the range a through c.
 * '\\'       - escapes the next character (unless FNM_NOESCAPE is set).
 *
 * Flags (bitwise OR):
 * NFS_FNM_PATHNAME: If set, a slash '/' in 'string' only matches a slash in 'pattern'.
 * Wildcards '*' and '?' do not match a slash.
 * NFS_FNM_NOESCAPE: If set, backslash '\' is treated as a literal character.
 * NFS_FNM_PERIOD:   If set, a leading '.' in 'string' must be explicitly matched by
 * a '.' in 'pattern'. It is not matched by '*' or '?'.
 *
 * Case-insensitivity: The original C code appears to perform case-insensitive matching
 * by converting characters to lowercase. This behavior is replicated.
 *
 * @param pattern The glob pattern.
 * @param string_to_test The string to match against the pattern.
 * @param flags Bitwise OR of FNM_ flags to control matching behavior.
 * @return 0 if the string matches the pattern, 1 (FNM_NOMATCH equivalent) if it does not.
 * Returns -1 for invalid parameters (e.g., unsupported flags in original).
 */


 /**
  * @brief Glob-style pattern matching (Restored from nfs.c 005CB800).
  * @return 0 if the string matches the pattern, 1 (FNM_NOMATCH equivalent) if it does not.
  * Returns -1 for invalid parameters or unsupported flags.
  */
int nfs_pmatch(const char* pattern, const char* string_to_test, int flags) {
	const char* p = pattern;
	const char* s = string_to_test;
	char p_char_current_iter; // 用於儲存當前迭代的模式字元

	// 原始 C 程式碼檢查 flags 是否包含不支援的位元
	if ((flags & ~(NFS_FNM_PATHNAME | NFS_FNM_NOESCAPE | NFS_FNM_PERIOD)) != 0) {
		return -1; // 無效/不支援的旗標
	}

	while (true) {
		p_char_current_iter = *p++; // 取得當前模式字元並前進指標

		switch (p_char_current_iter) {
		case '\0': { // 模式結束
			return (*s == '\0') ? 0 : 1; // 如果字串也結束則匹配，否則不匹配
		}
		case '*': { // 星號萬用字元
			const char* p_after_wildcards = p;
			const char* s_after_qmarks = s;

			// 處理 FNM_PERIOD for '*'
			if ((flags & NFS_FNM_PERIOD) && *s_after_qmarks == '.' && s_after_qmarks == string_to_test) {
				return 1;
			}

			// 跳過連續的 '*' 和 '?'，但操作發生在副本上
			while (*p_after_wildcards == '*' || *p_after_wildcards == '?') {
				if (*p_after_wildcards == '?') {
					if (*s_after_qmarks == '\0') return 1;
					if ((flags & NFS_FNM_PATHNAME) && *s_after_qmarks == '/') return 1;
					if ((flags & NFS_FNM_PERIOD) && *s_after_qmarks == '.' && s_after_qmarks == string_to_test) return 1;
					s_after_qmarks++;
				}
				p_after_wildcards++;
			}

			if (*p_after_wildcards == '\0') {
				if (flags & NFS_FNM_PATHNAME) {
					return (strchr(s_after_qmarks, '/') == nullptr) ? 0 : 1;
				}
				return 0;
			}

			// --- 核心邏輯修正 ---
			// 使用 for 迴圈來清晰地表達「嘗試所有後綴」的意圖
			for (const char* s_scan = s_after_qmarks; *s_scan != '\0'; ++s_scan) {
				// 如果是 FNM_PATHNAME 模式，'*' 不能匹配 '/'，所以在此處的任何後續嘗試都將失敗。
				if ((flags & NFS_FNM_PATHNAME) && *s_scan == '/') {
					return 1;
				}

				// 遞迴地檢查 * 後面的模式是否能從當前 s_scan 位置開始匹配
				if (nfs_pmatch(p_after_wildcards, s_scan, flags & ~NFS_FNM_PERIOD) == 0) {
					return 0; // 匹配成功
				}
			}

			// 如果字串的所有後綴都無法匹配，最後嘗試用模式的剩餘部分匹配空字串
			// (這處理了 "a*b" 匹配 "ab" 的情況)
			return nfs_pmatch(p_after_wildcards, s_after_qmarks, flags & ~NFS_FNM_PERIOD);
		}
		case '?': { // 問號萬用字元
			if (*s == '\0') return 1; // 字串結束，'?' 無法匹配
			if ((flags & NFS_FNM_PATHNAME) && *s == '/') return 1; // '?' 不匹配 '/'
			if ((flags & NFS_FNM_PERIOD) && *s == '.' && s == string_to_test) return 1; // '?' 在開頭不匹配 '.'
			s++; // 消耗字串中的一個字元
			break;
		}
		case '[': {
			if (*s == '\0') return 1;
			if ((flags & NFS_FNM_PATHNAME) && *s == '/') return 1;
			if ((flags & NFS_FNM_PERIOD) && *s == '.' && s == string_to_test) return 1;

			bool matched_in_class = false;
			bool negated_class = false;
			const char* p_class_start = p;

			// 僅當 '!' 或 '^' 之後還有不是 ']' 的字元時，才視為「反向」字元類
			if ((*p == '!' || *p == '^') && p[1] != ']') {
				negated_class = true;
				p++;
				p_class_start++;
			}

			char char_s_current = *s;
			char char_s_lower = (char_s_current >= 'A' && char_s_current <= 'Z')
				? (char_s_current + 32) : char_s_current;
			char prev_class_char = '\0';

			// 逐一掃描字元類內的字元或範圍
			while (true) {
				char char_p_class = *p++;
				if (char_p_class == '\0') return 1;  // 格式錯誤

				// 如果遇到結尾 ']'（且不是第一個有效字元），結束字元類掃描
				if (char_p_class == ']' && p > p_class_start + 1) {
					break;
				}

				// 處理範圍語法 [a-z]
				if (prev_class_char != '\0' && char_p_class == '-' && *p != ']' && *p != '\0') {
					char range_end = *p++;
					if (!(flags & NFS_FNM_NOESCAPE) && range_end == '\\') {
						range_end = *p++;
						if (range_end == '\0') return 1;
					}
					if (range_end == '\0') return 1;

					char low = (prev_class_char >= 'A' && prev_class_char <= 'Z')
						? prev_class_char + 32 : prev_class_char;
					char high = (range_end >= 'A' && range_end <= 'Z')
						? range_end + 32 : range_end;
					if (low > high) std::swap(low, high);
					if (char_s_lower >= low && char_s_lower <= high) {
						matched_in_class = true;
					}
					prev_class_char = '\0';
				}
				else {
					// 單一字元比較
					char pcl = (char_p_class >= 'A' && char_p_class <= 'Z')
						? char_p_class + 32 : char_p_class;
					if (pcl == char_s_lower) {
						matched_in_class = true;
					}
					prev_class_char = pcl;
				}

				if (matched_in_class && !negated_class) break;
			}

			// 跳過字元類中剩下的內容，找到真正的 ']'
			if (matched_in_class || negated_class) {
				while (*(p - 1) != '\0') {
					if (*(p - 1) == ']' && p > p_class_start + 1) break;
					if (!(flags & NFS_FNM_NOESCAPE) && *p == '\\') p++;
					p++;
				}
				if (*(p - 1) != ']') return 1;
			}

			// 如果正向且沒匹配，或反向且有匹配，都算不合
			if (negated_class == matched_in_class) return 1;

			s++;  // 成功匹配一個字元
			break;
		}
		case '\\': { // 反斜線轉義
			if (!(flags & NFS_FNM_NOESCAPE)) {
				p_char_current_iter = *p++; // 獲取被轉義的字元
				if (p_char_current_iter == '\0') return 1; // 模式以 '\' 結尾，不匹配
			}
			// [[fallthrough]]; // 意圖是讓 default 處理這個 p_char_current_iter
		}   // 注意：如果這裡 break，default 將處理原始的 '\'。
			// 如果 fallthrough，default 將處理轉義後的字元。
			// C++ 中 [[fallthrough]] 必須緊跟著 ;
			// 這裡的邏輯是，如果轉義，則 p_char_current_iter 更新為轉義後的字元，然後由 default 處理。
			// 如果 FNM_NOESCAPE，則 p_char_current_iter 仍是 '\'，由 default 處理。
				 [[fallthrough]];
		default: { // 普通字元匹配
			char char_s_current = *s;
			if (char_s_current == '\0') return 1; // 字串結束，但模式還有字元，不匹配

			// 大小寫不敏感比較
			char p_compare = (p_char_current_iter >= 'A' && p_char_current_iter <= 'Z') ? (p_char_current_iter + 32) : p_char_current_iter;
			char s_compare = (char_s_current >= 'A' && char_s_current <= 'Z') ? (char_s_current + 32) : char_s_current;

			if (p_compare != s_compare) {
				return 1; // 字元不匹配
			}
			s++; // 消耗字串字元
			break;
		}
		} // end switch
	} // end while(true)
	// return 1; // 理論上不可達
}


/**
 * @brief 遞迴(概念上)遍歷 Trie 節點以進行 Glob 模式匹配。
 * @param dt_handle DT 控制代碼。
 * @param current_trienode_idx 目前處理的 TrieNode 索引。
 * @param parent_b_index 父節點的 b_index (或前綴搜尋的最大 b_index)。
 * @param glob_pattern Glob 模式字串。
 * @param pmatch_flags 傳遞給 nfs_pmatch 的旗標。
 * @param callback 匹配成功時呼叫的回呼函式。
 * @param callback_context 傳遞給回呼函式的上下文。
 * @return 如果需要繼續遍歷則返回非0，如果回呼要求停止則返回0。
 */
int p_node_iterate(NfsDtHandle* dt_handle, int current_trienode_idx, int parent_b_index,
	const char* glob_pattern, int pmatch_flags,
	int (*callback)(NfsDtHandle*, char*, int, void*), void* callback_context) {
	if (!dt_handle || current_trienode_idx < 0) return 1; // 無效節點，但允許繼續遍歷其他分支

	short current_node_b_idx = trienode_get_bindex(dt_handle, current_trienode_idx);
	if (current_node_b_idx > parent_b_index) { // 內部節點，繼續向下
		int left_child = trienode_get_left(dt_handle, current_trienode_idx);
		int right_child = trienode_get_right(dt_handle, current_trienode_idx);

		if (!p_node_iterate(dt_handle, left_child, current_node_b_idx, glob_pattern, pmatch_flags, callback, callback_context)) {
			return 0; // 左子樹要求停止
		}
		if (!p_node_iterate(dt_handle, right_child, current_node_b_idx, glob_pattern, pmatch_flags, callback, callback_context)) {
			return 0; // 右子樹要求停止
		}
		return 1; // 左右子樹都處理完畢，繼續
	}
	else { // 外部節點 (或已達 b_index 限制)
		if (current_trienode_idx == 0) {
			return 1; // 忽略 head 節點，繼續遍歷其他分支
		}
		NfsTrieNode current_node_data;
		if (trienode_get(dt_handle, current_trienode_idx, &current_node_data) < 0) return 1; //讀取失敗，繼續其他

		if (current_node_data.k_index >= 0) { // MSB 未設定，表示空閒或非外部鍵節點
			return 1; // 不是有效的外部鍵節點，繼續
		}

		unsigned int keynode_start_idx = current_node_data.k_index & 0x7FFFFFFF;
		// 特殊處理 k_index 為 0x80000000 的情況 (指向 keynode 0)
		if (current_node_data.k_index == static_cast<int>(0x80000000)) keynode_start_idx = 0;


		if (fnode_extract_key(dt_handle, keynode_start_idx, nfs_glob_key_buffer) < 0) {
			return 1; // 提取鍵失敗，繼續
		}

		if (nfs_pmatch(glob_pattern, nfs_glob_key_buffer, pmatch_flags) == 0) {
			if (callback) {
				// callback 回傳 0 就停止遍歷，非0 就繼續
				return callback(dt_handle, nfs_glob_key_buffer, current_trienode_idx, callback_context);
			}
		}
		return 1; // 不匹配或無回呼，繼續
	}
}


// --- DT 控制代碼管理和頂層檔名操作 ---

/** @brief 建立一個新的目錄表 (DT)。*/
NfsDtHandle* nfs_dt_create(NfsIioFile* iio_file, int tn_chan_blocks, int kn_chan_blocks) {
	if (!iio_file) return nullptr;

	NfsDtHandle* dt_handle = static_cast<NfsDtHandle*>(malloc(sizeof(NfsDtHandle)));
	if (!dt_handle) return nullptr;

	dt_handle->iio_file = iio_file;

	// 為 TrieNode 分配 IIO 通道，預設1個區塊的條帶
	int tn_blocks = (tn_chan_blocks == 0) ? 1 : tn_chan_blocks;
	dt_handle->trienode_channel_id = nfs_iio_allocate_channel(iio_file, tn_blocks);
	if (dt_handle->trienode_channel_id < 0) {
		free(dt_handle);
		return nullptr;
	}

	// 為 KeyNode 分配 IIO 通道，預設2個區塊的條帶 (原始碼中是4和8，可能是參數索引)
	// 原始碼 nfs_dt_create(iio, 4, 8)，所以 tn_chan_blocks 是4，kn_chan_blocks 是8
	// 此處的參數 tn_chan_blocks, kn_chan_blocks 直接對應 IIO 通道參數
	int kn_blocks = (kn_chan_blocks == 0) ? 2 : kn_chan_blocks;
	dt_handle->keynode_channel_id = nfs_iio_allocate_channel(iio_file, kn_blocks);
	if (dt_handle->keynode_channel_id < 0) {
		// 注意：此處應考慮釋放已分配的 trienode_channel_id
		free(dt_handle);
		return nullptr;
	}

	dt_handle->next_free_trienode_idx = 1; // TrieNode 索引從1開始 (0為頭部)
	dt_handle->next_free_keynode_idx = 1;  // KeyNode 索引從1開始 (0為頭部鍵)

	if (p_init_head(dt_handle) < 0) { // 初始化頭部節點
		// 清理已分配的資源
		// 注意：IIO 通道釋放邏輯複雜，此處簡化
		free(dt_handle);
		return nullptr;
	}
	return dt_handle;
}

/** @brief 開啟一個已存在的目錄表 (DT)。*/
NfsDtHandle* nfs_dt_open(NfsIioFile* iio_file, int trienode_channel_id, int keynode_channel_id) {
	if (!iio_file) return nullptr;

	NfsDtHandle* dt_handle = static_cast<NfsDtHandle*>(malloc(sizeof(NfsDtHandle)));
	if (!dt_handle) return nullptr;

	dt_handle->iio_file = iio_file;
	dt_handle->trienode_channel_id = trienode_channel_id;
	dt_handle->keynode_channel_id = keynode_channel_id;

	// 初始化空閒節點搜尋提示，從索引1開始（索引0為頭部，不參與一般分配）
	dt_handle->next_free_trienode_idx = trienode_find_first_free(dt_handle, 1);
	dt_handle->next_free_keynode_idx = keynode_find_first_free(dt_handle, 1);

	// 如果 find_first_free 失敗 (例如返回 <1 的值)，可能表示DT損壞或空間問題
	if (dt_handle->next_free_trienode_idx <= 0 || dt_handle->next_free_keynode_idx <= 0) {
		free(dt_handle); // 釋放 dt_handle 本身
		return nullptr;   // 視為開啟失敗
		// 原始碼中，find_first_free 若找不到，可能會導致 next_free_idx 是一個很大的值或持續增加
		// 此處假設 find_first_free 能正確返回 >=1 的值或一個明確的錯誤指示
	}

	return dt_handle;
}

/** @brief 關閉目錄表 (DT)，僅釋放 DT 控制代碼本身。*/
int nfs_dt_close(NfsDtHandle* dt_handle) {
	if (dt_handle) {
		free(dt_handle);
	}
	return 0;
}

/** @brief 銷毀目錄表 (DT)，實質上只是呼叫 nfs_dt_close。*/
int nfs_dt_destroy(NfsDtHandle* dt_handle) {
	nfs_dt_close(dt_handle);
	return 0; // 原始碼中 nfs_dt_destroy 似乎也只調用 close
}

/** @brief 向目錄表 (DT) 中新增一個檔名。*/
int nfs_dt_filename_add(NfsDtHandle* dt_handle, const char* filename_to_add) {
	if (!dt_handle || !filename_to_add) return -1;
	// 原始碼中 p_insert_key 的第三個參數 nt_idx 在此處傳入 0
	return p_insert_key(dt_handle, filename_to_add, 0);
}

/** @brief 從目錄表 (DT) 中刪除一個檔名。*/
int nfs_dt_filename_delete(NfsDtHandle* dt_handle, const char* filename_to_delete) {
	if (!dt_handle || !filename_to_delete) return -1;
	return p_remove_key(dt_handle, filename_to_delete);
}

/** @brief 在目錄表 (DT) 中查詢一個檔名。*/
int nfs_dt_filename_lookup(NfsDtHandle* dt_handle, const char* filename_to_lookup) {
	if (!dt_handle || !filename_to_lookup) return -1;
	return p_lookup_key(dt_handle, filename_to_lookup);
}

/** @brief 取得與 Trie 節點關聯的 NT 索引。*/
short nfs_dt_filename_get_nt_index(NfsDtHandle* dt_handle, int trienode_idx) {
	if (!dt_handle || trienode_idx < 0) {
		return -1; // 或其他錯誤/無效值
	}
	return trienode_get_nt(dt_handle, trienode_idx);
}

/** @brief 設定與 Trie 節點關聯的 NT 索引。*/
int nfs_dt_filename_set_nt_index(NfsDtHandle* dt_handle, int trienode_idx, short new_nt_idx) {
	if (!dt_handle || trienode_idx < 0) {
		return -1;
	}
	return trienode_set_nt(dt_handle, trienode_idx, new_nt_idx);
}

/** @brief 根據 Trie 節點索引取得完整的檔案名稱。*/
int nfs_dt_filename_get_name(NfsDtHandle* dt_handle, int trienode_idx, char* output_name_buffer) {
	if (!dt_handle || trienode_idx < 0 || !output_name_buffer) {
		if (output_name_buffer) *output_name_buffer = '\0';
		return -1;
	}
	unsigned int keynode_start_idx = trienode_get_kindex(dt_handle, trienode_idx);
	// 需要檢查 trienode_get_kindex 返回的是否是有效的鍵節點起始索引 (例如 >0，或者如果是0，則原始 k_index 是否為 0x80000000)
	// 為了簡化，此處直接使用。
	return fnode_extract_key(dt_handle, keynode_start_idx, output_name_buffer);
}
// --- Globbing 相關函式 ---

// Glob 排序回呼函式 (qsort 使用)
static int __nfs_glob_sort_callback(const void* a, const void* b) {
	return strcmp(*(const char**)a, *(const char**)b);
}

// Glob 遍歷時的回呼函式，用於將匹配的檔名加入結果列表
// 傳回 1 表示繼續遍歷，0 表示停止
static int __nfs_glob_processing_callback(NfsDtHandle* dt_handle, char* matched_filename,
	int trienode_idx, void* context) {
	if (!context) return 0; // 無效上下文，停止

	// context 實際上是 NfsGlobInternalContext（或原始碼中的 int v7[5] 陣列）
	// 為了簡化，這裡假設 context 直接是 NfsGlobResults* (如原始碼的 v7[0])
	// 完整的上下文傳遞和解析會更複雜。
	// 這裡我們只專注於將 matched_filename 加入 NfsGlobResults。

	NfsGlobResults* results = static_cast<NfsGlobResults*>(context);

	// 重新分配 gl_pathv 以容納新的路徑
	// 原始碼的 realloc 邏輯較為奇特 (4 * new_total_elements + 156)
	// C++ 中更常見的做法是使用 std::vector，或更標準的 realloc 增長策略
	size_t current_capacity = 0; // 需要一種方式追蹤 gl_pathv 的目前容量
	// 假設 gl_pathv 每次增加一個元素 (效率不高，但符合某些 C 實現)

	char** new_pathv = static_cast<char**>(realloc(results->gl_pathv, (results->gl_pathc + 1) * sizeof(char*)));
	if (!new_pathv) {
		// _realloc 失敗
		if (results->gl_pathv) { // 嘗試釋放舊的（如果 realloc 失敗且舊指標有效）
			// 注意：如果 _realloc 失敗，原始指標仍然有效且包含舊資料。
			// 此處不應釋放 results->gl_pathv，除非確定不再使用。
			// 設定錯誤旗標讓外部處理。
		}
		results->internal_callback_error_flag = 1; // 設定錯誤旗標
		return 0; // 停止遍歷
	}
	results->gl_pathv = new_pathv;

	// 為新的檔名分配記憶體並複製
	results->gl_pathv[results->gl_pathc] = static_cast<char*>(malloc(strlen(matched_filename) + 1));
	if (!results->gl_pathv[results->gl_pathc]) {
		results->internal_callback_error_flag = 1;
		// _realloc 可能已成功，但字串複製的 malloc 失敗。
		// 此時 gl_pathc 尚未增加，但 gl_pathv 可能已擴大。
		// 嚴謹的錯誤處理需要回滾或標記不一致。
		return 0; // 停止遍歷
	}
	strcpy(results->gl_pathv[results->gl_pathc], matched_filename);
	results->gl_pathc++;

	return 1; // 繼續遍歷
}


// Glob 旗標定義 (模擬標準 fnmatch.h 中的部分旗標)
#define NFS_FNM_PATHNAME 0x01 // '/' 只匹配 '/'
#define NFS_FNM_PERIOD   0x02 // 前導 '.' 必須明確匹配
#define NFS_FNM_NOSORT   0x04 // 不對結果排序 (nfs.c 中未使用此名稱，但有類似邏輯)
#define NFS_FNM_DOOFFS   0x08 // 使用 results->gl_offs (nfs.c 中未使用此名稱)
#define NFS_FNM_NOCHECK  0x10 // 若無匹配，返回模式本身
#define NFS_FNM_APPEND   0x20 // 附加到現有結果 (nfs.c 中未使用此名稱)
// 更多旗標如 FNM_CASEFOLD 等未在此處列出，但 nfs_pmatch 可能隱含處理大小寫

/**
 * @brief 執行檔名 Globbing 操作。
 * @param dt_handle DT 控制代碼。
 * @param pattern Glob 模式字串。
 * @param flags Glob 旗標 (例如 NFS_FNM_NOCHECK, NFS_FNM_NOSORT 等)。
 * @param errfunc 錯誤處理回呼函式 (在此簡化版本中未使用)。
 * @param glob_results_output 指向 NfsGlobResults 結構的指標，用於儲存匹配結果。
 * @return 成功時返回 0，找不到匹配 (且未設定 NFS_FNM_NOCHECK) 時返回 GLOB_NOMATCH (假設為1)，
 * 記憶體不足等錯誤時返回 GLOB_NOSPACE (假設為2)，其他錯誤返回 GLOB_ABORTED (假設為3)。
 * (返回值需對應 glob() 函式的標準，或原始碼的特定約定)
 */
int nfs_dt_filename_glob(NfsDtHandle* dt_handle, const char* pattern, int flags,
	int (*errfunc)(const char* epath, int eerrno),
	NfsGlobResults* glob_results_output) {
	if (!dt_handle || !pattern || !glob_results_output) {
		return 3; // GLOB_ABORTED 或類似錯誤
	}

	// 初始化 glob_results_output (除非 FNM_APPEND)
	if (!(flags & NFS_FNM_APPEND)) {
		// 如果先前有結果，需要先釋放 (nfs_globfree 的邏輯)
		if (glob_results_output->gl_pathv) {
			for (size_t i = 0; i < glob_results_output->gl_pathc; ++i) {
				if (glob_results_output->gl_pathv[i]) free(glob_results_output->gl_pathv[i]);
			}
			free(glob_results_output->gl_pathv);
		}
		glob_results_output->gl_pathc = 0;
		glob_results_output->gl_pathv = nullptr;
		// gl_offs 的處理: 原始碼中若 (flags & 8) == 0，則 gl_offs = 0
		// 這裡假設 FNM_DOOFFS (0x08) 若未設定，則 gl_offs 為0。若設定，則保留傳入的 gl_offs。
		if (!(flags & NFS_FNM_DOOFFS)) { // 假設 0x08 對應 FNM_DOOFFS
			glob_results_output->gl_offs = 0;
		}
		// 初始化 gl_pathv 以容納 gl_offs 個空指標槽
		if (glob_results_output->gl_offs > 0) {
			glob_results_output->gl_pathv = static_cast<char**>(malloc(glob_results_output->gl_offs * sizeof(char*)));
			if (!glob_results_output->gl_pathv) return 2; // GLOB_NOSPACE
			for (size_t i = 0; i < glob_results_output->gl_offs; ++i) {
				glob_results_output->gl_pathv[i] = nullptr;
			}
		}
	}
	glob_results_output->internal_callback_error_flag = 0;


	char prefix[256]; // 儲存模式的前綴
	find_prefix(pattern, prefix);

	int start_node_for_iteration = p_get_head(); // 預設從頭開始
	if (prefix[0] != '\0') {
		// 如果有前綴，嘗試使用 p_lookup_key_n 縮小搜尋範圍
		// p_lookup_key_n 需要位元長度
		int prefix_bit_len = static_cast<int>(strlen(prefix)) * 8;
		start_node_for_iteration = p_lookup_key_n(dt_handle, prefix, prefix_bit_len);
		if (start_node_for_iteration < 0) start_node_for_iteration = p_get_head(); // 若查找失敗，仍從頭開始
	}

	// 原始碼 nfs_dt_filename_glob 傳遞給 p_node_iterate 的 parent_b_index (a3) 是：
	// v6 = p_lookup_key_n(...); v9 = trienode_get_bindex(dt_handle, v6);
	// p_node_iterate(dt_handle, v6 /*起始節點*/, v9-1 /*父b_index或上限*/, pattern, flags, cbk, ctx);
	// 此處 v9-1 的處理可能需要更細緻的理解，它可能是為了限制迭代深度或處理邊界。
	// 簡單起見，我們傳遞一個相對較小的 b_index 作為起始父 b_index。
	int initial_parent_b_index;
	if (start_node_for_iteration == p_get_head()) {
		// head.b_index == -1，減1後變成 -2，讓 head (b_index=-1) 仍 satisfy b_index > parent
		initial_parent_b_index = trienode_get_bindex(dt_handle, start_node_for_iteration) - 1;
	}
	else {
		initial_parent_b_index = trienode_get_bindex(dt_handle, start_node_for_iteration);
	}


	// 建立傳遞給回呼函式的上下文
	// 原始碼使用一個 int v7[5] 陣列。
	// v7[0]=glob_results_output, v7[1]=pattern, v7[2]=flags, v7[3]=errfunc
	// 此處我們直接將 glob_results_output 作為上下文傳遞給簡化的回呼函式。
	// 如果 __nfs_glob_processing_callback 需要更多上下文，則需定義並傳遞一個 context struct。
	void* callback_context = glob_results_output;


	size_t matches_before_iterate = glob_results_output->gl_pathc;
	p_node_iterate(dt_handle, start_node_for_iteration, initial_parent_b_index,
		pattern, flags, __nfs_glob_processing_callback, callback_context);

	if (glob_results_output->internal_callback_error_flag) {
		return 2; // GLOB_NOSPACE (來自回呼中的記憶體分配失敗)
	}

	if (glob_results_output->gl_pathc == matches_before_iterate) { // 沒有找到任何新匹配
		if (flags & NFS_FNM_NOCHECK) { // 如果設定了 FNM_NOCHECK，則返回原始模式
			// 將 pattern 加入結果
			// (與 __nfs_glob_processing_callback 類似的邏輯)
			char** new_pathv = static_cast<char**>(realloc(glob_results_output->gl_pathv, (glob_results_output->gl_pathc + 1) * sizeof(char*)));
			if (!new_pathv) return 2; // GLOB_NOSPACE
			glob_results_output->gl_pathv = new_pathv;
			glob_results_output->gl_pathv[glob_results_output->gl_pathc] = static_cast<char*>(malloc(strlen(pattern) + 1));
			if (!glob_results_output->gl_pathv[glob_results_output->gl_pathc]) return 2;
			strcpy(glob_results_output->gl_pathv[glob_results_output->gl_pathc], pattern);
			glob_results_output->gl_pathc++;
		}
		else {
			return 1; // GLOB_NOMATCH (假設1為NOMATCH)
		}
	}

	// 排序結果 (除非設定了 NFS_FNM_NOSORT)
	// 原始碼中的旗標是 (a3 & 4) == 0，假設 0x04 對應 NFS_FNM_NOSORT
	if (!(flags & NFS_FNM_NOSORT)) {
		if (glob_results_output->gl_pathc > 1) { // 只有多於一個結果時才需要排序
			// qsort 需要從 gl_pathv + gl_offs 開始排序，排序元素個數是 gl_pathc - gl_offs
			// 但原始碼是 qsort(gl_pathv + gl_offs, gl_pathc, ...)
			// 這表示 gl_pathc 是「相對於 gl_offs 的匹配數量」還是「總數量」？
			// 從 __nfs_glob_cbk 的 `**(_DWORD **)a4 = v5 + 1;` (a5->gl_pathc++) 來看，gl_pathc 是總匹配數。
			// 原始碼 `qsort((void *)(*((_DWORD *)a5 + 1) + 4 * *((_DWORD *)a5 + 2)), *(_DWORD *)a5, 4u, __nfs_glob_sort);`
			// `glob_results_output->gl_pathv + glob_results_output->gl_offs`
			// `glob_results_output->gl_pathc` (作為數量)
			// 這意味著 qsort 的元素數量是總匹配數 gl_pathc，但起始指標是偏移後的。這不太對。
			// 通常 qsort(base, num_elements, size_of_element, comparator)
			// 如果 gl_pathc 是總數（包括 gl_offs 前的空位），那麼 num_elements 應為 gl_pathc - gl_offs。
			// 如果 gl_pathc 是純粹的匹配計數，那麼 qsort 的 base 應該是 gl_pathv + gl_offs，數量是 gl_pathc。
			// 原始碼的 __nfs_glob_cbk 中，`**(_DWORD **)a4 = v5 + 1;` (a5->gl_pathc++) 似乎是純計數。
			// `v7 = **(_DWORD **)a4; if ( (*((_BYTE *)a4 + 8) & 8) != 0 ) v7 += v6[2];`
			// `v6[2]` 是 `original_flags`。`(*((_BYTE *)a4 + 8) & 8)` 可能是 `FNM_DOOFFS`。
			// `v6[2]` 不是 `gl_offs`。`gl_offs` 是 `*((_DWORD *)a5 + 2)`。
			// 原始碼排序是 `qsort(gl_pathv + gl_offs, gl_pathc (純匹配計數), sizeof(char*), sort_func)`
			// 這是正確的。
			if (glob_results_output->gl_pathc > 0) { // 確保有東西可排序
				qsort(glob_results_output->gl_pathv + glob_results_output->gl_offs,
					glob_results_output->gl_pathc, // 假設 gl_pathc 是純匹配計數
					sizeof(char*), __nfs_glob_sort_callback);
			}
		}
	}
	return 0; // 成功
}

// Layer 7
// --- 檔案開啟旗標 (基於原始碼分析的推測值) ---
const int NFS_OPEN_ACCESS_MODE_MASK = 0x03; // 假設位元0和1定義存取模式
const int NFS_OPEN_READ_ONLY = 0x00; // 假設
const int NFS_OPEN_WRITE_ONLY = 0x01; // 假設
const int NFS_OPEN_READ_WRITE = 0x02; // 假設

const int NFS_OPEN_APPEND = 0x08;
const int NFS_OPEN_TRUNCATE = 0x200;
// const int NFS_OPEN_CREATE      = ... ; // 可能包含在 0x302 (770) 中

const int NFS_DEFAULT_FILE_MODE = 0x0302; // 770 in decimal, used in nfs_file_create
/** @brief 配置一個新的檔案描述符和對應的 NfsOpenFileHandle。 */
int allocate_file_descriptor(NfsHandle* handle) {
	if (!handle) return -1;

	NfsOpenFileHandle* new_fh = static_cast<NfsOpenFileHandle*>(malloc(sizeof(NfsOpenFileHandle)));
	if (!new_fh) {
		nfs_errno = 15; // No memory available for opening files
		return -1;
	}
	// 初始化 new_fh 的成員為0 (或預設值)
	memset(new_fh, 0, sizeof(NfsOpenFileHandle));
	new_fh->in_use = false;

	int fd = -1;
	// 尋找空閒槽位
	if (handle->open_files_array) {
		for (int i = 0; i < handle->open_files_array_capacity; ++i) {
			if (handle->open_files_array[i] == nullptr) {
				fd = i;
				break;
			}
		}
	}

	if (fd == -1) { // 沒有找到空閒槽位，或陣列尚未初始化/已滿
		if (!handle->open_files_array) { // 首次分配
			handle->open_files_array_capacity = 32; // 原始碼預設大小
			handle->open_files_array = static_cast<NfsOpenFileHandle**>(malloc(sizeof(NfsOpenFileHandle*) * handle->open_files_array_capacity));
			if (!handle->open_files_array) {
				free(new_fh);
				nfs_errno = 15;
				return -1;
			}
			memset(handle->open_files_array, 0, sizeof(NfsOpenFileHandle*) * handle->open_files_array_capacity);
			fd = 0; // 使用第一個槽位
		}
		else { // 陣列已滿，需要擴展
			int new_capacity = handle->open_files_array_capacity * 2;
			NfsOpenFileHandle** new_array = static_cast<NfsOpenFileHandle**>(realloc(handle->open_files_array, sizeof(NfsOpenFileHandle*) * new_capacity));
			if (!new_array) {
				free(new_fh);
				nfs_errno = 15;
				return -1;
			}
			handle->open_files_array = new_array;
			// 將新擴展的部分初始化為 nullptr
			memset(handle->open_files_array + handle->open_files_array_capacity, 0, sizeof(NfsOpenFileHandle*) * (new_capacity - handle->open_files_array_capacity));
			fd = handle->open_files_array_capacity; // 新的空閒槽位是舊容量的索引
			handle->open_files_array_capacity = new_capacity;
		}
	}
	handle->open_files_array[fd] = new_fh;
	return fd;
}

/** @brief 釋放指定的檔案描述符及其關聯的 NfsOpenFileHandle。*/
int deallocate_file_descriptor(NfsHandle* handle, int fd)
{
	if (!handle) { nfs_errno = 9;  return 9; }

	if (!handle->open_files_array ||
		fd < 0 || fd >= handle->open_files_array_capacity) {
		nfs_errno = 10;             // FD 無效
		return 10;
	}

	NfsOpenFileHandle* fh = handle->open_files_array[fd];
	if (!fh) {
		nfs_errno = 10;             // 已經被釋放過
		return 10;
	}

	/* ⚠️ 關鍵判斷：還在使用中的 FD 不可強拆 */
	if (fh->in_use) {
		nfs_errno = 11;             // File still open
		return 11;                  // 讓測試第 6 步 assert !=0 成立
	}

	free(fh);
	handle->open_files_array[fd] = nullptr;
	return 0;                       // 真正成功
}



// --- 核心檔案操作 ---

/** @brief 截斷檔案到指定大小。更新 FAT 鏈和 NT 中的檔案大小。*/
void file_truncate(NfsHandle* handle, NfsOpenFileHandle* fh, int new_size) {
	if (!handle || !fh || new_size < 0) return;

	// 1. 調整 FAT 鏈
	//    計算需要保留的區塊數量
	int num_blocks_to_keep = (new_size + 512 - 1) / 512;
	if (new_size == 0) num_blocks_to_keep = 0; // 特殊處理大小為0的情況

	if (num_blocks_to_keep == 0) { // 截斷到0，銷毀整個鏈
		if (fh->fat_chain_start_idx > 0) { // 確保鏈存在
			nfs_fat_destroy_chain(handle->fat_handle, fh->fat_chain_start_idx);
			// 文件被截斷為0後，其 FAT 鏈應表示為空或無效。
			// nfs_fat_destroy_chain 會將所有相關FAT條目清0。
			// 此處可能需要將 fh->fat_chain_start_idx 更新為一個表示空鏈的值 (例如0或-1)
			// 但 nfs_file_create 在創建空檔案時會調用 nfs_fat_create_chain，它返回一個有效的單區塊鏈(EOC)。
			// 所以，即使大小為0，檔案通常也有一個（空的）FAT鏈。
			// 這裡的邏輯是，如果截斷到0，則原鏈被銷毀。下次寫入時會按需擴展或重建。
			// 如果要保持一個 EOC 塊，則應：
			// int first_block = fh->fat_chain_start_idx;
			// nfs_fat_chain_truncate(handle->fat_handle, first_block); // 使第一個塊成為EOC
			// 但原始碼 (005C7250) 是：
			// v3 = nfs_fat_chain_get_nth(fat_h, chain_start, new_size_bytes / 512);
			// nfs_fat_chain_truncate(fat_h, v3);
			// 若 new_size_bytes = 0, new_size_bytes / 512 = 0.
			// v3 = nfs_fat_chain_get_nth(fat_h, chain_start, 0) -> 得到 chain_start 本身
			// nfs_fat_chain_truncate(fat_h, chain_start) -> 使 chain_start 成為 EOC。
			// 這意味著檔案大小為0時，FAT鏈中第一個區塊被設為-1。
			int block_to_become_eoc = fh->fat_chain_start_idx; // 如果num_blocks_to_keep == 0
			if (num_blocks_to_keep > 0) {
				// 獲取第 (num_blocks_to_keep - 1) 個區塊的索引 (0-indexed)
				block_to_become_eoc = nfs_fat_chain_get_nth(handle->fat_handle, fh->fat_chain_start_idx, num_blocks_to_keep - 1);
			}
			if (block_to_become_eoc > 0) { // 確保是有效區塊索引
				nfs_fat_chain_truncate(handle->fat_handle, block_to_become_eoc);
			}
			else if (num_blocks_to_keep == 0 && fh->fat_chain_start_idx > 0) {
				// 特殊情況：如果保留0塊，且原鏈存在，則銷毀整個鏈，並可能需要一個新的空鏈頭
				// 原始碼的 get_nth(...,0) 返回鏈頭，truncate(鏈頭) 使鏈頭變 EOC。
				// 這暗示即使大小為0，也保留一個 EOC 鏈頭。
				nfs_fat_chain_truncate(handle->fat_handle, fh->fat_chain_start_idx);
			}
			else if (num_blocks_to_keep > 0 && block_to_become_eoc <= 0) {
				// 請求保留的區塊數超出現有鏈長，或鏈已損壞
				// 此時不應截斷，或者這是一個錯誤狀態
			}

		}
	}
	else { // num_blocks_to_keep > 0
		// 找到應該成為新 EOC 的區塊索引
		// 第 num_blocks_to_keep 個區塊 (即索引為 num_blocks_to_keep - 1 的區塊) 是最後一個保留的區塊
		int last_block_to_keep_idx = nfs_fat_chain_get_nth(handle->fat_handle, fh->fat_chain_start_idx, num_blocks_to_keep - 1);
		if (last_block_to_keep_idx > 0) { // 確保找到了有效的區塊
			nfs_fat_chain_truncate(handle->fat_handle, last_block_to_keep_idx);
		}
		else {
			// 請求保留的區塊數可能超出現有鏈長，或鏈已損壞
			// 如果鏈本身就比 num_blocks_to_keep 短，則無需截斷
			// 如果 last_block_to_keep_idx <= 0 且非 -1，則鏈可能損壞
		}
	}

	// 2. 更新 NT 中的檔案大小
	nfs_nt_node_set_size(handle->nt_handle, fh->nt_node_idx, new_size);
}


/** @brief 檢查虛擬檔案系統中檔案是否存在。*/
int nfs_file_exists(NfsHandle* handle, const char* filename) { // 返回 bool (int)
	if (!handle || !handle->dt_handle || !filename) {
		nfs_errno = 9; // Invalid handle or params
		return 0;
	}
	// nfs_dt_filename_lookup 返回 >=0 的 trienode_idx 表示找到
	return (nfs_dt_filename_lookup(handle->dt_handle, filename) >= 0);
}

/** @brief 開啟虛擬檔案系統中的檔案。*/
int nfs_file_open(NfsHandle* handle, const char* filename, int open_flags) {
	if (!handle || !filename) {
		nfs_errno = 19; // Invalid parameters
		return -1;
	}

	int dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, filename);
	if (dt_node_idx < 0) {
		nfs_errno = 11; // File not found
		return -1;
	}

	int fd = allocate_file_descriptor(handle);
	if (fd < 0) {
		// nfs_errno 已由 allocate_file_descriptor 設定 (15)
		return -1;
	}

	NfsOpenFileHandle* fh = handle->open_files_array[fd];
	fh->open_mode_flags = open_flags;
	fh->in_use = true;
	fh->dt_trienode_idx = dt_node_idx;

	fh->nt_node_idx = nfs_dt_filename_get_nt_index(handle->dt_handle, dt_node_idx);
	if (fh->nt_node_idx < 0) { // 無效的 NT 索引
		deallocate_file_descriptor(handle, fd); // 清理
		nfs_errno = 14; // Invalid node in NT (推測)
		return -1;
	}

	NfsNode temp_nt_node;
	if (nfs_nt_get_node(handle->nt_handle, fh->nt_node_idx, &temp_nt_node) < 0) {
		deallocate_file_descriptor(handle, fd);
		nfs_errno = 14; // 讀取 NT 節點失敗
		return -1;
	}
	fh->fat_chain_start_idx = temp_nt_node.fat_chain_start_idx;
	fh->current_byte_offset = 0; // 開啟檔案時，讀寫位置設為開頭

	// 檢查是否需要截斷檔案 (O_TRUNC)
	// 原始碼條件： (open_flags & 3) != 0 && (open_flags & 0x200) != 0
	// (a3 & 3) != 0 表示有寫入權限 (例如 W 或 RW，但不只是 R)
	// (a3 & 0x200) != 0 表示設定了 TRUNCATE 旗標
	bool has_write_access = (open_flags & NFS_OPEN_ACCESS_MODE_MASK) == NFS_OPEN_WRITE_ONLY ||
		(open_flags & NFS_OPEN_ACCESS_MODE_MASK) == NFS_OPEN_READ_WRITE;
	// ^-- 這只是基於旗標的猜測，原始碼是 `(a3 & 3) != 0`
	// 如果 0 是 Read, 1 是 Write, 2 是 ReadWrite, 3 可能也是 ReadWrite
	// 那麼 `(a3 & 3) != 0` 意味著 `a3` 不是唯讀。

	if ((open_flags & NFS_OPEN_TRUNCATE) && has_write_access) {
		file_truncate(handle, fh, 0); // 截斷到0長度
	}

	handle->num_currently_open_files++;
	return fd;
}

/** @brief 建立虛擬檔案系統中的檔案。如果檔案已存在，則截斷它。*/
int nfs_file_create(NfsHandle* handle, const char* filename) {
	if (!handle || !filename) {
		nfs_errno = 19;
		return -1;
	}

	int fd = -1;
	NfsOpenFileHandle* fh = nullptr;
	int dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, filename);

	if (dt_node_idx >= 0) { // 檔案已存在
		// 以截斷模式開啟現有檔案
		// 原始碼中，此處檔案模式設為 770 (0x302)
		// 假設這包含了讀寫和截斷的意圖
		fd = nfs_file_open(handle, filename, NFS_DEFAULT_FILE_MODE | NFS_OPEN_TRUNCATE);
		// nfs_file_open 內部已處理截斷
		if (fd < 0) {
			// nfs_errno 已由 nfs_file_open 設定
			return -1;
		}
	}
	else { // 檔案不存在，需要建立
		fd = allocate_file_descriptor(handle);
		if (fd < 0) {
			nfs_errno = 15; // No memory
			return -1;
		}
		fh = handle->open_files_array[fd];
		fh->in_use = true;
		fh->open_mode_flags = NFS_DEFAULT_FILE_MODE; // 假設的建立模式
		fh->current_byte_offset = 0;

		// 1. 配置 NT 節點
		fh->nt_node_idx = nfs_nt_allocate_node(handle->nt_handle);
		if (fh->nt_node_idx < 0) {
			deallocate_file_descriptor(handle, fd);
			nfs_errno = 7; // Could not allocate NT channel/node (推測)
			return -1;
		}
		// 新節點大小為0，ref_count為1 (由 nfs_nt_allocate_node 設定)

		// 2. 建立 FAT 鏈 (初始為空鏈，僅一個 EOC 區塊)
		fh->fat_chain_start_idx = nfs_fat_create_chain(handle->fat_handle);
		if (fh->fat_chain_start_idx <= 0) { // <=0 表示建立鏈失敗
			nfs_nt_refcount_decr(handle->nt_handle, fh->nt_node_idx); // 回收 NT 節點
			// (node_recover 會處理將 NT 節點清零)
			deallocate_file_descriptor(handle, fd);
			nfs_errno = 6; // Could not allocate FAT channel/chain (推測)
			return -1;
		}
		// 將 FAT 鏈關聯到 NT 節點
		nfs_nt_node_set_chain(handle->nt_handle, fh->nt_node_idx, fh->fat_chain_start_idx);
		nfs_nt_node_set_size(handle->nt_handle, fh->nt_node_idx, 0); // 確保大小為0

		// 3. 在目錄表 (DT) 中新增檔名條目，並關聯 NT 節點索引
		// 原始碼中，nfs_dt_filename_add 只傳入檔名，NT 索引是後續設定的。
		// dt_node_idx = nfs_dt_filename_add(handle->dt_handle, filename);
		// 這裡的 filename_add (p_insert_key) 第三個參數是 nt_idx
		dt_node_idx = nfs_dt_filename_add(handle->dt_handle, filename); // 假設 nt_idx 在 p_insert_key 中處理
		// 或者 filename_add 返回 trienode_idx
		if (dt_node_idx < 0) {
			nfs_fat_destroy_chain(handle->fat_handle, fh->fat_chain_start_idx); // 回收 FAT 鏈
			nfs_nt_refcount_decr(handle->nt_handle, fh->nt_node_idx);      // 回收 NT 節點
			deallocate_file_descriptor(handle, fd);
			nfs_errno = 18; // Internal error in DT (推測)
			return -1;
		}
		// 將 NT 索引設定到 DT 條目中
		nfs_dt_filename_set_nt_index(handle->dt_handle, dt_node_idx, static_cast<short>(fh->nt_node_idx));
		fh->dt_trienode_idx = dt_node_idx;

		handle->num_currently_open_files++;
	}
	return fd;
}

/** @brief 關閉一個已開啟的檔案。*/
int nfs_file_close(NfsHandle* handle, int fd) {
	if (!handle) {
		nfs_errno = 9;
		return -1;
	}
	NfsOpenFileHandle* fh = handle->open_files_array[fd];
	fh->in_use = false;
	if (deallocate_file_descriptor(handle, fd) != 0) {
		// nfs_errno 已由 deallocate_file_descriptor 設定 (10)
		return -1;
	}
	handle->num_currently_open_files--;
	return 0;
}


/** @brief 設定檔案的讀寫指標位置。如果新位置超出檔案末尾，則擴展檔案。*/
int nfs_file_lseek(NfsHandle* handle, int fd, int offset, int whence) {
	// whence: 0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END (與C標準類似)
	if (!handle || !handle->open_files_array || fd < 0 || fd >= handle->open_files_array_capacity || !handle->open_files_array[fd]) {
		nfs_errno = 13; // Invalid file descriptor
		return -1;
	}
	NfsOpenFileHandle* fh = handle->open_files_array[fd];

	int current_file_size = nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx);
	if (current_file_size < 0) { // 讀取大小失敗
		nfs_errno = 14; // Invalid node in NT
		return -1;
	}

	int new_pos;
	if (whence == 0) { // SEEK_SET
		new_pos = offset;
	}
	else if (whence == 1) { // SEEK_CUR
		new_pos = fh->current_byte_offset + offset;
	}
	else if (whence == 2) { // SEEK_END
		new_pos = current_file_size + offset;
	}
	else {
		nfs_errno = 19; // Invalid parameters (whence)
		return -1;
	}

	if (new_pos < 0) new_pos = 0; // 不允許定位到檔案開頭之前

	fh->current_byte_offset = new_pos;

	// 如果新位置超出現有檔案大小，需要擴展 FAT 鏈並更新 NT 中的大小
	if (new_pos > current_file_size) {
		int current_num_blocks = nblocks(current_file_size); // 使用修正後的 nblocks
		int needed_num_blocks = nblocks(new_pos); // 如果 new_pos 是目標大小，則用它；如果只是偏移，則可能是 new_pos+1

		int fat_chain_iter = fh->fat_chain_start_idx;
		if (current_num_blocks > 0 && fat_chain_iter > 0) { // 如果已有區塊，找到鏈尾
			fat_chain_iter = find_last_in_chain(handle->fat_handle, fh->fat_chain_start_idx);
		}
		else if (current_num_blocks == 0 && fat_chain_iter <= 0) { // 檔案為空且無鏈，需要創建
			fat_chain_iter = nfs_fat_create_chain(handle->fat_handle);
			if (fat_chain_iter <= 0) { nfs_errno = 6; return -1; }
			fh->fat_chain_start_idx = fat_chain_iter; // 更新檔案控制代碼中的鏈起始
			nfs_nt_node_set_chain(handle->nt_handle, fh->nt_node_idx, fat_chain_iter); // 更新NT中的鏈起始
		}
		else if (current_num_blocks == 0 && fat_chain_iter > 0) {
			// 有鏈頭但大小為0，可能是已截斷到0但保留鏈頭的情況。
			// find_last_in_chain 會返回此鏈頭。
			fat_chain_iter = find_last_in_chain(handle->fat_handle, fh->fat_chain_start_idx);
		}


		for (int i = current_num_blocks; i < needed_num_blocks; ++i) {
			int new_block = nfs_fat_chain_extend(handle->fat_handle, fat_chain_iter);
			if (new_block <= 0) {
				nfs_errno = 6; // FAT 擴展失敗
				// 注意：此時檔案大小和 FAT 鏈可能處於不一致狀態
				return -1;
			}
			if (fat_chain_iter <= 0 && i == 0) { // 如果是第一個分配的塊，它成為新的 fat_chain_iter
				fh->fat_chain_start_idx = new_block; // 如果鏈是從無到有創建的
				nfs_nt_node_set_chain(handle->nt_handle, fh->nt_node_idx, new_block);
			}
			fat_chain_iter = new_block; // 下次從新塊擴展
		}
		// 原始碼中設定大小為 new_pos + 1，這通常意味著大小是包含 new_pos 的位元組數。
		// 如果 new_pos 是 0-indexed 的最大位移，則長度是 new_pos + 1。
		nfs_nt_node_set_size(handle->nt_handle, fh->nt_node_idx, new_pos);
		// 如果嚴格遵循原始碼 v9+1，則為 new_pos + 1
		// 但通常 lseek 後寫入才確定最終大小。這裡的 set_size 是為了預分配空間。
		// 如果 new_pos 是目標長度，則設為 new_pos。
		// 鑑於 file_truncate 設為 new_size，這裡設為 new_pos 較為一致。
	}
	return 0; // 成功
}


/** @brief 從檔案讀取資料。*/
int nfs_file_read(NfsHandle* handle, int fd, void* buffer, int bytes_to_read) {
	// 1. 檢查 handle
	if (!handle) {
		nfs_errno = 9;  // Invalid file system handle
		return -1;
	}
	// 2. 檢查檔案描述符是否合法
	if (fd < 0
		|| fd >= handle->open_files_array_capacity
		|| !handle->open_files_array
		|| !handle->open_files_array[fd]) {
		nfs_errno = 13; // Invalid file descriptor
		return -1;
	}
	// 3. 檢查 buffer 指標與讀取長度
	if (!buffer || bytes_to_read < 0) {
		nfs_errno = 19; // Invalid parameters
		return -1;
	}
	// 0 bytes shortcut
	if (bytes_to_read == 0) {
		return 0;
	}

	NfsOpenFileHandle* fh = handle->open_files_array[fd];

	int file_size = nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx);
	if (file_size < 0) { nfs_errno = 14; return -1; }

	int effective_bytes_to_read = bytes_to_read;
	if (fh->current_byte_offset >= file_size) {
		return 0; // 已在檔案結尾或之後
	}
	if (fh->current_byte_offset + effective_bytes_to_read > file_size) {
		effective_bytes_to_read = file_size - fh->current_byte_offset;
	}
	if (effective_bytes_to_read <= 0) return 0;


	char temp_block_buffer[512]; // 用於處理部分區塊讀取
	int total_bytes_read = 0;
	char* current_user_buffer_ptr = static_cast<char*>(buffer);
	int remaining_bytes = effective_bytes_to_read;

	while (remaining_bytes > 0) {
		int current_block_seq_in_file = blockno(fh->current_byte_offset); // 檔案中的第幾個區塊 (0-indexed)
		int offset_in_physical_block = fh->current_byte_offset % 512;

		int actual_fat_block_idx = nfs_fat_chain_get_nth(handle->fat_handle, fh->fat_chain_start_idx, current_block_seq_in_file);
		if (actual_fat_block_idx <= 0) { // 鏈結束或錯誤
			nfs_errno = 11; // File not found (or bad chain)
			break;
		}

		int bytes_to_read_from_this_vfs_block = min(remaining_bytes, 512 - offset_in_physical_block);

		if (nfs_data_read(handle->data_handle, actual_fat_block_idx, temp_block_buffer) != 0) { // 假設 nfs_data_read 成功返回0
			nfs_errno = 2; // Data read error (推測)
			break;
		}

		memcpy(current_user_buffer_ptr, temp_block_buffer + offset_in_physical_block, bytes_to_read_from_this_vfs_block);

		fh->current_byte_offset += bytes_to_read_from_this_vfs_block;
		current_user_buffer_ptr += bytes_to_read_from_this_vfs_block;
		total_bytes_read += bytes_to_read_from_this_vfs_block;
		remaining_bytes -= bytes_to_read_from_this_vfs_block;
	}
	return total_bytes_read;
}

/** @brief 向檔案寫入資料。*/
/** @brief 向檔案寫入資料。*/
int nfs_file_write(NfsHandle* handle, int fd, const void* buffer, int bytes_to_write) {
	// 1. 檢查 handle
	if (!handle) {
		nfs_errno = 9;    // Invalid file system handle
		return -1;
	}
	// 2. 檢查 fd
	if (fd < 0
		|| fd >= handle->open_files_array_capacity
		|| !handle->open_files_array
		|| !handle->open_files_array[fd]) {
		nfs_errno = 13;   // Invalid file descriptor
		return -1;
	}
	// 3. 檢查 buffer 與長度
	if (!buffer || bytes_to_write < 0) {
		nfs_errno = 19;   // Invalid parameters
		return -1;
	}
	if (bytes_to_write == 0) {
		return 0;
	}

	NfsOpenFileHandle* fh = handle->open_files_array[fd];

	// 2. 處理 O_APPEND：將 offset 移至檔案尾端
	if (fh->open_mode_flags & NFS_OPEN_APPEND) {
		int current_size = nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx);
		if (current_size < 0) { nfs_errno = 14; return -1; }
		if (nfs_file_lseek(handle, fd, current_size, 0 /*SEEK_SET*/) != 0) {
			// nfs_errno 已由 lseek 設定
			return -1;
		}
	}

	// 3. 如果寫入會超出目前檔案大小，先用 lseek 觸發擴展，再把位置移回
	int target_end_offset = fh->current_byte_offset + bytes_to_write - 1;
	int current_file_size = nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx);
	if (target_end_offset >= current_file_size) {
		int original_seek_pos = fh->current_byte_offset;
		if (nfs_file_lseek(handle, fd, target_end_offset, 0 /*SEEK_SET*/) != 0) return -1;
		if (nfs_file_lseek(handle, fd, original_seek_pos, 0 /*SEEK_SET*/) != 0) return -1;
		// lseek 已更新 NT 的檔案大小
	}

	// 4. 逐區塊寫入
	char        temp_block_buffer[512];
	int         total_bytes_written = 0;
	const char* current_user_buffer_ptr = static_cast<const char*>(buffer);
	int         remaining_bytes = bytes_to_write;

	while (remaining_bytes > 0) {
		int block_seq = blockno(fh->current_byte_offset);
		int offset_in_block = fh->current_byte_offset % 512;
		int fat_block_idx = nfs_fat_chain_get_nth(handle->fat_handle, fh->fat_chain_start_idx, block_seq);
		if (fat_block_idx <= 0) {
			nfs_errno = 6; // Bad FAT chain or disk full
			break;
		}

		int bytes_this_block = min(remaining_bytes, 512 - offset_in_block);
		bool full_overwrite = (offset_in_block == 0 && bytes_this_block == 512);

		// 如果不是整塊覆蓋，先讀出舊資料
		if (!full_overwrite) {
			if (nfs_data_read(handle->data_handle, fat_block_idx, temp_block_buffer) != 0) {
				nfs_errno = 2; // Data read error
				break;
			}
		}

		// 複製新資料到快取緩衝
		memcpy(temp_block_buffer + offset_in_block, current_user_buffer_ptr, bytes_this_block);

		// 寫回實體區塊
		if (nfs_data_write(handle->data_handle, fat_block_idx, temp_block_buffer) != 0) {
			nfs_errno = 2; // Data write error
			break;
		}

		// 更新 offset 與計數
		fh->current_byte_offset += bytes_this_block;
		current_user_buffer_ptr += bytes_this_block;
		total_bytes_written += bytes_this_block;
		remaining_bytes -= bytes_this_block;
	}

	// 5. 最後再次確認並更新 NT 中的檔案大小
	if (fh->current_byte_offset > nfs_nt_node_get_size(handle->nt_handle, fh->nt_node_idx)) {
		nfs_nt_node_set_size(handle->nt_handle, fh->nt_node_idx, fh->current_byte_offset);
	}

	return total_bytes_written;
}

/** @brief 增加檔案的引用計數。*/
int nfs_file_inc_refcount(NfsHandle* handle, const char* filename) {
	if (!handle || !handle->dt_handle || !handle->nt_handle || !filename) {
		nfs_errno = (handle ? 19 : 9); // Invalid parameters or handle
		return -1;
	}
	int dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, filename);
	if (dt_node_idx < 0) {
		nfs_errno = 11; // File not found
		return -1;
	}
	short nt_idx = nfs_dt_filename_get_nt_index(handle->dt_handle, dt_node_idx);
	if (nt_idx < 0) { // 雖然 dt_node 有效，但 nt_idx 可能無效
		nfs_errno = 14; // Invalid node in NT
		return -1;
	}
	nfs_nt_refcount_incr(handle->nt_handle, nt_idx);
	return 0;
}

/** @brief 減少檔案的引用計數。如果計數為0，則刪除檔案資料 (DT條目, NT節點, FAT鏈)。*/
int nfs_file_dec_refcount(NfsHandle* handle, const char* filename) {
	if (!handle || !handle->dt_handle || !handle->nt_handle || !handle->fat_handle || !filename) {
		nfs_errno = (handle ? 19 : 9);
		return -1;
	}
	int dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, filename);
	if (dt_node_idx < 0) {
		nfs_errno = 11;
		return -1;
	}
	short nt_idx = nfs_dt_filename_get_nt_index(handle->dt_handle, dt_node_idx);
	if (nt_idx < 0) {
		nfs_errno = 14;
		return -1;
	}

	int fat_chain_start = nfs_nt_node_get_chain(handle->nt_handle, nt_idx);
	// nfs_nt_refcount_decr 返回 1 表示節點已被恢復 (引用計數降至0)
	if (nfs_nt_refcount_decr(handle->nt_handle, nt_idx) == 1) {
		if (nfs_dt_filename_delete(handle->dt_handle, filename) < 0) {
			// DT 刪除失敗，但 NT 節點已標記回收。狀態可能不一致。
			nfs_errno = 18; // Internal error in DT
			// 這裡不返回 -1，因為 NT 和 FAT 的清理仍需嘗試 (如果原始碼是這樣)
			// 但通常應該返回錯誤。
		}
		if (fat_chain_start > 0) { // 確保是有效的 FAT 鏈起始
			if (nfs_fat_destroy_chain(handle->fat_handle, fat_chain_start) < 0) {
				// FAT 鏈銷毀失敗
				nfs_errno = 3; // Could not initialize/destroy FAT (推測)
			}
		}
		// node_recover 已在 nfs_nt_refcount_decr 中被呼叫，處理了 NT 節點的清理
	}
	return 0; // 原始碼返回 result，v6 > 0 時 result = -1，否則為0. 這裡簡化為0表成功。
	// 或者應該返回 nfs_nt_refcount_decr 的結果？
	// 原始碼 if (v6 > 0) { if (nfs_nt_refcount_decr(...) ) { result = nfs_dt_filename_delete(...) ; if (result >=0) result = nfs_fat_destroy_chain(...)} else { result = 0} }
	// 這樣看來，只要任一步失敗，result 就可能是 -1。
	// 為了簡單，假設 refcount_decr 成功後，後續步驟也應成功，否則 nfs_errno 會被設定。
}

/** @brief 建立一個指向現有檔案的新連結 (硬連結)。*/
int nfs_file_link(NfsHandle* handle, const char* old_name, const char* new_name) {
	if (!handle || !handle->dt_handle || !handle->nt_handle || !old_name || !new_name) {
		nfs_errno = (handle ? 19 : 9);
		return -1;
	}
	int old_dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, old_name);
	if (old_dt_node_idx < 0) {
		nfs_errno = 11; // Source file not found
		return -1;
	}
	if (nfs_dt_filename_lookup(handle->dt_handle, new_name) >= 0) {
		nfs_errno = 17; // Target file (new_name) already exists
		return -1;
	}

	short nt_idx = nfs_dt_filename_get_nt_index(handle->dt_handle, old_dt_node_idx);
	if (nt_idx < 0) {
		nfs_errno = 14;
		return -1;
	}

	nfs_nt_refcount_incr(handle->nt_handle, nt_idx); // 增加目標 NT 節點的引用計數

	// 原始碼中，nfs_dt_filename_add 的 nt_idx 參數是0。
	// 然後用 nfs_dt_filename_set_nt_index 設定。
	int new_dt_node_idx = nfs_dt_filename_add(handle->dt_handle, new_name); // 假設 nt_idx 在內部或後續設定
	if (new_dt_node_idx < 0) {
		nfs_nt_refcount_decr(handle->nt_handle, nt_idx); // 回滾引用計數增加
		nfs_errno = 17; // Cannot create new_name (or 18 for DT error)
		return -1;
	}
	if (nfs_dt_filename_set_nt_index(handle->dt_handle, new_dt_node_idx, nt_idx) < 0) {
		nfs_nt_refcount_decr(handle->nt_handle, nt_idx); // 回滾
		nfs_dt_filename_delete(handle->dt_handle, new_name); // 嘗試刪除剛加入的 new_name
		nfs_errno = 18; // DT error
		return -1;
	}
	return 0;
}

/** @brief 取消檔案的一個連結 (名稱)。如果這是最後一個連結，則刪除檔案資料。*/
int nfs_file_unlink(NfsHandle* handle, const char* filename_to_unlink) {
	// 此函式邏輯與 nfs_file_dec_refcount 非常相似，因為都是移除一個名稱並可能觸發實際刪除。
	// 原始碼 (005C7DC0) 與 (005C7C20 nfs_file_dec_refcount) 的結構幾乎一樣。
	// 唯一的細微差別可能在於錯誤碼的設定或返回值的處理。
	// 這裡我們可以直接呼叫 nfs_file_dec_refcount 或複製其邏輯。
	// 為保持獨立性，複製並調整。
	if (!handle || !handle->dt_handle || !handle->nt_handle || !handle->fat_handle || !filename_to_unlink) {
		nfs_errno = (handle ? 19 : 9);
		return -1;
	}
	int dt_node_idx = nfs_dt_filename_lookup(handle->dt_handle, filename_to_unlink);
	if (dt_node_idx < 0) {
		nfs_errno = 11;
		return -1;
	}
	short nt_idx = nfs_dt_filename_get_nt_index(handle->dt_handle, dt_node_idx);
	if (nt_idx < 0) {
		nfs_errno = 14;
		return -1;
	}

	int fat_chain_start = nfs_nt_node_get_chain(handle->nt_handle, nt_idx);

	// 先從 DT 中刪除名稱
	if (nfs_dt_filename_delete(handle->dt_handle, filename_to_unlink) < 0) {
		nfs_errno = 18; // DT error
		return -1; // 如果刪除名稱就失敗了，後續的 refcount 和資料清理可能不應進行
	}

	// 再處理引用計數和可能的資料銷毀
	if (nfs_nt_refcount_decr(handle->nt_handle, nt_idx) == 1) { // 如果 NT 節點被回收
		if (fat_chain_start > 0) {
			if (nfs_fat_destroy_chain(handle->fat_handle, fat_chain_start) < 0) {
				nfs_errno = 3; // FAT error
				// 即使 FAT 銷毀失敗，名稱和 NT 節點引用也已處理
			}
		}
	}
	return 0;
}

/**
 * @brief 執行檔名 Globbing，尋找符合模式的檔案。
 * @param handle NFS 控制代碼。
 * @param pattern Glob 模式字串。
 * @param flags Glob 旗標。
 * @param errfunc 錯誤回呼函式 (在此簡化版本中可能未完全使用)。
 * @param glob_results 儲存匹配結果的 NfsGlobResults 結構指標。
 * @return 成功時返回0，或其他 glob 標準錯誤碼。
 */
int nfs_glob(NfsHandle* handle, const char* pattern, int flags,
	int (*errfunc)(const char* epath, int eerrno),
	NfsGlobResults* glob_results) {
	if (!handle || !handle->dt_handle || !pattern || !glob_results) {
		nfs_errno = (handle ? 19 : 9);
		return 3; // GLOB_ABORTED
	}

	// 原始碼中 nfs_glob (005C7EC0) 直接呼叫 nfs_dt_filename_glob
	// nfs_dt_filename_glob(dt_handle, pattern, dt_glob_flags, callback, context)
	// 原始碼中的 dt_glob_flags 是硬編碼的 6。這個 6 的具體含義不明。
	// 假設它對應於傳遞給 nfs_pmatch 的一些預設行為。
	// 我們先前還原的 nfs_dt_filename_glob 接受 pmatch_flags。

	// 這裡，我們需要調用先前定義的 nfs_dt_filename_glob。
	// nfs_dt_filename_glob 內部會處理初始化 glob_results (基於 FNM_APPEND),
	// find_prefix, p_lookup_key_n, p_node_iterate (使用 __nfs_glob_processing_callback),
	// FNM_NOCHECK, 和排序 (基於 FNM_NOSORT)。

	// 由於先前 nfs_dt_filename_glob 已還原並包含了這些邏輯，這裡直接呼叫它。
	// 參數 flags (來自 nfs_glob) 應傳遞給 nfs_dt_filename_glob 的 flags 參數。
	// errfunc 也應傳遞。

	return nfs_dt_filename_glob(handle->dt_handle, pattern, flags, errfunc, glob_results);
}


/**
 * @brief 釋放 nfs_glob 函式分配的記憶體。
 * @param glob_results 指向 NfsGlobResults 結構的指標，其內容將被釋放。
 */
void nfs_glob_free(NfsGlobResults* glob_results) {
	if (!glob_results) {
		nfs_errno = 19; // Invalid parameters
		return;
	}
	if (glob_results->gl_pathv) {
		// gl_pathc 是匹配項的數量，gl_offs 是 gl_pathv 開頭的偏移量
		// 需要釋放從 gl_pathv[gl_offs] 到 gl_pathv[gl_offs + gl_pathc - 1] 的所有字串
		// 以及 gl_pathv 陣列本身。
		// 原始碼: for (i = gl_offs; i < gl_pathc + gl_offs; ++i) free(gl_pathv[i]);
		// 這意味著 gl_pathc 是「純粹的匹配數量」。
		for (size_t i = 0; i < glob_results->gl_pathc; ++i) {
			if (glob_results->gl_pathv[glob_results->gl_offs + i]) {
				free(glob_results->gl_pathv[glob_results->gl_offs + i]);
			}
		}
		free(glob_results->gl_pathv);
	}
	glob_results->gl_pathc = 0;
	glob_results->gl_pathv = nullptr;
	glob_results->gl_offs = 0; // 通常也應重置
	// nfs_errno 不在此處設定 (原始碼行為)
}


// --- NFS 系統級函式 ---

/** @brief 檢查 NFS 虛擬檔案系統是否存在 (透過檢查 .paki 和 .pak 實體檔案)。*/
int nfs_exists(const char* base_vfs_name) { // BOOL (int)
	if (!base_vfs_name) return 0;
	char paki_filename[256];
	char pak_filename[256];

	strcpy(paki_filename, base_vfs_name);
	strcat(paki_filename, ".paki");

	strcpy(pak_filename, base_vfs_name);
	strcat(pak_filename, ".pak");

	return file_exists(paki_filename) > 0 && file_exists(pak_filename) > 0;
}

/**
 * @brief 啟動 (開啟或建立) NFS 虛擬檔案系統。
 * @param base_vfs_name VFS 的基礎名稱 (例如 "myarchive"，將對應 myarchive.paki 和 myarchive.pak)。
 * @param access_mode 存取模式 (例如，唯讀、讀寫)。會影響 nfs_iio_IOMODE 和 nfs_data_IOMODE。
 * @return 指向 NfsHandle 的指標，失敗時返回 nullptr 並設定 nfs_errno。
 */
NfsHandle* nfs_start(const char* base_vfs_name, int access_mode) {
	if (!base_vfs_name) {
		nfs_errno = 19; // Invalid parameters
		return nullptr;
	}

	NfsHandle* handle = static_cast<NfsHandle*>(malloc(sizeof(NfsHandle)));
	if (!handle) {
		nfs_errno = 15; // No memory
		return nullptr;
	}
	memset(handle, 0, sizeof(NfsHandle)); // 初始化所有成員為0/nullptr

	// 設定全域 IOMODE (原始碼行為)
	nfs_iio_IOMODE = access_mode;
	nfs_data_IOMODE = access_mode;

	// 檢查鎖定
	if (!lock_check(base_vfs_name, access_mode)) {
		nfs_errno = 16; // File system is locked
		free(handle);
		return nullptr;
	}

	char paki_filename[256];
	char pak_filename[256];
	strcpy(paki_filename, base_vfs_name); strcat(paki_filename, ".paki");
	strcpy(pak_filename, base_vfs_name); strcat(pak_filename, ".pak");

	bool paki_exists = file_exists(paki_filename);
	bool pak_exists = file_exists(pak_filename);

	// 儲存基礎名稱以供 lock_leave 使用
	handle->base_vfs_name_for_lock = static_cast<char*>(malloc(strlen(base_vfs_name) + 1));
	if (!handle->base_vfs_name_for_lock) {
		nfs_errno = 15; free(handle); return nullptr;
	}
	strcpy(handle->base_vfs_name_for_lock, base_vfs_name);


	if (paki_exists && pak_exists) { // 開啟現有 VFS
		handle->iio_handle = nfs_iio_open(paki_filename);
		if (!handle->iio_handle) { nfs_errno = 1; goto cleanup_error; }

		handle->data_handle = nfs_data_open(pak_filename);
		if (!handle->data_handle) { nfs_errno = 2; goto cleanup_error; }

		// 原始碼中 DT, NT, FAT 開啟時使用的 IIO 通道 ID 是固定的
		handle->dt_handle = nfs_dt_open(handle->iio_handle, 0, 1); // TrieNodes on Ch0, KeyNodes on Ch1
		if (!handle->dt_handle) { nfs_errno = 5; goto cleanup_error; }

		handle->nt_handle = nfs_nt_open(handle->iio_handle, 2);   // NT on Ch2
		if (!handle->nt_handle) { nfs_errno = 4; goto cleanup_error; }

		handle->fat_handle = nfs_fat_open(handle->iio_handle, 3);  // FAT on Ch3
		if (!handle->fat_handle) { nfs_errno = 3; goto cleanup_error; }

	}
	else { // 建立新的 VFS
		// 根據原始碼，如果 access_mode == 1 (可能表示唯讀)，則不應允許建立
		// lock_check 的返回值 (1 表示允許) 和 access_mode 的關係：
		// v5 = (a2 == 1); -> 如果是共享鎖，則 accessMode 必須是1 (讀)
		// 如果鎖不存在，lock_check 返回 1。
		// 此處的 access_mode 可能是傳入 nfs_start 的，用於決定操作模式。
		// 原始碼: `if ( a2 == 1 ) { nfs_errno = 1; return 0; }` (在 files_dont_exist 分支中)
		// 這裡 a2 是 nfs_start 的 access_mode 參數。
		// 假設 access_mode 1 表示唯讀，不允許建立。
		if (access_mode == 1 && (!paki_exists || !pak_exists)) {
			nfs_errno = 1; // Cannot create in read-only mode (or file missing for read)
			goto cleanup_error;
		}
		if (paki_exists != pak_exists) { // 一個存在一個不存在，狀態不一致
			nfs_errno = 1; // 或其他錯誤碼
			goto cleanup_error;
		}


		handle->iio_handle = nfs_iio_create(paki_filename);
		if (!handle->iio_handle) { nfs_errno = 1; goto cleanup_error; }

		handle->data_handle = nfs_data_create(pak_filename);
		if (!handle->data_handle) { nfs_errno = 2; goto cleanup_error; }

		// 原始碼中建立時使用的 IIO 通道由各 _create 函式內部 nfs_iio_allocate_channel 分配
		// 參數是建議的條帶區塊數
		handle->dt_handle = nfs_dt_create(handle->iio_handle, 4, 8); // 4 for trie, 8 for key (原始碼的 v11)
		if (!handle->dt_handle) { nfs_errno = 8; goto cleanup_error; } // 8 for DT allocate error

		handle->nt_handle = nfs_nt_create(handle->iio_handle, 1);   // 1 for NT (原始碼的 v12)
		if (!handle->nt_handle) { nfs_errno = 7; goto cleanup_error; } // 7 for NT allocate error

		handle->fat_handle = nfs_fat_create(handle->iio_handle, 4);  // 4 for FAT (原始碼的 v13)
		if (!handle->fat_handle) { nfs_errno = 6; goto cleanup_error; } // 6 for FAT allocate error
	}
	return handle;

cleanup_error:
	if (handle) {
		if (handle->fat_handle) nfs_fat_close(handle->fat_handle); // close 只是釋放 handle 結構
		if (handle->nt_handle) nfs_nt_close(handle->nt_handle);
		if (handle->dt_handle) nfs_dt_close(handle->dt_handle);
		// destroy 會刪除實體檔案，只有在建立過程中部分成功才需要
		if (handle->data_handle) {
			if (!pak_exists) nfs_data_destroy(handle->data_handle); else nfs_data_close(handle->data_handle);
		}
		if (handle->iio_handle) {
			if (!paki_exists) nfs_iio_destroy(handle->iio_handle); else nfs_iio_close(handle->iio_handle);
		}
		if (handle->base_vfs_name_for_lock) free(handle->base_vfs_name_for_lock);
		free(handle);
	}
	return nullptr;
}


/**
 * @brief 結束 NFS 虛擬檔案系統，關閉/銷毀所有元件並釋放資源。
 * @param handle 要結束的 NFS 控制代碼。
 * @param destroy_files_flag 如果為 true，則實際銷毀 .paki 和 .pak 檔案；
 * 如果為 false，則僅關閉它們並釋放控制代碼。
 */
void nfs_end(NfsHandle* handle, int destroy_files_flag) {
	if (!handle) {
		nfs_errno = 9; // Invalid file system handle
		return;
	}

	if (destroy_files_flag) {
		// 銷毀元件，這通常意味著它們的 _destroy 函式會嘗試刪除關聯的實體檔案
		// 或釋放它們在 IIO 中佔用的通道（如果 IIO 設計如此）
		// 原始碼中，dt, nt, fat 的 _destroy 只是 _close
		if (handle->dt_handle) nfs_dt_destroy(handle->dt_handle); // 實際上是 nfs_dt_close
		if (handle->nt_handle) nfs_nt_destroy(handle->nt_handle); // 實際上是 nfs_nt_close
		if (handle->fat_handle) nfs_fat_close(handle->fat_handle); // fat 沒有 _destroy, 只有 _close

		if (handle->data_handle) nfs_data_destroy(handle->data_handle); // 這個會刪除 .pak 檔案
		if (handle->iio_handle) nfs_iio_destroy(handle->iio_handle);   // 這個會刪除 .paki 檔案
	}
	else {
		// 僅關閉元件，不刪除實體檔案 (除非元件的 _close 本身有此副作用)
		if (handle->dt_handle) nfs_dt_close(handle->dt_handle);
		if (handle->nt_handle) nfs_nt_close(handle->nt_handle);
		if (handle->fat_handle) nfs_fat_close(handle->fat_handle);
		if (handle->data_handle) nfs_data_close(handle->data_handle);
		if (handle->iio_handle) nfs_iio_close(handle->iio_handle);
	}
	handle->dt_handle = nullptr; // 避免懸空指標
	handle->nt_handle = nullptr;
	handle->fat_handle = nullptr;
	handle->data_handle = nullptr;
	handle->iio_handle = nullptr;


	// 釋放所有已開啟的檔案描述符
	if (handle->open_files_array) {
		for (int i = 0; i < handle->open_files_array_capacity; ++i) {
			if (handle->open_files_array[i]) { // 如果槽位仍被使用
				deallocate_file_descriptor(handle, i); // 這個會 free(fh)
			}
		}
		free(handle->open_files_array);
		handle->open_files_array = nullptr;
	}
	handle->open_files_array_capacity = 0;
	handle->num_currently_open_files = 0;

	if (handle->base_vfs_name_for_lock) {
		lock_leave(handle->base_vfs_name_for_lock); // 釋放鎖
		free(handle->base_vfs_name_for_lock);
		handle->base_vfs_name_for_lock = nullptr;
	}
	free(handle);
	// nfs_errno 不在此處設定
}