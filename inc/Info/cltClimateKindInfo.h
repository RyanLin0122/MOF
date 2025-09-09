#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

/*
  依 ClimateKindInfo.txt 欄位順序（單筆 64 bytes）
  反編譯寫入序（// 後註解為中文欄位說明）：
   +0  WORD  kind                // 1) 氣候代碼 S/R/E…(5碼) -> TranslateKindCode
   +2  WORD  nameTextId          // 3) 「기후명 ID」
   +4  WORD  moveSpeedPermil     // 4) 移動速度減少(‰)
   +6  WORD  _pad0               // 僅為對齊（讓下個 DWORD 位於 +8）
   +8  DWORD hpRegenIncPermil    // 5) HP自回增加(‰)  ※允許負數（原始以 atoi 讀入）
   +12 DWORD mpRegenDecPermil    // 6) MP自增減少(‰)   ※允許負數
   +16 WORD  expGainIncPermil    // 7) 獲得經驗增加(‰)
   +18 WORD  _pad1               // 對齊到 +20
   +20 DWORD lightningInterval   // 8) 落雷生成間隔(秒)
   +24 WORD  lightningCreatePermil // 9) 落雷生成機率(‰)
   +26 WORD  lightningHitPermil  // 10) 被雷擊中機率(‰)
   +28 WORD  itemDropIncPermil   // 11) 物品掉落率增加(‰)
   +30 WORD  _pad2               // 12) 「賦予屬性」原檔存在，但反編譯略過 -> 不存
   +32 DWORD durationSec         // 13) 持續時間(秒)
   +36 WORD  unitCount           // 14) 單位數量
   +38 WORD  _pad3               // 對齊到 +40
   +40 DWORD resourceId          // 15) 資源ID（以 %x 解析十六進位字串）
   +44 WORD  startBlock          // 16) 起始區塊
   +46 WORD  maxBlockCount       // 17) 最大區塊數
   +48 WORD  animation           // 18) 動畫（數值）
   +50 WORD  snowSpeed           // 19) 下雪速度
   +52 DWORD isSnowing           // 20) 是否下雪(0/1)
   +56 DWORD iconResId           // 21) 氣候圖示資源ID（%x）
   +60 WORD  blockId             // 22) 區塊ID
   +62 WORD  _pad4               // 保持 64 bytes 結尾對齊
*/
#pragma pack(push, 1)
struct strClimateInfo {
    uint16_t kind;
    uint16_t nameTextId;
    uint16_t moveSpeedPermil;
    uint16_t _pad0;

    int32_t  hpRegenIncPermil;
    int32_t  mpRegenDecPermil;
    uint16_t expGainIncPermil;
    uint16_t _pad1;

    int32_t  lightningInterval;
    uint16_t lightningCreatePermil;
    uint16_t lightningHitPermil;
    uint16_t itemDropIncPermil;
    uint16_t _pad2;

    int32_t  durationSec;
    uint16_t unitCount;
    uint16_t _pad3;

    uint32_t resourceId;
    uint16_t startBlock;
    uint16_t maxBlockCount;
    uint16_t animation;
    uint16_t snowSpeed;
    int32_t  isSnowing;
    uint32_t iconResId;
    uint16_t blockId;
    uint16_t _pad4;
};
#pragma pack(pop)
static_assert(sizeof(strClimateInfo) == 64, "strClimateInfo must be 64 bytes");

class cltClimateKindInfo {
public:
    cltClimateKindInfo();

    // 反編譯：int Initialize(this, char* filename)；成功回傳 1
    int  Initialize(char* filename);

    // 反編譯：void Free(this)
    void Free();

    // 反編譯：strClimateInfo* GetClimateKindInfo(this, uint16_t code)
    strClimateInfo* GetClimateKindInfo(uint16_t code);

    // 反編譯：static uint16_t TranslateKindCode(char* s)（5碼：首字母+4位數）
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline int size() const { return m_count; }
    inline const strClimateInfo* data() const { return m_items; }

private:
    int              m_count; // *(_DWORD*)this
    strClimateInfo* m_items; // *((_DWORD*)this + 1)
};
