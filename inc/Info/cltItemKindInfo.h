#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <windows.h> // For MessageBoxA
#include "Info/cltSpecialtyKindInfo.h"
#include "Info/cltMakingItemKindInfo.h"
#include "Info/cltPandoraKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Info/cltPetSkillKindInfo.h"
#include "Info/cltPetKindInfo.h"
#include "Info/cltMyItemKindInfo.h"
#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include "Other/cltAttackAtb.h"
#include "global.h"

// Helper function to check if a string contains only digits.
// This is used multiple times in the parsing logic.
inline bool IsDigit(const char* str) {
    if (!str || *str == '\0') return false;
    for (const char* p = str; *p; ++p) {
        if (!isdigit((unsigned char)*p)) {
            return false;
        }
    }
    return true;
}

// Helper function to check if a string is alphanumeric.
inline bool IsAlphaNumeric(const char* str) {
    if (!str || *str == '\0') return false;
    for (const char* p = str; *p; ++p) {
        if (!isalnum((unsigned char)*p)) {
            return false;
        }
    }
    return true;
}

// Enumerations for item properties, derived from string comparisons in the code.
// 아이템 분류 (itemkindinfo.txt)
enum EItemClass {
    ITEM_CLASS_INSTANT = 0,
    ITEM_CLASS_HUNT = 1,
    ITEM_CLASS_FASHION = 2,
    ITEM_CLASS_TRAINING = 3,
    ITEM_CLASS_ETC = 4,
    ITEM_CLASS_NPCITEM = 5,
    ITEM_CLASS_GENERICBAG = 6,
    ITEM_CLASS_UPGRADE = 7,
    ITEM_CLASS_CASH_INSTANCE = 8,
    ITEM_CLASS_CASH_HUNT = 9,
    ITEM_CLASS_CASH_FASHION = 10,
    ITEM_CLASS_QUIZ = 11,
    ITEM_CLASS_CASH_TRAINING = 12,
    ITEM_CLASS_STOREHOUSE = 13,
    ITEM_CLASS_EVENT_INSTANT = 14,
    ITEM_CLASS_EVENT_FASHION = 15,
    ITEM_CLASS_ETC_NOTTRADE = 16,
    ITEM_CLASS_EXPAND_CIRCLEMEMBER = 17,
    ITEM_CLASS_PCBANG_INSTANT = 18,
    ITEM_CLASS_EVENT_HUNT = 19,
    ITEM_CLASS_EVENT_TRAINING = 20,
    ITEM_CLASS_EVENT_STOREHOUSE = 21,
    ITEM_CLASS_EVENT_UPGRADE = 22,
    ITEM_CLASS_COIN = 23,
    ITEM_CLASS_EVENT_NO_EX_STORAGE_BAG = 24,
};

// 아이템 타입 (itemkindinfo.txt)
enum EItemType {
    ITEM_TYPE_SHORT_SWORD = 0,
    ITEM_TYPE_LONG_SWORD = 1,
    ITEM_TYPE_LONG_SWORD_TH = 2,
    ITEM_TYPE_HAMMER = 3,
    ITEM_TYPE_TOMAHAWK = 4,
    ITEM_TYPE_SPEAR = 5,
    ITEM_TYPE_BOW = 6,
    ITEM_TYPE_GUN = 7,
    ITEM_TYPE_STAFF = 8,
    ITEM_TYPE_DUAL_SWORD = 9,
    ITEM_TYPE_CROSSBOW = 10,
    ITEM_TYPE_SPELLBOOK = 11,
    ITEM_TYPE_HOLYCROSS = 12,
    ITEM_TYPE_HELMET = 13,
    ITEM_TYPE_ARMOR = 14,
    ITEM_TYPE_LEGGIN = 15,
    ITEM_TYPE_GLOVE = 16,
    ITEM_TYPE_SHOES = 17,
    ITEM_TYPE_SHIELD = 18,
    ITEM_TYPE_RING = 19,
    ITEM_TYPE_NECKLACE = 20,
    ITEM_TYPE_INSTANT = 21,
    ITEM_TYPE_INCHANT_STONE = 22,
    ITEM_TYPE_MAKE_WAY = 23,
    ITEM_TYPE_SKILL_BOOK = 24,
    ITEM_TYPE_ETC = 25,
    ITEM_TYPE_QUIZ = 26,
    ITEM_TYPE_MAKING_WEAPON = 27,
    ITEM_TYPE_MAKING_INSTANT = 28,
    ITEM_TYPE_MAKING_MATERIAL = 29,
    ITEM_TYPE_COIN_ITEM = 30,
    ITEM_TYPE_COIN_BOX = 31,
    ITEM_TYPE_NPCITEM = 32,
};

