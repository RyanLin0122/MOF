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

// ------------------------------------------------------------
// strEmblemKindInfo：依 IDA 寫入 offset + size 還原成 416 bytes
// 重點推斷：
// - ItemImageFileId：_sscanf("%x", v6+32) => 寫入 4 bytes => uint32_t
// - AcquireConditionMonsterType：strcpy 到 v6+44，且下一欄位在 172 => char[128]
// - offset 16~23 在 GetBuyableEmblems 被當作 QWORD 檢查，且 Initialize 會寫入 (16,20)
//   => 以 uint64_t 表示（非單純 dwAcquireConditionJobChange）
// - 結尾補齊到 416 bytes
// ------------------------------------------------------------
#pragma pack(push, 1)
struct strEmblemKindInfo
{
    uint16_t wEmblemId;                 // 0
    uint16_t wEquipConditionLevelFrom;  // 2
    uint16_t wEquipConditionLevelTo;    // 4
    uint16_t wAcquireConditionLevel;    // 6
    uint16_t wAcquireConditionQuest;    // 8

    uint8_t  padding_0[6];             // 10..15（保留/未使用；對應你給的自動補位）

    uint64_t qwAcquireConditionJobChangeAtb; // 16..23（GetClassAtb 寫入，且 GetBuyableEmblems 檢查 QWORD）

    uint32_t dwPrice;                  // 24..27
    uint16_t wEmblemNameCode;          // 28..29
    uint16_t wEmblemDescriptionCode;   // 30..31

    uint32_t dwItemImageFileId;        // 32..35（_sscanf("%x", v6+32)）

    uint16_t wItemImageBlockId;        // 36..37
    uint8_t  padding_3[2];             // 38..39（對齊 40）

    uint32_t dwRarityType;             // 40..43

    char     AcquireConditionMonsterType[128]; // 44..171（strcpy 到 v6+44）

    uint32_t dwAcquireConditionMonsterKillCount; // 172
    uint32_t dwAcquireConditionDeathRate;        // 176
    uint32_t dwAcquireConditionMpPotionRate;     // 180
    uint32_t dwAcquireConditionHpPotionRate;     // 184
    uint32_t dwAcquireConditionDragonUsageRate;  // 188
    uint32_t dwAcquireConditionCritReceivedRate; // 192
    uint32_t dwAcquireConditionCritDealtRate;    // 196
    uint32_t dwAcquireConditionPersonalShopRate; // 200
    uint32_t dwAcquireConditionRunePayRate;      // 204
    uint32_t dwAcquireConditionShopUsageRate;    // 208
    uint32_t dwAcquireConditionPublicQuestCompletionRate; // 212
    uint32_t dwAcquireConditionSwordClassCompletionRate;  // 216
    uint32_t dwAcquireConditionArcheryClassCompletionRate;// 220
    uint32_t dwAcquireConditionMagicClassCompletionRate;  // 224
    uint32_t dwAcquireConditionTheologyClassCompletionRate;// 228
    uint32_t dwAcquireConditionEnchantRate;               // 232
    uint32_t dwAcquireConditionQuestImmediateAcquire;     // 236
    uint32_t dwAcquireConditionAllBossKill;               // 240
    uint32_t dwAcquireConditionCircleTaskCompletion;      // 244
    uint32_t dwAcquireConditionItemSale;                  // 248
    uint32_t dwAcquireConditionAreaAttack;                // 252
    uint32_t dwAcquireConditionJobChangeQuestCompletion;  // 256
    uint32_t dwExtraExperienceRate;                       // 260
    uint32_t dwExtraEvasionRate;                          // 264
    uint32_t dwExtraHitRate;                              // 268
    uint32_t dwExtraCriticalRate;                         // 272
    uint32_t dwVsUndeadExtraAttackPowerRate;              // 276
    uint32_t dwVsDemonExtraAttackPowerRate;               // 280
    uint32_t dwVsAnimalExtraAttackPowerRate;              // 284
    uint32_t dwVsOtherMonsterExtraAttackPowerRate;        // 288
    uint32_t dwDeathPenaltyChangeRate;                    // 292
    uint32_t dwHpPotionEffectChangeRate;                  // 296
    uint32_t dwHpAutoRecoveryChangeRate;                  // 300
    uint32_t dwMpPotionEffectChangeRate;                  // 304
    uint32_t dwMpAutoRecoveryChangeRate;                  // 308
    uint32_t dwExtraAttackPowerRate;                      // 312
    uint32_t dwExtraDefenseRate;                          // 316
    uint32_t dwExtraHpRate;                               // 320
    uint32_t dwExtraMpRate;                               // 324
    uint32_t dwStunRate;                                  // 328
    uint32_t dwStunDurationMs;                            // 332
    uint32_t dwBlockRate;                                 // 336
    uint32_t dwBlockDurationMs;                           // 340
    uint32_t dwFreezeRate;                                // 344
    uint32_t dwFreezeDurationMs;                          // 348
    uint32_t dwShopPurchasePriceChangeRate;               // 352
    uint32_t dwRunePayFeeChange;                          // 356
    uint32_t dwDragonUsageFeeChangeRate;                  // 360
    uint32_t dwHuntingPublicQuestExtraPoints;             // 364
    uint32_t dwSwordClassExtraPointsRate;                 // 368
    uint32_t dwArcheryClassExtraPointsRate;               // 372
    uint32_t dwMagicClassExtraPointsRate;                 // 376
    uint32_t dwTheologyClassExtraPointsRate;              // 380
    uint32_t dwEnchantSuccessRate;                        // 384
    uint32_t dwSkillAreaAttackIncreaseRate;               // 388

