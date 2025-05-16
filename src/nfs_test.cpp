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

// 測試 header_size 函數中的 _exit 條件 (如果可以安全地觸發)
// 這比較棘手，因為它會終止程式。在單元測試框架中，通常避免直接測試 _exit。
// 可以考慮將該檢查邏輯分離出來單獨測試，或者在測試中確保不會觸發它。
// 此處我們假設 nfs_iio_BLOCK_SIZEv 足夠大，不會觸發 _exit。

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

	// 最終清理
    std::filesystem::remove(TEST_ABS_IO_FILENAME);
    std::filesystem::remove(TEST_WRITE_HEADER_FILENAME);
	cleanup_test_files(TEST_VFS_BASENAME);
	cleanup_lock_file(LOCK_FILE_BASENAME_UNIT_TEST);
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