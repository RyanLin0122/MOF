#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>
#include "Info/cltMineKindInfo.h"
#include "Info/cltDebuffKindInfo.h"
#include "Other/cltAttackAtb.h"
#include "global.h"

class cltClassKindInfo;
struct strClassKindInfo;

// ──────────────────────────────────────────────────────────────────────────
// stSkillKindInfo: 嚴格 400 bytes（與反編譯一致）
//
// 同一個 400-byte 區塊在兩種模式下使用：
//   - Passive (P*): 被動技能
//   - Active  (A*): 主動技能
// 共用區（+0..+111）由兩者都會寫入；+112..+399 兩者意義不同，故以 union 切換。
// 對應 mofclient.c 中 cltSkillKindInfo::Initialize_P / Initialize_A 的填欄位置。
// ──────────────────────────────────────────────────────────────────────────

#pragma pack(push, 1)

// 主動技能視角下的後段欄位（+112..+399，共 288 bytes）
struct stSkillKindInfoActive {
    uint8_t  effectType;            // +112 主動效果類型 (ATTACK_ONE..DEBUFF → 1..11)
    uint8_t  pad113;                // +113 填充
    uint16_t hpCost;                // +114 HP 消耗
    uint16_t mpCost;                // +116 MP 消耗
    uint16_t pad118;                // +118 填充
    uint32_t remainHpAfterUse;      // +120 使用後剩餘 HP
    uint32_t remainMpAfterUse;      // +124 使用後剩餘 MP
    uint32_t missHitWeight;         // +128 未命中加權
    uint16_t normalHitWeight;       // +132 一般命中加權
    uint16_t critHitWeight;         // +134 爆擊命中加權 (千分率)
    uint16_t durationTime;          // +136 持續時間
    uint16_t repeatTime;            // +138 重複時間
    uint32_t castTime;              // +140 詠唱時間
    uint32_t valueColumn;           // +144 數值欄
    char     soundName[16];         // +148..+163 音效字串 (例: "S0001")
    uint8_t  isLong;                // +164 是否遠距 ("LONG"=1，其他=0)
    uint8_t  pad165;                // +165 填充
    uint16_t targetRangeX;          // +166 目標範圍 X
    uint16_t targetRangeY;          // +168 目標範圍 Y
    uint8_t  castAnim;              // +170 詠唱動畫 (NORMAL/SPELL/PRAY/TRIPLE/DOUBLE → 0..4)
    uint8_t  hitAtb;                // +171 受擊屬性 (NONE/NORMAL/FIRE/ELEC/ICE → 0..4)
    uint32_t attackAtb;             // +172 攻擊屬性 (cltAttackAtb::GetAttackAtb)
    uint32_t atkStunProb;           // +176 攻擊時暈眩機率 (千分率)
    uint32_t atkStunTime;           // +180 攻擊時暈眩時間
    uint32_t atkBlockProb;          // +184 攻擊時封鎖機率 (千分率)
    uint32_t atkBlockTime;          // +188 攻擊時封鎖時間
    uint32_t atkFreezeProb;         // +192 攻擊時結冰機率 (千分率)
    uint32_t atkFreezeTime;         // +196 攻擊時結冰時間
    uint16_t mineKindCode;          // +200 地雷 KindCode (cltMineKindInfo::TranslateKindCode)
    uint16_t pad202;                // +202 填充
    uint32_t fastRun;               // +204 高速移動
    uint32_t castCount;             // +208 施放次數
    uint32_t spiritBonus;           // +212 精神獎勵
    uint32_t agilityBonus;          // +216 敏捷獎勵
    uint32_t staminaBonus;          // +220 體力獎勵
    uint32_t intelligenceBonus;     // +224 智力獎勵
    uint32_t partyAtkPower;         // +228 隊伍成員加成攻擊力
    uint32_t partyAgility;          // +232 隊伍成員加成敏捷
    uint32_t partySpirit;           // +236 隊伍成員加成精神
    uint32_t partyStamina;          // +240 隊伍成員加成體力
    uint32_t partyAtkAtbFlag;       // +244 隊伍攻擊屬性旗標 ("0"→0, 否則→1)
    uint32_t partyAtkAtb;           // +248 隊伍攻擊屬性 (GetAttackAtb)
    uint16_t value1;                // +252 數值欄 1 (含轉變 KindCode 等)
    uint16_t value2;                // +254 數值欄 2
    uint16_t value3;                // +256 數值欄 3
    uint16_t pad258;                // +258 填充
    uint32_t atkUseProb;            // +260 攻擊使用機率
    uint32_t atkUseEff;             // +264 攻擊使用效率
    uint32_t atkSpeedActive;        // +268 攻擊速度
    uint32_t damageReceiveConvert;  // +272 受傷時轉換成 (밀초기)
    uint16_t atkLineFx;             // +276 攻擊連動效果
    uint16_t pad278;                // +278 填充
    uint32_t triggerProbDamage;     // +280 發動機率 (傷害)
    uint32_t triggerProbCurse;      // +284 發動機率 (詛咒/傷害)
    uint32_t pad288;                // +288 填充
    uint8_t  debuffStage;           // +292 Debuff 階段
    uint8_t  pad293[3];             // +293..+295 填充
    uint32_t debuffRideTime;        // +296 Debuff 續存時間
    uint32_t effectStage;           // +300 效果階段
    uint16_t debuffKindCode;        // +304 Debuff KindCode (cltDebuffKindInfo::TranslateKindCode)
    uint16_t pad306;                // +306 填充
    uint32_t expGainPermil;         // +308 經驗值取得量 (밀초기)
    uint32_t partyExpGainPermil;    // +312 隊伍成員經驗值取得量 (밀초기)
    uint32_t weaponAtkMode;         // +316 武器攻擊方式 (atoi)
    uint32_t weaponAtkMultiFlag;    // +320 武器攻擊方式 ("MULTI"→1, 其他→0)
    uint32_t centeredOnMonster;     // +324 以怪物為中心套用的技能
    uint16_t effectProjectile;      // +328 效果5 (投射物)
    uint8_t  padTail[70];           // +330..+399 尾段填充
};

