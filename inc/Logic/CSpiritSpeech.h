#pragma once

#include <cstdint>

class cltLevelSystem;
class cltQuestSystem;
class cltLessonSystem;
class cltSkillSystem;
class cltPlayerAbility;

class CSpiritSpeech {
public:
    typedef int (CSpiritSpeech::*CheckFunc)(std::uint16_t);

    CSpiritSpeech();
    ~CSpiritSpeech();

    void Free();
    void SetCheckFunc();

    int CallCheckFunc(std::uint16_t condType, std::uint16_t value);

    int SetSpiritSpeech(cltLevelSystem* levelSys, cltQuestSystem* questSys,
                        cltLessonSystem* lessonSys, cltSkillSystem* skillSys,
                        cltPlayerAbility* playerAbility);

    char* CheckSpiritSpeech();
    int ReleaseSpiritSpeech(std::uint16_t spiritId, std::uint16_t scriptId);

    // Check functions (indexed by ESpiritSpeechConditionType)
    int CheckLowLevel(std::uint16_t value);
    int CheckMapID(std::uint16_t value);
    int CheckQuestID(std::uint16_t value);
    int CheckLessonPoint(std::uint16_t value);
    int CheckHaveSkill(std::uint16_t value);
    int CheckHaveNotSkill(std::uint16_t value);
    int CheckMapKind(std::uint16_t value);
    int CheckMonsterKill(std::uint16_t value);
    int CheckUnEquip(std::uint16_t value);
    int CheckStatUpdate(std::uint16_t value);
    int CheckLowExp(std::uint16_t value);
    int CheckLevelUp(std::uint16_t value);
    int CheckAddSkill(std::uint16_t value);
    int CheckQuestCount(std::uint16_t value);
    int CheckHighLevel(std::uint16_t value);
    int CheckAllMonsterKill(std::uint16_t value);
    int CheckPetLowSatiety(std::uint16_t value);
    int CheckPetFullSatiety(std::uint16_t value);
    int CheckPetEat(std::uint16_t value);
    int CheckPetEatPenguine(std::uint16_t value);
    int CheckPetEatBone(std::uint16_t value);

    // Event handlers
    void OnMapEntered(std::uint16_t mapId, std::uint16_t mapKind);
    void OnMonsterKill(std::uint16_t monsterKind);
    void OnMonsterKillAll();
    void OnEquip(int equipFlag);
    void OnLowExpUp(std::uint16_t level);
    void OnLevelUp();
    void OnAddActiveSkill();
    void OnPetEat(std::uint16_t itemKind);

    // Release functions
    void ReleaseMonsterKill();
    void ReleaseLevelUp();
    void ReleaseAddSkill();
    void ReleaseLowExpUp();
    void ReleaseAllMonsterKill();
    void ReleasePetEat();

private:
    static constexpr int kMaxCheckFuncs = 22;

    CheckFunc m_checkFuncs[kMaxCheckFuncs];

    std::uint16_t m_wMapId;
    std::uint16_t m_wMapKind;
    std::uint16_t m_wMonsterKind;
    std::int32_t  m_bMonsterKillFlag;
    std::int32_t  m_bAllMonsterKillFlag;
    std::int32_t  m_nEquipFlag;
    std::uint16_t m_wLowExpLevel;
    std::uint16_t m_wLevelUpLevel;
    std::int32_t  m_bLevelUpFlag;
    std::int32_t  m_bAddSkillFlag;
    std::int32_t  m_bLowExpFlag;
    std::uint16_t m_wPetEatItemKind;

    cltLevelSystem*   m_pLevelSystem;
    cltQuestSystem*   m_pQuestSystem;
    cltLessonSystem*  m_pLessonSystem;
    cltSkillSystem*   m_pSkillSystem;
    cltPlayerAbility* m_pPlayerAbility;
};
