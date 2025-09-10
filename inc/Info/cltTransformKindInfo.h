#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include "global.h"

// ---------------------------------------------------------------------
// 單筆「變身種別」紀錄的實際記憶體布局（嚴格對齊反編譯碼的 52 bytes）
// 版位（offset）對應：
//   0x00: uint16_t kind                ← GetTransformKind() 的結果（1..17）
//   0x02: uint16_t _pad                ← 對齊用
//   0x04: int canAttack                ← T/F（攻擊可能與否）
//   0x08: int canBeAttacked            ← T/F（可被攻擊與否）
//   0x0C: int canMove                  ← T/F（可移動與否）
//   0x10: int nearCharHPInc            ← 周邊角色 HP 增加值
//   0x14: int nearCharMPInc            ← 周邊角色 MP 增加值
//   0x18: int nearPartyHPInc           ← 周邊隊友 HP 增加值
//   0x1C: int nearPartyMPInc           ← 周邊隊友 MP 增加值
//   0x20: int influenceRangeLR         ← 影響範圍（左右半徑）
//   0x24: int influenceInterval        ← 影響給予間隔（ms）
//   0x28: char monsterCode[8]          ← 怪物代碼短字串（如 J0593 / "0"）
//   0x30: int isStealth                ← T/F（是否「隱身術」）
// 合計 52 bytes；與反編譯碼中每筆步幅「+52」完全一致。
// ---------------------------------------------------------------------
#pragma pack(push, 1)
struct strTransformKindInfo {
    uint16_t kind;
    uint16_t _pad;
    int32_t  canAttack;
    int32_t  canBeAttacked;
    int32_t  canMove;
    int32_t  nearCharHPInc;
    int32_t  nearCharMPInc;
    int32_t  nearPartyHPInc;
    int32_t  nearPartyMPInc;
    int32_t  influenceRangeLR;
    int32_t  influenceInterval;
    char     monsterCode[8]; // 原始碼以 strcpy 複製到 (v5+8)；此處限定 8 以避免覆蓋 0x30
    int32_t  isStealth;
};
#pragma pack(pop)
static_assert(sizeof(strTransformKindInfo) == 52, "strTransformKindInfo must be 52 bytes");

// ---------------------------------------------------------------------
// 物件本身即為 17 筆連續紀錄的容器，起點位移 0。
// 反編譯中的指標運算：
//   v5 = (DWORD*)((char*)this + 8);
//   *((WORD*)v5 - 4) 寫入 offset 0 的 kind
//   每處理一列 v5 += 13 (13 * 4 = 52 bytes) 前進到下一筆。
// ---------------------------------------------------------------------
class cltTransformKindInfo {
public:
    cltTransformKindInfo();                 // 與原始碼一致：不做額外初始化
    int  LoadTransformKindInfo(char* path); // 成功讀到 EOF 回傳 1；否則 0

    // 將 "TRFM_KIND_***" 轉為 1..17（未知回 0）
    int  GetTransformKind(char* s);

    // 依 kind (uint16) 回傳指向紀錄的指標（找不到回 nullptr）
    strTransformKindInfo* GetTransfromKindInfo(uint16_t kind);

private:
    // 固定 17 筆（反編譯的 for 亦以 17 為上限）
    strTransformKindInfo records_[17];
};