// 被動技能視角下的後段欄位（+112..+399，共 288 bytes）
struct stSkillKindInfoPassive {
    uint16_t shortDescCode;         // +112 技能短說明代碼
    uint16_t hpUpPct;               // +114 HP 上升 [%]
    uint16_t mpUpPct;               // +116 MP 上升 [%]
    uint16_t pad118;                // +118 填充
    uint32_t hitRate;               // +120 命中率
    uint32_t critRateUp;            // +124 爆擊機率上升
    uint32_t skillAtkPowerPct;      // +128 技能攻擊力修正 [%]
    uint32_t defAdjMonsterEtc;      // +132 防禦修正 (怪物-其他) [%]
    uint32_t defAdjMonsterStone;    // +136 防禦修正 (怪物-石) [%]
    uint32_t defAdjMonsterPlant;    // +140 防禦修正 (怪物-植物) [%]
    uint32_t defAdjMonsterAlien;    // +144 防禦修正 (怪物-異物) [%]
    uint32_t defAdjSpirit;          // +148 防禦修正 (精靈) [%]
    uint32_t defAdjCurse;           // +152 防禦修正 (詛咒) [%]
    uint32_t defAdjDevil;           // +156 防禦修正 (惡魔) [%]
    uint32_t defAdjDragon;          // +160 防禦修正 (龍) [%]
    uint32_t atkAdjMonsterEtc;      // +164 攻擊力修正 (怪物-其他) [%]
    uint32_t atkAdjMonsterStone;    // +168 攻擊力修正 (怪物-石) [%]
    uint32_t atkAdjMonsterPlant;    // +172 攻擊力修正 (怪物-植物) [%]
    uint32_t atkAdjMonsterAlien;    // +176 攻擊力修正 (怪物-異物) [%]
    uint32_t atkAdjSpirit;          // +180 攻擊力修正 (精靈) [%]
    uint32_t atkAdjCurse;           // +184 攻擊力修正 (詛咒) [%]
    uint32_t atkAdjDevil;           // +188 攻擊力修正 (惡魔) [%]
    uint32_t atkAdjDragon;          // +192 攻擊力修正 (龍) [%]
    uint32_t triggerRate;           // +196 觸發機率 (千分率)
    uint32_t triggerTime;           // +200 觸發時間
    uint32_t linkSkills[10];        // +204..+243 連動技能代碼陣列 (& 分隔，最多 10 個)
    uint32_t linkSkillCount;        // +244 連動技能數量
    uint32_t workingType;           // +248 觸發種類 (ATTACK_ONCE..USESKILL_SUSTAIN → 1..6)
    int32_t  triggerAttackTargetAtb;// +252 觸發時攻擊目標屬性 ('0'→-1)
    uint32_t durationMin;           // +256 持續時間 (分)；Circle skill 判定亦讀此欄
    uint32_t triggerOnHpChange;     // +260 發動時間 (HP 變化時點)
    uint32_t triggerOnSkillUse;     // +264 發動時間 (技能使用時)
    uint32_t maxConcurrentCycle;    // +268 最大同時循環人數
    uint32_t addRecoverPermil;      // +272 追加恢復量 (千分率基準)
    uint32_t addHpRecoverPermil;    // +276 HP 追加恢復量 (千分率)
    uint32_t addMpRecoverPermil;    // +280 MP 追加恢復量 (千分率)
    uint32_t party2pAtkUp;          // +284 隊伍 2 人攻擊力增加 [%]
    uint32_t party2pDefUp;          // +288 隊伍 2 人防禦增加 [%]
    uint32_t party2pHitUp;          // +292 隊伍 2 人命中率增加 (千分率)
    uint32_t party3pAtkUp;          // +296 隊伍 3 人攻擊力增加 [%]
    uint32_t party3pDefUp;          // +300 隊伍 3 人防禦增加 [%]
    uint32_t party3pHitUp;          // +304 隊伍 3 人命中率增加 (千分率)
    uint32_t party4pAtkUp;          // +308 隊伍 4 人攻擊力增加 [%]
    uint32_t party4pDefUp;          // +312 隊伍 4 人防禦增加 [%]
    uint32_t party4pHitUp;          // +316 隊伍 4 人命中率增加 (千分率)
    uint32_t party5pAtkUp;          // +320 隊伍 5 人攻擊力增加 [%]
    uint32_t party5pDefUp;          // +324 隊伍 5 人防禦增加 [%]
    uint32_t party5pHitUp;          // +328 隊伍 5 人命中率增加 (千分率)
    uint32_t hpRecover;             // +332 HP 恢復
    uint32_t mpRecover;             // +336 MP 恢復
    uint32_t atkBlockProb;          // +340 攻擊時封鎖發動機率 (千分率)
    uint32_t atkBlockTime;          // +344 攻擊時封鎖時間
    uint32_t atkFreezeProb;         // +348 攻擊時結冰發動機率 (千分率)
    uint32_t atkFreezeTime;         // +352 攻擊時結冰時間
    uint32_t atkStunProb;           // +356 攻擊時暈眩發動機率 (千分率)
    uint32_t atkStunTime;           // +360 攻擊時暈眩時間
    uint32_t attackSpeed;           // +364 攻擊速度 (影響遊戲傷害)
    uint32_t damageReceiveConvert;  // +368 受傷時轉換成 (밀초기)
    uint32_t skillAfterAdvantage;   // +372 技能使用後增益 (밀초기)
    uint32_t monsterDamageReduce;   // +376 從怪物受到的傷害減免
    uint32_t monsterDefAdj;         // +380 對怪物防禦修正 (밀초기)
    uint32_t monsterAtkAdj;         // +384 對怪物攻擊力修正 (밀초기)
    uint32_t monsterHitAdj;         // +388 對怪物命中率修正 (千分率)
    uint32_t monsterEvadeAdj;       // +392 對怪物迴避力修正 (千分率)
    uint32_t monsterCritAdj;        // +396 對怪物爆擊率修正 (千分率)
};

