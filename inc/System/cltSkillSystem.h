#pragma once

#include <cstdint>
#include <vector>

class cltSkillKindInfo;
class cltClassKindInfo;
class cltItemKindInfo;
class cltQuickSlotSystem;
class cltLessonSystem;
class cltLevelSystem;
class cltClassSystem;
class cltEquipmentSystem;
class cltPlayerAbility;
class cltTitleSystem;

class cltSkillSystem {
public:
    static void InitializeStaticVariable(
        cltSkillKindInfo* skillKindInfo,
        cltClassKindInfo* classKindInfo,
        cltItemKindInfo* itemKindInfo,
        void (*onAcquireCircleSkill)(unsigned int));

    void Initialize(
        cltQuickSlotSystem* quickSlotSystem,
        cltLessonSystem* lessonSystem,
        cltLevelSystem* levelSystem,
        cltClassSystem* classSystem,
        cltEquipmentSystem* equipmentSystem,
        cltPlayerAbility* playerAbility,
        cltTitleSystem* titleSystem = nullptr);
    void Initialize(
        cltQuickSlotSystem* quickSlotSystem,
        cltLessonSystem* lessonSystem,
        cltLevelSystem* levelSystem,
        cltClassSystem* classSystem,
        cltEquipmentSystem* equipmentSystem,
        cltPlayerAbility* playerAbility,
        cltTitleSystem* titleSystem,
        std::uint16_t skillNum,
        const std::uint16_t* skillKinds,
        unsigned int userData1);

    void Free();
    std::uint16_t GetSkillNum() const;
    std::uint16_t GetSkillKind(unsigned int index) const;
    int GetClassActiveSkill(std::uint16_t* outSkillKinds, int* outAcquiredFlags) const;
    int GetClassPassiveSkill(std::uint16_t* outSkillKinds, int* outAcquiredFlags) const;
    void AddSkill(std::uint16_t skillKind, int* replaced = nullptr, std::uint16_t* replacedSkillKind = nullptr);
    bool IsSkillCountOver() const;
    int IsExistSkill(std::uint16_t skillKind) const;
    std::uint16_t GetActiveSkill(std::uint16_t* out, int onlyValid) const;
    std::uint16_t GetPassiveSkill(std::uint16_t* out, int onlyValid, int workingOnly) const;
    std::uint16_t GetPassiveSkill_Workingable(std::uint16_t* out) const;
    int IsValidSkill(std::uint16_t skillKind) const;
    int IsAcquiredSkill(std::uint16_t skillKind, std::uint16_t* acquiredKind) const;
    int CanAcquireSkill(std::uint16_t skillKind) const;
    int CanBuySkill(std::uint16_t skillKind) const;
    void BuySkill(std::uint16_t skillKind);
    std::uint16_t GetBuyAbleSkillList(unsigned int skillClass, std::uint16_t* outSkillKinds, int highClassOnly) const;
    bool IsAcquireSkill_Run() const;
    bool IsActiveSkill(std::uint16_t skillKind) const;
    int CanCreateCircle(int* outCircleKind) const;
    unsigned int GetUserData1() const;
    void DeleteSkill(std::uint16_t skillKind);

    std::uint16_t GetMaxHPAdvantage() const;
    std::uint16_t GetMaxMPAdvantage() const;
    std::uint16_t GetMaxHPAdvantage_P() const;
    std::uint16_t GetMaxHPAdvantage_A() const;
    std::uint16_t GetMaxMPAdvantage_P() const;
    std::uint16_t GetMaxMPAdvantage_A() const;
    int GetAPowerAdvantage(int a2) const;
    int GetAPowerAdvantage_P(int a2, int a3, std::uint16_t* a4, int a5) const;
    std::uint16_t GetAPowerAdvantage_A() const;
    std::uint16_t GetDPowerAdvantage() const;
    std::uint16_t GetDPowerAdvantage_P(int a2) const;
    int GetHitRateAdvantage() const;
    int GetCriticalRateAdvantage() const;
    int GetSkillAPowerAdvantage() const;
    int GetMissRateAdvantage() const;
    int GetPartyAPowerAdvantage(void* party) const;
    int GetPartyDPowerAdvantage(void* party) const;
    int GetPartyHitRateAdvantage(void* party) const;
    int GetAutoRecoverHPAdvantage() const;
    int GetAutoRecoverManaAdvantage() const;
    int GetSkillManaAdvantage() const;
    int GetAddBuffNum() const;
    bool GetMaxFaintingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    bool GetMaxConfusionInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    bool GetMaxFreezingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const;
    std::uint16_t GetGeneralPartyAdvantageSkillKind() const;
    int MT_GetDPowerAdvantage() const;
    int MT_GetAPowerAdvantage() const;
    int MT_GetHitRateAdvantage() const;
    int MT_GetMissRateAdvantage() const;
    int MT_GetCriticalHitRateAdvantage() const;

    void UpdateValidity();

private:
    int FindSkillIndex(std::uint16_t skillKind) const;

    cltQuickSlotSystem* m_pQuickSlotSystem = nullptr;
    cltLessonSystem* m_pLessonSystem = nullptr;
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltEquipmentSystem* m_pEquipmentSystem = nullptr;
    cltPlayerAbility* m_pPlayerAbility = nullptr;
    cltTitleSystem* m_pTitleSystem = nullptr;
    std::vector<std::uint16_t> m_skillKinds;
    std::vector<int> m_skillValidity;
    unsigned int m_userData1 = 0;

public:
    static cltItemKindInfo* m_pclItemKindInfo;
    static cltSkillKindInfo* m_pclSkillKindInfo;
    static cltClassKindInfo* m_pclClassKindInfo;
    static void (*m_pOnAcquireCircleSkillFuncPtr)(unsigned int);
};
