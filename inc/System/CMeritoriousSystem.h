#pragma once

#include <array>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "System/cltMoneySystem.h"
#include "Info/cltCharKindInfo.h"
#include "System/cltQuestSystem.h"
#include "System/cltEmblemSystem.h"
#include "System/cltSpecialtySystem.h"

class CExpRewardParser;
class CMeritoriousGradeParser;
class CMeritoriousRewardParser;
class CSupplyMeritoriousParser;

struct stMonsterKind;

class CMeritoriousSystem {
public:
    static void InitializeStaticVariable(cltCharKindInfo*, CSupplyMeritoriousParser*, CMeritoriousRewardParser*,
                                         CMeritoriousGradeParser*, CExpRewardParser*);

    CMeritoriousSystem();
    ~CMeritoriousSystem();

    void Free();
    int Initialize(cltQuestSystem* questSystem, cltBaseInventory* inventory, cltSpecialtySystem* specialty,
                   cltMoneySystem* moneySystem, cltEmblemSystem* emblemSystem,
                   unsigned int point, unsigned int totalPoint,
                   std::uint16_t grade, std::uint16_t gradePoint,
                   bool warQuestPlaying, char supplyQuestPlaying,
                   std::uint16_t warQuestMonCount,
                   const std::uint16_t* warQuestMonKinds,
                   const std::uint16_t* warQuestMonGoals);

    unsigned int StartWarMeritoriousQuest(std::uint16_t mapKind, int difficulty, std::uint16_t* outQuestKinds);
    unsigned int CanStartWarMeritoriousQuest(int minLv, int maxLv, std::uint16_t needClass, std::uint16_t needNation,
                                             std::uint16_t monsterKind, stMonsterKind* outMonsterInfo);
    int CanStartWarMeritoriousQuest();
    void PlayWarMeritoriousQuest(int playing);
    unsigned int CompleteWarMeritoriousQuest(unsigned int seed, std::int64_t* outExp, int* outMoney,
                                             std::uint16_t* outQuestKinds, unsigned int* outQuestValues);
    unsigned int GetRewardWarMeritoriousExp();
    unsigned int GetRewardWarMeritoriousPoint();
    unsigned int GetRewardSupplyMeritoriousLibi();
    unsigned int GetRewardSupplyMeritoriousPoint();
    unsigned int GetSupplyMeritoriousItemCount();

    void SetMeritoriousUpGrade(std::uint16_t grade, std::uint16_t gradePoint);
    unsigned int CanCompleteWarMeritoriousQuest();
    void InitCompleteWarMeritoriousQuest();
    std::uint16_t GetWarMeritoriousQuestMonsterSize();
    std::uint16_t GetWarMeritoriousMonsterKillCount(std::uint16_t kind);
    std::uint16_t GetWarMeritoriousMonsterGoalKillCount(std::uint16_t kind);
    int GetWarMeritoriousMonsterKind(std::uint16_t* outSize, std::uint16_t* outKinds);

    void IncreaseMeritoriousPoint(std::uint16_t point);
    void DecreaseMeritoriousPoint(std::uint16_t point);
    int IsExistWarQuestMonsterKind(std::uint16_t kind);

    int CanGiveUpWarMeritorious();
    void GiveUpWarMeritorious();
    int CanGiveUpSupplyMeritorious();
    void GiveUpSupplyMeritorious();

    unsigned int StartSupplyMeritoriousQuest(std::uint16_t supplyKind);
    int CanStartSupplyMeritoriousQuest();
    void CompleteSupplyMeritoriousQuest(int count, cltItemList* itemList, std::uint8_t* outChangedSlots,
                                        int* outMoney, std::uint16_t* outQuestKinds, unsigned int* outQuestValues);
    unsigned int CanCompleteSupplyMeritoriousQuest(int count);
    void InitCompleteSupplyMeritoriousQuest();

    int RewardMeritoriousItem(std::uint16_t itemKind, unsigned int qty);
    int RewardMeritoriousItem(std::uint16_t itemKind, unsigned int* outPos, std::uint8_t* outChangedSlots);
    int CanRewardMeritoriousItem(std::uint16_t itemKind);

    void SetMeritoriousInfo(unsigned int point, unsigned int totalPoint,
                            std::uint16_t grade, std::uint16_t gradePoint,
                            bool warQuestPlaying, char supplyQuestPlaying,
                            std::uint16_t warQuestMonCount,
                            const std::uint16_t* warQuestMonKinds,
                            const std::uint16_t* warQuestMonGoals);
    void SetWarMeritoriousQuest(int playing, int difficulty);
    int CalcMeritoriousGrade(std::uint16_t* outGrade, std::uint16_t* outGradePoint);

private:
    static cltCharKindInfo* m_pclCharKindInfo;
    static CExpRewardParser* m_pclExpRewardParser;
    static CMeritoriousGradeParser* m_pclMeritoriousGradeParser;
    static CMeritoriousRewardParser* m_pclMeritoriousRewardParser;
    static CSupplyMeritoriousParser* m_pclSupplyMeritoriousParser;

    cltQuestSystem* questSystem_{};
    cltBaseInventory* inventory_{};
    cltSpecialtySystem* specialtySystem_{};
    cltMoneySystem* moneySystem_{};
    cltEmblemSystem* emblemSystem_{};

    unsigned int point_{};
    unsigned int totalPoint_{};
    std::uint16_t grade_{};
    std::uint16_t gradePoint_{};

    bool warQuestPlaying_{};
    char supplyQuestPlaying_{};
    int warQuestDifficulty_{};
    std::uint16_t supplyQuestKind_{};

    std::array<std::uint16_t, 16> warQuestMonKinds_{};
    std::array<std::uint16_t, 16> warQuestMonGoals_{};
    std::array<std::uint16_t, 16> warQuestMonKills_{};
    std::uint16_t warQuestMonCount_{};
};