// 장착 위치 (item_hunt.txt, item_fashion.txt)
enum EEquipAtb {
    EQUIP_ATB_F_CAP = 1 << 0,
    EQUIP_ATB_F_EYE = 1 << 1,
    EQUIP_ATB_F_FACE = 1 << 2,
    EQUIP_ATB_F_MANTEAU = 1 << 3,
    EQUIP_ATB_F_SHIRTS = 1 << 4,
    EQUIP_ATB_F_PANTS = 1 << 5,
    EQUIP_ATB_F_GLOVE = 1 << 6,
    EQUIP_ATB_F_SHOES = 1 << 7,
    EQUIP_ATB_F_FULLSET = 1 << 8,
    EQUIP_ATB_F_NOT_EQUIP = 1 << 9,
    EQUIP_ATB_F_HAIR = 1 << 10,
    EQUIP_ATB_F_NAMETAG = 1 << 11,
    EQUIP_ATB_F_CHATBALLON = 1 << 12,
    EQUIP_ATB_B_CAP = 1 << 12,
    EQUIP_ATB_B_RING = 1 << 13,
    EQUIP_ATB_B_NECKLACE = 1 << 14,
    EQUIP_ATB_B_WEAPON_OH = 1 << 15,
    EQUIP_ATB_B_WEAPON_TH = 1 << 16,
    EQUIP_ATB_B_SHIELD = 1 << 17,
    EQUIP_ATB_B_ARMOR = 1 << 18,
    EQUIP_ATB_B_LEGGIN = 1 << 19,
    EQUIP_ATB_B_GLOVE = 1 << 20,
    EQUIP_ATB_B_SHOES = 1 << 21,
    EQUIP_ATB_B_SPIRIT = 1 << 22,
};

// 효과타입 (item_instant.txt)
enum EInstantEffectType {
    INSTANT_EFFECT_ONCE = 0,
    INSTANT_EFFECT_SUSTAIN = 1,
    INSTANT_EFFECT_FREQUENCY = 2,
    INSTANT_EFFECT_SPECIAL = 3,
};

// 物品類別屬性位元旗標 (Item Class Attribute Bit Flags)
enum EItemTypeAttribute {
    ITEM_ATTR_USE = 1 << 0,  // 사용 가능 (可使用)
    ITEM_ATTR_SELL = 1 << 1,  // 상점 판매 가능 (可販售)
    ITEM_ATTR_DROP = 1 << 2,  // 드랍 가능 (可丟棄)
    ITEM_ATTR_EQUIP = 1 << 3,  // 장착 가능 (可裝備)
    ITEM_ATTR_TRADE = 1 << 4,  // 거래 가능 (可交易)
    ITEM_ATTR_QUICKSLOT = 1 << 5,  // 퀵슬롯 등록 가능 (可登錄快捷欄)
    ITEM_ATTR_STORAGE = 1 << 6,  // 창고 보관 가능 (可存倉庫)
    ITEM_ATTR_SELL_AGENCY = 1 << 7,  // 위탁 판매 가능 (可代售)
    ITEM_ATTR_CASH = 1 << 8,  // 캐시 아이템 (商城物品)
    ITEM_ATTR_TIMER = 1 << 9,  // 시간제 아이템 (計時物品)
    ITEM_ATTR_QUIZ = 1 << 10, // 퀴즈 아이템 (測驗物品)
    ITEM_ATTR_TRAINING = 1 << 11, // 트레이닝 카드 (訓練卡)
    ITEM_ATTR_EX_STORAGE = 1 << 12, // 통합 창고 보관 가능 (可存共用倉庫)
    ITEM_ATTR_PET_INVEN = 1 << 13, // 펫 인벤토리 보관 가능 (可存寵物背包)
    ITEM_ATTR_COIN = 1 << 14, // 코인 아이템 (錢幣物品)
};

#pragma pack(push, 1)
// =============================================================================
// stItemKindInfo : 0x128 (296) bytes — must match mofclient.c byte-for-byte.
//
// Part 1 (offsets 0..79) is loaded from `itemkindinfo.txt` for every item.
// Part 2 (offsets 80..295) is a tagged union whose interpretation depends on
// `m_byItemClass` (loaded above) and is filled in by one of:
//   - `item_instant.txt`        (Instant)
//   - `item_hunt.txt`           (Equip.Hunt)
//   - `item_fashion.txt`        (Equip.Fashion)
//   - `koreatext.txt`           (Quiz)
//   - `TrainningCardInfo.txt`   (Training)
//
// Each comment line carries (offset · 한글 · 中文) so the layout can be cross
// referenced against the Korean column headers in the .txt files.
// =============================================================================
struct stItemKindInfo {
    // ---------------- Part 1 (loaded from itemkindinfo.txt) ----------------
    unsigned short m_wKind;              //   0 · 아이템 ID                · 物品 ID
    unsigned short m_wTextCode;          //   2 · Text_Code                · 名稱文字代碼
    unsigned short m_wDescCode;          //   4 · 설명_Code                · 描述文字代碼
    char           _padding_6[2];        //   6 · padding (align m_dwPrice)
    unsigned int   m_dwPrice;            //   8 · 아이템 가격              · 物品價格 (≤10,000,000)
    unsigned int   m_dwPVPPoint;         //  12 · PVP 구매 포인트          · PVP 點數
    unsigned int   m_dwPrice1;           //  16 · 금화 가격                · 金幣價格
    unsigned int   m_dwPrice2;           //  20 · 은화 가격                · 銀幣價格
    unsigned int   m_dwPrice3;           //  24 · 동화 가격                · 銅幣價格
    unsigned int   m_dwIconResID;        //  28 · 아이템 그림 파일 id (HEX)· 圖檔 ID
    unsigned short m_wIconBlockID;       //  32 · 아이템 그림 블록 id      · 圖檔方塊
    unsigned char  m_byItemClass;        //  34 · 아이템 분류              · 物品類別 (EItemClass)
    unsigned char  m_byMaxPileUpNum;     //  35 · 최대 묶음 수             · 最大堆疊
    char           m_szUseSound[16];     //  36 · 사용 효과음              · 使用音效 (strcpy, ≤15)
    unsigned short m_wItemType;          //  52 · 아이템 타입              · 物品類型 (EItemType)
    unsigned char  m_byQuestCollect;     //  54 · 퀘스트 수집 아이템       · 任務收集
    char           _padding_55[1];       //  55 · padding (align)
    unsigned int   m_dwMakingCategory;   //  56 · 제작 분류                · 製造分類
    unsigned short m_wUseTerm;           //  60 · 사용 기한                · 使用期限
    char           _padding_62[2];       //  62 · padding (align)
    unsigned int   m_dwWeight;           //  64 · 정렬 값                  · 排序權重
    int            m_bHomepageView;      //  68 · 홈페이지 노출 여부 (Y=1) · 首頁公開
    int            m_bCanSealItem;       //  72 · 봉인 가능 여부 ('1'=true)· 可封印
    char           _padding_76[4];       //  76 · padding (align union @80)

