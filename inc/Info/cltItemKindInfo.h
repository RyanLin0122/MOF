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

#pragma pack(push, 1)
// Main structure for item data, size 0x128 (296 bytes).
struct stItemKindInfo {
    // Part 1: Loaded from itemkindinfo.txt
    unsigned short m_wKind;              // 0 物品 ID
    unsigned short m_wTextCode;          // 2 文字代碼
    unsigned short m_wDescCode;          // 4 描述代碼
    char _padding_6[2];
    unsigned int m_dwPrice;              // 8 物品價格
    unsigned int m_dwPVPPoint;           // 12 PVP 購買點數
    unsigned int m_dwPrice1;             // 16 金幣價格
    unsigned int m_dwPrice2;             // 20 銀幣價格
    unsigned int m_dwPrice3;             // 24 銅幣價格
    unsigned int m_dwIconResID;          // 28 物品圖像檔案 ID
    unsigned short m_wIconBlockID;       // 32 物品圖像方塊 ID
    unsigned char m_byItemClass;         // 34 物品類別
    unsigned char m_byMaxPileUpNum;      // 35 最大堆疊數量
    char m_szUseSound[16];               // 36 聲音代碼
    unsigned short m_wItemType;          // 52 物品類型
    unsigned char m_byQuestCollect;      // 54 任務物品
    char _padding_55[1];
    unsigned int m_dwMakingCategory;     // 56 製造類別
    unsigned short m_wUseTerm;           // 60 使用期限
    char _padding_62[2];
    unsigned int m_dwWeight;             // 64 排序值
    int m_bHomepageView;                 // 68 首頁視圖
    int m_bCanSealItem;                  // 72 已密封
    char _padding_76[4];
    // Part 2: Overlapping data for different item types.
    union {
        // --- For INSTANT items --- (from item_instant.txt)
        struct {
            unsigned char m_byInstantEffectType;  // 80 效果週期
            char _padding_81[1];
            unsigned short m_wAddHP;              // 82 生命值增加
            unsigned short m_wAddMP;              // 84 魔法值增加
            unsigned short m_wAddHP_Percent;      // 86 生命值增加百分比
            unsigned short m_wAddMP_Percent;      // 88 魔法值增加百分比
            char _padding_90[2];
            unsigned int m_dwSustainTime;         // 92 持續時間
            char _padding_96[4];
            unsigned short m_wAddStr;             // 100 力量增加
            unsigned short m_wAddDex;             // 102 敏捷增加
            unsigned short m_wAddInt;             // 104 智力增加
            unsigned short m_wAddSta;             // 106 體力增加
            unsigned short m_wAddMaxHP_Percent;   // 108 最大生命值增加 (%)
            unsigned short m_wAddMaxMP_Percent;   // 110 最大魔法值增加 (%)
            unsigned short m_wAddAccuracy;        // 112 命中增加
            unsigned short m_wAddEvasion;         // 114 閃避增加
            unsigned short m_wAddCritical;        // 116 暴擊幾率增加    

            unsigned short m_wAddHit_Beast;       // 118 野獸類型攻擊力加成 (%)
            unsigned short m_wAddHit_Monster;     // 120 怪物類型攻擊力加成 (%)
            unsigned short m_wAddHit_Undead;      // 122 亡靈類型攻擊力加成 (%)
            unsigned short m_wAddHit_Demon;       // 124 惡魔類型攻擊力加成 (%)
            unsigned short m_wAddDef_Beast;       // 126 野獸型防禦力加成 (%)
            unsigned short m_wAddDef_Monster;     // 128 怪物類型防禦力加成 (%)
            unsigned short m_wAddDef_Undead;      // 130 亡靈類型防禦力加成 (%)
            unsigned short m_wAddDef_Demon;       // 132 惡魔型防禦力加成 (%)

            unsigned short m_wAddMagicResist;     // 134 魔法抗性
            unsigned char m_wElementGive;         // 136 屬性賦予
            unsigned char m_wItemType;            // 137 物品類型
            unsigned short m_RequiredSpecialty;   // 138 所需專精
            unsigned int m_AddedCraftItemCode;    // 140 新增製作物品代碼

            int m_CharRevival;                    // 144 角色復活
            int m_StatInitialize;                 // 148 能力值重置 
            int m_SkillSpecInitialize;            // 152 技能專精重置 
            int m_SkillCircleInitialize;          // 156 技能循環重置 
            int m_JobChange;                      // 160 職業變更
            int m_Sprint;                         // 164 奔跑
            int m_GenderChange;                   // 168 性別變更
            char m_bHairGender;                   // 172 髮型 - 性別
            char _padding_173[3];
            int m_HairStyleID;                    // 176 髮型
            char m_HairStyleValue;                // 180 髮型 - 數值
            char m_FaceGender;                    // 181 臉部修飾性別
            char _padding_182[2];
            int m_FaceStyleID;                    // 184 整形/美容 
            char m_FaceStyleValue;                // 188 臉部修飾 - 數值
            char _padding_189[3];
            int m_HairColorCode;                  // 192 頭髮染色色碼 

