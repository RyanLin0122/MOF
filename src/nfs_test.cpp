#include <cassert> // 可以使用 assert 進行簡單斷言
#include <filesystem> // C++17, 用於檔案系統操作 (如刪除測試檔案)
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <nfs.h>
#include <nfs_test.h>


// 簡單的測試結果記錄
static int tests_passed = 0;
static int tests_failed = 0;
// 測試檔案的基礎名稱
const char* TEST_VFS_BASENAME = "test_vfs_main";
const char* TEST_PAKI_FILENAME = "test_vfs_main.paki";
const char* TEST_PAK_FILENAME = "test_vfs_main.pak";
const char* LOCK_FILE_BASENAME_UNIT_TEST = "test_lock_unit";

// 單元測試
void test_bit_operations();

#define RUN_TEST(test_func) \
    do { \
        std::cout << "Running test: " #test_func "..." << std::endl; \
        try { \
            test_func(); \
            tests_passed++; \
            std::cout << #test_func << " ... PASSED" << std::endl; \
        } catch (const std::exception& e) { \
            tests_failed++; \
            std::cout << #test_func << " ... FAILED: " << e.what() << std::endl; \
        } catch (...) { \
            tests_failed++; \
            std::cout << #test_func << " ... FAILED: Unknown exception" << std::endl; \
        } \
        std::cout << "----------------------------------------" << std::endl; \
    } while(0)


// 清理測試產生的檔案
void cleanup_test_files(const std::string& base_name) {
	std::filesystem::remove(base_name + ".paki");
	std::filesystem::remove(base_name + ".pak");
	std::filesystem::remove(base_name + ".lock");
}

void cleanup_lock_file(const std::string& base_name) {
	std::filesystem::remove(base_name + ".lock");
}

void test_bit_operations() {
	// Test bit_get
	const char bytes1[] = { 0x01, 0x02, (char)0x80 }; // 00000001 00000010 10000000
	assert(bit_get(bytes1, 0) == 1);
	assert(bit_get(bytes1, 1) == 0);
	assert(bit_get(bytes1, 7) == 0);
	assert(bit_get(bytes1, 8) == 0);
	assert(bit_get(bytes1, 9) == 1);
	assert(bit_get(bytes1, 15) == 0);
	assert(bit_get(bytes1, 16) == 0);
	assert(bit_get(bytes1, 23) == 1);
	assert(bit_get(bytes1, -1) == 2); // 根據原始碼

	// Test bitfirst_different
	const char* s1_a = "apple";
	const char* s1_b = "apply"; // 'e' vs 'y' at index 4
	// 'e' = 01100101, 'y' = 01111001. First diff at 5th bit of char (bit index 4)
	// byteIndex = 4. local_bit_offset = 2 (0110*0*101 vs 0111*1*001)
	// Global bit index = 4*8 + 2 = 32 + 2 = 34
	assert(bitfirst_different(s1_a, s1_b) == (4 * 8 + 2));


	// s2: "test" vs "text"，在 byteIndex = 2 ('s' vs 'x')
	// 's' = 0x73 = 01110011₂ → bit0=1, bit1=1, bit2=0, bit3=0, bit4=1, bit5=1, bit6=1, bit7=0
	// 'x' = 0x78 = 01111000₂ → bit0=0, bit1=0, bit2=0, bit3=1, bit4=1, bit5=1, bit6=1, bit7=0
	// 第一個不同位元在 offset = 0 → 全域索引 = 2*8 + 0 = 16
	const char* s2_a = "test";
	const char* s2_b = "text";
	assert(bitfirst_different(s2_a, s2_b) == (2 * 8 + 0));

	// s3: "abc" vs "abd"，在 byteIndex = 2 ('c' vs 'd')
	// 'c' = 0x63 = 01100011₂ → bit0=1, bit1=1, bit2=0, ...
	// 'd' = 0x64 = 01100100₂ → bit0=0, bit1=0, bit2=1, ...
	// 第一個不同位元在 offset = 0 → 全域索引 = 2*8 + 0 = 16
	const char* s3_a = "abc";
	const char* s3_b = "abd";
	assert(bitfirst_different(s3_a, s3_b) == (2 * 8 + 0));

	// s4: "\x01" vs "\x00"
	// 0x01 → bit0=1；0x00 → bit0=0
	// 第一個不同位元在 offset = 0 → 全域索引 = 0
	const char* s4_a = "\x01";
	const char* s4_b = "\x00";
	assert(bitfirst_different(s4_a, s4_b) == 0);

	// s5: "\x02" vs "\x00"
	// 0x02 → bit1=1；0x00 → bit1=0；bit0 相同為 0
	// 第一個不同位元在 offset = 1 → 全域索引 = 1
	const char* s5_a = "\x02";
	const char* s5_b = "\x00";
	assert(bitfirst_different(s5_a, s5_b) == 1);
}

// 輔助函數：創建一個 lock 檔案
// lockFlags: 4 位元組的鎖定標誌
// lockCount: 鎖定計數
bool create_test_lock_file(const char* baseName, const char* lockFlags, int lockCount) {
    char fileName[256];
    strcpy(fileName, baseName);
    strcat(fileName, ".lock");
    FILE* f = fopen(fileName, "wb");
    if (!f) {
        std::cerr << "Failed to create test lock file: " << fileName << std::endl;
        return false;
    }
    if (fwrite(lockFlags, 1, 4, f) != 4) {
        std::cerr << "Failed to write lockFlags to " << fileName << std::endl;
        fclose(f);
        remove(fileName);
        return false;
    }
    if (fwrite(&lockCount, sizeof(int), 1, f) != 1) {
        std::cerr << "Failed to write lockCount to " << fileName << std::endl;
        fclose(f);
        remove(fileName);
        return false;
    }
    fclose(f);
    return true;
}

// 輔助函數：讀取 lock 檔案的計數
int read_lock_file_count(const char* baseName) {
    char fileName[256];
    strcpy(fileName, baseName);
    strcat(fileName, ".lock");
    FILE* f = fopen(fileName, "rb");
    if (!f) {
        return -999; // 表示檔案不存在或無法開啟
    }
    char flags[4];
    int count;
    if (fread(flags, 1, 4, f) != 4) {
        fclose(f);
        return -998; // 讀取標誌錯誤
    }
    if (fread(&count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return -997; // 讀取計數錯誤
    }
    fclose(f);
    return count;
}


// --- lock_check 測試 ---
void test_lock_check_no_lock_file() {
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST); // 確保檔案不存在
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 1); // 檔案不存在，應可存取 (讀取模式)
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 0) == 1); // 檔案不存在，應可存取 (非讀取模式)
}

void test_lock_check_exclusive_lock() {
    // lockFlags[0] bit 1 (0x02) set -> 獨佔鎖
    char flags[] = { 0x02, 0x00, 0x00, 0x00 }; // 獨佔鎖 (位元1設為1)
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, 1));
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 0); // 獨佔鎖，不可存取 (即使請求讀取)
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 0) == 0); // 獨佔鎖，不可存取
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
}

void test_lock_check_not_readable_lock() {
    // lockFlags[0] bit 0 (0x01) clear -> 不可讀
    char flags[] = { 0x00, 0x00, 0x00, 0x00 }; // 不可讀 (位元0設為0)
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, 1));
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 0); // 不可讀，不可存取
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);

    char flags2[] = { 0x04, 0x00, 0x00, 0x00 }; // 位元0為0，其他位元不影響此判斷
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags2, 1));
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 0);
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
}

void test_lock_check_shared_lock_readable() {
    // lockFlags[0] bit 1 (0x02) clear AND bit 0 (0x01) set -> 共享鎖且可讀
    char flags[] = { 0x01, 0x00, 0x00, 0x00 }; // 共享且可讀
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, 1));
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 1); // 共享鎖，請求讀取模式 (1)，應可存取
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 0) == 0); // 共享鎖，請求非讀取模式 (0)，應不可存取
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 2) == 0); // 共享鎖，請求其他模式 (2)，應不可存取
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);

    char flags2[] = { 0x05, 0x00, 0x00, 0x00 }; // 00000101 (bit0=1, bit2=1, bit1=0) -> 共享且可讀
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags2, 1));
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 1);
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 0) == 0);
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
}

void test_lock_check_read_error() {
    // 創建一個不完整的 lock 檔案 (例如只有2個位元組)
    char fileName[256];
    strcpy(fileName, LOCK_FILE_BASENAME_UNIT_TEST);
    strcat(fileName, ".lock");
    FILE* f = fopen(fileName, "wb");
    assert(f != nullptr);
    char short_flags[] = { 0x01, 0x00 }; // 只有2位元組
    fwrite(short_flags, 1, sizeof(short_flags), f);
    fclose(f);
    assert(lock_check(LOCK_FILE_BASENAME_UNIT_TEST, 1) == 0); // 讀取錯誤，應視為不可存取
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
}


// --- lock_leave 測試 ---
void test_lock_leave_no_lock_file() {
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == -1); // 檔案不存在，應返回 -1
}

void test_lock_leave_decrement_count() {
    char flags[] = { 0x01, 0x00, 0x00, 0x00 }; // 共享鎖
    int initial_count = 3;
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, initial_count));

    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == 0); // 第一次離開
    assert(read_lock_file_count(LOCK_FILE_BASENAME_UNIT_TEST) == (initial_count - 1));
    assert(file_exists((std::string(LOCK_FILE_BASENAME_UNIT_TEST) + ".lock").c_str())); // 檔案應仍然存在

    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == 0); // 第二次離開
    assert(read_lock_file_count(LOCK_FILE_BASENAME_UNIT_TEST) == (initial_count - 2));
    assert(file_exists((std::string(LOCK_FILE_BASENAME_UNIT_TEST) + ".lock").c_str()));

    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
}

void test_lock_leave_remove_on_zero_count() {
    char flags[] = { 0x01, 0x00, 0x00, 0x00 };
    int initial_count = 1;
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, initial_count));

    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == 0); // 離開，計數變為0，檔案被移除
    assert(read_lock_file_count(LOCK_FILE_BASENAME_UNIT_TEST) == -999); // -999 表示檔案不存在
    assert(!file_exists((std::string(LOCK_FILE_BASENAME_UNIT_TEST) + ".lock").c_str())); // 檔案應已被移除

    // 再次創建檔案，計數為 0
    assert(create_test_lock_file(LOCK_FILE_BASENAME_UNIT_TEST, flags, 0));
    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == 0); // 離開，計數變為-1，檔案被移除
    assert(!file_exists((std::string(LOCK_FILE_BASENAME_UNIT_TEST) + ".lock").c_str()));

    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST); // 確保清理
}

void test_lock_leave_read_write_errors() {
    // 測試讀取標誌錯誤 (不完整檔案)
    char fileName[256];
    strcpy(fileName, LOCK_FILE_BASENAME_UNIT_TEST);
    strcat(fileName, ".lock");
    FILE* f = fopen(fileName, "wb");
    assert(f != nullptr);
    char short_content[] = { 0x01 }; // 只有1位元組
    fwrite(short_content, 1, sizeof(short_content), f);
    fclose(f);
    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == -1); // 讀取標誌錯誤
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);

    // 測試讀取計數錯誤 (只有標誌，沒有計數)
    char flags_only[] = { 0x01, 0x00, 0x00, 0x00 };
    f = fopen(fileName, "wb");
    assert(f != nullptr);
    fwrite(flags_only, 1, sizeof(flags_only), f);
    fclose(f);
    assert(lock_leave(LOCK_FILE_BASENAME_UNIT_TEST) == -1); // 讀取計數錯誤
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);

    // 寫入錯誤的測試比較難模擬，因為 fclose 通常會處理緩衝區刷新。
    // fseek 錯誤也可以測試，但需要更底層的控制或特定檔案系統狀態。
}

// 簡易創建 NfsIioCachePage
// 注意：此處的 buffer 僅為一個 uintptr_t 值，並非實際分配的記憶體，
// 因為這些底層函數主要測試指標的 LSB 操作。
NfsIioCachePage* helper_create_cache_page(uintptr_t buffer_val, int disk_pos, int access_time) {
    NfsIioCachePage* page = (NfsIioCachePage*)malloc(sizeof(NfsIioCachePage));
    if (!page) {
        std::cerr << "helper_create_cache_page: malloc failed for NfsIioCachePage" << std::endl;
        return nullptr;
    }
    page->buffer_ptr_and_syncflag_storage = buffer_val; // 直接賦值
    page->disk_block_position = disk_pos;
    page->last_access_time = access_time;
    // sync 狀態由 buffer_val 的 LSB 決定，或由 cache_page_set_sync 設定
    return page;
}

void helper_free_cache_page(NfsIioCachePage* page) {
    if (page) {
        // 在此輔助函數中，我們不釋放 page->buffer_ptr_and_syncflag_storage 指向的 "buffer"
        // 因為它在測試中通常是模擬的指標值。
        free(page);
    }
}

// 簡易創建 NfsIioChannel，包含一個 NfsIioCache
NfsIioChannel* helper_create_channel_with_cache(int num_pages_allocated_in_cache, int blocks_per_stripe_val = 1) {
    NfsIioChannel* channel = (NfsIioChannel*)malloc(sizeof(NfsIioChannel));
    if (!channel) {
        std::cerr << "helper_create_channel_with_cache: malloc failed for NfsIioChannel" << std::endl;
        return nullptr;
    }
    channel->blocks_per_stripe = blocks_per_stripe_val;
    channel->current_size_bytes = 0;
    channel->current_seek_position = 0;
    channel->cache_header = (NfsIioCache*)malloc(sizeof(NfsIioCache));
    if (!channel->cache_header) {
        std::cerr << "helper_create_channel_with_cache: malloc failed for NfsIioCache" << std::endl;
        free(channel);
        return nullptr;
    }
    channel->cache_header->num_pages_active = 0; // 初始化 active pages
    channel->cache_header->num_pages_allocated = 0;
    channel->cache_header->pages = nullptr;

    if (num_pages_allocated_in_cache > 0) {
        channel->cache_header->pages = (NfsIioCachePage**)malloc(sizeof(NfsIioCachePage*) * num_pages_allocated_in_cache);
        if (!channel->cache_header->pages) {
            std::cerr << "helper_create_channel_with_cache: malloc failed for pages array" << std::endl;
            free(channel->cache_header);
            free(channel);
            return nullptr;
        }
        channel->cache_header->num_pages_allocated = num_pages_allocated_in_cache;
        for (int i = 0; i < num_pages_allocated_in_cache; ++i) {
            channel->cache_header->pages[i] = nullptr; // 重要：初始化為 nullptr
        }
    }
    return channel;
}