struct stSkillKindInfo {
    // ── 共用欄位（+0..+111，Passive 與 Active 寫入位置一致） ───────────────
    uint16_t skillCode;             // +0   技能 ID (P/A 前綴 + 5 位數字)
    uint16_t initialSkillCode;      // +2   初始 ID (初次取得時的 ID)
    uint16_t nameCode;              // +4   DCT 名稱代碼
    uint16_t descCode;              // +6   DCT 技能說明代碼
    uint32_t iconID;                // +8   圖示資源 ID (hex 字串解析)
    uint16_t iconBlock;             // +12  圖示圖塊
    uint16_t pad14;                 // +14  填充
    uint32_t smallIconID;           // +16  小圖示資源 ID (hex)
    uint16_t smallIconBlock;        // +20  小圖示圖塊
    uint16_t pad22;                 // +22  填充
    uint32_t attackPowerUp;         // +24  攻擊力上升 [%] (Active: 攻擊力加成)
    uint32_t defensePowerUp;        // +28  防禦力上升 [%] (Active: 防禦力加成)
    uint8_t  skillLevel;            // +32  技能等級 (Initialize 後計算填入)
    uint8_t  pad33[3];              // +33  填充
    uint32_t cooltime;              // +36  重新使用時間 (毫秒；Active 專用，Passive 恆為 0)
    uint64_t acquireClassMask;      // +40  學習職業遮罩 (GetReqClasses 結果)
    uint32_t skillBuyClass;         // +48  技能購買類型 (SWORD/BOW/THEOLOGY/MAGIC/SPECIALTY → 0..4)
    uint32_t needFigPt;             // +52  需要戰士點數
    uint32_t needArcPt;             // +56  需要弓手點數
    uint32_t needMagPt;             // +60  需要法師點數
    uint32_t needPriPt;             // +64  需要祭司點數
    uint16_t reqLevel;              // +68  需要等級
    uint16_t prevSkillCode;         // +70  上位/前置技能代碼
    // +72..+87: 需要武器位旗標 (16 bytes)
    //   索引 1=LONG_SWORD, 2=SHORT_SWORD, 3=HAMMER, 4=LONG_SWORD_TH, 5=TOMAHAWK,
    //        6=SPEAR, 7=STAFF, 8=BOW, 9=GUN, 10=DUAL_SWORD, 11=CROSSBOW,
    //        12=SPELLBOOK, 13=HOLYCROSS, 14=SHIELD
    uint8_t  reqWeaponFlags[16];    // +72  需要武器位旗標
    uint32_t reqWeaponCount;        // +88  需要武器數量 (reqWeaponFlags 中為 1 的數量)
    uint16_t effectTopUp1;          // +92  效果1 (上層-1)
    uint16_t effectTopUp2;          // +94  效果2 (上層-2)
    uint16_t effectBotDown1;        // +96  效果3 (下層-1)
    uint16_t effectBotDown2;        // +98  效果4 (下層-2)
    uint16_t effectHittedTopUp1;    // +100 效果7 (受擊上層-1)
    uint16_t effectHittedTopUp2;    // +102 效果8 (受擊上層-2)
    uint16_t effectHittedBotDown1; // +104 效果9 (受擊下層-1)
    uint16_t effectHittedBotDown2; // +106 效果10 (受擊下層-2)
    uint16_t effectFlyTop;          // +108 效果5 (飛行軌跡-上)
    uint16_t effectFlyBot;          // +110 效果6 (飛行軌跡-下)

    // ── Passive 與 Active 後段欄位（+112..+399） ─────────────────────────────
    union {
        stSkillKindInfoActive  active;   // 主動技能視角
        stSkillKindInfoPassive passive;  // 被動技能視角
    } u;
};