    uint16_t wTrainingCard;                               // 392
    uint8_t  padding_5[2];                                // 394..395

    uint32_t dwPublicQuestMonsterCountDiscountRate;       // 396
    uint32_t dwItemSalePriceDiscountRate;                 // 400
    uint32_t dwCircleEvaluationScore;                     // 404
    uint16_t wPurchaseConditionDescription;               // 408

    uint8_t  padding_tail[416 - 410];                     // 410..415（補齊到 416）
};
#pragma pack(pop)

static_assert(sizeof(strEmblemKindInfo) == 416, "strEmblemKindInfo size must be 416 bytes");

// ------------------------------------------------------------
// cltEmblemKindInfo：依 IDA 42 個 DWORD 的 layout 還原
// ------------------------------------------------------------
class cltEmblemKindInfo
{
public:
    cltEmblemKindInfo();

    static void InitializeStaticVariable(cltClassKindInfo* p);
    int  Initialize(char* filename);
    void Free();

    strEmblemKindInfo* GetEmblemKindInfo(uint16_t emblemId);
    static uint16_t TranslateKindCode(char* s);

    int GetBuyableEmblems(uint16_t* outIds);

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
    // IDA：dword[0]..dword[41]
    strEmblemKindInfo* m_pInfos;    // [0]
    int32_t                 m_nCount;    // [1]

    strEmblemKindInfo** m_ppOnDead;  // [2]
    int32_t                 m_nOnDead;   // [3]

    strEmblemKindInfo** m_ppOnUsingRecoverHP; // [4]
    int32_t                 m_nOnUsingRecoverHP;  // [5]

    strEmblemKindInfo** m_ppOnUsingRecoverMP; // [6]
    int32_t                 m_nOnUsingRecoverMP;  // [7]

    strEmblemKindInfo** m_ppOnBuyItemFromNPC; // [8]
    int32_t                 m_nOnBuyItemFromNPC;  // [9]

    strEmblemKindInfo** m_ppOnRegistrySellingAgency; // [10]
    int32_t                 m_nOnRegistrySellingAgency;  // [11]

    strEmblemKindInfo** m_ppOnCompleteQuest; // [12]
    int32_t                 m_nOnCompleteQuest;  // [13]

    strEmblemKindInfo** m_ppOnKillBossMonster; // [14]
    int32_t                 m_nOnKillBossMonster;  // [15]

    strEmblemKindInfo** m_ppOnTeleportDragon; // [16]
    int32_t                 m_nOnTeleportDragon;  // [17]

    strEmblemKindInfo** m_ppOnCompleteMeritous; // [18]
    int32_t                 m_nOnCompleteMeritous;  // [19]

    strEmblemKindInfo** m_ppOnCompleteSwordLesson; // [20]
    int32_t                 m_nOnCompleteSwordLesson;  // [21]

    strEmblemKindInfo** m_ppOnCompleteBowLesson; // [22]
    int32_t                 m_nOnCompleteBowLesson;  // [23]

    strEmblemKindInfo** m_ppOnCompleteMagicLesson; // [24]
    int32_t                 m_nOnCompleteMagicLesson;  // [25]

    strEmblemKindInfo** m_ppOnCompleteTheologyLesson; // [26]
    int32_t                 m_nOnCompleteTheologyLesson;  // [27]

    strEmblemKindInfo** m_ppOnEnchantItem; // [28]
    int32_t                 m_nOnEnchantItem;  // [29]

    strEmblemKindInfo** m_ppOnCompleteCircleQuest; // [30]
    int32_t                 m_nOnCompleteCircleQuest;  // [31]

    strEmblemKindInfo** m_ppOnSellItemToNPC; // [32]
    int32_t                 m_nOnSellItemToNPC;  // [33]

    strEmblemKindInfo** m_ppOnMultiAttack; // [34]
    int32_t                 m_nOnMultiAttack;  // [35]

    strEmblemKindInfo** m_ppOnBeAttackedCritically; // [36]
    int32_t                 m_nOnBeAttackedCritically;  // [37]

    strEmblemKindInfo** m_ppOnAttackCritically; // [38]
    int32_t                 m_nOnAttackCritically;  // [39]

    strEmblemKindInfo** m_ppOnChangeClass; // [40]
    int32_t                 m_nOnChangeClass;  // [41]

private:
    static cltClassKindInfo* m_pclClassKindInfo;

private:
    static bool IsDigitStr(const char* s);
    static bool IsAlphaNumericStr(const char* s);
};