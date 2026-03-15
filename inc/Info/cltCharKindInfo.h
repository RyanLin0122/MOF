#pragma once
#include <cstdint>

class cltCharKindInfo {
public:
    // 5 碼字串 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetCharKindInfo(uint16_t kindCode);
    uint16_t GetRealCharID(uint16_t charKind);
};
