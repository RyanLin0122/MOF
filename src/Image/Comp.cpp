#include "Image/comp.h"

// 模擬 IDA Pro 反編譯中使用的 memset32 函式
extern "C" void memset32(void* dest, int val, size_t count) {
    auto d = static_cast<int*>(dest);
    for (size_t i = 0; i < count; ++i) {
        d[i] = val;
    }
}

// 計算檔案長度
FILE* file_length(const char* const FileName) {
    FILE* result = nullptr;
    FILE* v2 = nullptr;
    FILE* v3 = nullptr;

    result = fopen(FileName, "rb"); // 以二進位讀取模式打開檔案
    v2 = result;
    if (result) {
        fseek(result, 0, SEEK_END); // 將檔案指標移到檔案末尾
        v3 = reinterpret_cast<FILE*>(static_cast<long>(ftell(v2))); // 取得檔案大小
        fclose(v2); // 關閉檔案
        result = v3;
    }
    return result;
}

// 位元組壓縮
FILE* comp_BYTE(unsigned char* a1, unsigned int a2) {
    FILE* v2 = nullptr;
    FILE* result = nullptr;
    size_t v4 = 0;
    int v5 = 0;
    unsigned int v6 = 0;
    FILE* v7 = nullptr;
    char v8;
    size_t v9;
    unsigned int i = 0;
    char Buffer[4]; // 至少需要3個位元組來儲存 -76, -76, count
    unsigned char v12 = 0;
    char v13 = 0;

    v2 = fopen("file.cmp", "wb"); // 以二進位寫入模式打開檔案
    if (!v2) {
        SetFileAttributesA("file.cmp", FILE_ATTRIBUTE_NORMAL); // 如果打開失敗，設定為普通檔案屬性再試
        result = fopen("file.cmp", "wb");
        v2 = result;
        if (!v2)
            return nullptr;
    }

    v4 = 0;
    v5 = 0;
    v6 = 0;
    v7 = nullptr;
    for (i = 0; i < a2; v6 = i) {
        v8 = a1[v6];
        switch (v5) {
            case 0:
                if (v8 == static_cast<char>(0xB4)) { // 檢查是否為標記位元組 -76 (0xB4)
                    Buffer[v4++] = static_cast<char>(0xB4);
                    Buffer[v4++] = static_cast<char>(0xB4);
                    fwrite(Buffer, 1, v4, v2); // 寫入兩個 -76
                    v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                    v4 = 0;
                    v5 = 0;
                } else {
                    Buffer[v4++] = v8;
                    v5 = 1;
                }
                ++i;
                break;
            case 1:
                if (v8 != Buffer[0]) { // 如果與前一個位元組不同
                    fwrite(Buffer, 1, v4, v2); // 寫入緩衝區內容
                    v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                    v4 = 0;
                    v5 = 0;
                    // 重新處理當前位元組
                    if (v8 == static_cast<char>(0xB4)) {
                        Buffer[v4++] = static_cast<char>(0xB4);
                        Buffer[v4++] = static_cast<char>(0xB4);
                        fwrite(Buffer, 1, v4, v2);
                        v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                        v4 = 0;
                        v5 = 0;
                    } else {
                        Buffer[v4++] = v8;
                        v5 = 1;
                    }
                } else {
                    Buffer[v4++] = v8;
                    v5 = 2;
                }
                ++i;
                break;
            case 2:
                if (v8 != v12) { // v12 應該是 Buffer[1]
                    fwrite(Buffer, 1, v4, v2);
                    v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                    v4 = 0;
                    v5 = 0;
                    // 重新處理當前位元組
                    if (v8 == static_cast<char>(0xB4)) {
                        Buffer[v4++] = static_cast<char>(0xB4);
                        Buffer[v4++] = static_cast<char>(0xB4);
                        fwrite(Buffer, 1, v4, v2);
                        v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                        v4 = 0;
                        v5 = 0;
                    } else {
                        Buffer[v4++] = v8;
                        v5 = 1;
                    }
                } else {
                    v12 = a1[v6]; // 這裡是原始碼中的 v12 = a1[v6]; 這似乎是個錯誤，應該是Buffer[0]
                    v5 = 3;
                    Buffer[0] = static_cast<char>(0xB4); // 標記位元組
                    Buffer[1] = v12; // 重複的位元組
                    v13 = 3; // 計數
                    v4 = 3; // 緩衝區大小
                }
                ++i;
                break;
            default:
                if (v5 > 2) {
                    if (v8 != v12 || v5 > 254) { // v12 應該是 Buffer[1]
                        fwrite(Buffer, 1, v4, v2);
                        v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                        v4 = 0;
                        v5 = 0;
                        // 重新處理當前位元組
                        if (v8 == static_cast<char>(0xB4)) {
                            Buffer[v4++] = static_cast<char>(0xB4);
                            Buffer[v4++] = static_cast<char>(0xB4);
                            fwrite(Buffer, 1, v4, v2);
                            v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
                            v4 = 0;
                            v5 = 0;
                        } else {
                            Buffer[v4++] = v8;
                            v5 = 1;
                        }
                    } else {
                        ++v5;
                        Buffer[2] = static_cast<char>(v5); // 更新計數
                    }
                }
                ++i;
                break;
        }
    }
    if (v4 > 0) { // 寫入剩餘緩衝區的內容
        fwrite(Buffer, 1, v4, v2);
        v7 = reinterpret_cast<FILE*>(reinterpret_cast<char*>(v7) + v4);
    }
    fclose(v2);
    result = v7;
    return result;
}