void helper_free_channel_with_cache(NfsIioChannel* channel) {
    if (!channel) return;
    if (channel->cache_header) {
        if (channel->cache_header->pages) {
            for (int i = 0; i < channel->cache_header->num_pages_allocated; ++i) {
                if (channel->cache_header->pages[i]) {
                    helper_free_cache_page(channel->cache_header->pages[i]);
                }
            }
            free(channel->cache_header->pages);
        }
        free(channel->cache_header);
    }
    free(channel);
}

// 簡易創建 NfsIioFile
NfsIioFile* helper_create_iio_file(short num_channels) {
    NfsIioFile* file = (NfsIioFile*)malloc(sizeof(NfsIioFile));
    if (!file) {
        std::cerr << "helper_create_iio_file: malloc failed for NfsIioFile" << std::endl;
        return nullptr;
    }
    file->file_handle = nullptr; // 測試底層 cache 函數時通常不需要實際檔案句柄
    file->file_name = nullptr;
    file->num_channels = num_channels;
    if (num_channels > 0) {
        file->channels = (NfsIioChannel**)malloc(sizeof(NfsIioChannel*) * num_channels);
        if (!file->channels) {
            std::cerr << "helper_create_iio_file: malloc failed for channels array" << std::endl;
            free(file);
            return nullptr;
        }
        for (short i = 0; i < num_channels; ++i) {
            file->channels[i] = nullptr; // 初始化
        }
    }
    else {
        file->channels = nullptr;
    }
    // nfs_iio_BLOCK_SIZEv 和 nfs_iio_CLOCK 是全域變數，測試前可以按需設定。
    // 假設 nfs_iio_BLOCK_SIZEv 已被初始化 (例如在 nfs.cpp 中有預設值)。
    return file;
}

void helper_free_iio_file(NfsIioFile* file) {
    if (!file) return;
    if (file->channels) {
        for (short i = 0; i < file->num_channels; ++i) {
            if (file->channels[i]) {
                helper_free_channel_with_cache(file->channels[i]);
            }
        }
        free(file->channels);
    }
    // file->file_name 通常在這些輔助創建的版本中為 nullptr
    free(file);
}
// --- Tests for cache_page_get_buffer, cache_page_set_sync, cache_page_get_sync ---

void test_cache_page_sync_and_buffer_operations() {
    NfsIioCachePage* page = nullptr;
    uintptr_t mock_buffer_address = 0x1000; // 選擇一個偶數地址作為模擬緩衝區

    // 測試 page 為 nullptr 的情況
    assert(cache_page_get_buffer(nullptr) == nullptr);
    cache_page_set_sync(nullptr, 1); // 應該不執行任何操作且不崩潰
    assert(cache_page_get_sync(nullptr) == 0); // 根據實現，page為nullptr時返回0

    page = helper_create_cache_page(mock_buffer_address, 0, 0); // 初始 LSB 為 0 (dirty)
    assert(page != nullptr);

    // 測試初始狀態 (假設 helper_create_cache_page 將 LSB 設為 0)
    // 或者我們手動設定
    page->buffer_ptr_and_syncflag_storage = mock_buffer_address; // LSB is 0
    assert(cache_page_get_sync(page) == 0); // 應為 dirty (0)
    assert(reinterpret_cast<uintptr_t>(cache_page_get_buffer(page)) == mock_buffer_address);

    // 測試 cache_page_set_sync
    cache_page_set_sync(page, 1); // 設定為 clean (1)
    assert(cache_page_get_sync(page) == 1);
    assert((page->buffer_ptr_and_syncflag_storage & 1) == 1); // 檢查 LSB
    assert(reinterpret_cast<uintptr_t>(cache_page_get_buffer(page)) == mock_buffer_address); // buffer 地址應不變

    cache_page_set_sync(page, 0); // 設定為 dirty (0)
    assert(cache_page_get_sync(page) == 0);
    assert((page->buffer_ptr_and_syncflag_storage & 1) == 0); // 檢查 LSB
    assert(reinterpret_cast<uintptr_t>(cache_page_get_buffer(page)) == mock_buffer_address);

    cache_page_set_sync(page, 2); // 設定為 dirty (偶數 -> 0)
    assert(cache_page_get_sync(page) == 0);
    assert((page->buffer_ptr_and_syncflag_storage & 1) == 0);

    cache_page_set_sync(page, 3); // 設定為 clean (奇數 -> 1)
    assert(cache_page_get_sync(page) == 1);
    assert((page->buffer_ptr_and_syncflag_storage & 1) == 1);

    // 測試帶有奇數地址的緩衝區 (雖然實際上指標不應該是奇數，但為了測試 LSB 操作的穩健性)
    uintptr_t mock_odd_buffer_address = 0x1001;
    page->buffer_ptr_and_syncflag_storage = mock_odd_buffer_address; // LSB is 1
    assert(cache_page_get_sync(page) == 1); // 應為 clean (1)
    assert(reinterpret_cast<uintptr_t>(cache_page_get_buffer(page)) == (mock_odd_buffer_address & ~static_cast<uintptr_t>(1))); // LSB 應被清除

    cache_page_set_sync(page, 0); // 設定為 dirty (0)
    assert(cache_page_get_sync(page) == 0);
    // 即使原始地址是奇數，set_sync(0) 也應該將 LSB 設為 0
    assert((page->buffer_ptr_and_syncflag_storage & 1) == 0);
    // 獲取 buffer 時，原始地址的 LSB (可能是1) 會被清除，再加上 set_sync(0) 使 LSB 為0，結果應為 mock_odd_buffer_address & ~1
    assert(reinterpret_cast<uintptr_t>(cache_page_get_buffer(page)) == (mock_odd_buffer_address & ~static_cast<uintptr_t>(1)));

    helper_free_cache_page(page);
}
// --- Tests for cache_page_choose_best_to_reuse ---

void test_cache_page_choose_best_to_reuse_basic_lru() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(3); // Cache with 3 allocated page slots
    assert(file->channels[0] != nullptr && file->channels[0]->cache_header != nullptr);

    NfsIioCache* cache = file->channels[0]->cache_header;
    cache->pages[0] = helper_create_cache_page(0x1000, 10, 200); // access_time = 200
    cache->pages[1] = helper_create_cache_page(0x2000, 20, 100); // access_time = 100 (LRU)
    cache->pages[2] = helper_create_cache_page(0x3000, 30, 300); // access_time = 300
    cache->num_pages_allocated = 3; // 實際在用的頁面數
    cache->num_pages_active = 3;


    nfs_iio_CLOCK = 400; // 模擬全局時鐘
    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 1); // page[1] 是 LRU

    helper_free_iio_file(file);
}

void test_cache_page_choose_best_to_reuse_with_exclusion() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(3);
    assert(file->channels[0] != nullptr);

    NfsIioCache* cache = file->channels[0]->cache_header;
    cache->pages[0] = helper_create_cache_page(0x1000, 10, 200);
    cache->pages[1] = helper_create_cache_page(0x2000, 20, 100); // LRU
    cache->pages[2] = helper_create_cache_page(0x3000, 30, 300);
    cache->num_pages_allocated = 3;
    cache->num_pages_active = 3;

    assert(cache_page_choose_best_to_reuse(file, 0, 1) == 0); // Exclude page[1], so page[0] (time 200) is next LRU

    helper_free_iio_file(file);
}

void test_cache_page_choose_best_to_reuse_single_page() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(1);
    assert(file->channels[0] != nullptr);

    NfsIioCache* cache = file->channels[0]->cache_header;
    cache->pages[0] = helper_create_cache_page(0x1000, 10, 200);
    cache->num_pages_allocated = 1;
    cache->num_pages_active = 1;


    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 0); // Only one page, should be chosen
    assert(cache_page_choose_best_to_reuse(file, 0, 0) == 0); // Exclude the only page, still returns 0 (original behavior if no other choice)

    helper_free_iio_file(file);
}

void test_cache_page_choose_best_to_reuse_empty_cache() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0); // No pages allocated
    assert(file->channels[0] != nullptr);
    file->channels[0]->cache_header->num_pages_allocated = 0;
    file->channels[0]->cache_header->num_pages_active = 0;


    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 0); // Empty cache, should return 0 (default best_idx)

    // Test with allocated slots but all are nullptr
    helper_free_channel_with_cache(file->channels[0]);
    file->channels[0] = helper_create_channel_with_cache(3);
    assert(file->channels[0] != nullptr);
    file->channels[0]->cache_header->num_pages_allocated = 3; // Slots allocated but pages are nullptr
    file->channels[0]->cache_header->num_pages_active = 0;


    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 0);

    helper_free_iio_file(file);
}

void test_cache_page_choose_best_to_reuse_null_inputs() {
    assert(cache_page_choose_best_to_reuse(nullptr, 0, -1) == 0); // Null file

    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    // Channel at index 0 is nullptr
    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 0);

    file->channels[0] = helper_create_channel_with_cache(1);
    assert(file->channels[0] != nullptr);
    free(file->channels[0]->cache_header); // Make cache_header nullptr
    file->channels[0]->cache_header = nullptr;
    assert(cache_page_choose_best_to_reuse(file, 0, -1) == 0);

    helper_free_iio_file(file); // Will handle the partially freed channel
}

// --- Tests for cache_expand ---

void test_cache_expand_initial_allocation() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = (NfsIioChannel*)malloc(sizeof(NfsIioChannel)); // Create channel but no cache_header yet
    assert(file->channels[0] != nullptr);
    file->channels[0]->cache_header = nullptr;
    file->channels[0]->blocks_per_stripe = 1; // Needed by cache_expand if it calls other fns

    int new_size = cache_expand(file, 0, 5); // required_page_array_idx = 5
    // Expected: 1 -> 2 -> 4 -> 8 -> 16 ... until >= 1024 (since 5 < 1024)
    // Actually, the loop is `while (new_allocated_count <= required || new_allocated_count < 1024)`
    // If current=0, new=1. Loop1: 1<=5 OR 1<1024 -> new=2. Loop2: 2<=5 OR 2<1024 -> new=4.
    // Loop3: 4<=5 OR 4<1024 -> new=8. ... LoopN: 512<=5 (F) OR 512<1024 (T) -> new=1024.
    // LoopN+1: 1024<=5 (F) OR 1024<1024 (F) -> Loop ends. So expected is 1024.
    assert(new_size == 1024);
    assert(file->channels[0]->cache_header != nullptr);
    assert(file->channels[0]->cache_header->num_pages_allocated == 1024);
    assert(file->channels[0]->cache_header->pages != nullptr);
    for (int i = 0; i < 1024; ++i) {
        assert(file->channels[0]->cache_header->pages[i] == nullptr);
    }

    helper_free_iio_file(file);
}

void test_cache_expand_to_meet_requirement_below_1024() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0); // Channel with empty cache
    assert(file->channels[0] != nullptr);
    file->channels[0]->blocks_per_stripe = 1;


    // Initial: 0. Required: 10. Min: 1024
    // 0->1. 1<=10 or 1<1024 (T) -> 2
    // 2<=10 or 2<1024 (T) -> 4
    // 4<=10 or 4<1024 (T) -> 8
    // 8<=10 or 8<1024 (T) -> 16
    // ...
    // 512<=10 (F) or 512<1024 (T) -> 1024
    // 1024<=10 (F) or 1024<1024 (F) -> exit. Result 1024.
    int new_size = cache_expand(file, 0, 10);
    assert(new_size == 1024);
    assert(file->channels[0]->cache_header->num_pages_allocated == 1024);

    helper_free_iio_file(file);
}

void test_cache_expand_to_meet_requirement_above_1024() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0] != nullptr);
    file->channels[0]->blocks_per_stripe = 1;


    // Initial: 0. Required: 1500. Min: 1024
    // 0->1. 1<=1500 or 1<1024 (T) -> 2
    // ...
    // 1024<=1500 (T) or 1024<1024 (F) -> 2048
    // 2048<=1500 (F) or 2048<1024 (F) -> exit. Result 2048.
    int new_size = cache_expand(file, 0, 1500);
    assert(new_size == 2048);
    assert(file->channels[0]->cache_header->num_pages_allocated == 2048);

    helper_free_iio_file(file);
}

void test_cache_expand_already_sufficient() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    // Create a channel with a cache that is already large enough
    file->channels[0] = helper_create_channel_with_cache(2048); // current_allocated_count = 2048
    assert(file->channels[0] != nullptr);
    file->channels[0]->blocks_per_stripe = 1;


    // Required_idx = 500. Current_allocated = 2048.
    // Loop condition: new_allocated_count (2048) <= 500 (F) || new_allocated_count (2048) < 1024 (F) -> loop doesn't run.
    // So, it should not expand further.
    // The code: `new_allocated_count = current_allocated_count;`
    // `while (new_allocated_count <= required_page_array_idx || new_allocated_count < 1024)`
    // If current_allocated_count is 2048, required is 500.
    // 2048 <= 500 (F) || 2048 < 1024 (F). Loop condition is false.
    // So new_allocated_count remains 2048.
    int new_size = cache_expand(file, 0, 500);
    assert(new_size == 2048); // Should not expand
    assert(file->channels[0]->cache_header->num_pages_allocated == 2048);

    // Required_idx = 1500. Current_allocated = 1024.
    // new_allocated_count = 1024.
    // 1024 <= 1500 (T) || 1024 < 1024 (F) -> loop runs. new_allocated_count becomes 2048.
    // 2048 <= 1500 (F) || 2048 < 1024 (F) -> loop exits. Result 2048.
    helper_free_channel_with_cache(file->channels[0]);
    file->channels[0] = helper_create_channel_with_cache(1024);
    assert(file->channels[0] != nullptr);
    file->channels[0]->blocks_per_stripe = 1;

    new_size = cache_expand(file, 0, 1500);
    assert(new_size == 2048);
    assert(file->channels[0]->cache_header->num_pages_allocated == 2048);


    helper_free_iio_file(file);
}

