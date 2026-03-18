#pragma once
#include <cstdint>

struct stCharKindInfo {
    // Partial reconstruction. TODO: verify exact offsets from binary analysis.
    char _unknown_prefix[8]; // placeholder; actual offset unknown
    int moneyRule;            // TODO: verify offset
    int levelOrRankBase;      // TODO: verify offset
};

class cltCharKindInfo {
public:
    // 5 碼字串 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetCharKindInfo(uint16_t kindCode);
    uint16_t GetRealCharID(uint16_t charKind);

    // Returns monster name/info block for the given kind code.
    stCharKindInfo* GetMonsterNameByKind(unsigned short kind);

    // Returns all char kind infos that reference the given drop item kind code.
    // outChars must point to an array of at least 65535 stCharKindInfo* elements.
    // Returns the number of entries written.
    int GetCharKindInfoByDropItemKind(uint16_t dropItemKindCode, stCharKindInfo** outChars);
};

extern cltCharKindInfo g_clCharKindInfo;
