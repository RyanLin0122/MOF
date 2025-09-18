#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>

// 與反編譯相符的介面：只提供 TranslateKindCode
class cltPandoraKindInfo
{
public:
    // 將 5 碼代號（首字母 + 4位數）轉成 16-bit kind code。
    // 規則（與反編譯一致）：
    // 1) 長度必為 5；否則回傳 0
    // 2) hi = (toupper(s[0]) + 31) << 11
    // 3) lo = atoi(s + 1)；必須 < 0x800
    // 4) 回傳 (hi | lo)，否則 0
    static std::uint16_t TranslateKindCode(char* s);
};
