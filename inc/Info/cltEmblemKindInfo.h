#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltQuestKindInfo.h"
#include "global.h"

#pragma pack(push, 1)

// 每筆資料 416 bytes；欄位以反編譯位移標註（+offset）
struct strEmblemKindInfo
{
    // 基本資料
    uint16_t kind;                 // +0  Emblem ID(5碼→16bit)
    uint16_t equipMinLv;           // +2  裝備條件(最低等)
    uint16_t equipMaxLv;           // +4  裝備條件(最高等)
    uint16_t acquireLevelReach;    // +6  獲得條件(等級到達)
    uint16_t acquireQuestKind;     // +8  獲得條件(指定任務ID，5碼→16bit)
    char     _pad0A[6];            // +0A..0F：保留（對齊到 +16）

    // 職業屬性（由 ClassKind 字串解析成 64-bit bitmask）
    uint32_t classAtbLow;          // +16
    uint32_t classAtbHigh;         // +20

    // 商店/圖像/名稱等
    int32_t  price;                // +24  價格
    uint16_t nameTextCode;         // +28  名稱文本碼
    uint16_t descTextCode;         // +30  說明文本碼
    uint32_t imageFileIdHex;       // +32  圖檔ID(十六進位字串→uint32)
    uint16_t imageBlockId;         // +36  圖塊ID
    uint16_t _pad26;               // +38..39：對齊
    int32_t  rareDiv;              // +40  稀有度/分類

    // 取得條件(怪物種類字串等) —— 檔案多為 "0"
    char     condMonsterType[128]; // +44..+171

    // 取得條件(怪物擊殺數)
    int32_t  condMonsterKillCnt;   // +172

    // 觸發：機率(萬分率/千分率) —— 只要非 0 就會被分類到對應清單
    int32_t  prob_OnDead;                  // +176
    int32_t  prob_OnUsingRecoverHPItem;    // +180
    int32_t  prob_OnUsingRecoverManaItem;  // +184
    int32_t  prob_OnTeleportDragon;        // +188
    int32_t  prob_OnBeAttackedCritically;  // +192
    int32_t  prob_OnAttackCritically;      // +196
    int32_t  misc200;                      // +200（未分組使用）
    int32_t  prob_OnRegistrySellingAgency; // +204
    int32_t  prob_OnBuyItemFromNPC;        // +208
    int32_t  prob_OnCompleteMeritous;      // +212
    int32_t  prob_OnCompleteSwordLesson;   // +216
    int32_t  prob_OnCompleteBowLesson;     // +220
    int32_t  prob_OnCompleteMagicLesson;   // +224
    int32_t  prob_OnCompleteTheologyLesson;// +228
    int32_t  prob_OnEnchantItem;           // +232
    uint32_t questKindOnComplete;          // +236（5碼→16bit，原碼存入32位）

    int32_t  prob_OnKillBossMonster;       // +240
    int32_t  prob_OnCompleteCircleQuest;   // +244
    int32_t  prob_OnSellItemToNPC;         // +248
    int32_t  prob_OnMultiAttack;           // +252
    int32_t  prob_OnChangeClass;           // +256

    // 大量數值加成/調整（依檔案欄位序填入；此區間位移交錯出現）
    // 涵蓋 +260..+384（124 bytes）
    char     statsBlock[124];              // +260..+383（包含經驗/攻防/HP/MP/命中/閃避/屬性/價格/費用/分數等）

    // 後段欄位
    int32_t  stat384;               // +384（檔案尾端數值之一）
    int32_t  skillAoEIncrease;      // +388  스킬범위공격증가(千分率)
    uint16_t trainingCardKind;      // +392  트레이닝 카드(5碼→16bit)
    uint16_t _pad394;               // +394..+395：對齊
    int32_t  publicMonsterDiscount; // +396  공적 몬스터 수 할인율(千分率)
    int32_t  itemSellPriceDiscount; // +400  아이템 판매 가격 할인율(千分率)
    int32_t  circleScore;           // +404  서클평가점수
    uint16_t purchaseCondTextCode;  // +408  구입 조건 설명(文本碼)
    char     _pad410[6];            // +410..+415 補齊 416 bytes
};
static_assert(sizeof(strEmblemKindInfo) == 416, "strEmblemKindInfo must be 416 bytes");

#pragma pack(pop)

class cltEmblemKindInfo
{
public:
    // 供外部在載入前注入 ClassKindInfo（必需）
    static void InitializeStaticVariable(cltClassKindInfo* p) { m_pclClassKindInfo = p; }

    // 建構/解構
    cltEmblemKindInfo();
    ~cltEmblemKindInfo() { Free(); }

