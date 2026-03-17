#pragma once

#include <cstdint>
#include <cstring>

// Forward declarations
class cltBaseInventory;
class cltMoneySystem;
class cltPlayerAbility;
class cltLevelSystem;
class cltLessonSystem;
class cltSkillSystem;
class cltClassSystem;
class cltEmblemSystem;
class cltTitleSystem;
class cltPetInventorySystem;
class cltTASSystem;
class cltSpecialtySystem;
class CMeritoriousSystem;
class cltPKRankSystem;
class cltGradeSystem;
class cltMyItemSystem;
class cltQuestKindInfo;
class cltNPCInfo;
class cltLessonKindInfo;
class cltCharKindInfo;
class cltClassKindInfo;
class cltInstantDungeonKindInfo;
class cltItemList;

// 任務進行中資訊 — 20 bytes，與反編譯記憶體佈局一致
#pragma pack(push, 1)
struct stPlayingQuestInfo {
    std::uint16_t wNPCID;       // offset 0   — 任務所屬 NPC ID
    std::uint16_t wQuestID;     // offset 2   — 任務 ID
    std::uint8_t  bStatus;      // offset 4   — 0=進行中, 1=已完成, 2=已放棄
    std::uint8_t  _padding[3];  // offset 5~7
    std::uint32_t dwValue;      // offset 8   — 進度值 (獵殺數、配送狀態等)
    std::uint32_t dwStartTime;  // offset 12  — 接取時間 (秒)
    std::uint32_t dwTimeLimit;  // offset 16  — 時間限制 (秒)
};
#pragma pack(pop)

static_assert(sizeof(stPlayingQuestInfo) == 20, "stPlayingQuestInfo must be 20 bytes");

class cltQuestSystem {
public:
    static constexpr int MAX_QUEST_SLOTS = 100;
    static constexpr int MAX_RUNNING_QUESTS = 15;

    cltQuestSystem();
    virtual ~cltQuestSystem();

    void Free();

    // 注意：原始命名為 Initailize（拼字錯誤），保持相容
    void Initailize(
        cltBaseInventory* pInventory,
        cltMoneySystem* pMoneySystem,
        cltPlayerAbility* pPlayerAbility,
        cltLevelSystem* pLevelSystem,
        cltLessonSystem* pLessonSystem,
        cltSkillSystem* pSkillSystem,
        cltClassSystem* pClassSystem,
        cltEmblemSystem* pEmblemSystem,
        cltTitleSystem* pTitleSystem,
        cltPetInventorySystem* pPetInventorySystem,
        cltTASSystem* pTASSystem,
        cltSpecialtySystem* pSpecialtySystem,
        CMeritoriousSystem* pMeritoriousSystem,
        cltPKRankSystem* pPKRankSystem,
        cltGradeSystem* pGradeSystem,
        cltMyItemSystem* pMyItemSystem,
        const stPlayingQuestInfo* questData,
        int giveupPermanently,
        unsigned int accountID
    );

    static void InitializeStaticVariable(
        cltQuestKindInfo* pQuestKindInfo,
        cltNPCInfo* pNPCInfo,
        cltLessonKindInfo* pLessonKindInfo,
        cltCharKindInfo* pCharKindInfo,
        cltClassKindInfo* pClassKindInfo,
        cltInstantDungeonKindInfo* pInstantDungeonKindInfo,
        int (*pExternIsJoinedCircleFunc)(unsigned int),
        void (*pQuestMarkRefreshFunc)(std::uint16_t)
    );

    // --- 任務欄位管理 ---
    std::uint8_t GetPlayingQuestCount();
    std::uint8_t GetRunningQuestCount();
    std::uint8_t GetQuestIndexByQuestID(std::uint16_t questID);

    // --- 任務資訊存取 ---
    stPlayingQuestInfo* GetPlayingQuestInfoByNPCID(std::uint16_t npcKind);
    stPlayingQuestInfo* GetPlayingQuestInfoByQuestID(std::uint16_t questKind);
    stPlayingQuestInfo* GetPlayingQuestInfoByIndex(std::uint8_t index);
    stPlayingQuestInfo* GetRunningQuestInfoByIndex(std::uint8_t index);
    stPlayingQuestInfo* GetRunningQuestInfoByQuestID(std::uint16_t questID);
    stPlayingQuestInfo* GetPlayingQuestInfoByIndunID(std::uint16_t indunID);

    // --- 任務完成 / 結束 ---
    void FinishQuest(std::uint16_t index);
    void FinishQuest(std::uint16_t index, std::uint16_t questID, std::uint16_t npcID);

    // --- 新增任務 ---
    void AddQuest(cltItemList* itemList, std::uint8_t* outSlots);
    void AddQuest();

    // --- 獵殺任務更新 ---
    std::uint16_t UpdateHuntQuest(std::uint16_t monsterKind);
    std::uint16_t UpdateHuntQuest(std::uint16_t monsterKind, std::uint16_t* outNPCID, unsigned int* outValue);

    // --- 獎勵判定 ---
    std::uint16_t CanReward(int npcID);
    std::uint16_t CanReward(int npcID, std::uint16_t* outDeliveryNPC);
    void RewardQuest(std::uint16_t npcID, unsigned int* outExp);
    void RewardQuest(
        std::uint16_t npcID,
        std::uint16_t* outDelNPC,
        int* outMoney,
        unsigned int* outExp,
        unsigned int* outSword,
        unsigned int* outBow,
        unsigned int* outMagic,
        unsigned int* outTheology,
        std::int16_t* outMajor,
        std::uint16_t* outMerit,
        int* outRank,
        cltItemList* delItemList,
        cltItemList* addItemList,
        std::uint8_t* outDelSlots,
        std::uint8_t* outAddSlots
    );
    std::uint16_t GetRewardItem(std::uint16_t npcID, std::uint8_t groupIndex);
    std::uint8_t GetRewardItemNum(std::uint16_t npcID, std::uint8_t groupIndex);
    std::uint16_t CanAddRewarItem(std::uint16_t npcID);
    bool CanIncreaseRewardMoney(std::uint16_t npcID);