            int m_dwPandora;                      // 196 潘朵拉
            int m_dwShout;                        // 200 呼喊
            int m_bMessage;                       // 204 訊息
            unsigned short m_wAcquireSkillID;     // 208 獲得的技能ID

            unsigned short m_wWeather;            // 210 氣象/天氣
            unsigned short m_wUsablePetKind[10];  // 212 可使用的寵物種類 ID 陣列 (20 bytes)
            int m_nUsablePetCount;                // 232 陣列中有效寵物的數量 (4 bytes)
            unsigned short m_PetBirth;            // 236 寵物出生
            unsigned short m_PetAquiredSkillID;   // 238 寵物獲取技能
            unsigned int m_PetFullnessPerItem;    // 240 寵物飽和度提升（每件）
            unsigned int m_PetAffectionIncrease;// 244 寵物好感度提升（查看備註）
            unsigned short m_wPetStorageExpansion;// 248 寵物儲存空間擴充
            char _padding_238[2];
            unsigned int m_dwPetRenameTicket;     // 252 更改寵物名稱
            unsigned short m_wPetDyeID;           // 256 寵物染色ID

            unsigned int m_dwAddAttack;           // 260 攻擊力提升
            unsigned int m_dwAddDefense;          // 264 防禦力提升
            unsigned int m_dwStatusCure;          // 268 狀態回覆

            unsigned short m_wMyItemKind;         // 272 我的物品種類
            char _padding_262[2];
            unsigned int m_dwMyItemPeriod;        // 276 我的物品週期
            unsigned short m_wCoupleRingId;       // 280 婚戒ID
            unsigned short m_wEffectId;           // 282 效果ID
            unsigned int m_dwChangeCharNameId;    // 284 更名券ID
            unsigned char m_bChangeCoinType;      // 288 錢幣分類
            unsigned char m_bUseChangeServer;     // 289 伺服器轉移券
            unsigned char m_bIsSealItem;          // 290 封印物品 
            char _padding_291[6];
        } Instant;

        // --- For HUNT and FASHION items --- (from item_hunt.txt / item_fashion.txt)
        struct {
            unsigned int m_dwEquipAtb;            // 80 裝備位置
            // Hunt / Fashion Overlap
            union {
                // Hunt
                struct {
                    unsigned short m_wWeaponType;       // 84 武器類型
                    unsigned short m_wMinAttack;        // 86 最小攻擊力
                    unsigned short m_wMaxAttack;        // 88 最大攻擊力
                    unsigned char m_byAttackSpeed;      // 90 攻擊速度
                    char _padding_91[1];
                    int m_dwAttackType;                 // 92 攻擊方式
                    unsigned short m_wRange;            // 96 投射物
                    unsigned short m_wDef;              // 98 防禦力
                    unsigned short m_wNeedStr;          // 100 力量要求
                    unsigned short m_wNeedSta;          // 102 體力要求
                    unsigned short m_wNeedDex;          // 104 敏捷要求
                    unsigned short m_wNeedInt;          // 106 智力要求
                    char m_szEquipableClass[12];        // ??? 職業要求
                    unsigned char m_byLevel;            // 120 等級要求
                    char _padding_121[1];
                    unsigned short m_wAddStr;           // 122 攻擊力提升
                    unsigned short m_wAddSta;           // 124 體力提升
                    unsigned short m_wAddDex;           // 126 敏捷提升
                    unsigned short m_wAddInt;           // 128 智力提升
                    char _padding_130[2];
                    int m_dwAddAtt;                     // 132 普通攻擊力提升 (100%)
                    unsigned short m_wAddSkillAtt;      // 136 技能攻擊力提升 (100%)
                    char _padding_138[2];
                    int m_dwAddDefence;                 // 140 防禦力提升 (100%)
                    unsigned short m_wAddMaxHP_Percent; // 144 生命值提升 (100%)
                    unsigned short m_wAddMaxMP_Percent; // 146 魔法值提升 (100%)
                    unsigned short m_wAddHPRegen;       // 148 生命值恢復提升
                    unsigned short m_wAddMPRegen;       // 150 魔法值恢復提升
                    unsigned short m_wAddAccuracy;      // 152 命中率增加
                    unsigned short m_wAddEvasion;       // 154 閃避增加
                    unsigned short m_wAddCritical;      // 156 暴擊幾率增加

                    unsigned short m_wAddHit_Beast;     // 158 野獸類型額外攻擊力 (100%)
                    unsigned short m_wAddHit_Monster;   // 160 普通怪物額外攻擊力 (100%)
                    unsigned short m_wAddHit_Undead;    // 162 亡靈類型額外攻擊力 (100%)
                    unsigned short m_wAddHit_Demon;     // 164 惡魔類型額外攻擊力 (100%)
                    unsigned short m_wAddDef_Beast;     // 166 野獸型額外防禦力
                    unsigned short m_wAddDef_Monster;   // 168 普通怪物額外防禦力
                    unsigned short m_wAddDef_Undead;    // 170 亡靈類型額外防禦力
                    unsigned short m_wAddDef_Demon;     // 172 惡魔類型額外防禦力

