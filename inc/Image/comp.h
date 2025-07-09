#ifndef DECOMP_H
#define DECOMP_H

#include <cstdio> // 包含 FILE 類型所需的頭文件
#include <cstddef> // 包含 size_t 所需的頭文件
#include <iostream>
#include <windows.h> // 包含 SetFileAttributesA 函式所需的頭文件

// 模擬 IDA Pro 反編譯中使用的 memset32 函式
// 由於 memset32 不是標準 C/C++ 函式庫的一部分，如果需要在其他檔案中使用，
// 則應在其定義所在的原始碼檔案中提供其實現。
// 如果它只在 decomp.c 內部使用且未被外部呼叫，則不需要在標頭檔中宣告。
// 這裡為了完整性假設它可能需要被外部使用，因此宣告。
#ifdef __cplusplus
extern "C" {
#endif
void memset32(void* dest, int val, size_t count);
#ifdef __cplusplus
}
#endif

// 函式宣告
// 計算檔案長度
FILE* file_length(const char* const FileName);

// 位元組壓縮
FILE* comp_BYTE(unsigned char* a1, unsigned int a2);

// 雙位元組壓縮
int comp_WORD(unsigned char* a1, unsigned int a2);

// 四位元組壓縮
int comp_DWORD(unsigned char* a1, unsigned int a2);

// 執行長度壓縮的入口函式
FILE* run_length_comp(unsigned char* a1, unsigned int a2, unsigned char a3);

// 位元組解壓縮
// 原始反編譯程式碼中 a1 和 a2 參數被標記為 @<ebx> 和 @<ebp>，
// 這表示它們是透過特定暫存器傳遞的，但在標準 C++ 函式宣告中，
// 我們只需按照其類型和名稱宣告即可。
void decomp_BYTE(int a1_ebx_param, unsigned char* a2, unsigned int a3, unsigned char* a4);

// 雙位元組解壓縮
void decomp_WORD(int a1_ebp_param, unsigned char* a2, unsigned int a3, unsigned char* a4);

// 四位元組解壓縮
void decomp_DWORD(unsigned char* a1, unsigned int a2, unsigned char* a3);

// 執行長度解壓縮的入口函式
// 原始反編譯程式碼中 a1 和 a2 參數被標記為 @<ebx> 和 @<ebp>，
// 這表示它們是透過特定暫存器傳遞的，但在標準 C++ 函式宣告中，
// 我們只需按照其類型和名稱宣告即可。
void run_length_decomp(int a1_ebx_param, int a2_ebp_param, unsigned char* a3, unsigned int a4, unsigned char* a5, unsigned int a6, unsigned char a7);

#endif // DECOMP_H