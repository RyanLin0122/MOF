#include "Info/cltItemKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>

// 與 IDA 相符的格式：1 字母 + 4 數字 + '\0'（額外留白以防萬一）
static char s_szKindCode[8] = { 0 };

uint16_t cltItemKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;

    // 長度必須為 5，如 "A0123"
    if (std::strlen(a1) != 5)
        return 0;

    // 高 5 bits 以 (toupper(first) + 31) 編碼
    int high = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;

    // 低 11 bits：4 位數字（0..2047）
    int low = std::atoi(a1 + 1);
    if (low < 0x800)
        return static_cast<uint16_t>(high | low);

    return 0;
}

char* cltItemKindInfo::TranslateKindCode(uint16_t a1)
{
    // 邏輯反推：first = (a1 >> 11) - 31（確保與上式對稱、可逆）
    char ch = static_cast<char>((a1 >> 11) - 31);

    // 低 11 bits
    int  num = static_cast<int>(a1 & 0x7FF);

    std::snprintf(s_szKindCode, sizeof(s_szKindCode), "%c%04d", ch, num);
    return s_szKindCode;
}
