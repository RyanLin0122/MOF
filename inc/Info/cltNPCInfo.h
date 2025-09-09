#pragma once
#include <cstdint>

class cltNPCInfo {
public:
    // 將 "N0234" 之類字串轉為 16-bit 種類碼
    // 規則：((toupper(s[0]) + 31) << 11) | atoi(s+1)；若條件不符回 0
    static uint16_t TranslateKindCode(char* s);
};
