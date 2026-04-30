#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

/*
  ClimateKindInfo.txt 解析後單筆 64 bytes（與反編譯 mofclient.c:294435 的
  寫入序與偏移完全一致）。
  欄位 [n] = TXT 第 n 個 strtok token；** 標記表示 GT 寫入記錄，未標 ** 者只
  做 NULL 檢查不存入結構。

   off  寬   名稱                    [tok] 韓文 / 中文 / 用途
   ----+----+------------------------+-----+----------------------------------
   +0   WORD kind                   ** [0] 기후 아이디 / 氣候代碼（5 碼，
                                          TranslateKindCode → ((toupper+31)<<11)|atoi）
   ---  --   *(name, discarded)        [1] 이름 / 名稱（企劃用）
   +2   WORD nameTextId             ** [2] 기후명 ID / 氣候名 ID
   +4   WORD moveSpeedPermil        ** [3] 이동 속도 감소율(‰) / 移動速度減少率
   +6   WORD _pad0                       對齊（下個 DWORD 落在 +8）
   +8   I32  hpRegenIncPermil       ** [4] HP 자동회복 증가(‰) / HP 自動回復
                                          ※可為負（GT 以 atoi 讀入 DWORD）
   +12  I32  mpRegenDecPermil       ** [5] MP 자동증가 감소(‰) / MP 自動增加減少
                                          ※可為負
   +16  WORD expGainIncPermil       ** [6] 획득 경험치 증가(‰) / 獲得經驗增加
   +18  WORD _pad1                       對齊到 +20
   +20  I32  lightningInterval      ** [7] 낙뢰 생성 시간 간격(秒) / 落雷生成間隔
   +24  WORD lightningCreatePermil  ** [8] 낙뢰 생성 확률(‰) / 落雷生成機率
   +26  WORD lightningHitPermil     ** [9] 낙뢰 명중 확률(‰) / 落雷命中機率
   +28  WORD itemDropIncPermil      ** [10] 아이템 드롭률(‰) / 物品掉落率增加
   +30  WORD _pad2                  ** [11] 부여 속성 / 賦予屬性
                                          ※TXT 有此欄，反編譯只 NULL-check 不存
   +32  I32  durationSec            ** [12] 지속시간(秒) / 持續時間
   +36  WORD unitCount              ** [13] 단위 개수 / 單位數量
   +38  WORD _pad3                       對齊到 +40
   +40  U32  resourceId             ** [14] 리소스 아이디(hex) / 資源 ID
                                          （IsAlphaNumeric → sscanf "%x"）
   +44  WORD startBlock             ** [15] 시작 블럭 / 起始區塊
   +46  WORD maxBlockCount          ** [16] 최대 블럭 수 / 最大區塊數
   +48  WORD animation              ** [17] 애니메이션 / 動畫
   +50  WORD snowSpeed              ** [18] 떨어지는 속도 / 下雪速度
   +52  I32  isSnowing              ** [19] 눈오는지의 여부 / 是否下雪（0/1）
   +56  U32  iconResId              ** [20] 기후 아이콘 리소스 ID(hex) /
                                          氣候圖示資源 ID
   +60  WORD blockId                ** [21] 블럭 아이디 / 區塊 ID
                                          （最後欄；GT 為 (!tok || !IsDigit) 合併判斷）
   +62  WORD _pad4                       結構結尾對齊；保持 64 bytes
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

    // 反編譯：mofclient.c:8350 / cltMapInfo::~cltMapInfo 於 Free 後呼叫此 dtor
    ~cltClimateKindInfo() { Free(); }

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