    // -------- Part 2 (item-class-specific union, offsets 80..295) ----------
    union {
        // ============ INSTANT items (item_instant.txt, 68 columns) =========
        struct {
            unsigned char  m_byInstantEffectType;  //  80 · 효과 주기            · 效果週期 (ONCE/SUSTAIN/FREQUENCY/SPECIAL)
            char           _padding_81[1];         //  81 · padding
            unsigned short m_wAddHP;               //  82 · HP 증가              · 增 HP
            unsigned short m_wAddMP;               //  84 · MP 증가              · 增 MP
            unsigned short m_wAddHP_Percent;       //  86 · HP 증가(%)           · 增 HP %
            unsigned short m_wAddMP_Percent;       //  88 · MP 증가(%)           · 增 MP %
            char           _padding_90[2];         //  90 · padding (align dword)
            unsigned int   m_dwSustainTime;        //  92 · 지속 시간            · 持續時間
            char           _padding_96[4];         //  96 · padding
            unsigned short m_wAddStr;              // 100 · 힘 증가              · 增力量
            unsigned short m_wAddDex;              // 102 · 민첩 증가            · 增敏捷
            unsigned short m_wAddInt;              // 104 · 지력 증가            · 增智力
            unsigned short m_wAddSta;              // 106 · 체력 증가            · 增體力
            unsigned short m_wAddMaxHP_Percent;    // 108 · 최대 HP 증가(%)      · 增最大 HP %
            unsigned short m_wAddMaxMP_Percent;    // 110 · 최대 MP 증가(%)      · 增最大 MP %
            unsigned short m_wAddAccuracy;         // 112 · 명중 증가            · 命中
            unsigned short m_wAddEvasion;          // 114 · 회피 증가            · 閃避
            unsigned short m_wAddCritical;         // 116 · 크리티컬 확률 증가   · 暴擊
            unsigned short m_wAddHit_Beast;        // 118 · 야수형 추가 공격(%)  · 對野獸 +攻 %
            unsigned short m_wAddHit_Monster;      // 120 · 일반 몬스터 추가 공격· 對怪物 +攻 %
            unsigned short m_wAddHit_Undead;       // 122 · 언데드 추가 공격(%)  · 對亡靈 +攻 %
            unsigned short m_wAddHit_Demon;        // 124 · 데몬 추가 공격(%)    · 對惡魔 +攻 %
            unsigned short m_wAddDef_Beast;        // 126 · 야수형 추가 방어(%)  · 對野獸 +防 %
            unsigned short m_wAddDef_Monster;      // 128 · 일반 몬스터 추가 방어· 對怪物 +防 %
            unsigned short m_wAddDef_Undead;       // 130 · 언데드 추가 방어(%)  · 對亡靈 +防 %
            unsigned short m_wAddDef_Demon;        // 132 · 데몬 추가 방어(%)    · 對惡魔 +防 %
            unsigned short m_wAddMagicResist;      // 134 · 마법 저항            · 魔抗
            unsigned char  m_wElementGive;         // 136 · 속성 부여            · 屬性賦予 (BYTE)
            unsigned char  m_wItemType;            // 137 · 아이템 타입          · RETURN/TELEPORT/TOWNPORTAL = 1/2/3
            unsigned short m_RequiredSpecialty;    // 138 · 필요 특기            · 所需專精 (translate)
            unsigned short m_AddedCraftItemCode;   // 140 · 추가 제작 아이템 코드· 新增製作物品 (translate, WORD)
            char           _padding_142[2];        // 142 · padding (matches GT)
            int            m_CharRevival;          // 144 · 캐릭터 부활 (Y=1)    · 角色復活
            int            m_StatInitialize;       // 148 · 능력치 초기화 (Y=1)  · 能力重置
            int            m_SkillSpecInitialize;  // 152 · 스킬 특기 초기화     · 技能專精重置
            int            m_SkillCircleInitialize;// 156 · 스킬 서클 초기화     · 技能循環重置
            int            m_JobChange;            // 160 · 직업 변경 (Y=1)      · 轉職
            int            m_Sprint;               // 164 · 달리기               · 衝刺
            int            m_GenderChange;         // 168 · 성별 변경            · 變性別
            char           m_bHairGender;          // 172 · 헤어 성별 (M/F)      · 髮型性別
            char           _padding_173[3];        // 173 · padding
            int            m_HairStyleID;          // 176 · 헤어 스타일 ID       · 髮型 ID
            char           m_HairStyleValue;       // 180 · 헤어 스타일 값       · 髮型值 (BYTE)
            char           m_FaceGender;           // 181 · 얼굴 성형 성별 (M/F) · 臉型性別
            char           _padding_182[2];        // 182 · padding
            int            m_FaceStyleID;          // 184 · 성형/미용 ID         · 臉型 ID
            char           m_FaceStyleValue;       // 188 · 성형 값              · 臉型值 (BYTE)
            char           _padding_189[3];        // 189 · padding
            int            m_HairColorCode;        // 192 · 헤어 염색 코드 (HEX) · 髮色十六進
            unsigned short m_dwPandora;            // 196 · 판도라               · 潘朵拉 (translate, WORD)
            char           _padding_198[2];        // 198 · padding (high WORD unused)
            int            m_dwShout;              // 200 · 외침                 · 呼喊 (atoi)
            int            m_bMessage;             // 204 · 메시지               · 訊息 (atoi)
            unsigned short m_wAcquireSkillID;      // 208 · 획득 스킬 ID         · 獲得技能 (translate)
            unsigned short m_wWeather;             // 210 · 날씨                 · 天氣 (translate)
            unsigned short m_wUsablePetKind[10];   // 212 · 사용 가능한 펫       · 可用寵物列表 (10×WORD)
            int            m_nUsablePetCount;      // 232 · 펫 개수              · 寵物計數
            unsigned short m_PetBirth;             // 236 · 펫 출생              · 寵物出生 (translate)
            unsigned short m_PetAquiredSkillID;    // 238 · 펫 획득 스킬         · 寵物獲技 (translate)
            unsigned int   m_PetFullnessPerItem;   // 240 · 펫 포만도 +개당      · 飽食度+
            unsigned int   m_PetAffectionIncrease; // 244 · 펫 호감도 +          · 好感度+
            unsigned int   m_dwPetStorageExpansion;// 248 · 펫 창고 확장         · 寵物倉擴 (DWORD)
            unsigned int   m_dwPetRenameTicket;    // 252 · 펫 이름 변경         · 寵物改名券
            unsigned short m_wPetDyeID;            // 256 · 펫 염색 ID           · 寵物染色 (translate)
            char           _padding_258[2];        // 258 · padding (matches GT)
            unsigned int   m_dwAddAttack;          // 260 · 공격력 증가          · 攻擊+
            unsigned int   m_dwAddDefense;         // 264 · 방어력 증가          · 防禦+
            unsigned int   m_dwStatusCure;         // 268 · 상태 회복            · 狀態回復
            unsigned short m_wMyItemKind;          // 272 · myitemkind           · 我的物品 (translate)
            char           _padding_274[2];        // 274 · padding
            unsigned int   m_dwMyItemPeriod;       // 276 · myitemperiod         · 我的物品週期
            unsigned short m_wCoupleRingId;        // 280 · 커플링 id            · 婚戒 (translate)
            unsigned short m_wEffectId;            // 282 · 이펙트 id            · 特效 (translate)
            unsigned int   m_dwChangeCharNameId;   // 284 · 이름 변경권 id       · 改名券
            unsigned char  m_bChangeCoinType;      // 288 · 코인 분류 0/G/S/B    · 0/GOLD/SILVER/BRONZE
            unsigned char  m_bUseChangeServer;     // 289 · 서버이전권           · 跨服券
            unsigned char  m_bIsSealItem;          // 290 · 봉인 아이템          · 封印物品
            char           _padding_291[5];        // 291..295 · padding to 296
        } Instant;