    // --- 接取判定 ---
    std::uint16_t CanAccept(std::uint16_t npcID);
    std::uint16_t GetNewQuestID(std::uint16_t npcID);
    int CanShowQuestAcceptMark(std::uint16_t npcID);
    int GetPrecedenceQuestInfo(std::uint16_t npcID, std::uint16_t* outPrereqNPC, std::uint16_t* outPrereqQuest);
    char GetNPCIDByLevel(std::uint8_t level, std::uint16_t* outNPCIDs, std::uint16_t* outQuestIDs);

    // --- 配送 NPC 遭遇 ---
    std::uint16_t OnMeetNPC(std::uint16_t npcID);
    std::uint16_t OnMeetNPC(std::uint16_t npcID, std::uint16_t* outNPCID);
    void DelDeliveryItem();
    void DelDeliveryItem(cltItemList* itemList, std::uint8_t* outSlots);

    // --- 放棄任務 ---
    unsigned int CanGiveupQuest(std::uint16_t npcID);
    void GiveupQuest(std::uint16_t npcID);
    void GiveupQuest(std::uint16_t npcID, cltItemList* itemList, std::uint8_t* outSlots);
    stPlayingQuestInfo* CanEternalGiveupQuest(std::uint16_t npcID);
    int EternalGiveUpQuest(std::uint16_t npcID);
    int EternalGiveUpQuest(std::uint16_t npcID, cltItemList* itemList, std::uint8_t* outSlots);

    // --- 時間 ---
    unsigned int ProcRemainTime(std::uint16_t questID);

    // --- 副本 ---
    int CompleteInstansDungeonMission(std::uint16_t indunID);

    // --- 狀態查詢 ---
    int IsQuestMonster(std::uint16_t monsterKind);
    int IsCompleteQuest(int questID);
    int IsGiveupQuestPermanently();
    int CompleteFunctionQuest(std::int16_t functionType);
    int CompleteFunctionQuest(char functionType, std::uint16_t* outNPCID, unsigned int* outValue);

private:
    // ---- 實例資料（記憶體佈局與反編譯一致）----
    stPlayingQuestInfo m_quests[MAX_QUEST_SLOTS];        // offset 4,  size 2000
    std::uint8_t  m_bPlayingQuestCount;                  // offset 2004
    std::uint8_t  m_runningQuestIndices[MAX_RUNNING_QUESTS]; // offset 2005~2019

    // 系統指標
    cltBaseInventory*      m_pInventory;                 // offset 2020
    cltMoneySystem*        m_pMoneySystem;               // offset 2024
    cltPlayerAbility*      m_pPlayerAbility;             // offset 2028
    cltLevelSystem*        m_pLevelSystem;               // offset 2032
    cltLessonSystem*       m_pLessonSystem;              // offset 2036
    cltSkillSystem*        m_pSkillSystem;               // offset 2040
    cltClassSystem*        m_pClassSystem;               // offset 2044
    cltEmblemSystem*       m_pEmblemSystem;              // offset 2048
    cltTitleSystem*        m_pTitleSystem;               // offset 2052
    cltPetInventorySystem* m_pPetInventorySystem;        // offset 2056
    cltTASSystem*          m_pTASSystem;                 // offset 2060
    cltSpecialtySystem*    m_pSpecialtySystem;           // offset 2064
    CMeritoriousSystem*    m_pMeritoriousSystem;         // offset 2068
    cltPKRankSystem*       m_pPKRankSystem;              // offset 2072
    cltGradeSystem*        m_pGradeSystem;               // offset 2076
    cltMyItemSystem*       m_pMyItemSystem;              // offset 2080

    // 暫存欄位
    std::uint16_t m_wPendingNPCID;                       // offset 2084
    std::uint16_t m_wPendingQuestID;                     // offset 2086
    std::uint16_t m_wDeliveryItemKind;                   // offset 2088
    std::uint16_t _padding1;                             // offset 2090
    stPlayingQuestInfo* m_pDeliveryQuestInfo;             // offset 2092
    std::uint16_t m_wRewardNPCID;                        // offset 2096
    std::uint16_t m_wRewardItemKind;                     // offset 2098
    stPlayingQuestInfo* m_pRewardQuestInfo;               // offset 2100
    int           m_bGiveupPermanently;                  // offset 2104
    unsigned int  m_dwAccountID;                         // offset 2108

    // ---- 靜態成員 ----
    static cltQuestKindInfo*           m_pclQuestKindInfo;
    static cltNPCInfo*                 m_pclNPCInfo;
    static cltLessonKindInfo*          m_pclLessonKindInfo;
    static cltCharKindInfo*            m_pclCharKindInfo;
    static cltClassKindInfo*           m_pclClassKindInfo;
    static cltInstantDungeonKindInfo*  m_pclInstantDungeonKindInfo;
    static int  (*m_pExternIsJoinedCircleFuncPtr)(unsigned int);
    static void (*m_pQuestMarkRefreshFuncPtr)(std::uint16_t);
};

extern cltQuestSystem g_clQuestSystem;