#pragma pack(pop)

static_assert(sizeof(stSkillKindInfo) == 400, "stSkillKindInfo must be 400 bytes");
static_assert(sizeof(stSkillKindInfoActive) == 288, "Active sub-struct must be 288 bytes");
static_assert(sizeof(stSkillKindInfoPassive) == 288, "Passive sub-struct must be 288 bytes");

// ── 共用欄位偏移驗證 ────────────────────────────────────────────────────
static_assert(offsetof(stSkillKindInfo, skillCode)              ==   0, "skillCode @ +0");
static_assert(offsetof(stSkillKindInfo, initialSkillCode)       ==   2, "initialSkillCode @ +2");
static_assert(offsetof(stSkillKindInfo, nameCode)               ==   4, "nameCode @ +4");
static_assert(offsetof(stSkillKindInfo, descCode)               ==   6, "descCode @ +6");
static_assert(offsetof(stSkillKindInfo, iconID)                 ==   8, "iconID @ +8");
static_assert(offsetof(stSkillKindInfo, iconBlock)              ==  12, "iconBlock @ +12");
static_assert(offsetof(stSkillKindInfo, smallIconID)            ==  16, "smallIconID @ +16");
static_assert(offsetof(stSkillKindInfo, smallIconBlock)         ==  20, "smallIconBlock @ +20");
static_assert(offsetof(stSkillKindInfo, attackPowerUp)          ==  24, "attackPowerUp @ +24");
static_assert(offsetof(stSkillKindInfo, defensePowerUp)         ==  28, "defensePowerUp @ +28");
static_assert(offsetof(stSkillKindInfo, skillLevel)             ==  32, "skillLevel @ +32");
static_assert(offsetof(stSkillKindInfo, cooltime)               ==  36, "cooltime @ +36");
static_assert(offsetof(stSkillKindInfo, acquireClassMask)       ==  40, "acquireClassMask @ +40");
static_assert(offsetof(stSkillKindInfo, skillBuyClass)          ==  48, "skillBuyClass @ +48");
static_assert(offsetof(stSkillKindInfo, needFigPt)              ==  52, "needFigPt @ +52");
static_assert(offsetof(stSkillKindInfo, needArcPt)              ==  56, "needArcPt @ +56");
static_assert(offsetof(stSkillKindInfo, needMagPt)              ==  60, "needMagPt @ +60");
static_assert(offsetof(stSkillKindInfo, needPriPt)              ==  64, "needPriPt @ +64");
static_assert(offsetof(stSkillKindInfo, reqLevel)               ==  68, "reqLevel @ +68");
static_assert(offsetof(stSkillKindInfo, prevSkillCode)          ==  70, "prevSkillCode @ +70");
static_assert(offsetof(stSkillKindInfo, reqWeaponFlags)         ==  72, "reqWeaponFlags @ +72");
static_assert(offsetof(stSkillKindInfo, reqWeaponCount)         ==  88, "reqWeaponCount @ +88");
static_assert(offsetof(stSkillKindInfo, effectTopUp1)           ==  92, "effectTopUp1 @ +92");
static_assert(offsetof(stSkillKindInfo, effectTopUp2)           ==  94, "effectTopUp2 @ +94");
static_assert(offsetof(stSkillKindInfo, effectBotDown1)         ==  96, "effectBotDown1 @ +96");
static_assert(offsetof(stSkillKindInfo, effectBotDown2)         ==  98, "effectBotDown2 @ +98");
static_assert(offsetof(stSkillKindInfo, effectHittedTopUp1)     == 100, "effectHittedTopUp1 @ +100");
static_assert(offsetof(stSkillKindInfo, effectHittedTopUp2)     == 102, "effectHittedTopUp2 @ +102");
static_assert(offsetof(stSkillKindInfo, effectHittedBotDown1)   == 104, "effectHittedBotDown1 @ +104");
static_assert(offsetof(stSkillKindInfo, effectHittedBotDown2)   == 106, "effectHittedBotDown2 @ +106");
static_assert(offsetof(stSkillKindInfo, effectFlyTop)           == 108, "effectFlyTop @ +108");
static_assert(offsetof(stSkillKindInfo, effectFlyBot)           == 110, "effectFlyBot @ +110");
static_assert(offsetof(stSkillKindInfo, u)                      == 112, "union @ +112");