// 雙位元組壓縮
int comp_WORD(unsigned char* a1, unsigned int a2) {
    FILE* v2 = nullptr;
    int v3 = 0;
    int v5 = 0;
    int v6 = 0;
    short v7;
    int v8;
    unsigned int v9;
    unsigned int v10 = 0;
    short Buffer[3]; // 至少需要3個 short 來儲存 -19276, value, count
    short v12 = 0;
    short v13 = 0;
    unsigned int v14 = 0;

    v2 = fopen("file.cmp", "wb");
    v3 = 0;
    if (!v2) {
        SetFileAttributesA("file.cmp", FILE_ATTRIBUTE_NORMAL);
        v2 = fopen("file.cmp", "wb");
        if (!v2)
            return 0;
    }
    v5 = 0;
    v6 = 0;
    v10 = 0;
    v14 = a2 >> 1; // 處理的 WORD 數量
    if (v14) {
        do {
            v7 = *reinterpret_cast<short*>(a1);
            switch (v6) {
                case 0:
                    if (v7 == static_cast<short>(0xB4B4)) { // 檢查是否為標記 WORD
                        Buffer[v5++] = static_cast<short>(0xB4B4);
                        Buffer[v5++] = static_cast<short>(0xB4B4);
                        fwrite(Buffer, sizeof(short), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                    } else {
                        Buffer[v5++] = v7;
                        v6 = 1;
                    }
                    ++v10;
                    a1 += 2;
                    break;
                case 1:
                    if (v7 != Buffer[0]) {
                        fwrite(Buffer, sizeof(short), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                        // 重新處理當前 WORD
                        if (v7 == static_cast<short>(0xB4B4)) {
                            Buffer[v5++] = static_cast<short>(0xB4B4);
                            Buffer[v5++] = static_cast<short>(0xB4B4);
                            fwrite(Buffer, sizeof(short), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                        } else {
                            Buffer[v5++] = v7;
                            v6 = 1;
                        }
                    } else {
                        v9 = v10;
                        Buffer[v5++] = v7;
                        v6 = 2;
                        v10 = v9 + 1;
                        a1 += 2;
                    }
                    break;
                case 2:
                    if (v7 != v12) { // v12 應該是 Buffer[1]
                        fwrite(Buffer, sizeof(short), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                        // 重新處理當前 WORD
                        if (v7 == static_cast<short>(0xB4B4)) {
                            Buffer[v5++] = static_cast<short>(0xB4B4);
                            Buffer[v5++] = static_cast<short>(0xB4B4);
                            fwrite(Buffer, sizeof(short), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                        } else {
                            Buffer[v5++] = v7;
                            v6 = 1;
                        }
                    } else {
                        v12 = *reinterpret_cast<short*>(a1); // 這裡是原始碼中的 v12 = *(_WORD *)a1; 這似乎是個錯誤，應該是Buffer[0]
                        v5 = 3;
                        v6 = 3;
                        Buffer[0] = static_cast<short>(0xB4B4);
                        Buffer[1] = v12;
                        v13 = 3; // count
                        ++v10;
                        a1 += 2;
                    }
                    break;
                default:
                    if (v6 > 2) {
                        if (v7 != v12 || v6 > 65534) { // v12 應該是 Buffer[1]
                            fwrite(Buffer, sizeof(short), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                            // 重新處理當前 WORD
                            if (v7 == static_cast<short>(0xB4B4)) {
                                Buffer[v5++] = static_cast<short>(0xB4B4);
                                Buffer[v5++] = static_cast<short>(0xB4B4);
                                fwrite(Buffer, sizeof(short), v5, v2);
                                v3 += v5;
                                v5 = 0;
                                v6 = 0;
                            } else {
                                Buffer[v5++] = v7;
                                v6 = 1;
                            }
                        } else {
                            Buffer[2] = static_cast<short>(v6 + 1); // 更新計數
                            ++v6;
                            ++v10;
                            a1 += 2;
                        }
                    }
                    break;
            }
        } while (v10 < v14);
    }
    if (v5 > 0) { // 寫入剩餘緩衝區的內容
        fwrite(Buffer, sizeof(short), v5, v2);
        v3 += v5;
    }
    fclose(v2);
    return 2 * v3;
}

// 四位元組壓縮
int comp_DWORD(unsigned char* a1, unsigned int a2) {
    FILE* v2 = nullptr;
    int v3 = 0;
    int v5 = 0;
    int v6 = 0;
    int v7;
    int v8;
    unsigned int v9;
    unsigned int v10 = 0;
    int Buffer[3]; // 至少需要3個 int 來儲存 -1263225676, value, count
    int v12 = 0;
    int v13 = 0;
    unsigned int v14 = 0;

    v2 = fopen("file.cmp", "wb");
    v3 = 0;
    if (!v2) {
        SetFileAttributesA("file.cmp", FILE_ATTRIBUTE_NORMAL);
        v2 = fopen("file.cmp", "wb");
        if (!v2)
            return 0;
    }
    v5 = 0;
    v6 = 0;
    v10 = 0;
    v14 = a2 >> 2; // 處理的 DWORD 數量
    if (v14) {
        do {
            v7 = *reinterpret_cast<int*>(a1);
            switch (v6) {
                case 0:
                    if (v7 == -1263225676) { // 檢查是否為標記 DWORD
                        Buffer[v5++] = -1263225676;
                        Buffer[v5++] = -1263225676;
                        fwrite(Buffer, sizeof(int), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                    } else {
                        Buffer[v5++] = v7;
                        v6 = 1;
                    }
                    ++v10;
                    a1 += 4;
                    break;
                case 1:
                    if (v7 != Buffer[0]) {
                        fwrite(Buffer, sizeof(int), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                        // 重新處理當前 DWORD
                        if (v7 == -1263225676) {
                            Buffer[v5++] = -1263225676;
                            Buffer[v5++] = -1263225676;
                            fwrite(Buffer, sizeof(int), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                        } else {
                            Buffer[v5++] = v7;
                            v6 = 1;
                        }
                    } else {
                        v9 = v10;
                        Buffer[v5++] = v7;
                        v6 = 2;
                        v10 = v9 + 1;
                        a1 += 4;
                    }
                    break;
                case 2:
                    if (v7 != v12) { // v12 應該是 Buffer[1]
                        fwrite(Buffer, sizeof(int), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                        // 重新處理當前 DWORD
                        if (v7 == -1263225676) {
                            Buffer[v5++] = -1263225676;
                            Buffer[v5++] = -1263225676;
                            fwrite(Buffer, sizeof(int), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                        } else {
                            Buffer[v5++] = v7;
                            v6 = 1;
                        }
                    } else {
                        v12 = *reinterpret_cast<int*>(a1); // 這裡是原始碼中的 v12 = *(_DWORD *)a1; 這似乎是個錯誤，應該是Buffer[0]
                        v5 = 3;
                        v6 = 3;
                        Buffer[0] = -1263225676;
                        Buffer[1] = v12;
                        v13 = 3; // count
                        ++v10;
                        a1 += 4;
                    }
                    break;
                default:
                    if (v7 != v12 || v6 == -1) { // v12 應該是 Buffer[1]
                        fwrite(Buffer, sizeof(int), v5, v2);
                        v3 += v5;
                        v5 = 0;
                        v6 = 0;
                        // 重新處理當前 DWORD
                        if (v7 == -1263225676) {
                            Buffer[v5++] = -1263225676;
                            Buffer[v5++] = -1263225676;
                            fwrite(Buffer, sizeof(int), v5, v2);
                            v3 += v5;
                            v5 = 0;
                            v6 = 0;
                        } else {
                            Buffer[v5++] = v7;
                            v6 = 1;
                        }
                    } else {
                        ++v6;
                        Buffer[2] = static_cast<int>(v6); // 更新計數
                        ++v10;
                        a1 += 4;
                    }
                    break;
            }
        } while (v10 < v14);
    }
    if (v5 > 0) { // 寫入剩餘緩衝區的內容
        fwrite(Buffer, sizeof(int), v5, v2);
        v3 += v5;
    }
    fclose(v2);
    return 4 * v3;
}

// 執行長度壓縮
FILE* run_length_comp(unsigned char* a1, unsigned int a2, unsigned char a3) {
    switch (a3) {
        case 1:
            return comp_BYTE(a1, a2);
        case 2:
            return reinterpret_cast<FILE*>(static_cast<long>(comp_WORD(a1, a2)));
        case 4:
            return reinterpret_cast<FILE*>(static_cast<long>(comp_DWORD(a1, a2)));
        default:
            return nullptr;
    }
}

// 位元組解壓縮
void decomp_BYTE(int a1, unsigned char* a2, unsigned int a3, unsigned char* a4) {
    unsigned int v4 = 0;
    int v5 = 0;
    unsigned char* v6 = a4;
    unsigned char* v7 = a2;
    unsigned char v8;
    unsigned char v9;
    unsigned char v10;
    unsigned char* v11;
    int v12;
    int v13;
    int v14 = 0; // v14 作為解壓縮後的目標緩衝區偏移量

    if (a3) {
        do {
            v8 = v7[v4];
            if (v8 == 0xB4) { // 檢查是否為標記位元組 -76 (0xB4)
                v9 = v7[++v4];
                if (v9 == 0xB4) { // 如果連續兩個 0xB4，表示原始資料就是 0xB4
                    v6[v5] = 0xB4; // 寫入一個 0xB4
                    v14 = ++v5;
                } else { // 否則為 RLE 壓縮格式
                    v10 = v7[++v4]; // 取得重複次數
                    if (v10) {
                        v11 = &v6[v5];
                        // 這裡的邏輯是將 v9 (重複的位元組) 填充 v10 次
                        // 原始程式碼中對 a1 的操作似乎是為了構建一個 32 位元的重複值
                        // 但對於 BYTE 模式，只需要直接填充 v9
                        
                        // 根據原始碼的 memset32 行為推斷，它會用 v12 (32位元值) 填充
                        // 因此需要將 v9 轉換為 32 位元值
                        int fill_val = (v9 << 24) | (v9 << 16) | (v9 << 8) | v9;
                        
                        v13 = v10 >> 2; // 整數個 DWORD
                        memset32(v11, fill_val, v13); // 填充 DWORD
                        // 填充剩餘的位元組 (0-3 個)
                        for (int k = 0; k < (v10 & 3); ++k) {
                            v11[4 * v13 + k] = v9;
                        }
                        
                        v7 = a2; // 重置指標
                        v5 = v10 + v14; // 更新目標緩衝區偏移量
                        v6 = a4; // 重置指標
                        v14 = v5; // 更新下次寫入的起始偏移量
                    }
                }
            } else { // 非壓縮資料，直接拷貝
                v6[v5] = v8;
                v14 = ++v5;
            }
            ++v4; // 處理下一個來源位元組
        } while (v4 < a3);
    }
}

// 雙位元組解壓縮
void decomp_WORD(int a1, unsigned char* a2, unsigned int a3, unsigned char* a4) {
    bool v4_zf;
    unsigned int v5 = 0;
    int v6 = 0;
    unsigned char* v8 = a4;
    short v9;
    unsigned short v10;
    unsigned char* v11;
    char v12_cf;
    int v13;
    unsigned char* v14;
    int i;
    unsigned char* v16;
    unsigned int v17;

    v4_zf = (a3 >> 1) == 0; // 檢查是否有 WORD 資料
    v5 = 0;
    v17 = a3 >> 1; // 總共的 WORD 數量
    v6 = 0; // 目標緩衝區的 WORD 偏移量

    if (!v4_zf) {
        do {
            if (*reinterpret_cast<short*>(a2) == static_cast<short>(0xB4B4)) { // 檢查是否為標記 WORD
                v9 = *reinterpret_cast<short*>(a2 + 2); // 取得重複的 WORD 值
                a2 += 2; // 跳過標記 WORD
                ++v5; // 處理來源 WORD 數量
                if (v9 == static_cast<short>(0xB4B4)) { // 如果重複值也是標記，表示原始資料就是標記
                    *reinterpret_cast<short*>(&v8[2 * v6++]) = static_cast<short>(0xB4B4);
                } else { // 否則為 RLE 壓縮格式
                    v10 = *reinterpret_cast<short*>(a2 + 2); // 取得重複次數
                    a2 += 2; // 跳過重複次數
                    ++v5; // 處理來源 WORD 數量
                    if (v10) {
                        int fill_val = (v9 << 16) | (v9 & 0xFFFF); // 將 short 轉換為 32 位元值
                        v16 = reinterpret_cast<unsigned char*>(static_cast<long>(v10)); // 儲存原始重複次數
                        v11 = &v8[2 * v6]; // 目標緩衝區的起始位址
                        v12_cf = (v10 & 1) != 0; // 檢查是否奇數個 WORD
                        v13 = v10 >> 1; // 整數個 DWORD

                        memset32(v11, fill_val, v13); // 填充 DWORD

                        v14 = &v11[4 * v13]; // 剩餘 WORD 的起始位址
                        for (i = v12_cf; i; --i) { // 填充剩餘的 WORD (0 或 1 個)
                            *reinterpret_cast<short*>(v14) = v9;
                            v14 += 2;
                        }
                        v8 = a4; // 重置目標緩衝區基底位址
                        v6 += static_cast<int>(reinterpret_cast<long>(v16)); // 更新目標緩衝區偏移量
                    }
                }
            } else { // 非壓縮資料，直接拷貝
                *reinterpret_cast<short*>(&v8[2 * v6++]) = *reinterpret_cast<short*>(a2);
            }
            ++v5; // 處理下一個來源 WORD
            a2 += 2; // 移動來源指標
        } while (v5 < v17);
    }
}

// 四位元組解壓縮
void decomp_DWORD(unsigned char* a1, unsigned int a2, unsigned char* a3) {
    bool v3_zf;
    unsigned int v4 = 0;
    int v5 = 0;
    int v7;
    unsigned int v8;
    unsigned int v9;

    v3_zf = (a2 >> 2) == 0; // 檢查是否有 DWORD 資料
    v4 = 0;
    v9 = a2 >> 2; // 總共的 DWORD 數量
    v5 = 0; // 目標緩衝區的 DWORD 偏移量

    if (!v3_zf) {
        do {
            if (*reinterpret_cast<int*>(a1) == -1263225676) { // 檢查是否為標記 DWORD
                v7 = *reinterpret_cast<int*>(a1 + 4); // 取得重複的 DWORD 值
                a1 += 4; // 跳過標記 DWORD
                ++v4; // 處理來源 DWORD 數量
                if (v7 == -1263225676) { // 如果重複值也是標記，表示原始資料就是標記
                    ++v5;
                    *reinterpret_cast<int*>(&a3[4 * v5 - 4]) = -1263225676;
                } else { // 否則為 RLE 壓縮格式
                    v8 = *reinterpret_cast<unsigned int*>(a1 + 4); // 取得重複次數
                    a1 += 4; // 跳過重複次數
                    ++v4; // 處理來源 DWORD 數量
                    if (v8) {
                        memset32(&a3[4 * v5], v7, v8); // 填充 DWORD
                        v5 += v8; // 更新目標緩衝區偏移量
                    }
                }
            } else { // 非壓縮資料，直接拷貝
                ++v5;
                *reinterpret_cast<int*>(&a3[4 * v5 - 4]) = *reinterpret_cast<int*>(a1);
            }
            ++v4; // 處理下一個來源 DWORD
            a1 += 4; // 移動來源指標
        } while (v4 < v9);
    }
}

// 執行長度解壓縮
void run_length_decomp(int a1, int a2, unsigned char* a3, unsigned int a4, unsigned char* a5, unsigned int a6, unsigned char a7) {
    if (a3 && a5) {
        switch (a7) {
            case 1:
                decomp_BYTE(a1, a3, a4, a5);
                break;
            case 2:
                decomp_WORD(a2, a3, a4, a5);
                break;
            case 4:
                decomp_DWORD(a3, a4, a5);
                break;
            default:
                break;
        }
    }
}