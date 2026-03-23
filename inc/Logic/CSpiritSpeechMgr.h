#pragma once

#include <cstdint>

#include "Logic/CSpiritSpeech.h"
#include "Logic/CSpiritQuestSpeech.h"
#include "Logic/CSpiritGossip.h"

class cltLevelSystem;
class cltQuestSystem;
class cltLessonSystem;
class cltSkillSystem;
class cltPlayerAbility;
class cltBaseInventory;
class cltPetInventorySystem;

class CSpiritSpeechMgr : public CSpiritSpeech {
public:
    CSpiritSpeechMgr();
    ~CSpiritSpeechMgr();

    void Free();

    int SetSpiritSpeech(cltLevelSystem* levelSys, cltQuestSystem* questSys,
                        cltLessonSystem* lessonSys, cltSkillSystem* skillSys,
                        cltPlayerAbility* playerAbility, cltBaseInventory* inventory,
                        cltPetInventorySystem* petInventory);

    void SetSpiritID(std::uint16_t spiritId);

    int PlaySpiritSpeech();
    int StopSpiritSpeech();

    static void OnTimer_SpiritSpeech(unsigned int a1, CSpiritSpeechMgr* mgr);

    void UpdateQuestHunt(std::uint16_t questId);
    void UpdateQuestCollection(std::uint16_t itemKind);
    void UpdatePetEat();

    void SetSpiritSpeechScript();
    char* GetSpiritSpeechScript();

    void OnLevelUp();
    void OnEquip(int equipFlag);
    void OnMapEntered(std::uint16_t mapId, std::uint16_t mapKind);
    void OnMonsterKill(std::uint16_t monsterKind);
    void OnMonsterKillAll();
    void OnAddActiveSkill();
    void OnLowExpUp(std::uint16_t level);
    void OnPetEat(std::uint16_t itemKind);

private:
    static constexpr int kScriptBufferSize = 1024;
    static constexpr unsigned int kTimerInterval = 0x1F40; // 8000ms

    CSpiritQuestSpeech m_questSpeech;
    CSpiritGossip      m_gossip;
    unsigned int       m_nTimerID;
    int                m_nPlayFlag;
    char               m_szScript[kScriptBufferSize];
    int                m_nSpeechPhase;
};