// ── Active 偏移驗證（在 union 內，加上 +112） ──────────────────────────
static_assert(offsetof(stSkillKindInfo, u.active.effectType)            == 112, "active.effectType @ +112");
static_assert(offsetof(stSkillKindInfo, u.active.hpCost)                == 114, "active.hpCost @ +114");
static_assert(offsetof(stSkillKindInfo, u.active.mpCost)                == 116, "active.mpCost @ +116");
static_assert(offsetof(stSkillKindInfo, u.active.remainHpAfterUse)      == 120, "active.remainHpAfterUse @ +120");
static_assert(offsetof(stSkillKindInfo, u.active.remainMpAfterUse)      == 124, "active.remainMpAfterUse @ +124");
static_assert(offsetof(stSkillKindInfo, u.active.missHitWeight)         == 128, "active.missHitWeight @ +128");
static_assert(offsetof(stSkillKindInfo, u.active.normalHitWeight)       == 132, "active.normalHitWeight @ +132");
static_assert(offsetof(stSkillKindInfo, u.active.critHitWeight)         == 134, "active.critHitWeight @ +134");
static_assert(offsetof(stSkillKindInfo, u.active.durationTime)          == 136, "active.durationTime @ +136");
static_assert(offsetof(stSkillKindInfo, u.active.repeatTime)            == 138, "active.repeatTime @ +138");
static_assert(offsetof(stSkillKindInfo, u.active.castTime)              == 140, "active.castTime @ +140");
static_assert(offsetof(stSkillKindInfo, u.active.valueColumn)           == 144, "active.valueColumn @ +144");
static_assert(offsetof(stSkillKindInfo, u.active.soundName)             == 148, "active.soundName @ +148");
static_assert(offsetof(stSkillKindInfo, u.active.isLong)                == 164, "active.isLong @ +164");
static_assert(offsetof(stSkillKindInfo, u.active.targetRangeX)          == 166, "active.targetRangeX @ +166");
static_assert(offsetof(stSkillKindInfo, u.active.targetRangeY)          == 168, "active.targetRangeY @ +168");
static_assert(offsetof(stSkillKindInfo, u.active.castAnim)              == 170, "active.castAnim @ +170");
static_assert(offsetof(stSkillKindInfo, u.active.hitAtb)                == 171, "active.hitAtb @ +171");
static_assert(offsetof(stSkillKindInfo, u.active.attackAtb)             == 172, "active.attackAtb @ +172");
static_assert(offsetof(stSkillKindInfo, u.active.atkStunProb)           == 176, "active.atkStunProb @ +176");
static_assert(offsetof(stSkillKindInfo, u.active.atkStunTime)           == 180, "active.atkStunTime @ +180");
static_assert(offsetof(stSkillKindInfo, u.active.atkBlockProb)          == 184, "active.atkBlockProb @ +184");
static_assert(offsetof(stSkillKindInfo, u.active.atkBlockTime)          == 188, "active.atkBlockTime @ +188");
static_assert(offsetof(stSkillKindInfo, u.active.atkFreezeProb)         == 192, "active.atkFreezeProb @ +192");
static_assert(offsetof(stSkillKindInfo, u.active.atkFreezeTime)         == 196, "active.atkFreezeTime @ +196");
static_assert(offsetof(stSkillKindInfo, u.active.mineKindCode)          == 200, "active.mineKindCode @ +200");
static_assert(offsetof(stSkillKindInfo, u.active.fastRun)               == 204, "active.fastRun @ +204");
static_assert(offsetof(stSkillKindInfo, u.active.castCount)             == 208, "active.castCount @ +208");
static_assert(offsetof(stSkillKindInfo, u.active.spiritBonus)           == 212, "active.spiritBonus @ +212");
static_assert(offsetof(stSkillKindInfo, u.active.agilityBonus)          == 216, "active.agilityBonus @ +216");
static_assert(offsetof(stSkillKindInfo, u.active.staminaBonus)          == 220, "active.staminaBonus @ +220");
static_assert(offsetof(stSkillKindInfo, u.active.intelligenceBonus)     == 224, "active.intelligenceBonus @ +224");
static_assert(offsetof(stSkillKindInfo, u.active.partyAtkPower)         == 228, "active.partyAtkPower @ +228");
static_assert(offsetof(stSkillKindInfo, u.active.partyAgility)          == 232, "active.partyAgility @ +232");
static_assert(offsetof(stSkillKindInfo, u.active.partySpirit)           == 236, "active.partySpirit @ +236");
static_assert(offsetof(stSkillKindInfo, u.active.partyStamina)          == 240, "active.partyStamina @ +240");
static_assert(offsetof(stSkillKindInfo, u.active.partyAtkAtbFlag)       == 244, "active.partyAtkAtbFlag @ +244");
static_assert(offsetof(stSkillKindInfo, u.active.partyAtkAtb)           == 248, "active.partyAtkAtb @ +248");
static_assert(offsetof(stSkillKindInfo, u.active.value1)                == 252, "active.value1 @ +252");
static_assert(offsetof(stSkillKindInfo, u.active.value2)                == 254, "active.value2 @ +254");
static_assert(offsetof(stSkillKindInfo, u.active.value3)                == 256, "active.value3 @ +256");
static_assert(offsetof(stSkillKindInfo, u.active.atkUseProb)            == 260, "active.atkUseProb @ +260");
static_assert(offsetof(stSkillKindInfo, u.active.atkUseEff)             == 264, "active.atkUseEff @ +264");
static_assert(offsetof(stSkillKindInfo, u.active.atkSpeedActive)        == 268, "active.atkSpeedActive @ +268");
static_assert(offsetof(stSkillKindInfo, u.active.damageReceiveConvert)  == 272, "active.damageReceiveConvert @ +272");
static_assert(offsetof(stSkillKindInfo, u.active.atkLineFx)             == 276, "active.atkLineFx @ +276");
static_assert(offsetof(stSkillKindInfo, u.active.triggerProbDamage)     == 280, "active.triggerProbDamage @ +280");
static_assert(offsetof(stSkillKindInfo, u.active.triggerProbCurse)      == 284, "active.triggerProbCurse @ +284");
static_assert(offsetof(stSkillKindInfo, u.active.debuffStage)           == 292, "active.debuffStage @ +292");
static_assert(offsetof(stSkillKindInfo, u.active.debuffRideTime)        == 296, "active.debuffRideTime @ +296");
static_assert(offsetof(stSkillKindInfo, u.active.effectStage)           == 300, "active.effectStage @ +300");
static_assert(offsetof(stSkillKindInfo, u.active.debuffKindCode)        == 304, "active.debuffKindCode @ +304");
static_assert(offsetof(stSkillKindInfo, u.active.expGainPermil)         == 308, "active.expGainPermil @ +308");
static_assert(offsetof(stSkillKindInfo, u.active.partyExpGainPermil)    == 312, "active.partyExpGainPermil @ +312");
static_assert(offsetof(stSkillKindInfo, u.active.weaponAtkMode)         == 316, "active.weaponAtkMode @ +316");
static_assert(offsetof(stSkillKindInfo, u.active.weaponAtkMultiFlag)    == 320, "active.weaponAtkMultiFlag @ +320");
static_assert(offsetof(stSkillKindInfo, u.active.centeredOnMonster)     == 324, "active.centeredOnMonster @ +324");
static_assert(offsetof(stSkillKindInfo, u.active.effectProjectile)      == 328, "active.effectProjectile @ +328");

