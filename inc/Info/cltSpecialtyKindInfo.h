#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>

class cltSpecialtyKindInfo
{
public:
    // 將 5 碼代號（首字母 + 4 位數）轉為 16-bit kind code
    // 反編譯邏輯：
    // - 長度非 5 → 回傳 0
    // - hi = (toupper(s[0]) + 31) << 11
    // - lo = atoi(s+1)；必須 < 0x800
    // - 回傳 (hi | lo)，否則 0
    static std::uint16_t TranslateKindCode(char* s);
};
