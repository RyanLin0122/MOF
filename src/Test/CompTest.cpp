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

// ���U�禡�G�N�줸�զV�q�H�Q���i��榡�L�X
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

// ���U�禡�G�q Temp.dat Ū�����Y���G
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

// �q�δ��հ��澹
bool run_test(const std::string& test_name,
    unsigned char unit_size,

    const std::vector<unsigned char>& input,
    const std::vector<unsigned char>& expected_compressed) {
    std::cout << "===== Running Test: " << test_name << " (Unit Size: " << (int)unit_size << ") =====" << std::endl;
    print_bytes("Input", input);

    // --- �������Y ---
    // �`�N�G�B�z���w���� FILE* -> size_t �૬
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

    // --- ���ո����Y ---
    std::vector<unsigned char> decompressed_output(input.size() * 2 + 1024); // ���t�����j���Ŷ�
    run_length_decomp(actual_compressed.data(), actual_compressed.size(), decompressed_output.data(), decompressed_output.size(), unit_size);

    // �ھڭ�l��J�j�p�վ�����Y��V�q���j�p
    size_t effective_input_size = (input.size() / unit_size) * unit_size;

    // �ھڦ��Ī��סA�إߤ@�ӹw���������Y���G
    std::vector<unsigned char> expected_decompressed(input.begin(), input.begin() + effective_input_size);

    // �վ��ڸ����Y��X���ؤo�A�H�ǰt���Ī���
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

    // ���� 1: �� RLE �ǦC
    if (run_test("BYTE: Basic Run", 1,
        { 'A', 'B', 'C', 'C', 'C', 'C', 'D' },
        { 'A', 'B', 0xB4, 'C', 4, 'D' })) {
        passed++;
    } total++;

    // ���� 2: �L���ơA�L���Y
    if (run_test("BYTE: No Run", 1,
        { 'A', 'B', 'C', 'D', 'E', 'F' },
        { 'A', 'B', 'C', 'D', 'E', 'F' })) {
        passed++;
    } total++;

    // ���� 3: �B�z�аO�r�� 0xB4
    if (run_test("BYTE: Contains Marker", 1,
        { 'X', 'Y', 0xB4, 'Z' },
        { 'X', 'Y', 0xB4, 0xB4, 'Z' })) {
        passed++;
    } total++;

    // ���� 4: �s�򪺼аO�r��
    if (run_test("BYTE: Run of Markers", 1,
        { 0xB4, 0xB4, 0xB4 },
        { 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4 })) {
        passed++;
    } total++;

    // ���� 5: �ſ�J
    if (run_test("BYTE: Empty Input", 1,
        {},
        {})) {
        passed++;
    } total++;

    // ���� 6: ���� RLE
    if (run_test("BYTE: Run at End", 1,
        { 'A', 'B', 'C', 'C', 'C' },
        { 'A', 'B', 0xB4, 'C', 3 })) {
        passed++;
    } total++;

    // ���� 7: �p�ƾ��W�L 254
    std::vector<unsigned char> long_run(256, 'A');
    std::vector<unsigned char> long_run_compressed = { 0xB4, 'A', 254, 'A', 'A' };
    if (run_test("BYTE: Count > 254", 1, long_run, long_run_compressed)) { passed++; } total++;


    // =======================================================
    // ===               WORD (unit_size = 2)              ===
    // =======================================================

    // ���� 8: �� WORD RLE
    if (run_test("WORD: Basic Run", 2,
        { 0x01, 0x02, 0x03, 0x04, 0x03, 0x04, 0x03, 0x04, 0x05, 0x06 },
        { 0x01, 0x02, 0xB4, 0xB4, 0x03, 0x04, 0x03, 0x00, 0x05, 0x06 })) {
        passed++;
    } total++;
    // 0x0304 �X�{ 3 ���C ���Y�ᬰ marker(B4B4), value(0304), count(0003) -> �p�ݧ�

// ���� 9: �B�z WORD �аO 0xB4B4
    if (run_test("WORD: Contains Marker", 2,
        { 0x11, 0x22, 0xB4, 0xB4, 0x33, 0x44 },
        { 0x11, 0x22, 0xB4, 0xB4, 0xB4, 0xB4, 0x33, 0x44 })) {
        passed++;
    } total++;

    // ���� 10: ��J���׬��_�� (�����̫�@�� byte)
    if (run_test("WORD: Odd Length Input", 2,
        { 'A', 'B', 'C', 'D', 'E' },
        { 'A', 'B', 'C', 'D' })) {
        passed++;
    } total++; // 'E' �Q����


    // =======================================================
    // ===              DWORD (unit_size = 4)              ===
    // =======================================================

    // ���� 11: �� DWORD RLE
    if (run_test("DWORD: Basic Run", 4,
        { 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },
        { 0xB4, 0xB4, 0xB4, 0xB4, 0x01, 0x02, 0x03, 0x04, 0x03, 0x00, 0x00, 0x00, 0x05, 0x06, 0x07, 0x08 })) {
        passed++;
    } total++;
    // 0x04030201 �X�{ 3 ���C ���Y�ᬰ marker, value, count(3) -> �p�ݧ�

// ���� 12: �B�z DWORD �аO 0xB4B4B4B4
    if (run_test("DWORD: Contains Marker", 4,
        { 0x11, 0x22, 0x33, 0x44, 0xB4, 0xB4, 0xB4, 0xB4, 0x55, 0x66, 0x77, 0x88 },
        { 0x11, 0x22, 0x33, 0x44, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0xB4, 0x55, 0x66, 0x77, 0x88 })) {
        passed++;
    } total++;

    // ���� 13: ��J���פ��� 4 ������
    if (run_test("DWORD: Non-4-byte aligned", 4,
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A },
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 })) {
        passed++;
    } total++; // �̫��� byte �Q����


    // --- Final Report ---
    std::cout << "\n====================\n";
    std::cout << "      SUMMARY\n";
    std::cout << "====================\n";
    std::cout << passed << " / " << total << " tests passed." << std::endl;

    return (passed == total) ? 0 : 1;
}