    // 載入：成功完整讀畢(EOF)回傳 1；否則 0
    int Initialize(char* filename);
    void Free();

    // 查詢
    strEmblemKindInfo* GetEmblemKindInfo(uint16_t kind);
    static uint16_t    TranslateKindCode(char* s);

    // 可購買清單（依反編譯條件判斷）
    int GetBuyableEmblems(uint16_t* outKinds);

    // 事件分類取回（回傳數量；以輸出參數帶出陣列起點）
    int GetEmblemKindInfo_OnDead(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnUsingRecoverHPItem(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnUsingRecoverManaItem(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnBuyItemFromNPC(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnRegistrySellingAgency(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteQuest(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnKillBossMonster(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnTeleportDragon(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteMeritous(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteSwordLesson(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteBowLesson(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteMagicLesson(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteTheologyLesson(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnEnchantItem(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnCompleteCircleQuest(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnSellItemToNPC(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnMultiAttack(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnBeAttackedCritically(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnAttackCritically(strEmblemKindInfo*** out);
    int GetEmblemKindInfo_OnChangeClass(strEmblemKindInfo*** out);

private:
    // 小工具：整字串皆為數字 / 皆為英數（供防呆）
    static bool IsDigitStr(const char* s);
    static bool IsAlphaNumStr(const char* s);

    // 解析 ClassKind 串列（"KNI|SWO|..."）→ 64-bit bitmask
    static void ParseClassAtbMask(const char* classList, uint32_t& lo, uint32_t& hi);

private:
    // 主資料
    strEmblemKindInfo* m_list{ nullptr }; // +0
    int                m_count{ 0 };       // +1

    // 事件分類陣列（各自為指向 strEmblemKindInfo* 的陣列）
    strEmblemKindInfo** m_onDead{ nullptr };                 int m_onDeadCnt{ 0 };                 // +2/+3
    strEmblemKindInfo** m_onUseHP{ nullptr };                int m_onUseHPCnt{ 0 };                // +4/+5
    strEmblemKindInfo** m_onUseMP{ nullptr };                int m_onUseMPCnt{ 0 };                // +6/+7
    strEmblemKindInfo** m_onBuyNPC{ nullptr };               int m_onBuyNPCCnt{ 0 };               // +8/+9
    strEmblemKindInfo** m_onRegistry{ nullptr };             int m_onRegistryCnt{ 0 };             // +10/+11
    strEmblemKindInfo** m_onCompleteQuest{ nullptr };        int m_onCompleteQuestCnt{ 0 };        // +12/+13
    strEmblemKindInfo** m_onKillBoss{ nullptr };             int m_onKillBossCnt{ 0 };             // +14/+15
    strEmblemKindInfo** m_onTeleportDragon{ nullptr };       int m_onTeleportDragonCnt{ 0 };       // +16/+17
    strEmblemKindInfo** m_onCompleteMeritous{ nullptr };     int m_onCompleteMeritousCnt{ 0 };     // +18/+19
    strEmblemKindInfo** m_onSwordLesson{ nullptr };          int m_onSwordLessonCnt{ 0 };          // +20/+21
    strEmblemKindInfo** m_onBowLesson{ nullptr };            int m_onBowLessonCnt{ 0 };            // +22/+23
    strEmblemKindInfo** m_onMagicLesson{ nullptr };          int m_onMagicLessonCnt{ 0 };          // +24/+25
    strEmblemKindInfo** m_onTheologyLesson{ nullptr };       int m_onTheologyLessonCnt{ 0 };       // +26/+27
    strEmblemKindInfo** m_onEnchant{ nullptr };              int m_onEnchantCnt{ 0 };              // +28/+29
    strEmblemKindInfo** m_onCircleQuest{ nullptr };          int m_onCircleQuestCnt{ 0 };          // +30/+31
    strEmblemKindInfo** m_onSellToNPC{ nullptr };            int m_onSellToNPCCnt{ 0 };            // +32/+33
    strEmblemKindInfo** m_onMultiAttack{ nullptr };          int m_onMultiAttackCnt{ 0 };          // +34/+35
    strEmblemKindInfo** m_onBeCrit{ nullptr };               int m_onBeCritCnt{ 0 };               // +36/+37
    strEmblemKindInfo** m_onDoCrit{ nullptr };               int m_onDoCritCnt{ 0 };               // +38/+39
    strEmblemKindInfo** m_onChangeClass{ nullptr };          int m_onChangeClassCnt{ 0 };          // +40/+41

private:
    static cltClassKindInfo* m_pclClassKindInfo; // 由 InitializeStaticVariable 指定
};