        // === HUNT / FASHION (item_hunt.txt 53 cols, item_fashion.txt 25 cols)
        struct {
            unsigned int m_dwEquipAtb;             //  80 · 장착 위치            · 裝備位置 (EquipAtb bitmask)
            union {
                // -------- HUNT (item_hunt.txt) --------
                struct {
                    unsigned short m_wWeaponType;       //  84 · 무기 타입       · 武器型 (1..14)
                    unsigned short m_wMinAttack;        //  86 · 최소 공격력     · 最小攻擊
                    unsigned short m_wMaxAttack;        //  88 · 최대 공격력     · 最大攻擊 (<1000)
                    unsigned char  m_byAttackSpeed;     //  90 · 공격 속도       · 攻速
                    char           _padding_91[1];      //  91 · padding
                    int            m_dwAttackType;      //  92 · 공격 방식 SINGLE/MULTI · 攻擊方式 (BOOL)
                    unsigned short m_wRange;            //  96 · 발사체          · 投射物
                    unsigned short m_wDef;              //  98 · 방어력          · 防禦
                    unsigned short m_wNeedStr;          // 100 · 힘 요구치       · 需力量
                    unsigned short m_wNeedSta;          // 102 · 체력 요구치     · 需體力
                    unsigned short m_wNeedDex;          // 104 · 민첩 요구치     · 需敏捷
                    unsigned short m_wNeedInt;          // 106 · 지력 요구치     · 需智力
                    // m_szEquipableClass[12] : raw class string slot. Bytes
                    // 0..3 (offsets 108..111) are unused; bytes 4..11 hold the
                    // 64-bit class bitmask written by GetEquipableClassAtb
                    // (low DWORD at +112, high DWORD at +116).
                    char           m_szEquipableClass[12];// 108 · 직업 요구치     · 職業限制 (bitmask @ +4)
                    unsigned char  m_byLevel;           // 120 · 레벨 요구치     · 等級
                    char           _padding_121[1];     // 121 · padding
                    unsigned short m_wAddStr;           // 122 · 힘 +            · 力量+
                    unsigned short m_wAddSta;           // 124 · 체력 +          · 體力+
                    unsigned short m_wAddDex;           // 126 · 민첩 +          · 敏捷+
                    unsigned short m_wAddInt;           // 128 · 지력 +          · 智力+
                    char           _padding_130[2];     // 130 · padding
                    int            m_dwAddAtt;          // 132 · 일반 공격력 +(100%) · 一般攻擊+ (<100)
                    unsigned short m_wAddSkillAtt;      // 136 · 스킬 공격력 +(100%) · 技能攻擊+ (<100)
                    char           _padding_138[2];     // 138 · padding
                    int            m_dwAddDefence;      // 140 · 방어력 +(100%)  · 防禦+ (<100)
                    unsigned short m_wAddMaxHP_Percent; // 144 · HP +(100%)      · HP+ % (<100)
                    unsigned short m_wAddMaxMP_Percent; // 146 · MP +(100%)      · MP+ % (<100)
                    unsigned short m_wAddHPRegen;       // 148 · HP 회복         · HP 回復+ (<1000)
                    unsigned short m_wAddMPRegen;       // 150 · MP 회복         · MP 回復+ (<1000)
                    unsigned short m_wAddAccuracy;      // 152 · 명중률 +        · 命中+ (<1000)
                    unsigned short m_wAddEvasion;       // 154 · 회피 +          · 閃避+ (<1000)
                    unsigned short m_wAddCritical;      // 156 · 크리티컬 확률 + · 暴擊+ (<1000)
                    unsigned short m_wAddHit_Beast;     // 158 · 비스트형 추가공격(100%) · 對野獸 +攻
                    unsigned short m_wAddHit_Monster;   // 160 · 일반 몬스터 추가공격(100%) · 對怪物 +攻
                    unsigned short m_wAddHit_Undead;    // 162 · 언데드형 추가공격(100%) · 對亡靈 +攻
                    unsigned short m_wAddHit_Demon;     // 164 · 데몬형 추가공격(100%) · 對惡魔 +攻
                    unsigned short m_wAddDef_Beast;     // 166 · 비스트형 추가방어 · 對野獸 +防
                    unsigned short m_wAddDef_Monster;   // 168 · 일반 몬스터 추가방어 · 對怪物 +防
                    unsigned short m_wAddDef_Undead;    // 170 · 언데드형 추가방어 · 對亡靈 +防
                    unsigned short m_wAddDef_Demon;     // 172 · 데몬형 추가방어  · 對惡魔 +防
                    unsigned short m_wMagicResist;      // 174 · 마법 저항       · 魔抗
                    unsigned char  m_byAttribute;       // 176 · 속성            · 屬性
                    char           _padding_177[3];     // 177 · padding
                    unsigned int   m_dwEnchantAttribute;// 180 · 인챈트 가능 여부· 附魔屬性
                    unsigned int   m_dwAttSpeed;        // 184 · 공격 속도       · 攻速數值
                    unsigned char  m_byAniSpeed;        // 188 · 애니 속도       · 動畫速度
                    char           m_szSoundName[17];   // 189 · 어택 사운드     · 攻擊音效 (strcpy)
                    unsigned short m_wSearchRangeX;     // 206 · 탐지범위 x      · 搜尋範圍 X
                    unsigned short m_wSearchRangeY;     // 208 · 탐지범위 y      · 搜尋範圍 Y
                    unsigned short m_wAttackAreaX;      // 210 · 공격범위 x      · 攻擊範圍 X
                    unsigned short m_wAttackAreaY;      // 212 · 공격범위 y      · 攻擊範圍 Y
                    unsigned char  m_byEnchantLevel;    // 214 · 인챈트 단계     · 附魔等級
                    char           _padding_215[1];     // 215 · padding
                    unsigned short m_wItemEffect;       // 216 · 아이템 이펙트   · 物品特效 (translate)
                    unsigned char  m_byRareType;        // 218 · 레어 구분       · 稀有度
                    char           _padding_219[1];     // 219 · padding
                    unsigned int   m_dwAttackAtb;       // 220 · 무기 속성       · 武器屬性 (cltAttackAtb)
                    char           _padding_224[72];    // 224..295 · padding
                } Hunt;

