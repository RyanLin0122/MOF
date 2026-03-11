#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltClassKindInfo;
class cltClassSystem;
class cltEmblemKindInfo;
class cltLevelSystem;
class cltMoneySystem;
class cltQuestSystem;
class cltTitleSystem;
struct strClassKindInfo;
struct strEmblemKindInfo;

class cltEmblemSystem {
public:
    static void InitializeStaticVariable(
        cltEmblemKindInfo* emblemKindInfo,
        cltClassKindInfo* classKindInfo,
        int (*externIsKillMonster)(unsigned int, std::uint16_t, int),
        int (*externIsKillBossMonster)(unsigned int, int),
        void (*externOnAcquiredEmblem)(unsigned int, std::uint16_t));

    cltEmblemSystem();
    ~cltEmblemSystem();

    void Initialize(cltLevelSystem* levelSystem, cltClassSystem* classSystem, cltQuestSystem* questSystem,
                    cltMoneySystem* moneySystem, cltTitleSystem* titleSystem,
                    std::uint16_t emblemNum, const std::uint16_t* emblemKinds,
                    std::uint16_t usingEmblemKind, unsigned int userData0);
    void Free();

    std::uint16_t GetUsingEmblemKind();
    int CanAcquireEmblem(std::uint16_t emblemKind);
    void AcquireEmblem(std::uint16_t emblemKind);
    int IsAcquiredEmblem(std::uint16_t emblemKind);

    int CanSetUsingEmblem(std::uint16_t emblemKind);
    void SetUsingEmblem(std::uint16_t emblemKind);

    int IsKillMonster(char* monsterTypeList, int killCount);

    int GetExpAdvantage();
    int GetAPowerAdvantage(int monsterType);
    int GetDPowerAdvantage();
    int GetMaxHPAdvantage();
    int GetMaxManaAdvantage();
    int GetHitRateAdvantage();
    int GetMissRateAdvantage();
    int GetCriticalHitRateAdvantage();
    int GetDeadPenaltyExpAdvantage();
    int GetItemRecoverHPAdvantage();
    int GetItemRecoverManaAdvantage();
    int GetAutoRecoverHPAdvantage();
    int GetAutoRecoverManaAdvantage();
    int GetShopItemPriceAdvantage();
    int GetSellingAgencyTaxAdvantage();
    int GetTeleportDragonCostAdvantage();
    int GetMeritoriousAdvantage();
    int GetSwordLessonPtAdvantage();
    int GetBowLessonPtAdvantage();
    int GetMagicLessonPtAdvantage();
    int GetTheologyLessonPtAdvantage();
    int GetEnchantAdvantage();
    int GetSkillRangeAdvantage();
    std::uint16_t GetTraningCardItemKind();
    int GetWarMetoriousMonsterKillNumAdvantage();
    int GetSellItemPriceAdvantage();
    int GetCircleQuestRewardPtAdvantage();

    void FillOutEmblemInfo(CMofMsg* msg);
    static void UpdateValidity(cltEmblemSystem* self);
    int IsUsingEmblemValidity();
    unsigned int GetUserData0() const { return m_userData0; }

    int GetGenericEmblem(std::uint16_t* outEmblemKinds);
    strEmblemKindInfo* GetGenericEmblemByID(std::uint16_t emblemKind);
    int GetRareEmblem(std::uint16_t* outEmblemKinds);

    void GetFaintingInfo(int* outRate, int* outDuration);
    void GetConfusionInfo(int* outRate, int* outDuration);
    void GetFreezingInfo(int* outRate, int* outDuration);

    void OnEvent_Dead(unsigned int seed);
    void OnEvent_UsedRecoverHPItem();
    void OnEvent_UsedRecoverManaItem();
    void OnEvent_BeAttackedCritically(unsigned int seed);
    void OnEvent_AttackCritically(unsigned int seed);
    void OnEvent_RegistrySellingAgency(unsigned int seed);
    void OnEvent_BuyItemFromNPC(unsigned int seed);
    void OnEvent_CompleteQuest(std::uint16_t questKind);
    void OnEvent_KillBossMonster();
    void OnEvent_TeleportDragon(unsigned int seed);
    void OnEvent_CompleteMeritous(unsigned int seed);
    void OnEvent_CompleteSwordLesson(unsigned int seed);
    void OnEvent_CompleteBowLesson(unsigned int seed);
    void OnEvent_CompleteMagicLesson(unsigned int seed);
    void OnEvent_CompleteTheologyLesson(unsigned int seed);
    void OnEvent_EnchantItem();
    void OnEvent_CompleteCircleQuest(unsigned int seed);
    void OnEvent_SellItemToNpc(unsigned int seed);
    void OnEvent_MultiAttack(unsigned int seed);
    void OnEvent_ChangeClass();

    static cltEmblemKindInfo* m_pclEmblemKindInfo;
    static cltClassKindInfo* m_pclClassKindInfo;
    static int (*m_pExternIsKillMonsterFuncPtr)(unsigned int, std::uint16_t, int);
    static int (*m_pExternIsKillBossMonsterFuncPtr)(unsigned int, int);
    static void (*m_pExternOnAcquiredEmblemFuncPtr)(unsigned int, std::uint16_t);

private:
    int GetUsingValue(std::uint32_t strEmblemKindInfo::*field);

private:
    unsigned int m_userData0 = 0;
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltQuestSystem* m_pQuestSystem = nullptr;
    cltMoneySystem* m_pMoneySystem = nullptr;
    cltTitleSystem* m_pTitleSystem = nullptr;

    std::array<std::uint16_t, 100> m_acquiredEmblemKinds{};
    std::uint16_t m_acquiredEmblemNum = 0;
    std::uint16_t m_usingEmblemKind = 0;

    strEmblemKindInfo* m_pUsingEmblemInfo = nullptr;
    int m_isUsingEmblemValid = 0;
};