void test_cache_expand_realloc_initialization() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(2); // Start with 2 pages
    assert(file->channels[0] != nullptr);
    file->channels[0]->blocks_per_stripe = 1;

    // Fill the first two slots to check if they are preserved and new ones are null
    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x100, 1, 1);
    file->channels[0]->cache_header->pages[1] = helper_create_cache_page(0x200, 2, 2);

    // Expand to require index 5, which will result in allocation of 1024 (as per logic)
    int new_size = cache_expand(file, 0, 5);
    assert(new_size == 1024);
    assert(file->channels[0]->cache_header->num_pages_allocated == 1024);
    assert(file->channels[0]->cache_header->pages[0] != nullptr); // Should be preserved
    assert(file->channels[0]->cache_header->pages[1] != nullptr); // Should be preserved
    for (int i = 2; i < 1024; ++i) {
        assert(file->channels[0]->cache_header->pages[i] == nullptr); // New slots should be null
    }

    helper_free_iio_file(file); // This will free pages[0] and pages[1] as well
}

void test_cache_expand_null_inputs() {
    assert(cache_expand(nullptr, 0, 5) == 0);

    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    // Channel at index 0 is nullptr
    assert(cache_expand(file, 0, 5) == 0);

    helper_free_iio_file(file);
}

// 測試檔案的名稱
const char* TEST_ABS_IO_FILENAME = "test_abs_io.tmp";
const char* TEST_WRITE_HEADER_FILENAME = "test_header.tmp";

// 全域 nfs_iio_BLOCK_SIZEv 的測試值，可以按需調整
// 為了測試方便，可以設定一個較小的值，例如 512 或 1024
// 但為了與 nfs.cpp 中的 header_size() (目前返回 nfs_iio_BLOCK_SIZEv) 行為一致，
// 測試時應明確此值。這裡假設 nfs.cpp 中的預設值為 4096，測試也基於此。
// 如果需要，可以在測試 setup 中修改它：
// extern int nfs_iio_BLOCK_SIZEv;
// nfs_iio_BLOCK_SIZEv = 512; // Example for testing with smaller block size

// 輔助函數：創建並開啟 NfsIioFile 以進行底層 I/O 測試
NfsIioFile* helper_setup_iio_file_for_abs_io(const char* filename, const char* mode, short num_chans_for_header = 0) {
    std::filesystem::remove(filename); // 清理舊檔案

    NfsIioFile* file = (NfsIioFile*)malloc(sizeof(NfsIioFile));
    if (!file) {
        std::cerr << "helper_setup_iio_file_for_abs_io: Failed to alloc NfsIioFile" << std::endl;
        return nullptr;
    }

    file->file_name = (char*)malloc(strlen(filename) + 1);
    if (!file->file_name) {
        free(file);
        std::cerr << "helper_setup_iio_file_for_abs_io: Failed to alloc file_name" << std::endl;
        return nullptr;
    }
    strcpy(file->file_name, filename);

    file->file_handle = fopen(filename, mode);
    if (!file->file_handle) {
        free(file->file_name);
        free(file);
        std::cerr << "helper_setup_iio_file_for_abs_io: Failed to open file " << filename << " in mode " << mode << std::endl;
        return nullptr;
    }

    file->num_channels = num_chans_for_header;
    if (num_chans_for_header > 0) {
        file->channels = (NfsIioChannel**)calloc(num_chans_for_header, sizeof(NfsIioChannel*));
        if (!file->channels) {
            fclose(file->file_handle);
            free(file->file_name);
            free(file);
            std::cerr << "helper_setup_iio_file_for_abs_io: Failed to alloc channels array" << std::endl;
            return nullptr;
        }
        for (short i = 0; i < num_chans_for_header; ++i) {
            file->channels[i] = (NfsIioChannel*)calloc(1, sizeof(NfsIioChannel));
            if (!file->channels[i]) { // 清理部分分配的通道
                for (short k = 0; k < i; ++k) free(file->channels[k]);
                free(file->channels);
                fclose(file->file_handle);
                free(file->file_name);
                free(file);
                std::cerr << "helper_setup_iio_file_for_abs_io: Failed to alloc channel " << i << std::endl;
                return nullptr;
            }
            // 為 write_header 測試填充範例數據
            file->channels[i]->blocks_per_stripe = i + 1;
            file->channels[i]->current_size_bytes = (i + 1) * nfs_iio_BLOCK_SIZEv;
            file->channels[i]->current_seek_position = 0;
            file->channels[i]->cache_header = nullptr; // 這些底層測試不直接依賴 cache
        }
    }
    else {
        file->channels = nullptr;
    }
    return file;
}

// 輔助函數：關閉並清理 NfsIioFile
void helper_teardown_iio_file(NfsIioFile* file, bool remove_phys_file = true) {
    if (!file) return;
    if (file->file_handle) {
        fclose(file->file_handle);
    }
    if (remove_phys_file && file->file_name) {
        std::filesystem::remove(file->file_name);
    }
    if (file->file_name) {
        free(file->file_name);
    }
    if (file->channels) {
        for (short i = 0; i < file->num_channels; ++i) {
            if (file->channels[i]) {
                // 假設 NfsIioChannel 在此上下文中不包含需要深度釋放的 cache_header
                free(file->channels[i]);
            }
        }
        free(file->channels);
    }
    free(file);
}

// 輔助函數：填充緩衝區
void helper_fill_buffer_pattern(char* buffer, size_t size, int pattern_seed) {
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = (char)((pattern_seed + i) % 256);
    }
}

// 輔助函數：比較緩衝區
bool helper_verify_buffer_pattern(const char* buffer, size_t size, int pattern_seed) {
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] != (char)((pattern_seed + i) % 256)) {
            std::cerr << "Buffer mismatch at index " << i << ": expected "
                << (int)(unsigned char)((pattern_seed + i) % 256) << ", got " << (int)(unsigned char)buffer[i] << std::endl;
            return false;
        }
    }
    return true;
}

// --- Tests for write_absolute_block_n and read_absolute_block_n ---

void test_abs_block_io_single_block() {
    const int block_size = nfs_iio_BLOCK_SIZEv; // 使用全域的區塊大小
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b");
    assert(file != nullptr && file->file_handle != nullptr);

    std::vector<char> write_buf(block_size);
    std::vector<char> read_buf(block_size);
    helper_fill_buffer_pattern(write_buf.data(), block_size, 1);

    int target_block_idx = 0; // 寫入第一個數據區塊 (相對於 header 之後)
    int num_blocks = 1;

    // 寫入
    int bytes_written = write_absolute_block_n(file, target_block_idx, num_blocks, write_buf.data());
    assert(bytes_written == num_blocks * block_size);
    fflush(file->file_handle); // 確保寫入到磁碟

    // 讀回
    int bytes_read = read_absolute_block_n(file, target_block_idx, num_blocks, read_buf.data());
    assert(bytes_read == num_blocks * block_size);
    assert(helper_verify_buffer_pattern(read_buf.data(), block_size, 1));

    helper_teardown_iio_file(file);
}

void test_abs_block_io_multiple_blocks() {
    const int block_size = nfs_iio_BLOCK_SIZEv;
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b");
    assert(file != nullptr && file->file_handle != nullptr);

    int num_blocks = 3;
    std::vector<char> write_buf(num_blocks * block_size);
    std::vector<char> read_buf(num_blocks * block_size);
    helper_fill_buffer_pattern(write_buf.data(), write_buf.size(), 2);

    int target_block_idx = 5; // 寫入到檔案的後面一些的區塊

    // 寫入
    int bytes_written = write_absolute_block_n(file, target_block_idx, num_blocks, write_buf.data());
    assert(bytes_written == num_blocks * block_size);
    fflush(file->file_handle);

    // 讀回
    int bytes_read = read_absolute_block_n(file, target_block_idx, num_blocks, read_buf.data());
    assert(bytes_read == num_blocks * block_size);
    assert(memcmp(write_buf.data(), read_buf.data(), num_blocks * block_size) == 0);

    helper_teardown_iio_file(file);
}

void test_abs_block_io_read_past_eof() {
    const int block_size = nfs_iio_BLOCK_SIZEv;
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b");
    assert(file != nullptr && file->file_handle != nullptr);

    int num_written_blocks = 1;
    std::vector<char> write_buf(num_written_blocks * block_size);
    helper_fill_buffer_pattern(write_buf.data(), write_buf.size(), 3);

    int target_block_idx = 0;
    write_absolute_block_n(file, target_block_idx, num_written_blocks, write_buf.data());
    fflush(file->file_handle);

    // 嘗試讀取比寫入的更多的區塊
    int num_to_read = 3;
    std::vector<char> read_buf(num_to_read * block_size);
    memset(read_buf.data(), 0xFF, read_buf.size()); // 用非零值填充以檢查是否被清零

    int bytes_read = read_absolute_block_n(file, target_block_idx, num_to_read, read_buf.data());
    assert(bytes_read == num_to_read * block_size); // 函數應返回請求的大小

    // 驗證第一個區塊
    assert(memcmp(read_buf.data(), write_buf.data(), num_written_blocks * block_size) == 0);

    // 驗證超出 EOF 的部分是否被填零
    for (size_t i = num_written_blocks * block_size; i < read_buf.size(); ++i) {
        assert(read_buf[i] == 0);
    }

    helper_teardown_iio_file(file);
}

void test_abs_block_io_invalid_inputs() {
    const int block_size = nfs_iio_BLOCK_SIZEv;
    NfsIioFile* file_valid = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b");
    assert(file_valid != nullptr);
    std::vector<char> buffer(block_size);

    // file 為 nullptr
    assert(write_absolute_block_n(nullptr, 0, 1, buffer.data()) == -1);
    assert(read_absolute_block_n(nullptr, 0, 1, buffer.data()) == -1);

    // file->file_handle 為 nullptr
    NfsIioFile mock_file_no_handle = { 0 }; // 簡易模擬
    mock_file_no_handle.num_channels = 0; // 讓 header_size 不出錯
    assert(write_absolute_block_n(&mock_file_no_handle, 0, 1, buffer.data()) == -1);
    assert(read_absolute_block_n(&mock_file_no_handle, 0, 1, buffer.data()) == -1);

    // buffer 為 nullptr
    assert(write_absolute_block_n(file_valid, 0, 1, nullptr) == -1);
    assert(read_absolute_block_n(file_valid, 0, 1, nullptr) == -1);

    // num_blocks 為 0
    assert(write_absolute_block_n(file_valid, 0, 0, buffer.data()) == 0);
    assert(read_absolute_block_n(file_valid, 0, 0, buffer.data()) == 0);

    // num_blocks 為負數 (根據目前實現，等同於0)
    assert(write_absolute_block_n(file_valid, 0, -1, buffer.data()) == 0);
    assert(read_absolute_block_n(file_valid, 0, -1, buffer.data()) == 0);

    helper_teardown_iio_file(file_valid);
}
// --- Tests for write_header ---

// 輔助結構用於直接比較檔案頭部內容
#pragma pack(push, 1) // 確保緊密打包，與檔案佈局一致
struct TestFileHeaderPart {
    int magic_number;
    int num_channels_int;
};
struct TestChannelHeaderPart {
    int blocks_per_stripe;
    int current_size_bytes;
};
#pragma pack(pop)

void test_write_header_no_channels() {
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_WRITE_HEADER_FILENAME, "w+b", 0); // 0 個通道
    assert(file != nullptr && file->file_handle != nullptr);

    int result = write_header(file);
    assert(result == 0);
    fflush(file->file_handle);

    // 重新開啟並驗證
    FILE* verify_fp = fopen(TEST_WRITE_HEADER_FILENAME, "rb");
    assert(verify_fp != nullptr);

    TestFileHeaderPart fh_part;
    size_t read_count = fread(&fh_part, sizeof(TestFileHeaderPart), 1, verify_fp);
    assert(read_count == 1);
    assert(fh_part.magic_number == 1145258561); // 0x444F4941
    assert(fh_part.num_channels_int == 0);

    // 確保檔案中沒有更多的通道數據
    TestChannelHeaderPart ch_part_extra;
    read_count = fread(&ch_part_extra, sizeof(TestChannelHeaderPart), 1, verify_fp);
    assert(read_count == 0 || feof(verify_fp)); // 應該讀不到額外通道數據

    fclose(verify_fp);
    helper_teardown_iio_file(file);
}

void test_write_header_multiple_channels() {
    short num_test_channels = 3;
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_WRITE_HEADER_FILENAME, "w+b", num_test_channels);
    assert(file != nullptr && file->file_handle != nullptr);

    // helper_setup_iio_file_for_abs_io 已為通道填充範例數據
    // file->channels[0]->blocks_per_stripe = 1; file->channels[0]->current_size_bytes = 1 * nfs_iio_BLOCK_SIZEv;
    // file->channels[1]->blocks_per_stripe = 2; file->channels[1]->current_size_bytes = 2 * nfs_iio_BLOCK_SIZEv;
    // file->channels[2]->blocks_per_stripe = 3; file->channels[2]->current_size_bytes = 3 * nfs_iio_BLOCK_SIZEv;

    int result = write_header(file);
    assert(result == 0);
    fflush(file->file_handle);

    // 重新開啟並驗證
    FILE* verify_fp = fopen(TEST_WRITE_HEADER_FILENAME, "rb");
    assert(verify_fp != nullptr);

    TestFileHeaderPart fh_part;
    size_t read_count = fread(&fh_part, sizeof(TestFileHeaderPart), 1, verify_fp);
    assert(read_count == 1);
    assert(fh_part.magic_number == 1145258561);
    assert(fh_part.num_channels_int == num_test_channels);

    for (short i = 0; i < num_test_channels; ++i) {
        TestChannelHeaderPart ch_part;
        read_count = fread(&ch_part, sizeof(TestChannelHeaderPart), 1, verify_fp);
        assert(read_count == 1);
        assert(ch_part.blocks_per_stripe == file->channels[i]->blocks_per_stripe);
        assert(ch_part.current_size_bytes == file->channels[i]->current_size_bytes);
    }

    fclose(verify_fp);
    helper_teardown_iio_file(file);
}