                // -------- FASHION (item_fashion.txt) --------
                struct {
                    unsigned char  m_byGender;          //  84 · 장착 부위 성별  · 性別限定 (M/F)
                    char           _padding_85[3];      //  85 · padding
                    unsigned int   m_dwUsePeriod;       //  88 · 사용 기간       · 使用期 (0/1/7/15/30/365)
                    unsigned short m_wAddExpPercent;    //  92 · 경험치 보너스(%)· 經驗加成
                    char           _padding_94[2];      //  94 · padding
                    unsigned short m_wAddFame;          //  96 · 명성 증가       · 名聲+
                    unsigned short m_wAddHPRegen;       //  98 · HP 회복량 증가  · HP回復+
                    unsigned short m_wAddMPRegen;       // 100 · MP 회복량 증가  · MP回復+
                    char           _padding_102[2];     // 102 · padding
                    unsigned int   m_dwCloakEffect;     // 104 · 망토 효과 (HEX) · 披風特效 ID (sscanf %x)
                    unsigned int   m_dwCloakEffectOrder;// 108 · 망토 효과 순서  · 披風順序 (atoi)
                    unsigned char  m_byCloakEffectCount;// 112 · 망토 효과 갯수  · 披風數量
                    char           _padding_113[1];     // 113 · padding
                    unsigned short m_wCapeEffectVisual; // 114 · 망토 이펙트 비주얼 · 披風表現 (translate)
                    int            m_dwDamagePercent;   // 116 · 공격력 증가(%)  · 攻擊+ % (<100)
                    int            m_dwDefPercent;      // 120 · 방어력 증가(%)  · 防禦+ % (<100)
                    int            m_dwAccuracyThousand;// 124 · 명중률 +(천분)  · 命中‰ (<1000)
                    int            m_dwEvasionThousand; // 128 · 회피 +(천분)    · 閃避‰
                    int            m_dwAddStr;          // 132 · 힘 증가         · 力量+
                    int            m_dwAddDex;          // 136 · 민첩 증가       · 敏捷+
                    int            m_dwAddInt;          // 140 · 지력 증가       · 智力+
                    int            m_dwAddSta;          // 144 · 체력 증가       · 體力+
                    unsigned short m_wFashionHpBarOffset;// 148 · HP바 오프셋     · HP條偏移
                    unsigned short m_wNameTagKind;      // 150 · NameTagKind     · 名牌 (translate)
                    unsigned short m_wChatBallonKind;   // 152 · ChatBallonKind  · 聊天泡 (translate)
                    unsigned short m_wCarKindInfo;      // 154 · carkindinfo     · 載具 (translate)
                    char           _padding_156[140];   // 156..295 · padding
                } Fashion;
            };
        } Equip;

