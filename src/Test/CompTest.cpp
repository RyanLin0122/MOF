#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstdint> // For uintptr_t
#include <algorithm> // For std::equal
#include "Image/Comp.h"
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <cassert>

// 輔助函式：將位元組向量以十六進位格式印出
void print_bytes(const std::string& title, const std::vector<unsigned char>& bytes) {
    std::cout << title << " (" << bytes.size() << " bytes):" << std::endl;
    for (size_t i = 0; i < bytes.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec << std::endl << "----------------------------------------" << std::endl;
}

// 輔助函式：從 Temp.dat 讀取壓縮結果
std::vector<unsigned char> read_temp_file() {
    std::ifstream file("Temp.dat", std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open Temp.dat" << std::endl;
        return {};
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

// 通用測試執行器
bool run_test(const std::string& test_name,
    unsigned char unit_size,

    const std::vector<unsigned char>& input,
    const std::vector<unsigned char>& expected_compressed) {
    std::cout << "===== Running Test: " << test_name << " (Unit Size: " << (int)unit_size << ") =====" << std::endl;
    print_bytes("Input", input);

    // --- 測試壓縮 ---
    // 注意：處理不安全的 FILE* -> size_t 轉型
    FILE* result_ptr = run_length_comp(const_cast<unsigned char*>(input.data()), input.size(), unit_size);
    size_t compressed_size = reinterpret_cast<uintptr_t>(result_ptr);

    std::vector<unsigned char> actual_compressed = read_temp_file();

    if (compressed_size != actual_compressed.size()) {
        std::cerr << "[FAIL] Compressed size mismatch! Reported: " << compressed_size << ", Actual file size: " << actual_compressed.size() << std::endl;
        return false;
    }

    print_bytes("Expected Compressed", expected_compressed);
    print_bytes("Actual Compressed", actual_compressed);

    if (actual_compressed != expected_compressed) {
        std::cerr << "[FAIL] Compression output does not match expected output." << std::endl;
        return false;
    }
    std::cout << "[PASS] Compression" << std::endl;

    // --- 測試解壓縮 ---
    std::vector<unsigned char> decompressed_output(input.size() * 2 + 1024); // 分配足夠大的空間
    run_length_decomp(actual_compressed.data(), actual_compressed.size(), decompressed_output.data(), decompressed_output.size(), unit_size);

    // 根據原始輸入大小調整解壓縮後向量的大小
    size_t effective_input_size = (input.size() / unit_size) * unit_size;

    // 根據有效長度，建立一個預期的解壓縮結果
    std::vector<unsigned char> expected_decompressed(input.begin(), input.begin() + effective_input_size);

    // 調整實際解壓縮輸出的尺寸，以匹配有效長度
    decompressed_output.resize(effective_input_size);

    print_bytes("Expected Decompressed", expected_decompressed);
    print_bytes("Actual Decompressed", decompressed_output);

    if (decompressed_output != expected_decompressed) {
        std::cerr << "[FAIL] Decompression output does not match original input." << std::endl;
        return false;
    }
    std::cout << "[PASS] Decompression" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    return true;
}


int run_comp_test() {
    int passed = 0;
    int total = 0;

    // =======================================================
    // ===               BYTE (unit_size = 1)              ===
    // =======================================================

    // 測試 1: 基本 RLE 序列
    if (run_test("BYTE: Basic Run", 1,
        { 'A', 'B', 'C', 'C', 'C', 'C', 'D' },
        { 'A', 'B', 0xB4, 'C', 4, 'D' })) {
        passed++;
    } total++;

    // 測試 2: 無重複，無壓縮
    if (run_test("BYTE: No Run", 1,
        { 'A', 'B', 'C', 'D', 'E', 'F' },
        { 'A', 'B', 'C', 'D', 'E', 'F' })) {
        passed++;
    } total++;

    // 測試 3: 處理標記字元 0xB4
    if (run_test("BYTE: Contains Marker", 1,
        { 'X', 'Y', 0xB4, 'Z' },
        { 'X', 'Y', 0xB4, 0xB4, 'Z' })) {
        passed++;
    } total++;

    // 測試 4: 連續的標記字元
    if (run_test("BYTE: Run of Markers", 1,
        { 0xB4, 0xB4, 0xB4 },
        { 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4 })) {
        passed++;
    } total++;

    // 測試 5: 空輸入
    if (run_test("BYTE: Empty Input", 1,
        {},
        {})) {
        passed++;
    } total++;

    // 測試 6: 結尾 RLE
    if (run_test("BYTE: Run at End", 1,
        { 'A', 'B', 'C', 'C', 'C' },
        { 'A', 'B', 0xB4, 'C', 3 })) {
        passed++;
    } total++;

    // 測試 7: 計數器超過 254
    std::vector<unsigned char> long_run(256, 'A');
    std::vector<unsigned char> long_run_compressed = { 0xB4, 'A', 254, 'A', 'A' };
    if (run_test("BYTE: Count > 254", 1, long_run, long_run_compressed)) { passed++; } total++;


    // =======================================================
    // ===               WORD (unit_size = 2)              ===
    // =======================================================

    // 測試 8: 基本 WORD RLE
    if (run_test("WORD: Basic Run", 2,
        { 0x01, 0x02, 0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x05, 0x06 },
        { 0x01, 0x02, 0xB4, 0xB4, 0x03, 0x04, 0x03, 0x00, 0x05, 0x06 })) {
        passed++;
    } total++;
    // 0x0304 出現 3 次。 壓縮後為 marker(B4B4), value(0304), count(0003) -> 小端序

// 測試 9: 處理 WORD 標記 0xB4B4
    if (run_test("WORD: Contains Marker", 2,
        { 0x11, 0x22, 0xB4, 0xB4, 0x33, 0x44 },
        { 0x11, 0x22, 0xB4, 0xB4, 0xB4, 0xB4, 0x33, 0x44 })) {
        passed++;
    } total++;

    // 測試 10: 輸入長度為奇數 (忽略最後一個 byte)
    if (run_test("WORD: Odd Length Input", 2,
        { 'A', 'B', 'C', 'D', 'E' },
        { 'A', 'B', 'C', 'D' })) {
        passed++;
    } total++; // 'E' 被忽略


    // =======================================================
    // ===              DWORD (unit_size = 4)              ===
    // =======================================================

    // 測試 11: 基本 DWORD RLE
    if (run_test("DWORD: Basic Run", 4,
        { 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },
        { 0xB4, 0xB4, 0xB4, 0xB4, 0x01, 0x02, 0x03, 0x04, 0x03, 0x00, 0x00, 0x00, 0x05, 0x06, 0x07, 0x08 })) {
        passed++;
    } total++;
    // 0x04030201 出現 3 次。 壓縮後為 marker, value, count(3) -> 小端序

// 測試 12: 處理 DWORD 標記 0xB4B4B4B4
    if (run_test("DWORD: Contains Marker", 4,
        { 0x11, 0x22, 0x33, 0x44, 0xB4, 0xB4, 0xB4, 0xB4, 0x55, 0x66, 0x77, 0x88 },
        { 0x11, 0x22, 0x33, 0x44, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0x55, 0x66, 0x77, 0x88 })) {
        passed++;
    } total++;

    // 測試 13: 輸入長度不為 4 的倍數
    if (run_test("DWORD: Non-4-byte aligned", 4,
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A },
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 })) {
        passed++;
    } total++; // 最後兩個 byte 被忽略


    // --- Final Report ---
    std::cout << "\n====================\n";
    std::cout << "      SUMMARY\n";
    std::cout << "====================\n";
    std::cout << passed << " / " << total << " tests passed." << std::endl;

    return (passed == total) ? 0 : 1;
}