void test_write_header_invalid_inputs() {
    assert(write_header(nullptr) == -1); // file 為 nullptr

    NfsIioFile mock_file_no_handle = { 0 }; // 簡易模擬，file_handle 為 nullptr
    mock_file_no_handle.num_channels = 0;
    assert(write_header(&mock_file_no_handle) == -1);
}

// 輔助函數：設置 channel 的屬性 (如果 helper_create_channel_with_cache 不夠用)
void helper_set_channel_properties(NfsIioChannel* channel, int blocks_stripe, int size_bytes, int seek_pos) {
    if (channel) {
        channel->blocks_per_stripe = blocks_stripe;
        channel->current_size_bytes = size_bytes;
        channel->current_seek_position = seek_pos;
    }
}

// --- Tests for nfs_iio_blocks_per_chunk ---

void test_nfs_iio_blocks_per_chunk_null_file() {
    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(nullptr) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1); // Clock should increment
}

void test_nfs_iio_blocks_per_chunk_no_channels_array() {
    NfsIioFile* file = helper_create_iio_file(0); // num_channels = 0, so file->channels is initially nullptr or empty
    assert(file != nullptr);
    if (file->channels) { // 如果 helper_create_iio_file(0) 分配了空陣列而不是 nullptr
        free(file->channels);
        file->channels = nullptr; // 確保為 nullptr
    }
    file->num_channels = 2; // 模擬 num_channels > 0 但 channels 是 nullptr

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(file) == 0); // Should return 0 if channels array is null
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_blocks_per_chunk_zero_channels() {
    NfsIioFile* file = helper_create_iio_file(0); // num_channels = 0
    assert(file != nullptr);

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(file) == 0);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_blocks_per_chunk_single_channel() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0); // Cache details not important here
    assert(file->channels[0] != nullptr);
    helper_set_channel_properties(file->channels[0], 5, 0, 0); // blocks_per_stripe = 5

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(file) == 5);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_blocks_per_chunk_multiple_channels() {
    NfsIioFile* file = helper_create_iio_file(3);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    file->channels[1] = helper_create_channel_with_cache(0);
    file->channels[2] = helper_create_channel_with_cache(0);
    assert(file->channels[0] && file->channels[1] && file->channels[2]);

    helper_set_channel_properties(file->channels[0], 2, 0, 0);
    helper_set_channel_properties(file->channels[1], 3, 0, 0);
    helper_set_channel_properties(file->channels[2], 4, 0, 0);

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(file) == (2 + 3 + 4));
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_blocks_per_chunk_with_null_channel_in_array() {
    NfsIioFile* file = helper_create_iio_file(3);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    file->channels[1] = nullptr; // One channel is null
    file->channels[2] = helper_create_channel_with_cache(0);
    assert(file->channels[0] && file->channels[2]);

    helper_set_channel_properties(file->channels[0], 2, 0, 0);
    // channel[1] is null
    helper_set_channel_properties(file->channels[2], 4, 0, 0);

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_blocks_per_chunk(file) == (2 + 4)); // Should skip the null channel
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

// --- Tests for nfs_iio_seek ---

void test_nfs_iio_seek_valid() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0] != nullptr);
    helper_set_channel_properties(file->channels[0], 1, 1000, 0); // Initial seek_pos = 0

    int new_seek_pos = 512;
    int old_clock = nfs_iio_CLOCK;
    int result = nfs_iio_seek(file, 0, new_seek_pos);
    assert(result == new_seek_pos);
    assert(file->channels[0]->current_seek_position == new_seek_pos);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_seek_invalid_inputs() {
    NfsIioFile* file_valid = helper_create_iio_file(1);
    assert(file_valid != nullptr);
    file_valid->channels[0] = helper_create_channel_with_cache(0);
    assert(file_valid->channels[0] != nullptr);

    int old_clock = nfs_iio_CLOCK;

    // Null file
    assert(nfs_iio_seek(nullptr, 0, 100) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1); // Clock increments even on error in this case

    // Null channels array
    old_clock = nfs_iio_CLOCK;
    NfsIioFile mock_file_no_chans = { 0 }; // file_handle=null, file_name=null, num_channels=1
    mock_file_no_chans.num_channels = 1; // to pass first checks
    mock_file_no_chans.channels = nullptr;
    assert(nfs_iio_seek(&mock_file_no_chans, 0, 100) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);

    // Invalid channel_idx (negative)
    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_seek(file_valid, -1, 100) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);

    // Invalid channel_idx (out of bounds)
    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_seek(file_valid, 1, 100) == -1); // num_channels is 1, so index 1 is OOB
    assert(nfs_iio_CLOCK == old_clock + 1);

    // Null channel pointer at valid index
    old_clock = nfs_iio_CLOCK;
    helper_free_channel_with_cache(file_valid->channels[0]); // Free it
    file_valid->channels[0] = nullptr; // Set to null
    assert(nfs_iio_seek(file_valid, 0, 100) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);


    helper_free_iio_file(file_valid); // Will handle null channel[0]
}

// --- Tests for nfs_iio_get_channel ---

void test_nfs_iio_get_channel_valid() {
    NfsIioFile* file = helper_create_iio_file(2);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    file->channels[1] = helper_create_channel_with_cache(0);
    assert(file->channels[0] != nullptr && file->channels[1] != nullptr);

    int old_clock = nfs_iio_CLOCK;
    NfsIioChannel* ch0 = nfs_iio_get_channel(file, 0);
    assert(ch0 == file->channels[0]);
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    NfsIioChannel* ch1 = nfs_iio_get_channel(file, 1);
    assert(ch1 == file->channels[1]);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file);
}

void test_nfs_iio_get_channel_invalid_inputs() {
    NfsIioFile* file_valid = helper_create_iio_file(1);
    assert(file_valid != nullptr);
    file_valid->channels[0] = helper_create_channel_with_cache(0); // Only channel 0 exists
    assert(file_valid->channels[0] != nullptr);

    int old_clock = nfs_iio_CLOCK;

    assert(nfs_iio_get_channel(nullptr, 0) == nullptr);
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    NfsIioFile mock_file_no_chans_arr = { 0 };
    mock_file_no_chans_arr.num_channels = 1;
    mock_file_no_chans_arr.channels = nullptr;
    assert(nfs_iio_get_channel(&mock_file_no_chans_arr, 0) == nullptr);
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_get_channel(file_valid, -1) == nullptr); // Negative index
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_get_channel(file_valid, 1) == nullptr); // Index out of bounds
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_get_channel(file_valid, 5) == nullptr); // Index way out of bounds
    assert(nfs_iio_CLOCK == old_clock + 1);


    helper_free_iio_file(file_valid);
}

// --- Tests for nfs_iio_channel_size ---

void test_nfs_iio_channel_size_valid() {
    NfsIioChannel* channel = helper_create_channel_with_cache(0);
    assert(channel != nullptr);
    int expected_size = 2048;
    helper_set_channel_properties(channel, 1, expected_size, 0);

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_size(channel) == expected_size);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_channel_with_cache(channel);
}

void test_nfs_iio_channel_size_null_channel() {
    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_size(nullptr) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);
}

// --- Tests for nfs_iio_channel_blocks ---

void test_nfs_iio_channel_blocks_valid() {
    NfsIioChannel* channel = helper_create_channel_with_cache(0);
    assert(channel != nullptr);
    int expected_blocks = 7;
    helper_set_channel_properties(channel, expected_blocks, 0, 0);

    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_blocks(channel) == expected_blocks);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_channel_with_cache(channel);
}

void test_nfs_iio_channel_blocks_null_channel() {
    int old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_blocks(nullptr) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);
}

// --- Tests for nfs_iio_channel_truncate ---

void test_nfs_iio_channel_truncate_valid() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0] != nullptr);

    int initial_size = 2000;
    int seek_pos_for_truncate = 1500;
    helper_set_channel_properties(file->channels[0], 1, initial_size, seek_pos_for_truncate);

    int old_clock = nfs_iio_CLOCK;
    int result = nfs_iio_channel_truncate(file, 0);
    assert(result == 0);
    assert(file->channels[0]->current_size_bytes == seek_pos_for_truncate);
    assert(nfs_iio_CLOCK == old_clock + 1);

    // Truncate to 0
    helper_set_channel_properties(file->channels[0], 1, seek_pos_for_truncate, 0); // Seek to 0
    old_clock = nfs_iio_CLOCK;
    result = nfs_iio_channel_truncate(file, 0);
    assert(result == 0);
    assert(file->channels[0]->current_size_bytes == 0);
    assert(nfs_iio_CLOCK == old_clock + 1);


    helper_free_iio_file(file);
}

void test_nfs_iio_channel_truncate_invalid_inputs() {
    NfsIioFile* file_valid = helper_create_iio_file(1);
    assert(file_valid != nullptr);
    file_valid->channels[0] = helper_create_channel_with_cache(0);
    assert(file_valid->channels[0] != nullptr);
    helper_set_channel_properties(file_valid->channels[0], 1, 100, 50);

    int old_clock = nfs_iio_CLOCK;

    assert(nfs_iio_channel_truncate(nullptr, 0) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    NfsIioFile mock_file_no_chans_arr = { 0 };
    mock_file_no_chans_arr.num_channels = 1;
    mock_file_no_chans_arr.channels = nullptr;
    assert(nfs_iio_channel_truncate(&mock_file_no_chans_arr, 0) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_truncate(file_valid, -1) == -1); // Negative index
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    assert(nfs_iio_channel_truncate(file_valid, 1) == -1); // Index out of bounds
    assert(nfs_iio_CLOCK == old_clock + 1);

    old_clock = nfs_iio_CLOCK;
    helper_free_channel_with_cache(file_valid->channels[0]);
    file_valid->channels[0] = nullptr; // Null channel pointer
    assert(nfs_iio_channel_truncate(file_valid, 0) == -1);
    assert(nfs_iio_CLOCK == old_clock + 1);

    helper_free_iio_file(file_valid);
}

// --- Tests for channel_block_to_absolute_block ---

void test_channel_block_to_absolute_block_single_channel() {
    NfsIioFile* file = helper_create_iio_file(1); // 1 channel
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0] != nullptr);
    helper_set_channel_properties(file->channels[0], 4, 0, 0); // blocks_per_stripe = 4

    // Channel 0, rel_block 0 => (0 + 0) + (0 * 4) = 0
    assert(channel_block_to_absolute_block(file, 0, 0) == 0);
    // Channel 0, rel_block 3 => (3 + 0) + (0 * 4) = 3
    assert(channel_block_to_absolute_block(file, 0, 3) == 3);
    // Channel 0, rel_block 4 => (0 + 0) + (1 * 4) = 4 (chunk_idx=1, offset_in_stripe=0)
    assert(channel_block_to_absolute_block(file, 0, 4) == 4);
    // Channel 0, rel_block 7 => (3 + 0) + (1 * 4) = 7 (chunk_idx=1, offset_in_stripe=3)
    assert(channel_block_to_absolute_block(file, 0, 7) == 7);
    // Channel 0, rel_block 8 => (0 + 0) + (2 * 4) = 8
    assert(channel_block_to_absolute_block(file, 0, 8) == 8);

    helper_free_iio_file(file);
}

void test_channel_block_to_absolute_block_multiple_channels() {
    NfsIioFile* file = helper_create_iio_file(3); // 3 channels
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    file->channels[1] = helper_create_channel_with_cache(0);
    file->channels[2] = helper_create_channel_with_cache(0);
    assert(file->channels[0] && file->channels[1] && file->channels[2]);

    helper_set_channel_properties(file->channels[0], 2, 0, 0); // Ch0: bps=2
    helper_set_channel_properties(file->channels[1], 3, 0, 0); // Ch1: bps=3
    helper_set_channel_properties(file->channels[2], 1, 0, 0); // Ch2: bps=1
    // total_blocks_per_chunk = 2 + 3 + 1 = 6

    // Target Channel 0 (base_offset_in_chunk = 0)
    // rel_block 0: (0+0) + (0*6) = 0
    assert(channel_block_to_absolute_block(file, 0, 0) == 0);
    // rel_block 1: (1+0) + (0*6) = 1
    assert(channel_block_to_absolute_block(file, 0, 1) == 1);
    // rel_block 2 (new chunk for ch0): (0+0) + (1*6) = 6
    assert(channel_block_to_absolute_block(file, 0, 2) == 6);

    // Target Channel 1 (base_offset_in_chunk = 2 (from ch0))
    // rel_block 0: (0+2) + (0*6) = 2
    assert(channel_block_to_absolute_block(file, 1, 0) == 2);
    // rel_block 2: (2+2) + (0*6) = 4
    assert(channel_block_to_absolute_block(file, 1, 2) == 4);
    // rel_block 3 (new chunk for ch1): (0+2) + (1*6) = 8
    assert(channel_block_to_absolute_block(file, 1, 3) == 8);

    // Target Channel 2 (base_offset_in_chunk = 2 + 3 = 5)
    // rel_block 0: (0+5) + (0*6) = 5
    assert(channel_block_to_absolute_block(file, 2, 0) == 5);
    // rel_block 1 (new chunk for ch2): (0+5) + (1*6) = 11
    assert(channel_block_to_absolute_block(file, 2, 1) == 11);

    helper_free_iio_file(file);
}