        // ========== TRAINING cards (TrainningCardInfo.txt, 19 cols) ========
        struct {
            unsigned char m_byMultiplier;        //  80 · 배수             · 倍率
            char          _padding_81[3];        //  81 · padding
            // 8 skill rows. txt order: 검술1, 검술2, 궁술1, 궁술2,
            // 마법1, 마법2, 신학1, 신학2 → indices 0..7.
            struct {
                unsigned int   m_dwResID;        // +0 · 리소스 ID (HEX)  · 資源 (sscanf %x)
                unsigned short m_wBlockNum;      // +4 · 블록 번호        · 區塊
                char           _pad[2];          // +6 · padding (8B stride)
            } m_Skill[8];                        //  84..147 (64 bytes)
            char          _padding_148[148];     // 148..295 · padding
        } Training;

        // ============== QUIZ items (koreatext.txt, 4 cols) =================
        struct {
            unsigned short m_wHangleID;          //  80 · 인덱스           · 索引
            unsigned char  m_bySyllableType;     //  82 · 자모구분         · 字母區分 (1=한글, 2=영문)
            char           _padding_83[213];     //  83..295 · padding
        } Quiz;
    };
};
#pragma pack(pop)

// Compile-time verification of the layout. Any drift in the offsets of these
// fields means the on-disk byte layout no longer matches mofclient.c.
static_assert(sizeof(stItemKindInfo) == 296, "stItemKindInfo size must be 296 bytes");
static_assert(offsetof(stItemKindInfo, m_dwPrice)                       ==   8, "m_dwPrice must be at +8");
static_assert(offsetof(stItemKindInfo, m_dwIconResID)                   ==  28, "m_dwIconResID must be at +28");
static_assert(offsetof(stItemKindInfo, m_byItemClass)                   ==  34, "m_byItemClass must be at +34");
static_assert(offsetof(stItemKindInfo, m_szUseSound)                    ==  36, "m_szUseSound must be at +36");
static_assert(offsetof(stItemKindInfo, m_wItemType)                     ==  52, "m_wItemType must be at +52");
static_assert(offsetof(stItemKindInfo, m_dwMakingCategory)              ==  56, "m_dwMakingCategory must be at +56");
static_assert(offsetof(stItemKindInfo, m_dwWeight)                      ==  64, "m_dwWeight must be at +64");
static_assert(offsetof(stItemKindInfo, m_bHomepageView)                 ==  68, "m_bHomepageView must be at +68");
static_assert(offsetof(stItemKindInfo, m_bCanSealItem)                  ==  72, "m_bCanSealItem must be at +72");
static_assert(offsetof(stItemKindInfo, Instant.m_byInstantEffectType)   ==  80, "Instant @ +80");
static_assert(offsetof(stItemKindInfo, Instant.m_dwSustainTime)         ==  92, "Instant.m_dwSustainTime @ +92");
static_assert(offsetof(stItemKindInfo, Instant.m_wAddStr)               == 100, "Instant.m_wAddStr @ +100");
static_assert(offsetof(stItemKindInfo, Instant.m_wAddSta)               == 106, "Instant.m_wAddSta @ +106");
static_assert(offsetof(stItemKindInfo, Instant.m_wAddMagicResist)       == 134, "Instant.m_wAddMagicResist @ +134");
static_assert(offsetof(stItemKindInfo, Instant.m_AddedCraftItemCode)    == 140, "Instant.m_AddedCraftItemCode @ +140");
static_assert(offsetof(stItemKindInfo, Instant.m_CharRevival)           == 144, "Instant.m_CharRevival @ +144");
static_assert(offsetof(stItemKindInfo, Instant.m_JobChange)             == 160, "Instant.m_JobChange @ +160");
static_assert(offsetof(stItemKindInfo, Instant.m_HairColorCode)         == 192, "Instant.m_HairColorCode @ +192");
static_assert(offsetof(stItemKindInfo, Instant.m_dwPandora)             == 196, "Instant.m_dwPandora @ +196");
static_assert(offsetof(stItemKindInfo, Instant.m_wWeather)              == 210, "Instant.m_wWeather @ +210");
static_assert(offsetof(stItemKindInfo, Instant.m_wUsablePetKind)        == 212, "Instant.m_wUsablePetKind @ +212");
static_assert(offsetof(stItemKindInfo, Instant.m_nUsablePetCount)       == 232, "Instant.m_nUsablePetCount @ +232");
static_assert(offsetof(stItemKindInfo, Instant.m_PetBirth)              == 236, "Instant.m_PetBirth @ +236");
static_assert(offsetof(stItemKindInfo, Instant.m_PetAquiredSkillID)     == 238, "Instant.m_PetAquiredSkillID @ +238");
static_assert(offsetof(stItemKindInfo, Instant.m_dwPetStorageExpansion) == 248, "Instant.m_dwPetStorageExpansion @ +248");
static_assert(offsetof(stItemKindInfo, Instant.m_dwPetRenameTicket)     == 252, "Instant.m_dwPetRenameTicket @ +252");
static_assert(offsetof(stItemKindInfo, Instant.m_wPetDyeID)             == 256, "Instant.m_wPetDyeID @ +256");
static_assert(offsetof(stItemKindInfo, Instant.m_dwAddAttack)           == 260, "Instant.m_dwAddAttack @ +260");
static_assert(offsetof(stItemKindInfo, Instant.m_wMyItemKind)           == 272, "Instant.m_wMyItemKind @ +272");
static_assert(offsetof(stItemKindInfo, Instant.m_dwChangeCharNameId)    == 284, "Instant.m_dwChangeCharNameId @ +284");
static_assert(offsetof(stItemKindInfo, Instant.m_bIsSealItem)           == 290, "Instant.m_bIsSealItem @ +290");
static_assert(offsetof(stItemKindInfo, Equip.m_dwEquipAtb)              ==  80, "Equip.m_dwEquipAtb @ +80");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_wWeaponType)        ==  84, "Hunt.m_wWeaponType @ +84");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_szEquipableClass)   == 108, "Hunt.m_szEquipableClass @ +108");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_byLevel)            == 120, "Hunt.m_byLevel @ +120");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_dwAddAtt)           == 132, "Hunt.m_dwAddAtt @ +132");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_dwEnchantAttribute) == 180, "Hunt.m_dwEnchantAttribute @ +180");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_szSoundName)        == 189, "Hunt.m_szSoundName @ +189");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_wSearchRangeX)      == 206, "Hunt.m_wSearchRangeX @ +206");
static_assert(offsetof(stItemKindInfo, Equip.Hunt.m_dwAttackAtb)        == 220, "Hunt.m_dwAttackAtb @ +220");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_byGender)        ==  84, "Fashion.m_byGender @ +84");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_dwUsePeriod)     ==  88, "Fashion.m_dwUsePeriod @ +88");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_dwCloakEffect)   == 104, "Fashion.m_dwCloakEffect @ +104");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_wCapeEffectVisual)==114,"Fashion.m_wCapeEffectVisual @ +114");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_dwDamagePercent) == 116, "Fashion.m_dwDamagePercent @ +116");
static_assert(offsetof(stItemKindInfo, Equip.Fashion.m_wCarKindInfo)    == 154, "Fashion.m_wCarKindInfo @ +154");
static_assert(offsetof(stItemKindInfo, Training.m_byMultiplier)         ==  80, "Training.m_byMultiplier @ +80");
static_assert(offsetof(stItemKindInfo, Training.m_Skill)                ==  84, "Training.m_Skill @ +84");
static_assert(sizeof(((stItemKindInfo*)0)->Training.m_Skill)            ==  64, "Training.m_Skill total = 64 bytes");
static_assert(offsetof(stItemKindInfo, Quiz.m_wHangleID)                ==  80, "Quiz.m_wHangleID @ +80");
static_assert(offsetof(stItemKindInfo, Quiz.m_bySyllableType)           ==  82, "Quiz.m_bySyllableType @ +82");


