#pragma once
#include <cstdint>
#include <cstdio>
#include "Info/cltMapAreaKindInfo.h"
#include "Info/cltCharKindInfo.h"
#include "global.h"

// 一筆怪物設定：對齊反編譯的寫入（charKind 放在 HP 之前，並以 pad 保持 8 bytes 對齊）
#pragma pack(push, 1)
struct stExtraRegenMonster {
    uint16_t charKind;  // Jxxxx -> cltCharKindInfo::TranslateKindCode
    uint16_t _pad;      // 對齊
    int32_t  hp;        // 指定 HP
};
#pragma pack(pop)

// 反編譯顯示每列 200 bytes；以下欄位與偏移對齊寫入點
#pragma pack(push, 1)
struct strExtraRegenMonsterKindInfo {
    uint16_t kind;           // +0  ID（如 F0001）-> TranslateKindCode
    uint16_t mapArea[5];     // +2  最多 5 個地區 Bxxxx -> cltMapAreaKindInfo::TranslateKindCode
    int32_t  mapAreaCnt;     // +12 已填地區數（初始化 0，填入時遞增）
    int32_t  initCount;      // +16 初始生成數（초기 리젠 개수）
    int32_t  minRegen;       // +20 最小再生（min_regen）
    int32_t  maxRegen;       // +24 最大再生（max_regen）
    int32_t  interval;       // +28 間隔（interval，至少 1；且 (max-min)%interval==0 才繼續解析）
    int32_t  type;           // +32 生成類型：0=FIX/其他、1=RANDOM、2=ONCE
    stExtraRegenMonster mob[20]; // +36 開始：最多 20 組（charKind+HP），每組 8 bytes
    int32_t  mobCnt;         // +196 已填怪物數
};
#pragma pack(pop)

static_assert(sizeof(strExtraRegenMonsterKindInfo) == 200, "strExtraRegenMonsterKindInfo must be 200 bytes");

class cltExtraRegenMonsterKindInfo {
public:
    cltExtraRegenMonsterKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltExtraRegenMonsterKindInfo() { Free(); }

    // 反編譯：int __thiscall Initialize(this, char* filename)
    int  Initialize(char* filename);

    // 反編譯：void __thiscall Free(this)
    void Free();

    // 反編譯：int __thiscall GetExtraRegenMonsterNum(this)
    int  GetExtraRegenMonsterNum() const { return m_count; }

    // 反編譯：str* __thiscall GetExtraRegenMonsterKindInfo(this, uint16_t kind)
    strExtraRegenMonsterKindInfo* GetExtraRegenMonsterKindInfo(uint16_t kind);

    // 反編譯：str* __thiscall GetExtraRegenMonsterKindInfoByIndex(this, int idx)
    strExtraRegenMonsterKindInfo* GetExtraRegenMonsterKindInfoByIndex(int idx);

    // 反編譯：static uint16_t __cdecl TranslateKindCode(char* s)
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline const strExtraRegenMonsterKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strExtraRegenMonsterKindInfo* m_items; // 連續陣列，每筆固定 200 bytes
    int                           m_count; // 總筆數
};
