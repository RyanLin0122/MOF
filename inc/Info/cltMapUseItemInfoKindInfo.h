#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)

// 單筆 Map 使用型物品資訊（總大小 = 228 bytes / 0xE4，步距 228；與 mofclient.c 反編譯一致）
//
// 注意：欄位的「檔案出現順序」與「結構體記憶體順序」並不一致：
//   - 在 mapuseiteminfo.txt 中，서브아이템ID_1..5 是「最後 5 欄」（第 27~31 欄），
//     但 GT 把它們寫入結構體的 +4..+12（緊接在 ID/ClimateNameTextID 之後）。
//   - 在 mapuseiteminfo.txt 中，EaFileName(미리보기) 是第 25 欄、서브아이템갯수 是第 26 欄，
//     但 GT 先 strcpy(EaFileName) 到 +100、再把 SubItemCount 寫到 +96。
// 完整解析順序對照詳見 src/Info/cltMapUseItemInfoKindInfo.cpp。
struct strMapUseItemInfoKindInfo
{
    // 0x00 (=0)
    uint16_t ID;                 // ID（M**** → 16 位代碼，TranslateKindCode）

    // 0x02 (=2)
    uint16_t ClimateNameTextID;  // 기후명 Text_ID  // 氣候名文字 ID

    // 0x04 ~ 0x0D (=4..13)
    uint16_t SubItemID[5];       // 서브아이템ID_1..5 // 子物品 ID（五個；字串 5 碼→代碼，或 0）

    // 0x0E (=14)
    uint16_t DescTextID;         // 설명text         // 說明文字 ID

    // 0x10 (=16) — 以下為 DWORD 區
    uint32_t DurationMS;         // 지속시간(초:밀리세컨)            // 持續時間 ms

    // 0x14 (=20)
    uint32_t ResourceID;         // 리소스 아이디(HEX)               // 資源 ID（十六進位）

    // 0x18 (=24)
    uint32_t WeatherIconResID;   // 기상아이콘 리소스 아이디(HEX)     // 天氣圖示資源 ID

    // 0x1C (=28)
    uint32_t ItemIconResID;      // 아이템 아이콘 리소스 아이디(HEX)  // 物品圖示資源 ID

    // 0x20 (=32)
    uint32_t UnitCount;          // 유닛숫자                          // 單位數

    // 0x24 (=36)
    uint32_t StartBlock;         // 시작 블록                         // 起始 Block

    // 0x28 (=40)
    uint32_t MaxBlocks;          // 최대 블록수                       // 最大 Block 數

    // 0x2C (=44)
    uint32_t AnimStartBlockID;   // 애니메이션 시작 블록 아이디        // 動畫起始 BlockID

    // 0x30 (=48)
    uint32_t AnimSpeed;          // 애니메이션 속도                    // 動畫速度

    // 0x34 (=52)
    uint32_t AirSpeed;           // 공중 속도(떨어지는 속도)           // 空中（落下）速度

    // 0x38 (=56)
    uint32_t BlockID1;           // 블록아이디                         // 區塊 ID（一）

    // 0x3C (=60)
    uint32_t BlockID2;           // 블록 아이디                        // 區塊 ID（二）

    // 0x40 (=64)
    uint32_t Pattern;            // 패턴                               // 模式

    // 0x44 (=68)
    uint32_t HPRecov;            // HP 회복량(맵내)                    // HP 回復（場內）

    // 0x48 (=72)
    uint32_t MPRecov;            // MP 회복량(맵내)                    // MP 回復（場內）

    // 0x4C (=76)
    uint32_t EvasionPermille;    // 회피율(맵내)(천분율)               // 迴避（千分比）

    // 0x50 (=80)
    uint32_t AccuracyPermille;   // 명중률(맵내)(천분율)               // 命中（千分比）

    // 0x54 (=84)
    uint32_t CriticalPermille;   // 크리티컬 확률(맵내)(천분율)        // 爆擊（千分比）

    // 0x58 (=88)
    uint32_t PetAffinityPermille;// 펫 애정도 상승률(맵내)(천분율)      // 寵物好感（千分比）

    // 0x5C (=92)
    uint32_t EaIndex;            // EaIndex                            // EA 索引

    // 0x60 (=96)
    uint32_t SubItemCount;       // 서브아이템갯수                     // 子物品數

    // 0x64 (=100)
    char     EaFileName[128];    // EaFileName(미리보기)               // EA 檔名（預覽用）
};
static_assert(sizeof(strMapUseItemInfoKindInfo) == 228, "strMapUseItemInfoKindInfo size must be 228 bytes");

#pragma pack(pop)

class cltMapUseItemInfoKindInfo
{
public:
    cltMapUseItemInfoKindInfo() : table_(nullptr), count_(0) {}
    ~cltMapUseItemInfoKindInfo() { Free(); }

    // 讀取 mapuseiteminfo.txt；成功回傳 1，失敗回傳 0
    int Initialize(char* filename);

    void Free();

    // 依 ID（第一欄 M**** 轉碼）取得資料
    strMapUseItemInfoKindInfo* GetMapUseItemInfoKindInfo(uint16_t id);

    // 5碼 KindCode 轉 16位代碼（與反編譯一致）
    static uint16_t TranslateKindCode(char* s);

    const strMapUseItemInfoKindInfo* data()  const { return table_; }
    int                              count() const { return count_; }

private:
    static bool IsDigitString(const char* s);

private:
    strMapUseItemInfoKindInfo* table_;
    int                        count_;
};
