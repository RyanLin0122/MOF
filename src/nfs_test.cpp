#include <cassert> // 可以使用 assert 進行簡單斷言
#include <filesystem> // C++17, 用於檔案系統操作 (如刪除測試檔案)
#include <stdio.h>
#include <iostream>
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


void run_all_tests() {
	// --- 單元測試 ---
	RUN_TEST(test_bit_operations);

	// 最終清理
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