// ── Passive 偏移驗證 ───────────────────────────────────────────────────
static_assert(offsetof(stSkillKindInfo, u.passive.shortDescCode)        == 112, "passive.shortDescCode @ +112");
static_assert(offsetof(stSkillKindInfo, u.passive.hpUpPct)              == 114, "passive.hpUpPct @ +114");
static_assert(offsetof(stSkillKindInfo, u.passive.mpUpPct)              == 116, "passive.mpUpPct @ +116");
static_assert(offsetof(stSkillKindInfo, u.passive.hitRate)              == 120, "passive.hitRate @ +120");
static_assert(offsetof(stSkillKindInfo, u.passive.critRateUp)           == 124, "passive.critRateUp @ +124");
static_assert(offsetof(stSkillKindInfo, u.passive.skillAtkPowerPct)     == 128, "passive.skillAtkPowerPct @ +128");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjMonsterEtc)     == 132, "passive.defAdjMonsterEtc @ +132");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjMonsterStone)   == 136, "passive.defAdjMonsterStone @ +136");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjMonsterPlant)   == 140, "passive.defAdjMonsterPlant @ +140");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjMonsterAlien)   == 144, "passive.defAdjMonsterAlien @ +144");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjSpirit)         == 148, "passive.defAdjSpirit @ +148");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjCurse)          == 152, "passive.defAdjCurse @ +152");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjDevil)          == 156, "passive.defAdjDevil @ +156");
static_assert(offsetof(stSkillKindInfo, u.passive.defAdjDragon)         == 160, "passive.defAdjDragon @ +160");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjMonsterEtc)     == 164, "passive.atkAdjMonsterEtc @ +164");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjMonsterStone)   == 168, "passive.atkAdjMonsterStone @ +168");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjMonsterPlant)   == 172, "passive.atkAdjMonsterPlant @ +172");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjMonsterAlien)   == 176, "passive.atkAdjMonsterAlien @ +176");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjSpirit)         == 180, "passive.atkAdjSpirit @ +180");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjCurse)          == 184, "passive.atkAdjCurse @ +184");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjDevil)          == 188, "passive.atkAdjDevil @ +188");
static_assert(offsetof(stSkillKindInfo, u.passive.atkAdjDragon)         == 192, "passive.atkAdjDragon @ +192");
static_assert(offsetof(stSkillKindInfo, u.passive.triggerRate)          == 196, "passive.triggerRate @ +196");
static_assert(offsetof(stSkillKindInfo, u.passive.triggerTime)          == 200, "passive.triggerTime @ +200");
static_assert(offsetof(stSkillKindInfo, u.passive.linkSkills)           == 204, "passive.linkSkills @ +204");
static_assert(offsetof(stSkillKindInfo, u.passive.linkSkillCount)       == 244, "passive.linkSkillCount @ +244");
static_assert(offsetof(stSkillKindInfo, u.passive.workingType)          == 248, "passive.workingType @ +248");
static_assert(offsetof(stSkillKindInfo, u.passive.triggerAttackTargetAtb)==252, "passive.triggerAttackTargetAtb @ +252");
static_assert(offsetof(stSkillKindInfo, u.passive.durationMin)          == 256, "passive.durationMin @ +256");
static_assert(offsetof(stSkillKindInfo, u.passive.triggerOnHpChange)    == 260, "passive.triggerOnHpChange @ +260");
static_assert(offsetof(stSkillKindInfo, u.passive.triggerOnSkillUse)    == 264, "passive.triggerOnSkillUse @ +264");
static_assert(offsetof(stSkillKindInfo, u.passive.maxConcurrentCycle)   == 268, "passive.maxConcurrentCycle @ +268");
static_assert(offsetof(stSkillKindInfo, u.passive.addRecoverPermil)     == 272, "passive.addRecoverPermil @ +272");
static_assert(offsetof(stSkillKindInfo, u.passive.addHpRecoverPermil)   == 276, "passive.addHpRecoverPermil @ +276");
static_assert(offsetof(stSkillKindInfo, u.passive.addMpRecoverPermil)   == 280, "passive.addMpRecoverPermil @ +280");
static_assert(offsetof(stSkillKindInfo, u.passive.party2pAtkUp)         == 284, "passive.party2pAtkUp @ +284");
static_assert(offsetof(stSkillKindInfo, u.passive.party2pDefUp)         == 288, "passive.party2pDefUp @ +288");
static_assert(offsetof(stSkillKindInfo, u.passive.party2pHitUp)         == 292, "passive.party2pHitUp @ +292");
static_assert(offsetof(stSkillKindInfo, u.passive.party3pAtkUp)         == 296, "passive.party3pAtkUp @ +296");
static_assert(offsetof(stSkillKindInfo, u.passive.party3pDefUp)         == 300, "passive.party3pDefUp @ +300");
static_assert(offsetof(stSkillKindInfo, u.passive.party3pHitUp)         == 304, "passive.party3pHitUp @ +304");
static_assert(offsetof(stSkillKindInfo, u.passive.party4pAtkUp)         == 308, "passive.party4pAtkUp @ +308");
static_assert(offsetof(stSkillKindInfo, u.passive.party4pDefUp)         == 312, "passive.party4pDefUp @ +312");
static_assert(offsetof(stSkillKindInfo, u.passive.party4pHitUp)         == 316, "passive.party4pHitUp @ +316");
static_assert(offsetof(stSkillKindInfo, u.passive.party5pAtkUp)         == 320, "passive.party5pAtkUp @ +320");
static_assert(offsetof(stSkillKindInfo, u.passive.party5pDefUp)         == 324, "passive.party5pDefUp @ +324");
static_assert(offsetof(stSkillKindInfo, u.passive.party5pHitUp)         == 328, "passive.party5pHitUp @ +328");
static_assert(offsetof(stSkillKindInfo, u.passive.hpRecover)            == 332, "passive.hpRecover @ +332");
static_assert(offsetof(stSkillKindInfo, u.passive.mpRecover)            == 336, "passive.mpRecover @ +336");
static_assert(offsetof(stSkillKindInfo, u.passive.atkBlockProb)         == 340, "passive.atkBlockProb @ +340");
static_assert(offsetof(stSkillKindInfo, u.passive.atkBlockTime)         == 344, "passive.atkBlockTime @ +344");
static_assert(offsetof(stSkillKindInfo, u.passive.atkFreezeProb)        == 348, "passive.atkFreezeProb @ +348");
static_assert(offsetof(stSkillKindInfo, u.passive.atkFreezeTime)        == 352, "passive.atkFreezeTime @ +352");
static_assert(offsetof(stSkillKindInfo, u.passive.atkStunProb)          == 356, "passive.atkStunProb @ +356");
static_assert(offsetof(stSkillKindInfo, u.passive.atkStunTime)          == 360, "passive.atkStunTime @ +360");
static_assert(offsetof(stSkillKindInfo, u.passive.attackSpeed)          == 364, "passive.attackSpeed @ +364");
static_assert(offsetof(stSkillKindInfo, u.passive.damageReceiveConvert) == 368, "passive.damageReceiveConvert @ +368");
static_assert(offsetof(stSkillKindInfo, u.passive.skillAfterAdvantage)  == 372, "passive.skillAfterAdvantage @ +372");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterDamageReduce)  == 376, "passive.monsterDamageReduce @ +376");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterDefAdj)        == 380, "passive.monsterDefAdj @ +380");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterAtkAdj)        == 384, "passive.monsterAtkAdj @ +384");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterHitAdj)        == 388, "passive.monsterHitAdj @ +388");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterEvadeAdj)      == 392, "passive.monsterEvadeAdj @ +392");
static_assert(offsetof(stSkillKindInfo, u.passive.monsterCritAdj)       == 396, "passive.monsterCritAdj @ +396");