class cltItemKindInfo {
private:
    stItemKindInfo* m_pItemKindInfo[65536];         // Array of pointers to item data.
    unsigned short* m_pItemKindForMakingItemList;   // Array of item Kinds that are used for making.
    unsigned short m_wMakingItemKindListNum;        // Number of items in the making list.
    unsigned short m_pHangleID2ItemKind[100];       // Maps Hangle index to item Kind. (Offset 131075)
    unsigned short m_pEnglishID2ItemKind[100];      // Maps English index to item Kind.(Offset 131175)

    // Static pointers to other managers, set by InitializeStaticVariable.
    static cltClassKindInfo* m_pclClassKindInfo;
    static cltPetKindInfo* m_pclPetKindInfo;

public:
    cltItemKindInfo();
    ~cltItemKindInfo();

    // 韓文: 제작 분류 → unsigned int 마스크
    // 中文: 將 "POTION" / "SCROLL" / ... 等字串轉換為製作分類遮罩
    // 對應 mofclient.c:306737  cltItemKindInfo::GetMakingCategory
    // (cltSpecialtyKindInfo::SetMakingItemCategory 在解析 specialty.txt 時呼叫)
    static int GetMakingCategory(const char* str);

    // Initializes the manager by loading all item data files.
    int Initialize(const char* itemkindinfo, const char* item_instant, const char* item_hunt,
        const char* item_fashion, const char* koreatext, const char* trainningcardinfo);

    // Frees all allocated memory for item data.
    void Free();