                    unsigned short m_wMagicResist;      // 174 魔法抗性
                    unsigned char m_byAttribute;        // 176 屬性
                    char _padding_177[3];
                    unsigned int m_dwEnchantAttribute;  // 180 附魔屬性

                    unsigned int m_dwAttSpeed;          // 184 攻擊速度
                    unsigned char m_byAniSpeed;         // 188 動畫速度
                    char m_szSoundName[17];             // 189 攻擊音效
                    unsigned short m_wSearchRangeX;     // 206 搜尋範圍 x
                    unsigned short m_wSearchRangeY;     // 208 搜尋範圍 y
                    unsigned short m_wAttackAreaX;      // 210 攻擊範圍 x
                    unsigned short m_wAttackAreaY;      // 212 攻擊範圍 y
                    unsigned char m_byEnchantLevel;     // 214 附魔等級
                    char _padding_215[1];
                    unsigned short m_wItemEffect;       // 216 物品效果
                    unsigned char m_byRareType;         // 218 稀有度分類
                    char _padding_219[1];
                    unsigned int m_dwAttackAtb;         // 220 武器屬性
                    char _padding_224[72];
                } Hunt;

                // Fashion
                struct {
                    unsigned char m_byGender;           // 84 限定性別
                    char _padding_85[3];
                    unsigned int m_dwUsePeriod;         // 88 使用期限
                    unsigned short m_wAddExpPercent;    // 92 經驗值加成(%)
                    char _padding_94[2];
                    unsigned short m_wAddFame;          // 96 名聲增加
                    unsigned short m_wAddHPRegen;       // 98 HP 回復量增加
                    unsigned short m_wAddMPRegen;       // 100 MP 回復量增加
                    char _padding_102[2];
                    unsigned int m_dwCloakEffect;       // 104 披風效果
                    unsigned int m_dwCloakEffectOrder;  // 108 披風效果順序
                    unsigned char m_byCloakEffectCount; // 112 披風效果數量
                    char _padding_113[1];
                    unsigned short m_wCapeEffectVisual; // 114 披風效果特效

                    int m_dwDamagePercent;              // 116 攻擊力提升 - 百分率
                    int m_dwDefPercent;                 // 120 防禦力提升 - 百分率
                    int m_dwAccuracyThousand;           // 124 命中率提升 - 千分率
                    int m_dwEvasionThousand;            // 128 迴避率提升 - 千分率
                    int m_dwAddStr;                     // 132 攻擊增加
                    int m_dwAddDex;                     // 136 敏捷增加
                    int m_dwAddInt;                     // 140 智力增加
                    int m_dwAddSta;                     // 144 體力增加
                    unsigned short m_wFashionHpBarOffset; // 148 時裝與 HP 條顯示間距（像素）
                    unsigned short m_wNameTagKind;      // 150 名牌種類
                    unsigned short m_wChatBallonKind;   // 152 聊天氣泡種類
                    unsigned short m_wCarKindInfo;      // 154 車輛種類
                    char _padding_156[140];
                } Fashion;
            };
        } Equip;
        // --- For TRAINING cards --- (from TrainningCardInfo.txt)
        struct {
            unsigned char m_byMultiplier; // Offset: 80 倍率
            char _padding_81[3];
            struct {
                unsigned int m_dwResID;     // Offset: 84, 92, 100, 108, 116, 124, 132, 140
                unsigned short m_wBlockNum; // Offset: 88, 96, 104, 112, 120, 128, 136, 144
                char _pad[2];
            } m_Skill[8]; // (8 * 8 bytes) 順序: 劍術1, 劍術2, 弓箭1, 弓箭2, 魔法1, 魔法2, 神學1, 神學2
            char _padding_148[148];
        } Training;

        // --- For QUIZ items --- (from koreatext.txt)
        struct {
            unsigned short m_wHangleID;      // 80 인덱스 索引
            unsigned char m_bySyllableType;  // 82 자모구분 字母區分
            char _padding_83[213];
        } Quiz;
    };
};
#pragma pack(pop)
static_assert(sizeof(stItemKindInfo) == 296, "stItemKindInfo size must be 296 bytes"); //273


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

    // Initializes the manager by loading all item data files.
    int Initialize(const char* itemkindinfo, const char* item_instant, const char* item_hunt,
        const char* item_fashion, const char* koreatext, const char* trainningcardinfo);

    // Frees all allocated memory for item data.
    void Free();

    // Retrieves item data by its kind code.
    stItemKindInfo* GetItemKindInfoByIndex(unsigned int index);

    // Converts an item ID string (e.g., "A0001") to a numeric kind code.
    static unsigned short TranslateKindCode(const char* code);

    // "A0000" 形式 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 16-bit 代碼 → "A0000" 形式（使用靜態緩衝區；非 thread-safe）
    static char* TranslateKindCode(uint16_t a1);

    // Setters for static dependency pointers.
    static void InitializeStaticVariable(cltClassKindInfo* a1, cltPetKindInfo* a2);

private:
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
    static int GetMakingCategory(const char* str);
    static EInstantEffectType GetInstantEffectType(const char* str);
    static bool GetAttackType(const char* str);
    int GetEquipableClassAtb(char* str);
};