// ──────────────────────────────────────────────────────────────────────────
// 主類別
// ──────────────────────────────────────────────────────────────────────────
class cltSkillKindInfo {
public:
    // 反編譯：設定外部 Class 資訊供查詢
    static void InitializeStaticVariable(cltClassKindInfo* p) { m_pclClassKindInfo = p; }

    cltSkillKindInfo();
    ~cltSkillKindInfo() { Free(); }

    int  Initialize(char* passiveFile, char* activeFile);  // 先 P 再 A；並建立索引與等級
    int  Initialize_P(char* filename);                     // 解析 p_skillinfo.txt
    int  Initialize_A(char* filename);                     // 解析 a_skillinfo.txt
    void Free();

    // 依碼取記錄（位移行為對齊：原本為 *((_DWORD*)this + code)）
    stSkillKindInfo* GetSkillKindInfo(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_A(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P() { return m_passive.empty() ? nullptr : m_passive.data(); }
    stSkillKindInfo* GetSkillKindInfo_A() { return m_active.empty()  ? nullptr : m_active.data(); }

    // 數量查詢
    uint16_t GetPassiveSkillNum() const { return static_cast<uint16_t>(m_passive.size()); }
    uint16_t GetActiveSkillNum()  const { return static_cast<uint16_t>(m_active.size()); }
    int      GetTotalSkillNum()   const { return GetPassiveSkillNum() + GetActiveSkillNum(); }

    // 工具（與反編譯一致）
    static bool     IsPassiveSkill(uint16_t code);
    static bool     IsActiveSkill(uint16_t code);
    static uint16_t TranslateKindCode(const char* s);        // 'A'/'P' + 5 digits, 15-bit
    static uint16_t TranslateEffectKindCode(const char* s);  // 5 碼：字母 + 數字，(ch+31)<<11 | num<0x800

    static int      GetWorkingType(const char* s);           // ATTACK_ONCE/… → 1..6
    static uint8_t  GetActiveEffectType(const char* s);      // ATTACK_ONE/… → 1..11
    uint8_t         GetActiveEffectType(uint16_t code);      // 讀取記錄 u.active.effectType

    static int      GetSkillClass(const char* s);            // SWORD/BOW/THEOLOGY/MAGIC/SPECIALTY → 0..4

    // 來自表格欄位的需求解析
    static uint64_t GetReqClasses(cltSkillKindInfo* self, char* classes);   // 回傳 64-bit 遮罩
    static int      GetReqWeaponClasses(cltSkillKindInfo* self, char* weapons,
                                        uint8_t* outFlags /* 指向 reqWeaponFlags */);

    // 額外查詢
    char*    UseSkillSound(uint16_t code);   // 回傳 u.active.soundName
    uint16_t GetSkillLevel(uint16_t code);   // 依「上一級鏈」計算等級（至少 1）
    int      IsLastLevelSkill(uint16_t code);// 反編譯語意

    // (005945E0) cltSkillKindInfo::IsCircleSkillKind
    //   GT 邏輯：result = GetSkillKindInfo_P(this, code);
    //           if (result) result = (*((DWORD*)result + 64) != 0);  // 偏移 +256 = passive.durationMin
    static int IsCircleSkillKind(cltSkillKindInfo* self, uint16_t code);

    // 職業鏈判定（等級層級）
    int IsUnderTwoLevelClassSkill(int64_t atbMask);
    int IsTwoLevelClassSkill(int64_t atbMask);
    int IsUnderThreeLevelClassSkill(int64_t atbMask);
    int IsThreeLevelClassSkill(int64_t atbMask);
    int IsFourLevelClassSkill(int64_t atbMask);

    // 依職業取可用技能（沿職業樹向上蒐集；回傳寫入個數）
    int GetClassActiveSkill(uint16_t startClass, uint16_t* outCodes);
    int GetClassPassiveSkill(uint16_t startClass, uint16_t* outCodes);

    // 靜態外掛：職業資料
    static cltClassKindInfo* m_pclClassKindInfo;

    // 工具（與反編譯 IsDigit/IsAlphaNumeric 一致）
    static bool IsDigit(const char* s);            // 容許前導 +/- 後皆為 0-9; 空字串視為 true
    static bool IsAlphaNumeric(const char* s);     // 全為英數字; 空字串視為 true
    static bool NextTok(char*& out, const char* delims); // strtok 包裝

private:
    // 內部資料
    std::vector<stSkillKindInfo> m_passive;   // P*
    std::vector<stSkillKindInfo> m_active;    // A*
    stSkillKindInfo* m_idx[65536];            // 技能碼 → 記錄指標（含 P/A 全部）
};

// 可選：M 版（按反編譯行為，這裡給最小還原）
class cltSkillKindInfoM {
public:
    // M 版 P 表記錄：只保存 +0..+27 共 28 bytes 的部分共用欄位
    struct stSkillKindInfoMRec {
        uint16_t skillCode;         // +0   技能 ID
        uint16_t pad2;              // +2   填充
        uint32_t nameCode;          // +4   DCT 名稱代碼 (M 版以 DWORD 寫入)
        uint32_t iconID;            // +8   圖示資源 ID
        uint32_t iconBlock;         // +12  圖示圖塊
        uint32_t smallIconID;       // +16  小圖示資源 ID
        uint32_t smallIconBlock;    // +20  小圖示圖塊
        uint32_t prevField;         // +24  對應原表第 7 欄 (M 版以 DWORD 寫入)
    };
    static_assert(sizeof(stSkillKindInfoMRec) == 28, "stSkillKindInfoMRec must be 28 bytes");

    cltSkillKindInfoM();
    ~cltSkillKindInfoM() { Free(); }

    int  Initialize(char* pfile, char* afile);
    int  Initialize_P(char* filename);
    int  Initialize_A(char* filename) { (void)filename; return 1; }
    void Free();

    static uint16_t TranslateKindCode(const char* s) { return cltSkillKindInfo::TranslateKindCode(s); }
    static int      IsPassiveSkill(uint16_t c) { return cltSkillKindInfo::IsPassiveSkill(c); }
    static int      IsActiveSkill(uint16_t c) { return cltSkillKindInfo::IsActiveSkill(c); }

private:
    std::vector<stSkillKindInfoMRec> m_pList;       // 28 bytes 每筆
    std::vector<stSkillKindInfoMRec> m_aList;       // 未使用
    stSkillKindInfo* m_idx[65536]{};
};