void test_channel_block_to_absolute_block_invalid_inputs() {
    NfsIioFile* file_valid = helper_create_iio_file(1);
    assert(file_valid != nullptr);
    file_valid->channels[0] = helper_create_channel_with_cache(0);
    assert(file_valid->channels[0]);
    helper_set_channel_properties(file_valid->channels[0], 1, 0, 0);


    assert(channel_block_to_absolute_block(nullptr, 0, 0) == -1);
    assert(channel_block_to_absolute_block(file_valid, -1, 0) == -1); // Invalid channel_idx
    assert(channel_block_to_absolute_block(file_valid, 1, 0) == -1);  // channel_idx out of bounds

    NfsIioFile* file_null_channel_ptr = helper_create_iio_file(1);
    assert(file_null_channel_ptr); // channels[0] is nullptr by default from helper_create_iio_file
    assert(channel_block_to_absolute_block(file_null_channel_ptr, 0, 0) == -1);
    helper_free_iio_file(file_null_channel_ptr);

    helper_set_channel_properties(file_valid->channels[0], 0, 0, 0); // bps = 0
    assert(channel_block_to_absolute_block(file_valid, 0, 0) == -1); // Division by zero in logic

    helper_free_iio_file(file_valid);
}

// --- Tests for channel_pos_to_absolute_block ---

void test_channel_pos_to_absolute_block_valid() {
    // This function heavily relies on channel_block_to_absolute_block and nfs_iio_BLOCK_SIZEv.
    // We assume nfs_iio_BLOCK_SIZEv is, for example, 512 for these calculations.
    int old_block_size = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512; // Temporarily set for predictable testing

    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0]);
    helper_set_channel_properties(file->channels[0], 2, 0, 0); // bps = 2

    // byte_pos 0 -> rel_block 0. abs_block should be 0 (from single channel logic)
    assert(channel_pos_to_absolute_block(file, 0, 0) == 0);
    // byte_pos 511 -> rel_block 0. abs_block should be 0
    assert(channel_pos_to_absolute_block(file, 0, 511) == 0);
    // byte_pos 512 -> rel_block 1. abs_block should be 1
    assert(channel_pos_to_absolute_block(file, 0, 512) == 1);
    // byte_pos 1023 -> rel_block 1. abs_block should be 1
    assert(channel_pos_to_absolute_block(file, 0, 1023) == 1);
    // byte_pos 1024 -> rel_block 2. (chunk_idx=1, offset_in_stripe=0). total_bps = 2. abs_block = (0+0)+(1*2) = 2
    assert(channel_pos_to_absolute_block(file, 0, 1024) == 2);


    helper_free_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_block_size; // Restore global
}

void test_channel_pos_to_absolute_block_zero_block_size() {
    int old_block_size = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 0; // Invalid block size

    NfsIioFile* file = helper_create_iio_file(1);
    assert(file != nullptr);
    file->channels[0] = helper_create_channel_with_cache(0);
    assert(file->channels[0]);
    helper_set_channel_properties(file->channels[0], 1, 0, 0);

    assert(channel_pos_to_absolute_block(file, 0, 100) == -1);

    helper_free_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_block_size; // Restore
}

// Spy variables for write_absolute_block_n
static bool spy_write_abs_called = false;
static int spy_write_abs_file_id = -1; // Could use NfsIioFile* address if unique
static int spy_write_abs_block_idx = -1;
static int spy_write_abs_num_blocks = -1;
static std::vector<char> spy_write_abs_buffer_content;

// Spy variables for read_absolute_block_n
static bool spy_read_abs_called = false;
static int spy_read_abs_file_id = -1;
static int spy_read_abs_block_idx = -1;
static int spy_read_abs_num_blocks = -1;
static std::vector<char> mock_read_abs_buffer_content_to_provide; // Data to fill read buffer with

void reset_abs_io_spies() {
    spy_write_abs_called = false;
    spy_write_abs_file_id = -1;
    spy_write_abs_block_idx = -1;
    spy_write_abs_num_blocks = -1;
    spy_write_abs_buffer_content.clear();

    spy_read_abs_called = false;
    spy_read_abs_file_id = -1;
    spy_read_abs_block_idx = -1;
    spy_read_abs_num_blocks = -1;
    mock_read_abs_buffer_content_to_provide.clear();
}

// These would be 'mock' implementations if we were to replace the real ones.
// For now, we'll call the real ones but can use spies to augment checks.
// To truly mock, you'd need to link against a version of nfs.cpp where these
// functions are weak symbols or use a proper mocking library.
// As a simpler alternative for *these specific tests*, we can have the tests
// call wrapper functions that set spies *and then* call the real functions.
// Or, even simpler, just inspect the file contents directly after calls.

// For now, we will use real file I/O and direct inspection.

// --- Tests for cache_page_dump ---
// --- Tests for cache_page_dump ---

void test_cache_page_dump_valid_page() {
    const int block_size = nfs_iio_BLOCK_SIZEv;
    int old_bs = nfs_iio_BLOCK_SIZEv;
    // 為了方便檢查和計算，暫時使用一個較小的 block_size
    // 如果您的 nfs_iio_BLOCK_SIZEv 預設就是一個較小的值，或者您在測試 setup 中設定，則可以省略這兩行
    if (nfs_iio_BLOCK_SIZEv > 1024) { // 假設原始值較大
        nfs_iio_BLOCK_SIZEv = 512;
    }


    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file != nullptr && file->file_handle != nullptr);
    // Channel 0, 1 page slot, blocks_per_stripe = 2
    file->channels[0] = helper_create_channel_with_cache(1, 2);
    assert(file->channels[0] && file->channels[0]->cache_header);
    // 確保 channel 的 blocks_per_stripe 與 cache page 的緩衝區大小匹配
    helper_set_channel_properties(file->channels[0], 2, 0, 0);

    // 準備 cache page
    uintptr_t page_buffer_addr_val = 0x10000; // 模擬的緩衝區指標值
    int disk_position_for_page = 0; // 此頁面對應的通道內相對位元組偏移量
    int access_time_for_page = 123; // 任意的 access_time 值

    // **修正點：補上 access_time 引數**
    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(
        page_buffer_addr_val,
        disk_position_for_page,
        access_time_for_page
    );
    assert(file->channels[0]->cache_header->pages[0]);

    // 為了讓 cache_page_dump 能夠 "寫入" 某些內容到模擬的磁碟，
    // 我們需要讓 cache_page_get_buffer 返回一個有效的記憶體位址，其內容是我們期望寫入的。
    // page_content 的大小應為 channel->blocks_per_stripe * nfs_iio_BLOCK_SIZEv
    std::vector<char> page_content(file->channels[0]->blocks_per_stripe * nfs_iio_BLOCK_SIZEv);
    helper_fill_buffer_pattern(page_content.data(), page_content.size(), 10);

    // 讓 NfsIioCachePage 中的 buffer_ptr_and_syncflag_storage 指向我們準備好的 page_content
    // 注意：這是一個技巧，因為 buffer_ptr_and_syncflag_storage 的 LSB 用於同步旗標。
    // 我們需要確保 LSB 不影響指標本身。
    file->channels[0]->cache_header->pages[0]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(page_content.data());
    // 確保 LSB 不影響指標後，再設定同步旗標
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0); // 標記為 dirty，以便 dump 操作會執行

    int result = cache_page_dump(file, 0, 0);
    assert(result == 0);
    // 驗證頁面在 dump 後是否被標記為 clean (已同步)
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[0]) == 1);

    // 驗證內容是否已正確寫入（模擬的）磁碟檔案
    fflush(file->file_handle); // 確保 C 標準 I/O 緩衝區已刷新
    FILE* verify_fp = fopen(TEST_ABS_IO_FILENAME, "rb");
    assert(verify_fp != nullptr);

    // 計算預期的寫入位置：header_size + absolute_block_index * block_size
    // absolute_block_index 是由 channel_pos_to_absolute_block 計算得出
    // channel_pos_to_absolute_block(file, channel_idx, page->disk_block_position)
    int expected_abs_block_idx = channel_pos_to_absolute_block(file, 0, disk_position_for_page);
    assert(expected_abs_block_idx >= 0); // 確保轉換成功

    long expected_file_offset = header_size(file) + (long)expected_abs_block_idx * nfs_iio_BLOCK_SIZEv;
    fseek(verify_fp, expected_file_offset, SEEK_SET);

    std::vector<char> file_read_buf(page_content.size());
    size_t num_read = fread(file_read_buf.data(), 1, file_read_buf.size(), verify_fp);
    assert(num_read == file_read_buf.size());
    fclose(verify_fp);

    // 比較讀回的內容與原始頁面內容
    assert(memcmp(file_read_buf.data(), page_content.data(), page_content.size()) == 0);

    helper_teardown_iio_file(file); // 清理測試檔案和結構
    if (nfs_iio_BLOCK_SIZEv != old_bs) { // 恢復 BLOCK_SIZE
        nfs_iio_BLOCK_SIZEv = old_bs;
    }
}


void test_cache_page_dump_invalid_inputs() {
    NfsIioFile* file_valid = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file_valid);
    file_valid->channels[0] = helper_create_channel_with_cache(1, 1);
    assert(file_valid->channels[0] && file_valid->channels[0]->cache_header);
    file_valid->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x1000, 0, 0);
    assert(file_valid->channels[0]->cache_header->pages[0]);


    assert(cache_page_dump(nullptr, 0, 0) == -1);
    assert(cache_page_dump(file_valid, -1, 0) == -1);
    assert(cache_page_dump(file_valid, 1, 0) == -1); // channel_idx OOB
    assert(cache_page_dump(file_valid, 0, -1) == -1);
    assert(cache_page_dump(file_valid, 0, 1) == -1); // page_array_idx OOB

    // Null page pointer in array
    helper_free_cache_page(file_valid->channels[0]->cache_header->pages[0]);
    file_valid->channels[0]->cache_header->pages[0] = nullptr;
    assert(cache_page_dump(file_valid, 0, 0) == -1);

    helper_teardown_iio_file(file_valid);
}

// --- Tests for cache_pageflush ---

void test_cache_pageflush_dirty_page() {
    // This test relies on cache_page_dump working correctly.
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(1, 1); // bps=1
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 0, 0);


    static char buffer_for_flush_test[512]; // Page buffer
    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(reinterpret_cast<uintptr_t>(buffer_for_flush_test), 0, 0);
    assert(file->channels[0]->cache_header->pages[0]);
    helper_fill_buffer_pattern(buffer_for_flush_test, 512, 20);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0); // Mark dirty

    cache_pageflush(file, 0, 0);
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[0]) == 1); // Should be clean now

    // Verify file content (similar to test_cache_page_dump_valid_page)
    fflush(file->file_handle);
    FILE* verify_fp = fopen(TEST_ABS_IO_FILENAME, "rb");
    assert(verify_fp != nullptr);
    fseek(verify_fp, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Skip header
    std::vector<char> file_read_buf(512);
    fread(file_read_buf.data(), 1, 512, verify_fp);
    fclose(verify_fp);
    assert(helper_verify_buffer_pattern(file_read_buf.data(), 512, 20));

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

void test_cache_pageflush_clean_page() {
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1); // Real file for dump
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(1, 1);
    assert(file->channels[0] && file->channels[0]->cache_header);

    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x2000, 0, 0);
    assert(file->channels[0]->cache_header->pages[0]);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 1); // Mark clean

    // To verify dump wasn't called, we'd ideally use a mock/spy for cache_page_dump.
    // For now, we check that the sync status remains 1 and no error occurs.
    // And that the file (if it had prior content from a different source) is not overwritten by this page.
    // For simplicity, just check sync status.
    long file_pos_before = -1;
    if (file->file_handle) file_pos_before = ftell(file->file_handle);

    cache_pageflush(file, 0, 0);
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[0]) == 1);

    // Check if file pointer moved or file was written to (crude check)
    if (file->file_handle && file_pos_before != -1) {
        // If dump was called, it would fseek & fwrite. This isn't a perfect check.
        // A better check involves ensuring file contents *haven't* changed to reflect this page.
    }


    helper_teardown_iio_file(file);
}

void test_cache_pageflush_invalid_inputs() {
    // Similar to cache_page_dump_invalid_inputs, as flush calls dump.
    // Just ensure it doesn't crash and handles nulls gracefully.
    NfsIioFile* file_valid = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file_valid);
    file_valid->channels[0] = helper_create_channel_with_cache(1, 1);
    assert(file_valid->channels[0] && file_valid->channels[0]->cache_header);
    file_valid->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x1000, 0, 0);
    assert(file_valid->channels[0]->cache_header->pages[0]);

    cache_pageflush(nullptr, 0, 0); // Should not crash
    cache_pageflush(file_valid, -1, 0);
    cache_pageflush(file_valid, 1, 0); // channel_idx OOB
    // ... etc.

    helper_free_cache_page(file_valid->channels[0]->cache_header->pages[0]);
    file_valid->channels[0]->cache_header->pages[0] = nullptr;
    cache_pageflush(file_valid, 0, 0); // Page is null

    helper_teardown_iio_file(file_valid);
}

// --- Tests for cache_page_create ---
const int MAX_CACHE_PAGES_ACTIVE_THRESHOLD = 8; // From nfs.cpp logic