    // --- Getters ---
    stItemKindInfo* GetItemKindInfo(unsigned short a2);
    stItemKindInfo* GetItemKindInfoByIndex(unsigned int index);
    unsigned short GetHangleID2ItemKind(unsigned short a2);
    unsigned short GetEnglishID2ItemKind(unsigned short a2);
    unsigned char GetMaxPileUpNum(unsigned short a2);
    unsigned char GetSpecialUseItem(unsigned short a2);
    unsigned short GetQuizItemHangleID(unsigned short a2);
    unsigned char IsQuizItemConsonant(unsigned short a2);
    int GetLessonResourceByIndex(unsigned short item_kind, int skill_index, unsigned int* res_id, unsigned short* block_num);
    int GetLessonResourceByKind(unsigned short item_kind, unsigned char skill_kind, unsigned int* res_id, unsigned short* block_num);
    unsigned short GetReqSpecialtyKindByMakingItemKind(unsigned short making_item_kind);
    bool GetReqClassKindsForEquip(unsigned short item_kind, int* num_classes, unsigned short* out_class_kinds);
    const char* UseSound(unsigned short a2);

    // --- Boolean Checks ---
    bool IsValidItem(unsigned short a2);
    bool IsCoinItem(unsigned short a2);
    bool IsChangeCoin(unsigned short a2);
    bool IsUseItem(unsigned short a2);
    bool IsEquipItem(unsigned short a2);
    bool IsQuickSlotItem(unsigned short a2);
    bool IsDropItem(unsigned short a2);
    bool IsSellItem(unsigned short a2);
    bool IsTradeItem(unsigned short a2);
    bool IsCashItem(unsigned short a2);
    bool IsSellingAgencyItem(unsigned short a2);
    bool IsTimerItem(unsigned short a2);
    bool IsStorageItem(unsigned short a2);
    bool IsExStorageItem(unsigned short a2);
    bool IsQuizItem(unsigned short a2);
    bool IsPetInventoryItem(unsigned short a2);
    bool IsTraningCard(unsigned short a2);
    bool IsExStorageBagItem(unsigned short a2);
    bool IsReturnOrderSheet(unsigned short a2);
    bool IsTeleportOrderSheet(unsigned short a2);
    bool IsTownPortalOrderSheet(unsigned short a2);
    bool IsPostItItem(unsigned short a2);
    bool IsRareItem(unsigned short a2);
    bool IsCreatePetItem(unsigned short a2);
    bool IsExpandCircleMembersItem(unsigned short a2);
    bool IsCoupleRingItem(unsigned short a2);
    bool IsPCBangInstant(unsigned short a2);
    bool IsChangeCharName(unsigned short a2);
    bool IsPetCanUseItem(unsigned short pet_kind, unsigned short item_kind);
    bool IsUseChangeServer(unsigned short item_kind, unsigned short server_id);
    bool IsSealItem(unsigned short a2);
    bool CanSealItem(unsigned short a2);
    bool IsTwoHandWeaponByItemKind(unsigned short a2);
    bool IsFashionItem(unsigned short a2);
    bool IsEnchantMaterialItem(unsigned short a2);
    bool IsFullSetItem(unsigned short a2);
    bool IsSpecialUseItem(unsigned short a2);
    bool IsMultiTargetWeapon(unsigned short a2);
    bool IsQuickSlotRelinkableItem(unsigned short a2);

    // --- Utility ---
    void ExtractItemCodeToFile(char* FileName);

    // "A0000" 形式 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(const char* a1);

    // 16-bit 代碼 → "A0000" 形式（使用靜態緩衝區；非 thread-safe）
    static char* TranslateKindCode(uint16_t a1);

    // Setters for static dependency pointers.
    static void InitializeStaticVariable(cltClassKindInfo* a1, cltPetKindInfo* a2);

    static unsigned int GetEquipAtb(stItemKindInfo* pInfo);
private:
    // 物品類別屬性位元遮罩 (從反編譯中還原)
    // 每個 EItemClass 對應一個 unsigned int，用位元來儲存其屬性
    static unsigned int m_dwItemTypeAtb[25];

    // Internal file loading functions.
    bool LoadItemList(const char* filename);
    bool LoadInstantItem(const char* filename);
    bool LoadHuntItem(const char* filename);
    bool LoadFashionItem(const char* filename);
    bool LoadQuizItem(const char* filename);
    bool LoadTrainningCardItem(const char* filename);
    bool LoadHangleInfo(const char* filename);

    // Helper function to build the list of craftable items.
    void CreateItemKindForMakingItemList();

    // Static helper functions for parsing string values into enums/attributes.
    static EEquipAtb GetEquipAtb(const char* str);
    static EInstantEffectType GetInstantEffectType(const char* str);
    static bool GetAttackType(const char* str);
    int GetEquipableClassAtb(char* str);

    // --- Static Boolean Check Helpers (based on item class) ---
    static bool IsUseItem(EItemClass itemClass);
    static bool IsEquipItem(EItemClass itemClass);
    static bool IsQuickSlotItem(EItemClass itemClass);
    static bool IsDropItem(EItemClass itemClass);
    static bool IsSellItem(EItemClass itemClass);
    static bool IsTradeItem(EItemClass itemClass);
    static bool IsCashItem(EItemClass itemClass);
    static bool IsSellingAgencyItem(EItemClass itemClass);
    static bool IsTimerItem(EItemClass itemClass);
    static bool IsStorageItem(EItemClass itemClass);
    static bool IsExStorageItem(EItemClass itemClass);
    static bool IsQuizItem(EItemClass itemClass);
    static bool IsPetInventoryItem(EItemClass itemClass);
    static bool IsTraningCard(EItemClass itemClass);
    static bool IsTwoHandWeaponByItemClassType(unsigned short weaponType);
    static bool IsPetCanUseItem(unsigned short pet_kind, stItemKindInfo* pInfo);
};