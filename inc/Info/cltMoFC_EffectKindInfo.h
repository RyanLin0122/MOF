#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <new>
#include "global.h"

//---------------------------------------------------------------------
// 結構大小需為 0x84(132) bytes：
// [0..1]   : uint16_t kindCode
// [2..129] : char eaFile[128]
// [130]    : uint8_t skillType (1..6)
// [131]    : padding
//---------------------------------------------------------------------
#pragma pack(push, 1)
struct stEffectKindInfo {
    uint16_t kindCode;      // 由 TranslateKindCode 計算的 16-bit 種別碼
    char     eaFile[128];   // "EA 文件名" 欄位（韓文：EA 파일명）
    uint8_t  skillType;     // "技能類型"(ONCE/DIRECT/...) -> 1..6
    uint8_t  _pad;          // 填充，對齊 0x84 大小（照原記憶體版位）
};
#pragma pack(pop)

static_assert(sizeof(stEffectKindInfo) == 0x84, "stEffectKindInfo size must be 0x84");

//---------------------------------------------------------------------
// 邏輯與反編譯碼一致：
// - 物件持有一張以 KindCode(16-bit) 為索引的指標表
// - Initialize(): 讀檔，逐行以 "\t\n" 分隔，建立 stEffectKindInfo
// - GetEffectKindInfo(): 以 code 或字串查找
// - TranslateKindCode(): 5字元格式，首字母+31 左移11，後4碼數字<0x800
//---------------------------------------------------------------------
class cltMoFC_EffectKindInfo {
public:
    cltMoFC_EffectKindInfo();
    virtual ~cltMoFC_EffectKindInfo(); // 與反編譯碼一致：不釋放配置的節點

    // 讀取 SkillEffect.txt（或相同格式檔），回傳 1 表示「正常走到 EOF」，
    // 0 表示開檔失敗（或未成功完成至 EOF—依反編譯碼的行為）
    int Initialize(char* fileName);

    // 依 16-bit 種別碼取回
    stEffectKindInfo* GetEffectKindInfo(uint16_t code);

    // 依 "E0001" 等 5字元代碼取回
    stEffectKindInfo* GetEffectKindInfo(char* codeStr);

    // "E0001" → 16-bit 種別碼（不合法則回 0）
    uint16_t TranslateKindCode(char* codeStr);

private:
    // 指標表：索引即 TranslateKindCode 回傳值（0..65534）
    // 注意：反編譯碼內部以 (this + 4) 做為基底並以 +1 偏移，這裡以獨立陣列實作，行為等價
    static constexpr size_t TABLE_SIZE = 0xFFFF; // 65535
    stEffectKindInfo* table_[TABLE_SIZE];
};