void test_cache_page_create_new_page_below_threshold() {
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_create_iio_file(1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(MAX_CACHE_PAGES_ACTIVE_THRESHOLD, 2); // bps=2
    assert(file->channels[0] && file->channels[0]->cache_header);
    file->channels[0]->cache_header->num_pages_active = 0; // Start with no active pages

    int create_at_idx = 0;
    int old_clock = nfs_iio_CLOCK;

    cache_page_create(file, 0, create_at_idx);

    assert(file->channels[0]->cache_header->pages[create_at_idx] != nullptr);
    NfsIioCachePage* new_page = file->channels[0]->cache_header->pages[create_at_idx];
    assert(cache_page_get_buffer(new_page) != nullptr); // Buffer should be allocated
    assert(new_page->last_access_time == old_clock); // Clock is not incremented inside create for this path
    assert(cache_page_get_sync(new_page) == 1); // New pages are initially clean
    assert(file->channels[0]->cache_header->num_pages_active == 1);

    // Free the buffer allocated by cache_page_create
    if (cache_page_get_buffer(new_page)) {
        free(cache_page_get_buffer(new_page));
    }
    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

void test_cache_page_create_reuse_page_at_threshold() {
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512; // 確保測試時 BLOCK_SIZE 是我們期望的

    // TEST_ABS_IO_FILENAME 應該是一個唯一的檔案名，或者在每次測試前被正確清理
    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file && file->file_handle); // 確保檔案成功打開

    file->channels[0] = helper_create_channel_with_cache(MAX_CACHE_PAGES_ACTIVE_THRESHOLD, 1);
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 0, 0); // bps=1

    // 這裡的 static 緩衝區主要用於提供一個有效的記憶體地址和內容
    // 確保每個測試的隔離性，如果可能，避免在測試函數作用域之外共享狀態
    // 或者確保每次都完全重新初始化。
    static char page_buffers_for_reuse[MAX_CACHE_PAGES_ACTIVE_THRESHOLD][512];

    for (int i = 0; i < MAX_CACHE_PAGES_ACTIVE_THRESHOLD; ++i) {
        // 分配 NfsIioCachePage 元數據結構
        file->channels[0]->cache_header->pages[i] = helper_create_cache_page(
            reinterpret_cast<uintptr_t>(page_buffers_for_reuse[i]), // 初始 buffer_val
            i * nfs_iio_BLOCK_SIZEv, // disk_pos，確保使用當前的 nfs_iio_BLOCK_SIZEv
            100 + i * 10             // page 0 is LRU (time 100)
        );
        assert(file->channels[0]->cache_header->pages[i]);

        // 填充實際的緩衝區內容
        helper_fill_buffer_pattern(page_buffers_for_reuse[i], nfs_iio_BLOCK_SIZEv, 30 + i);

        // 將 NfsIioCachePage 的指標部分指向已填充的緩衝區，並設定 sync 狀態
        // 這一行是多餘的，因為 helper_create_cache_page 已經做了類似的事，
        // 並且 cache_page_set_sync 會處理 LSB。
        // 但為了確保，我們可以這樣寫：
        uintptr_t current_buffer_actual_addr = reinterpret_cast<uintptr_t>(page_buffers_for_reuse[i]);
        file->channels[0]->cache_header->pages[i]->buffer_ptr_and_syncflag_storage = current_buffer_actual_addr;
        cache_page_set_sync(file->channels[0]->cache_header->pages[i], 0); // Mark as dirty. LSB of current_buffer_actual_addr will be set to 0.
    }
    file->channels[0]->cache_header->num_pages_active = MAX_CACHE_PAGES_ACTIVE_THRESHOLD;
    file->channels[0]->cache_header->num_pages_allocated = MAX_CACHE_PAGES_ACTIVE_THRESHOLD;

    int reuse_target_idx = 1; // 我們將在這個索引處觸發頁面創建/重用
    // 假設 page 0 (LRU) 會被移動到 pages[reuse_target_idx]
    // 先保存可能在 reuse_target_idx 的頁面（如果有），並將其置空以觸發創建邏輯
    NfsIioCachePage* page_being_overwritten = file->channels[0]->cache_header->pages[reuse_target_idx];
    if (page_being_overwritten) {
        // 如果 reuse_target_idx 已經有頁面，理論上 cache_page_create 不會執行。
        // 測試的意圖是 reuse_target_idx 槽位是 nullptr。
        // 但如果 page_buffers_for_reuse[reuse_target_idx] 是該頁面的緩衝區，需要小心處理。
        // 這裡的測試邏輯是 reuse_target_idx 應該是空的，所以我們直接將其設為 nullptr。
    }
    file->channels[0]->cache_header->pages[reuse_target_idx] = nullptr;


    int clock_at_call_start = nfs_iio_CLOCK = 200;

    cache_page_create(file, 0, reuse_target_idx);

    // **在讀取驗證前刷新**
    if (file && file->file_handle) {
        fflush(file->file_handle);
    }

    assert(file->channels[0]->cache_header->pages[reuse_target_idx] != nullptr);
    NfsIioCachePage* reused_page_now_at_idx = file->channels[0]->cache_header->pages[reuse_target_idx];

    int expected_clock_at_assignment = clock_at_call_start + 1;

    assert(reused_page_now_at_idx->last_access_time == expected_clock_at_assignment);
    assert(file->channels[0]->cache_header->pages[0] == nullptr);
    assert(cache_page_get_sync(reused_page_now_at_idx) == 1);
    assert(file->channels[0]->cache_header->num_pages_active == MAX_CACHE_PAGES_ACTIVE_THRESHOLD);
    assert(nfs_iio_CLOCK == expected_clock_at_assignment);

    // 驗證 page 0 (LRU, disk_pos 0) 的內容是否已 flush 到檔案
    FILE* verify_fp = fopen(TEST_ABS_IO_FILENAME, "rb");
    assert(verify_fp != nullptr);

    long expected_offset_in_file = (long)header_size(file) + // 跳過檔案頭
        (long)0 * nfs_iio_BLOCK_SIZEv;  // page 0 的絕對區塊索引是 0
    int fseek_result = fseek(verify_fp, expected_offset_in_file, SEEK_SET);
    assert(fseek_result == 0);

    std::vector<char> file_read_buf(nfs_iio_BLOCK_SIZEv); // 讀取一個區塊
    size_t bytes_actually_read = fread(file_read_buf.data(), 1, file_read_buf.size(), verify_fp);
    // printf("DEBUG: Bytes read for verification: %zu\n", bytes_actually_read);
    // if (bytes_actually_read < file_read_buf.size()) {
    //     printf("DEBUG: ferror: %d, feof: %d\n", ferror(verify_fp), feof(verify_fp));
    // }
    assert(bytes_actually_read == file_read_buf.size()); // 確保讀到了足夠的數據
    fclose(verify_fp);

    assert(helper_verify_buffer_pattern(file_read_buf.data(), nfs_iio_BLOCK_SIZEv, 30 + 0));

    // 清理被覆蓋的頁面（如果它沒有被重用且包含動態資源）
    if (page_being_overwritten && page_being_overwritten != reused_page_now_at_idx) {
        // helper_free_cache_page(page_being_overwritten); // 取決於 page_being_overwritten 的分配方式
    }

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

// TODO: Add tests for cache_page_create with allocation failures (if possible to simulate without _exit)
// TODO: Add tests for cache_page_create where channel->blocks_per_stripe = 0

// --- Tests for cache_page_refresh ---

void test_cache_page_refresh_new_page() {
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512; // For easier calculation

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(0, 2); // bps=2. Initially 0 allocated pages in cache.
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 2, 2048, 0); // bps=2, size > one page

    // Prepare mock disk content for the first page (2 blocks for bps=2)
    // Page 0 covers byte 0 to 2*512 - 1.
    // Absolute block for this page will be channel_pos_to_absolute_block(file, 0, 0)
    // which is 0 (assuming single channel).
    // So, data is at file offset header_size() + 0.
    std::vector<char> disk_content_page0(2 * 512);
    helper_fill_buffer_pattern(disk_content_page0.data(), disk_content_page0.size(), 40);

    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Go past header
    fwrite(disk_content_page0.data(), 1, disk_content_page0.size(), file->file_handle);
    fflush(file->file_handle);

    int clock_before_refresh = nfs_iio_CLOCK = 300; // 設定並記錄調用前的 nfs_iio_CLOCK
    int result = cache_page_refresh(file, 0, 200);
    assert(result == 0);

    assert(file->channels[0]->cache_header->num_pages_allocated >= 1);
    NfsIioCachePage* refreshed_page = file->channels[0]->cache_header->pages[0];
    assert(refreshed_page != nullptr);
    assert(refreshed_page->disk_block_position == 0);

    // 預期 nfs_iio_CLOCK 因為 channel_pos_to_absolute_block -> ... -> nfs_iio_blocks_per_chunk 而增加 1
    int expected_clock_at_assignment = clock_before_refresh + 1;
    assert(refreshed_page->last_access_time == expected_clock_at_assignment);
    assert(cache_page_get_sync(refreshed_page) == 1);

    // 同時，也應該驗證 nfs_iio_CLOCK 在 cache_page_refresh 調用後的最終狀態
    // 賦值給 last_access_time 是 cache_page_refresh 中最後一次與 nfs_iio_CLOCK 相關的讀取操作
    // nfs_iio_CLOCK 的值在賦值後不應再改變（在 cache_page_refresh 內部）
    assert(nfs_iio_CLOCK == expected_clock_at_assignment);
    assert(cache_page_get_sync(refreshed_page) == 1);

    // Verify buffer content
    char* page_buffer = (char*)cache_page_get_buffer(refreshed_page);
    assert(page_buffer != nullptr);
    assert(memcmp(page_buffer, disk_content_page0.data(), disk_content_page0.size()) == 0);

    if (page_buffer) { // Buffer was allocated by cache_page_create called by refresh
        // free(page_buffer); // This will be handled by helper_teardown_iio_file
    }
    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

void test_cache_page_refresh_existing_page() {
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(1, 1); // 1 allocated page, bps=1
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 1024, 0);


    // Setup an existing page in cache
    static char existing_page_buffer[512];
    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(
        reinterpret_cast<uintptr_t>(existing_page_buffer),
        0, // disk_block_pos
        290  // old access time
    );
    assert(file->channels[0]->cache_header->pages[0]);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0); // Mark dirty (will be overwritten by refresh)
    helper_fill_buffer_pattern(existing_page_buffer, 512, 99); // Old content

    // Prepare mock disk content for this page
    std::vector<char> disk_content_for_refresh(512);
    helper_fill_buffer_pattern(disk_content_for_refresh.data(), 512, 50);
    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Go past header
    fwrite(disk_content_for_refresh.data(), 1, 512, file->file_handle);
    fflush(file->file_handle);

    int old_clock = nfs_iio_CLOCK = 300;
    int result = cache_page_refresh(file, 0, 0); // Refresh page 0 at byte_pos 0
    assert(result == 0);

    NfsIioCachePage* refreshed_page = file->channels[0]->cache_header->pages[0];
    assert(refreshed_page != nullptr);
    assert(refreshed_page->disk_block_position == 0);
    assert(refreshed_page->last_access_time == old_clock + 1);
    assert(cache_page_get_sync(refreshed_page) == 1);

    char* page_buffer = (char*)cache_page_get_buffer(refreshed_page);
    assert(page_buffer != nullptr);
    assert(memcmp(page_buffer, disk_content_for_refresh.data(), 512) == 0);

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

// TODO: Add tests for cache_page_refresh with cache_expand failing, create failing etc.
// TODO: Add tests for channel->blocks_per_stripe = 0
// 輔助函數：創建帶有實際 malloc 的緩衝區的 NfsIioCachePage
NfsIioCachePage* helper_create_page_with_real_buffer(size_t buffer_size, int disk_pos, int access_time, bool make_dirty = false) {
    NfsIioCachePage* page = (NfsIioCachePage*)malloc(sizeof(NfsIioCachePage));
    if (!page) {
        std::cerr << "helper_create_page_with_real_buffer: Failed to alloc NfsIioCachePage" << std::endl;
        return nullptr;
    }
    page->disk_block_position = disk_pos;
    page->last_access_time = access_time;

    if (buffer_size > 0) {
        void* buffer = malloc(buffer_size);
        if (!buffer) {
            std::cerr << "helper_create_page_with_real_buffer: Failed to alloc buffer of size " << buffer_size << std::endl;
            free(page);
            return nullptr;
        }
        // 用可識別的模式填充緩衝區，以便檢查是否被正確處理
        helper_fill_buffer_pattern(static_cast<char*>(buffer), buffer_size, disk_pos); // 使用 disk_pos 作為 seed
        page->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(buffer);
    }
    else {
        page->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(nullptr);
    }

    cache_page_set_sync(page, make_dirty ? 0 : 1);
    return page;
}
// --- Tests for cache_create (IIO version) ---

void test_cache_create_iio_valid_channel() {
    // 此測試與 test_cache_page_refresh_new_page 非常相似，只是 byte_pos 固定為 0
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(0, 1); // bps=1, 0 allocated pages
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 512, 0); // bps=1, size for one block

    // 準備磁碟內容
    std::vector<char> disk_content_block0(512);
    helper_fill_buffer_pattern(disk_content_block0.data(), 512, 60);
    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Skip header
    fwrite(disk_content_block0.data(), 1, 512, file->file_handle);
    fflush(file->file_handle);

    int clock_before_call = nfs_iio_CLOCK = 400;
    int result = cache_create(file, 0); // 等同於 cache_page_refresh(file, 0, 0)
    assert(result == 0);

    // 驗證 nfs_iio_CLOCK 的變化 (+1 是因為 refresh 內部調用了 channel_pos_to_absolute_block)
    assert(nfs_iio_CLOCK == clock_before_call + 1);

    NfsIioCachePage* page0 = file->channels[0]->cache_header->pages[0];
    assert(page0 != nullptr);
    assert(page0->disk_block_position == 0);
    assert(page0->last_access_time == clock_before_call + 1); // 使用更新後的 clock
    assert(cache_page_get_sync(page0) == 1);

    char* page_buffer = (char*)cache_page_get_buffer(page0);
    assert(page_buffer != nullptr);
    assert(memcmp(page_buffer, disk_content_block0.data(), 512) == 0);

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

void test_cache_create_iio_invalid_file() {
    assert(cache_create(nullptr, 0) == -1);
}

