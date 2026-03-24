#include "Logic/CSpiritSpeechMgr.h"

#include <cstring>

#include "global.h"
#include "Util/cltTimer.h"

CSpiritSpeechMgr::CSpiritSpeechMgr()
    : CSpiritSpeech()
    , m_questSpeech()
    , m_gossip()
    , m_nTimerID(0)
    , m_nPlayFlag(0)
    , m_nSpeechPhase(0)
{
    std::strcpy(m_szScript, "");
}

CSpiritSpeechMgr::~CSpiritSpeechMgr()
{
}

void CSpiritSpeechMgr::Free()
{
    if (m_nTimerID)
        g_clTimerManager.ReleaseTimer(m_nTimerID);
    m_nTimerID = 0;

    CSpiritSpeech::Free();
    m_questSpeech.Free();

    m_nPlayFlag = 0;
    std::strcpy(m_szScript, "");
    m_nSpeechPhase = 0;
}

int CSpiritSpeechMgr::SetSpiritSpeech(cltLevelSystem* levelSys, cltQuestSystem* questSys,
                                       cltLessonSystem* lessonSys, cltSkillSystem* skillSys,
                                       cltPlayerAbility* playerAbility, cltBaseInventory* inventory,
                                       cltPetInventorySystem* petInventory)
{
    CSpiritSpeech::SetSpiritSpeech(levelSys, questSys, lessonSys, skillSys, playerAbility);
    m_questSpeech.SetSpiritQuestSpeech(questSys, inventory, petInventory);
    return 1;
}

void CSpiritSpeechMgr::SetSpiritID(std::uint16_t spiritId)
{
    (void)spiritId;
}

int CSpiritSpeechMgr::PlaySpiritSpeech()
{
    m_nPlayFlag = 1;

    if (m_nTimerID)
        g_clTimerManager.ReleaseTimer(m_nTimerID);
    m_nTimerID = 0;

    m_nTimerID = g_clTimerManager.CreateTimer(
        0xFFFFFFFF,
        reinterpret_cast<unsigned int>(this),
        kTimerInterval,
        1, nullptr, nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(
            static_cast<void(*)(unsigned int, CSpiritSpeechMgr*)>(OnTimer_SpiritSpeech)),
        nullptr);

    return 1;
}

int CSpiritSpeechMgr::StopSpiritSpeech()
{
    m_nPlayFlag = 0;

    if (m_nTimerID)
        g_clTimerManager.ReleaseTimer(m_nTimerID);
    m_nTimerID = 0;

    return 1;
}

void CSpiritSpeechMgr::OnTimer_SpiritSpeech(unsigned int /*a1*/, CSpiritSpeechMgr* mgr)
{
    mgr->SetSpiritSpeechScript();
}

void CSpiritSpeechMgr::UpdateQuestHunt(std::uint16_t questId)
{
    if (m_nPlayFlag == 1)
    {
        std::strcpy(m_szScript, m_questSpeech.UpdateQuestHunt(questId));

        g_clTimerManager.ResetTimer(
            m_nTimerID,
            0xFFFFFFFF,
            reinterpret_cast<unsigned int>(this),
            kTimerInterval,
            1, nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(
                static_cast<void(*)(unsigned int, CSpiritSpeechMgr*)>(OnTimer_SpiritSpeech)));
    }
}

void CSpiritSpeechMgr::UpdateQuestCollection(std::uint16_t itemKind)
{
    if (m_nPlayFlag == 1)
    {
        std::strcpy(m_szScript, m_questSpeech.UpdateQuestCollection(itemKind));

        g_clTimerManager.ResetTimer(
            m_nTimerID,
            0xFFFFFFFF,
            reinterpret_cast<unsigned int>(this),
            kTimerInterval,
            1, nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(
                static_cast<void(*)(unsigned int, CSpiritSpeechMgr*)>(OnTimer_SpiritSpeech)));
    }
}

void CSpiritSpeechMgr::UpdatePetEat()
{
    if (m_nPlayFlag == 1)
    {
        SetSpiritSpeechScript();

        g_clTimerManager.ResetTimer(
            m_nTimerID,
            0xFFFFFFFF,
            reinterpret_cast<unsigned int>(this),
            kTimerInterval,
            1, nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(
                static_cast<void(*)(unsigned int, CSpiritSpeechMgr*)>(OnTimer_SpiritSpeech)));
    }
}

void CSpiritSpeechMgr::SetSpiritSpeechScript()
{
    char* text = nullptr;

    // Alternating phase: 0 = check speech conditions, 1 = check complete quest
    if (m_nSpeechPhase == 1 && !m_questSpeech.CheckCompleteQuest())
        m_nSpeechPhase = 0;

    if (m_nSpeechPhase)
    {
        text = m_questSpeech.CheckCompleteQuest();
        m_nSpeechPhase = 0;
    }
    else
    {
        text = CSpiritSpeech::CheckSpiritSpeech();
        m_nSpeechPhase = 1;
    }

    if (text)
    {
        std::strcpy(m_szScript, text);
    }
    else
    {
        char* gossipText = m_gossip.CheckSpiritGossip();
        if (gossipText)
            std::strcpy(m_szScript, gossipText);
        else
            std::strcpy(m_szScript, "");
    }
}

char* CSpiritSpeechMgr::GetSpiritSpeechScript()
{
    return m_szScript;
}

void CSpiritSpeechMgr::OnEquip(int equipFlag)
{
    CSpiritSpeech::OnEquip(equipFlag);
}

void CSpiritSpeechMgr::OnMapEntered(std::uint16_t mapId, std::uint16_t mapKind)
{
    CSpiritSpeech::OnMapEntered(mapId, mapKind);
}

void CSpiritSpeechMgr::OnMonsterKill(std::uint16_t monsterKind)
{
    CSpiritSpeech::OnMonsterKill(monsterKind);
}

void CSpiritSpeechMgr::OnMonsterKillAll()
{
    CSpiritSpeech::OnMonsterKillAll();
}

void CSpiritSpeechMgr::OnLowExpUp(std::uint16_t level)
{
    CSpiritSpeech::OnLowExpUp(level);
}

void CSpiritSpeechMgr::OnLevelUp()
{
    CSpiritSpeech::OnLevelUp();
}

void CSpiritSpeechMgr::OnAddActiveSkill()
{
    CSpiritSpeech::OnAddActiveSkill();
}

void CSpiritSpeechMgr::OnPetEat(std::uint16_t itemKind)
{
    CSpiritSpeech::OnPetEat(itemKind);
    UpdatePetEat();
}
