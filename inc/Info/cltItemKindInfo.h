#pragma once
#include <cstdint>

class cltItemKindInfo
{
public:
    // "A0000" 形式 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 16-bit 代碼 → "A0000" 形式（使用靜態緩衝區；非 thread-safe）
    static char* TranslateKindCode(uint16_t a1);
};