void test_cache_create_iio_invalid_channel_idx() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file);
    // file->channels[0] 保持 nullptr
    assert(cache_create(file, 0) == -1); // Channel 0 不存在 (nullptr)
    assert(cache_create(file, -1) == -1);
    assert(cache_create(file, 1) == -1); // Out of bounds

    helper_free_iio_file(file);
}

// --- Tests for cache_destroy (IIO version) ---

void test_cache_destroy_iio_null_cache() {
    assert(cache_destroy(nullptr) == -1); // 根據 nfs.cpp 實現
}

void test_cache_destroy_iio_empty_pages_array() {
    NfsIioCache* cache = (NfsIioCache*)malloc(sizeof(NfsIioCache));
    assert(cache);
    cache->num_pages_allocated = 0;
    cache->num_pages_active = 0;
    cache->pages = nullptr;
    assert(cache_destroy(cache) == 0); // 應該成功銷毀
}

void test_cache_destroy_iio_with_pages_and_buffers() {
    NfsIioCache* cache = (NfsIioCache*)malloc(sizeof(NfsIioCache));
    assert(cache);
    cache->num_pages_allocated = 3;
    cache->num_pages_active = 2; // 假設有2個活躍頁面
    cache->pages = (NfsIioCachePage**)malloc(sizeof(NfsIioCachePage*) * cache->num_pages_allocated);
    assert(cache->pages);

    // 創建帶有實際分配緩衝區的頁面
    cache->pages[0] = helper_create_page_with_real_buffer(100, 0, 10);
    assert(cache->pages[0]);
    cache->pages[1] = helper_create_page_with_real_buffer(200, 1, 20);
    assert(cache->pages[1]);
    cache->pages[2] = nullptr; // 一個空槽位

    // 這裡我們不能輕易驗證 free 是否被調用，除非使用 mock 或更高級的內存調試工具。
    // 但我們可以執行它並確保不會崩潰。
    assert(cache_destroy(cache) == 0);
    // 執行後，cache 指標及其內部所有分配的記憶體都應被釋放。
    // 再次訪問 cache 會導致未定義行為。
}

// --- Tests for is_in_cache (IIO version) ---

void test_is_in_cache_iio_basic_cases() {
    NfsIioFile* file = helper_create_iio_file(1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(5, 2); // 5 slots, bps=2
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 2, 0, 0); // bps=2

    // page_array_idx = channel_relative_block_idx / blocks_per_stripe

    // Slot 0 (for rel_block 0, 1) - 假設為 non-nullptr
    file->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x1000, 0, 0);
    assert(is_in_cache(file, 0, 0) == 1); // rel_block 0 -> page_idx 0/2 = 0. pages[0] is not null.
    assert(is_in_cache(file, 0, 1) == 1); // rel_block 1 -> page_idx 1/2 = 0. pages[0] is not null.

    // Slot 1 (for rel_block 2, 3) - 保持 nullptr
    assert(is_in_cache(file, 0, 2) == 0); // rel_block 2 -> page_idx 2/2 = 1. pages[1] is null.
    assert(is_in_cache(file, 0, 3) == 0); // rel_block 3 -> page_idx 3/2 = 1. pages[1] is null.

    // Slot 2 (for rel_block 4, 5) - 假設為 non-nullptr
    file->channels[0]->cache_header->pages[2] = helper_create_cache_page(0x2000, 0, 0);
    assert(is_in_cache(file, 0, 4) == 1); // rel_block 4 -> page_idx 4/2 = 2. pages[2] is not null.

    // Index out of allocated range
    assert(is_in_cache(file, 0, 10) == 0); // rel_block 10 -> page_idx 10/2 = 5. OOB for 5 slots.

    helper_free_iio_file(file);
}

void test_is_in_cache_iio_invalid_inputs_and_bps_zero() {
    NfsIioFile* file_valid = helper_create_iio_file(1);
    assert(file_valid);
    file_valid->channels[0] = helper_create_channel_with_cache(1, 1); // bps=1
    assert(file_valid->channels[0] && file_valid->channels[0]->cache_header);
    file_valid->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x100, 0, 0);

    assert(is_in_cache(nullptr, 0, 0) == 0);
    assert(is_in_cache(file_valid, -1, 0) == 0);
    assert(is_in_cache(file_valid, 1, 0) == 0); // OOB

    NfsIioFile* file_no_chan_struct = helper_create_iio_file(1); // channels[0] is nullptr
    assert(is_in_cache(file_no_chan_struct, 0, 0) == 0);
    helper_free_iio_file(file_no_chan_struct);

    NfsIioChannel* chan_no_cache = helper_create_channel_with_cache(0, 1);
    assert(chan_no_cache);
    free(chan_no_cache->cache_header); // Make cache_header null
    chan_no_cache->cache_header = nullptr;
    file_valid->channels[0] = chan_no_cache; // Replace existing channel with this one
    assert(is_in_cache(file_valid, 0, 0) == 0);
    // file_valid's original channels[0] and its page are now leaked unless helper_free_iio_file handles it.
    // The helper_free_iio_file needs to be robust. For this specific test structure,
    // it's better to re-create file_valid or manage its channel freeing carefully.
    // Let's simplify by just testing the bps=0 case on a fresh setup.

    helper_free_iio_file(file_valid); // Clean up previous file_valid

    NfsIioFile* file_bps_zero = helper_create_iio_file(1);
    file_bps_zero->channels[0] = helper_create_channel_with_cache(1, 0); // bps=0
    assert(file_bps_zero->channels[0]);
    file_bps_zero->channels[0]->cache_header->pages[0] = helper_create_cache_page(0x100, 0, 0);
    assert(is_in_cache(file_bps_zero, 0, 0) == 0); // bps=0 should result in false
    helper_free_iio_file(file_bps_zero);
}

// --- Tests for cache_update ---

void test_cache_update_page_not_in_cache() {
    // Simulates when the page for the given byte_pos is not yet created.
    // Expects cache_expand (if needed), cache_page_create, cache_pageflush (for the slot),
    // and cache_page_refresh to be called.
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(0, 1); // bps=1, 0 allocated pages
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 1024, 0); // bps=1

    // Prepare disk content for page 0 (byte_pos 0 to 511)
    std::vector<char> disk_content_p0(512);
    helper_fill_buffer_pattern(disk_content_p0.data(), 512, 70);
    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Skip header
    fwrite(disk_content_p0.data(), 1, 512, file->file_handle);
    fflush(file->file_handle);

    int old_clock = nfs_iio_CLOCK = 500;
    cache_update(file, 0, 100); // byte_pos 100 falls into page_array_idx 0

    // cache_update calls:
    // 1. cache_expand (if needed, here from 0 to 1024 if page_idx 0 requires it)
    // 2. cache_page_create (for page_idx 0, since pages[0] is initially null)
    //    - Inside create (new page path): new_page->last_access_time = CLOCK (500)
    // 3. cache_pageflush (for page_idx 0, but new page is clean, so no dump)
    // 4. cache_page_refresh (for page_idx 0, aligned_pos 0)
    //    - Inside refresh: channel_pos_to_absolute_block -> nfs_iio_blocks_per_chunk (CLOCK becomes 501)
    //    - Inside refresh: read_absolute_block_n
    //    - Inside refresh: target_page->last_access_time = CLOCK (now 501)

    assert(file->channels[0]->cache_header->num_pages_allocated >= 1);
    NfsIioCachePage* page0 = file->channels[0]->cache_header->pages[0];
    assert(page0 != nullptr);
    assert(page0->disk_block_position == 0); // Aligned for byte_pos 100
    assert(page0->last_access_time == old_clock + 1); // Updated by refresh
    assert(cache_page_get_sync(page0) == 1); // Clean after refresh

    char* page_buffer = (char*)cache_page_get_buffer(page0);
    assert(page_buffer != nullptr);
    assert(memcmp(page_buffer, disk_content_p0.data(), 512) == 0);

    assert(nfs_iio_CLOCK == old_clock + 1); // Final clock state

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}

void test_cache_update_page_already_in_cache_different_pos() {
    // Simulates updating a slot that was previously caching a *different* disk position.
    // Expects the old page in the slot to be flushed (if dirty) before the new one is refreshed.
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    // bps=1, 1 allocated page (slot 0)
    file->channels[0] = helper_create_channel_with_cache(1, 1);
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 2048, 0); // Enough size for multiple pages

    // Setup page slot 0 to initially hold data for disk_block_position 512 (page 1's data)
    // And mark it dirty.
    static char initial_page0_buffer_data[512]; // For the page *object*
    helper_fill_buffer_pattern(initial_page0_buffer_data, 512, 80); // Content for "page 1"
    file->channels[0]->cache_header->pages[0] = helper_create_page_with_real_buffer(0, 512, 550, true); // disk_pos 512, dirty
    assert(file->channels[0]->cache_header->pages[0]);
    // Override buffer to point to our static one
    free(cache_page_get_buffer(file->channels[0]->cache_header->pages[0])); // free the one from helper
    file->channels[0]->cache_header->pages[0]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(initial_page0_buffer_data);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0); // Ensure dirty


    // Prepare disk content for the *new* target page (page 0, disk_pos 0)
    std::vector<char> disk_content_for_target_page0(512);
    helper_fill_buffer_pattern(disk_content_for_target_page0.data(), 512, 75);
    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv, SEEK_SET); // Write for target page 0 (at abs block 0)
    fwrite(disk_content_for_target_page0.data(), 1, 512, file->file_handle);
    fflush(file->file_handle);


    int old_clock = nfs_iio_CLOCK = 600;
    // Request to update cache for byte_pos 100 (which is page 0, disk_pos 0)
    // This will use slot 0, which currently holds data for disk_pos 512.
    cache_update(file, 0, 100);

    // Expected sequence in cache_update for slot 0:
    // 1. Page exists (pages[0] is not null).
    // 2. cache_pageflush(file, 0, 0) is called.
    //    - Pages[0] is dirty and its disk_pos is 512.
    //    - cache_page_dump will write initial_page0_buffer_data (seed 80) to
    //      channel_pos_to_absolute_block(file, 0, 512) which is absolute block 1.
    //      (CLOCK becomes 601 due to nfs_iio_blocks_per_chunk in dump's path)
    //    - Pages[0] becomes clean.
    // 3. cache_page_refresh(file, 0, 0) is called (aligned_page_start_byte_pos is 0 for input 100).
    //    - channel_pos_to_absolute_block(file, 0, 0) -> abs block 0
    //      (CLOCK becomes 602 due to nfs_iio_blocks_per_chunk in refresh's path)
    //    - read_absolute_block_n loads disk_content_for_target_page0 (seed 75) into page buffer.
    //    - Pages[0]->disk_block_position becomes 0.
    //    - Pages[0]->last_access_time = CLOCK (now 602).

    NfsIioCachePage* page_in_slot0 = file->channels[0]->cache_header->pages[0];
    assert(page_in_slot0 != nullptr);
    assert(page_in_slot0->disk_block_position == 0); // Should now point to the new data
    assert(page_in_slot0->last_access_time == old_clock + 2);
    assert(cache_page_get_sync(page_in_slot0) == 1);

    // Verify buffer content of page in slot 0 (should be data for disk_pos 0, seed 75)
    char* current_page_buffer = (char*)cache_page_get_buffer(page_in_slot0);
    assert(current_page_buffer != nullptr);
    assert(helper_verify_buffer_pattern(current_page_buffer, 512, 75));

    // Verify that the *original* content of slot 0 (for disk_pos 512, seed 80) was flushed to disk
    std::vector<char> flushed_content_validation_buf(512);
    fseek(file->file_handle, nfs_iio_BLOCK_SIZEv + 512, SEEK_SET); // Read data for abs block 1
    fread(flushed_content_validation_buf.data(), 1, 512, file->file_handle);
    assert(helper_verify_buffer_pattern(flushed_content_validation_buf.data(), 512, 80));

    assert(nfs_iio_CLOCK == old_clock + 2);

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}
// TODO: Add tests for cache_update with bps=0 or block_size=0.
// TODO: Add tests for cache_update where cache_expand or cache_page_create fails.

// --- Tests for cacheflush (IIO channel level) ---

void test_cacheflush_iio_channel_multiple_pages() {
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 512;

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 1);
    assert(file);
    file->channels[0] = helper_create_channel_with_cache(3, 1); // 3 slots, bps=1
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 3 * 512, 0);


    // Page 0: dirty, disk_pos 0
    static char p0_buf[512]; helper_fill_buffer_pattern(p0_buf, 512, 100);
    file->channels[0]->cache_header->pages[0] = helper_create_page_with_real_buffer(0, 0, 0, true); // disk_pos 0, dirty
    assert(file->channels[0]->cache_header->pages[0]);
    free(cache_page_get_buffer(file->channels[0]->cache_header->pages[0]));
    file->channels[0]->cache_header->pages[0]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(p0_buf);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0);


    // Page 1: clean, disk_pos 512
    static char p1_buf[512]; helper_fill_buffer_pattern(p1_buf, 512, 101); // Content that won't be flushed
    file->channels[0]->cache_header->pages[1] = helper_create_page_with_real_buffer(0, 512, 0, false); // disk_pos 512, clean
    assert(file->channels[0]->cache_header->pages[1]);
    free(cache_page_get_buffer(file->channels[0]->cache_header->pages[1]));
    file->channels[0]->cache_header->pages[1]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(p1_buf);
    cache_page_set_sync(file->channels[0]->cache_header->pages[1], 1);


    // Page 2: dirty, disk_pos 1024
    static char p2_buf[512]; helper_fill_buffer_pattern(p2_buf, 512, 102);
    file->channels[0]->cache_header->pages[2] = helper_create_page_with_real_buffer(0, 1024, 0, true); // disk_pos 1024, dirty
    assert(file->channels[0]->cache_header->pages[2]);
    free(cache_page_get_buffer(file->channels[0]->cache_header->pages[2]));
    file->channels[0]->cache_header->pages[2]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(p2_buf);
    cache_page_set_sync(file->channels[0]->cache_header->pages[2], 0);

    int clock_before_call = nfs_iio_CLOCK = 700;
    cacheflush(file, 0); // Flush channel 0

    // Page 0 and Page 2 should now be clean. Page 1 remains clean.
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[0]) == 1);
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[1]) == 1);
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[2]) == 1);

    // CLOCK should increment twice (once for page 0 dump, once for page 2 dump)
    assert(nfs_iio_CLOCK == clock_before_call + 2);

    // Verify disk content for page 0 and page 2
    fflush(file->file_handle);
    FILE* vf = fopen(TEST_ABS_IO_FILENAME, "rb");
    assert(vf);
    std::vector<char> rbuf(512);

    fseek(vf, (long)nfs_iio_BLOCK_SIZEv + (long)0 * 512, SEEK_SET); fread(rbuf.data(), 1, 512, vf);
    assert(helper_verify_buffer_pattern(rbuf.data(), 512, 100)); // Page 0 content

    fseek(vf, (long)nfs_iio_BLOCK_SIZEv + (long)1 * 512, SEEK_SET); fread(rbuf.data(), 1, 512, vf);
    // Content for page 1 should NOT have been written by the flush, so it might be zeros or old data.
    // We cannot reliably verify its content here unless we wrote something else there first.

    fseek(vf, (long)nfs_iio_BLOCK_SIZEv + (long)2 * 512, SEEK_SET); fread(rbuf.data(), 1, 512, vf);
    assert(helper_verify_buffer_pattern(rbuf.data(), 512, 102)); // Page 2 content
    fclose(vf);

    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}
