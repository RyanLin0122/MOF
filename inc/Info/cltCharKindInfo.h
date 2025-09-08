#pragma once
#include <cstdint>

class cltCharKindInfo {
public:
    // 5 碼字串 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);
};
