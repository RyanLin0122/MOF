#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)

// 單筆 Map 使用型物品資訊（大小 = 228 bytes，步距 228；與反編譯一致）
struct strMapUseItemInfoKindInfo
{
    // 0x00
    uint16_t ID;                 // ID（M**** → 16位代碼）

    // 0x02
    uint16_t ClimateNameTextID;  // 기후명 Text_ID  // 氣候名文字ID

    // 0x04 ~ 0x12
    uint16_t SubItemID[5];       // 서브아이템ID_1..5 // 子物品ID（五個；字串5碼→代碼，或0）

    // 0x14
    uint16_t DescTextID;         // 설명text         // 說明文字ID

    // 0x16 (開始為 DWORD 區)
    uint32_t DurationMS;         // 지속시간(초:밀리세컨) // 持續時間ms
    uint32_t ResourceID;         // 리소스 아이디(HEX)   // 資源ID(十六進位)
    uint32_t WeatherIconResID;   // 기상아이콘 리소스 아이디(HEX) // 天氣圖示資源ID
    uint32_t ItemIconResID;      // 아이템 아이콘 리소스 아이디(HEX) // 物品圖示資源ID
    uint32_t UnitCount;          // 유닛숫자           // 單位數
    uint32_t StartBlock;         // 시작 블록         // 起始Block
    uint32_t MaxBlocks;          // 최대 블록수       // 最大Block數
    uint32_t AnimStartBlockID;   // 애니메이션 시작 블록 아이디 // 動畫起始BlockID
    uint32_t AnimSpeed;          // 애니메이션 속도    // 動畫速度
    uint32_t AirSpeed;           // 공중 속도          // 空中(落下)速度
    uint32_t BlockID1;           // 블록아이디         // 區塊ID(一)
    uint32_t BlockID2;           // 블록 아이디        // 區塊ID(二)
    uint32_t Pattern;            // 패턴               // 模式
    uint32_t HPRecov;            // HP 회복량(맵내)    // HP回復(場內)
    uint32_t MPRecov;            // MP 회복량(맵내)    // MP回復(場內)
    uint32_t EvasionPermille;    // 회피율(맵내)(천분율)// 迴避(千分比)
    uint32_t AccuracyPermille;   // 명중률(맵내)(천분율)// 命中(千分比)
    uint32_t CriticalPermille;   // 크리티컬 확률(맵내) // 爆擊(千分比)
    uint32_t PetAffinityPermille;// 펫 애정도 상승률    // 寵物好感(千分比)
    uint32_t EaIndex;            // EaIndex
    uint32_t SubItemCount;       // 서브아이템갯수     // 子物品數

    // 0x64 (=100)
    char     EaFileName[128];    // EaFileName(미리보기)// EA檔名(預覽用)
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