// TODO: Add tests for cacheflush with invalid inputs, null channel, empty cache.

// --- Tests for flush_data (IIO file level) ---

// --- Tests for flush_data (IIO file level) ---

void test_flush_data_iio_multiple_channels() {
    // This test is similar to test_cacheflush_iio_channel_multiple_pages,
    // but extended for multiple channels. It also tests fflush on the file handle.
    int old_bs = nfs_iio_BLOCK_SIZEv;
    nfs_iio_BLOCK_SIZEv = 256; // Use a smaller block size for variety

    NfsIioFile* file = helper_setup_iio_file_for_abs_io(TEST_ABS_IO_FILENAME, "w+b", 2); // 2 channels
    assert(file);

    // Channel 0 setup (1 slot, bps=1)
    file->channels[0] = helper_create_channel_with_cache(1, 1);
    assert(file->channels[0] && file->channels[0]->cache_header);
    helper_set_channel_properties(file->channels[0], 1, 256, 0);
    static char ch0_p0_buf[256]; helper_fill_buffer_pattern(ch0_p0_buf, 256, 200);
    file->channels[0]->cache_header->pages[0] = helper_create_page_with_real_buffer(0, 0, 0, true); // disk_pos 0, dirty
    assert(file->channels[0]->cache_header->pages[0]);
    free(cache_page_get_buffer(file->channels[0]->cache_header->pages[0]));
    file->channels[0]->cache_header->pages[0]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(ch0_p0_buf);
    cache_page_set_sync(file->channels[0]->cache_header->pages[0], 0);


    // Channel 1 setup (1 slot, bps=2)
    file->channels[1] = helper_create_channel_with_cache(1, 2);
    assert(file->channels[1] && file->channels[1]->cache_header);
    helper_set_channel_properties(file->channels[1], 2, 2 * 256, 0);
    static char ch1_p0_buf[2 * 256]; helper_fill_buffer_pattern(ch1_p0_buf, 2 * 256, 210);
    file->channels[1]->cache_header->pages[0] = helper_create_page_with_real_buffer(0, 0, 0, true); // disk_pos 0, dirty
    assert(file->channels[1]->cache_header->pages[0]);
    free(cache_page_get_buffer(file->channels[1]->cache_header->pages[0]));
    file->channels[1]->cache_header->pages[0]->buffer_ptr_and_syncflag_storage = reinterpret_cast<uintptr_t>(ch1_p0_buf);
    cache_page_set_sync(file->channels[1]->cache_header->pages[0], 0);

    int clock_before_call = nfs_iio_CLOCK = 800;
    flush_data(file); // Flush all channels and the file handle

    // All pages should be clean
    assert(cache_page_get_sync(file->channels[0]->cache_header->pages[0]) == 1);
    assert(cache_page_get_sync(file->channels[1]->cache_header->pages[0]) == 1);

    // CLOCK should increment once for each dirty page flushed
    // Ch0 had 1 dirty page. Ch1 had 1 dirty page. Total 2 increments.
    assert(nfs_iio_CLOCK == clock_before_call + 2);

    // Verify disk content
    // Note: fflush(file->file_handle) inside flush_data should ensure data is written.
    FILE* vf = fopen(TEST_ABS_IO_FILENAME, "rb");
    assert(vf);
    std::vector<char> rbuf_ch0(256);
    std::vector<char> rbuf_ch1(2 * 256);

    // Verify Ch0 Page 0 (disk_pos 0, abs_block_idx 0 for ch0)
    // abs_block_idx 0 for ch0 maps to file abs_block_idx 0 (assuming simple interleaving for this test setup)
    int abs_block_ch0_p0 = channel_pos_to_absolute_block(file, 0, 0); // Should be 0
    fseek(vf, (long)header_size(file) + (long)abs_block_ch0_p0 * nfs_iio_BLOCK_SIZEv, SEEK_SET);
    fread(rbuf_ch0.data(), 1, rbuf_ch0.size(), vf);
    assert(helper_verify_buffer_pattern(rbuf_ch0.data(), rbuf_ch0.size(), 200));

    // Verify Ch1 Page 0 (disk_pos 0, abs_block_idx 0 for ch1)
    // abs_block_idx 0 for ch1 maps to file abs_block_idx 1 (if ch0_bps=1, ch1_bps=2 -> chunk is 1+2=3 blocks. ch1 starts at offset 1 in chunk)
    int abs_block_ch1_p0 = channel_pos_to_absolute_block(file, 1, 0); // Depends on interleaving.
    // Ch0 bps=1, Ch1 bps=2. Total=3.
    // Ch1 rel_block 0 => chunk_idx 0, off_in_stripe 0. base_off 1. (0+1)+(0*3)=1
    // Ch1 rel_block 1 => chunk_idx 0, off_in_stripe 1. base_off 1. (1+1)+(0*3)=2
    assert(abs_block_ch1_p0 == 1); // For the first block of channel 1's page 0.
    fseek(vf, (long)header_size(file) + (long)abs_block_ch1_p0 * nfs_iio_BLOCK_SIZEv, SEEK_SET);
    fread(rbuf_ch1.data(), 1, rbuf_ch1.size(), vf); // Reads 2 blocks
    assert(helper_verify_buffer_pattern(rbuf_ch1.data(), rbuf_ch1.size(), 210));

    fclose(vf);
    helper_teardown_iio_file(file);
    nfs_iio_BLOCK_SIZEv = old_bs;
}
// TODO: Add tests for flush_data with null file, no file_handle.

void run_all_tests() {
	// --- 單元測試 ---
	RUN_TEST(test_bit_operations);

    // lock_check 測試
    RUN_TEST(test_lock_check_no_lock_file);
    RUN_TEST(test_lock_check_exclusive_lock);
    RUN_TEST(test_lock_check_not_readable_lock);
    RUN_TEST(test_lock_check_shared_lock_readable);
    RUN_TEST(test_lock_check_read_error);

    // lock_leave 測試
    RUN_TEST(test_lock_leave_no_lock_file);
    RUN_TEST(test_lock_leave_decrement_count);
    RUN_TEST(test_lock_leave_remove_on_zero_count);
    RUN_TEST(test_lock_leave_read_write_errors);

    // Cache Page Sync and Buffer Operations
    RUN_TEST(test_cache_page_sync_and_buffer_operations);

    // Cache Page Choose Best to Reuse
    RUN_TEST(test_cache_page_choose_best_to_reuse_basic_lru);
    RUN_TEST(test_cache_page_choose_best_to_reuse_with_exclusion);
    RUN_TEST(test_cache_page_choose_best_to_reuse_single_page);
    RUN_TEST(test_cache_page_choose_best_to_reuse_empty_cache);
    RUN_TEST(test_cache_page_choose_best_to_reuse_null_inputs);

    // Cache Expand
    RUN_TEST(test_cache_expand_initial_allocation);
    RUN_TEST(test_cache_expand_to_meet_requirement_below_1024);
    RUN_TEST(test_cache_expand_to_meet_requirement_above_1024);
    RUN_TEST(test_cache_expand_already_sufficient);
    RUN_TEST(test_cache_expand_realloc_initialization);
    RUN_TEST(test_cache_expand_null_inputs);

    // Absolute Block I/O Tests
    RUN_TEST(test_abs_block_io_single_block);
    RUN_TEST(test_abs_block_io_multiple_blocks);
    RUN_TEST(test_abs_block_io_read_past_eof);
    RUN_TEST(test_abs_block_io_invalid_inputs);

    // Write Header Tests
    RUN_TEST(test_write_header_no_channels);
    RUN_TEST(test_write_header_multiple_channels);
    RUN_TEST(test_write_header_invalid_inputs);

    // nfs_iio_blocks_per_chunk tests
    RUN_TEST(test_nfs_iio_blocks_per_chunk_null_file);
    RUN_TEST(test_nfs_iio_blocks_per_chunk_no_channels_array);
    RUN_TEST(test_nfs_iio_blocks_per_chunk_zero_channels);
    RUN_TEST(test_nfs_iio_blocks_per_chunk_single_channel);
    RUN_TEST(test_nfs_iio_blocks_per_chunk_multiple_channels);
    RUN_TEST(test_nfs_iio_blocks_per_chunk_with_null_channel_in_array);

    // nfs_iio_seek tests
    RUN_TEST(test_nfs_iio_seek_valid);
    RUN_TEST(test_nfs_iio_seek_invalid_inputs);

    // nfs_iio_get_channel tests
    RUN_TEST(test_nfs_iio_get_channel_valid);
    RUN_TEST(test_nfs_iio_get_channel_invalid_inputs);

    // nfs_iio_channel_size tests
    RUN_TEST(test_nfs_iio_channel_size_valid);
    RUN_TEST(test_nfs_iio_channel_size_null_channel);

    // nfs_iio_channel_blocks tests
    RUN_TEST(test_nfs_iio_channel_blocks_valid);
    RUN_TEST(test_nfs_iio_channel_blocks_null_channel);

    // nfs_iio_channel_truncate tests
    RUN_TEST(test_nfs_iio_channel_truncate_valid);
    RUN_TEST(test_nfs_iio_channel_truncate_invalid_inputs);

    // channel_block_to_absolute_block tests
    RUN_TEST(test_channel_block_to_absolute_block_single_channel);
    RUN_TEST(test_channel_block_to_absolute_block_multiple_channels);
    RUN_TEST(test_channel_block_to_absolute_block_invalid_inputs);

    // channel_pos_to_absolute_block tests
    RUN_TEST(test_channel_pos_to_absolute_block_valid);
    RUN_TEST(test_channel_pos_to_absolute_block_zero_block_size);

    // cache_page_dump tests
    RUN_TEST(test_cache_page_dump_valid_page);
    RUN_TEST(test_cache_page_dump_invalid_inputs);

    // cache_pageflush tests
    RUN_TEST(test_cache_pageflush_dirty_page);
    RUN_TEST(test_cache_pageflush_clean_page);
    RUN_TEST(test_cache_pageflush_invalid_inputs);

    // cache_page_create tests
    RUN_TEST(test_cache_page_create_new_page_below_threshold);
    RUN_TEST(test_cache_page_create_reuse_page_at_threshold);
    // Add more cache_page_create tests for edge cases and error conditions

    // cache_page_refresh tests
    RUN_TEST(test_cache_page_refresh_new_page);
    RUN_TEST(test_cache_page_refresh_existing_page);
    // Add more cache_page_refresh tests
    
    // cache_create (IIO version) tests
    RUN_TEST(test_cache_create_iio_valid_channel);
    RUN_TEST(test_cache_create_iio_invalid_file);
    RUN_TEST(test_cache_create_iio_invalid_channel_idx);

    // cache_destroy (IIO version) tests
    RUN_TEST(test_cache_destroy_iio_null_cache);
    RUN_TEST(test_cache_destroy_iio_empty_pages_array);
    RUN_TEST(test_cache_destroy_iio_with_pages_and_buffers);

    // is_in_cache (IIO version) tests
    RUN_TEST(test_is_in_cache_iio_basic_cases);
    RUN_TEST(test_is_in_cache_iio_invalid_inputs_and_bps_zero);

    // cache_update tests
    RUN_TEST(test_cache_update_page_not_in_cache);
    RUN_TEST(test_cache_update_page_already_in_cache_different_pos);

    // cacheflush (IIO channel level) tests
    RUN_TEST(test_cacheflush_iio_channel_multiple_pages);

    // flush_data (IIO file level) tests
    RUN_TEST(test_flush_data_iio_multiple_channels);

	// 最終清理
    cleanup_test_files(TEST_VFS_BASENAME);
    cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
    std::filesystem::remove(TEST_ABS_IO_FILENAME);
    std::filesystem::remove(TEST_WRITE_HEADER_FILENAME);
    std::filesystem::remove(TEST_ABS_IO_FILENAME); // Cleanup for these tests
}

int print_test_result() {
	std::cout << "========================================" << std::endl;
	std::cout << "All tests completed." << std::endl;
	std::cout << "Tests Passed: " << tests_passed << std::endl;
	std::cout << "Tests Failed: " << tests_failed << std::endl;
	std::cout << "========================================" << std::endl;

	if (tests_failed > 0) {
		std::cerr << "THERE WERE TEST FAILURES!" << std::endl;
		return 1; // 返回非零表示錯誤
	}
	return 0; // 所有測試通過
}