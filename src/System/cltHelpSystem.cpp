#include "System/cltHelpSystem.h"

#include "Info/cltMapInfo.h"
#include "Info/cltNPCInfo.h"
#include "Info/cltQuestKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltSkillSystem.h"

class cltQuestSystem {
public:
    void* GetPlayingQuestInfoByNPCID(std::uint16_t);
    void* GetPlayingQuestInfoByQuestID(std::uint16_t);
    int CanReward(int);
};

class stPlayingQuestInfo {
public:
    std::uint16_t questKind;
    std::uint8_t completed;
};

extern cltMapInfo g_Map;

void (*cltHelpSystem::m_pPopupMessagePtr)(HelpKind) = nullptr;
cltCharKindInfo* cltHelpSystem::m_pclCharKindInfo = nullptr;
cltSkillKindInfo* cltHelpSystem::m_pclSkillKindInfo = nullptr;

void cltHelpSystem::InitializeStaticVariable(void (*popup)(HelpKind), cltCharKindInfo* charKindInfo, cltSkillKindInfo* skillKindInfo) {
    m_pPopupMessagePtr = popup;
    m_pclCharKindInfo = charKindInfo;
    m_pclSkillKindInfo = skillKindInfo;
}

void cltHelpSystem::Initialize(cltLevelSystem* levelSystem, cltQuestSystem* questSystem, cltSkillSystem* skillSystem,
    cltEquipmentSystem* equipmentSystem, cltPartySystem* partySystem, cltLessonSystem* lessonSystem) {
    m_pLevelSystem = levelSystem;
    m_pQuestSystem = questSystem;
    m_pSkillSystem = skillSystem;
    m_pEquipmentSystem = equipmentSystem;
    m_pPartySystem = partySystem;
    m_pLessonSystem = lessonSystem;
    m_needReceiptQuestCheck = 1;
}

void cltHelpSystem::OnReceiptedQuest(std::uint16_t questKind) {
    Check_SetSchedule(questKind);
    Check_DoLesson(questKind);
    Check_GoTo_Curuno(questKind);
}

void cltHelpSystem::OnMeetNPC(std::uint16_t questKind) { Check_GoTo_Rora_For_Reward(questKind); }

void cltHelpSystem::OnMapEntered(std::uint16_t mapKind) {
    Check_ReceiptQuest();
    Check_AttackMonster(mapKind);
    Check_ViewWorldMap(mapKind);
    Check_See_Diary(mapKind);
    Check_DoLessonEachSubject(mapKind);
    m_needReceiptQuestCheck = 0;
}

void cltHelpSystem::OnCompletedQuest(std::uint16_t) {}

void cltHelpSystem::OnSkillAcquired(std::uint16_t skillKind) { Check_QuickslotSkill(skillKind); }

void cltHelpSystem::OnLevelUp() {
    Check_UseBonusPoint();
    Check_Can_Acquire_ActiveSkill();
    Check_Can_Upgrade_Weapon();
    Check_Can_Upgrade_Class();
}

void cltHelpSystem::OnOpenCharInfoWindow() { m_popupOpened[HELP_USE_BONUS_POINT] = 1; }
void cltHelpSystem::OnOpenSkillWindow() { m_popupOpened[HELP_QUICKSLOT_SKILL] = 1; }
void cltHelpSystem::OnOpenWorldMapWindow() { m_popupOpened[HELP_VIEW_WORLDMAP] = 1; }
void cltHelpSystem::OnKillMonster(std::uint16_t charKind) { Check_Party_Caution1(charKind); }
void cltHelpSystem::OnAttack() {}
void cltHelpSystem::OnLeaveNPC() {
    Check_SetScheduleQuestComplete(0);
    Check_GoToRoraForRewar();
}

void cltHelpSystem::OnMapEnteredHelpWindow(std::uint16_t mapKind) {
    Check_SkillHelper(mapKind);
    Check_MajorHelper(mapKind);
    m_needReceiptQuestCheck = 0;
}

void cltHelpSystem::Check_ReceiptQuest() {
    if (m_popupOpened[HELP_RECEIPT_QUEST] != 1 && m_needReceiptQuestCheck && m_pPopupMessagePtr && m_pQuestSystem) {
        const auto npcKind = cltNPCInfo::TranslateKindCode("N0015");
        if (m_pQuestSystem->GetPlayingQuestInfoByNPCID(npcKind)) {
            return;
        }
        m_pPopupMessagePtr(HELP_RECEIPT_QUEST);
    }
}

void cltHelpSystem::Check_SetSchedule(std::uint16_t questKind) {
    if (m_popupOpened[HELP_SET_SCHEDULE] != 1 && questKind == cltQuestKindInfo::TranslateKindCode("Q0001") && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_SET_SCHEDULE);
    }
}

void cltHelpSystem::Check_DoLesson(std::uint16_t questKind) {
    if (m_popupOpened[HELP_DO_LESSON] != 1 && questKind == cltQuestKindInfo::TranslateKindCode("Q0002") && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_DO_LESSON);
    }
}

void cltHelpSystem::Check_AcquireRunSkill(std::uint16_t questKind) {
    if (m_popupOpened[HELP_ACQUIRE_RUN_SKILL] != 1 && questKind == cltQuestKindInfo::TranslateKindCode("Q0002") && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_ACQUIRE_RUN_SKILL);
    }
}

void cltHelpSystem::Check_DoRun(std::uint16_t skillKind) {
    if (m_popupOpened[HELP_DO_RUN] != 1 && skillKind == cltSkillKindInfo::TranslateKindCode("P00001") && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_DO_RUN);
    }
}

void cltHelpSystem::Check_GoTo_Curuno(std::uint16_t questKind) {
    if (questKind == cltQuestKindInfo::TranslateKindCode("Q1501") && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(static_cast<HelpKind>(14));
    }
}

void cltHelpSystem::Check_GoTo_Rora_For_Reward(std::uint16_t questKind) {
    if (questKind == cltQuestKindInfo::TranslateKindCode("Q1501") && m_pPopupMessagePtr && m_pQuestSystem) {
        auto* info = reinterpret_cast<stPlayingQuestInfo*>(m_pQuestSystem->GetPlayingQuestInfoByQuestID(questKind));
        if (info && !info->completed) {
            m_pPopupMessagePtr(static_cast<HelpKind>(15));
        }
    }
}

void cltHelpSystem::Check_AttackMonster(std::uint16_t mapKind) {
    if (m_popupOpened[HELP_ATTACK_MONSTER] != 1 && m_pLevelSystem && m_pLevelSystem->GetLevel() <= 1 && g_Map.GetMapCaps(mapKind) == 1 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_ATTACK_MONSTER);
    }
}

void cltHelpSystem::Check_EquipItem() {
    if (m_popupOpened[HELP_EQUIP_ITEM] == 1 || !m_pEquipmentSystem || !m_pPopupMessagePtr) {
        return;
    }
    for (unsigned int i = 0; i < 11; ++i) {
        if (m_pEquipmentSystem->GetEquipItem(1, i) != 0) {
            return;
        }
    }
    m_pPopupMessagePtr(HELP_EQUIP_ITEM);
}

void cltHelpSystem::Check_UseBonusPoint() {
    if (m_popupOpened[HELP_USE_BONUS_POINT] != 1 && m_pLevelSystem && m_pLevelSystem->GetLevel() == 2 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_USE_BONUS_POINT);
    }
}

void cltHelpSystem::Check_QuickslotSkill(std::uint16_t skillKind) {
    std::uint16_t skills[100]{};
    if (m_popupOpened[HELP_QUICKSLOT_SKILL] != 1 && m_pSkillSystem && m_pSkillSystem->GetActiveSkill(skills, 0) == 1
        && cltSkillKindInfo::IsActiveSkill(skillKind)
        && m_pclSkillKindInfo && m_pclSkillKindInfo->GetSkillLevel(skillKind) <= 1 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_QUICKSLOT_SKILL);
    }
}

void cltHelpSystem::Check_ViewWorldMap(std::uint16_t mapKind) {
    if (m_popupOpened[HELP_VIEW_WORLDMAP] != 1 && m_pLevelSystem && m_pLevelSystem->GetLevel() <= 3
        && (mapKind == cltMapInfo::TranslateKindCode("M0004") || mapKind == cltMapInfo::TranslateKindCode("M0005"))
        && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(HELP_VIEW_WORLDMAP);
    }
}

void cltHelpSystem::Check_Party_Caution1(std::uint16_t) {}

void cltHelpSystem::Check_Can_Acquire_ActiveSkill() {
    if (m_pLevelSystem && m_pLevelSystem->GetLevel() == 3 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(static_cast<HelpKind>(11));
    }
}

void cltHelpSystem::Check_Can_Upgrade_Weapon() {
    if (m_pLevelSystem && m_pLevelSystem->GetLevel() == 5 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(static_cast<HelpKind>(12));
    }
}

void cltHelpSystem::Check_Can_Upgrade_Class() {
    if (m_pLevelSystem && m_pLevelSystem->GetLevel() == 20 && m_pPopupMessagePtr) {
        m_pPopupMessagePtr(static_cast<HelpKind>(13));
    }
}

void cltHelpSystem::Check_See_Diary(std::uint16_t mapKind) {
    if (!m_pLevelSystem || !m_pSkillSystem || !m_pPopupMessagePtr) {
        return;
    }
    if (m_pLevelSystem->GetLevel() <= 3 && mapKind == cltMapInfo::TranslateKindCode("V0032")) {
        std::uint16_t passives[100]{};
        const auto count = m_pSkillSystem->GetPassiveSkill(passives, 0, 0);
        const auto runSkill = cltSkillKindInfo::TranslateKindCode("P00001");
        for (std::uint16_t i = 0; i < count; ++i) {
            if (passives[i] == runSkill) {
                m_pPopupMessagePtr(static_cast<HelpKind>(16));
                break;
            }
        }
    }
}

void cltHelpSystem::Check_SetScheduleQuestComplete(std::uint16_t) {
    if (!m_pPopupMessagePtr || !m_pQuestSystem) {
        return;
    }
    const auto q = cltQuestKindInfo::TranslateKindCode("Q0001");
    auto* info = reinterpret_cast<stPlayingQuestInfo*>(m_pQuestSystem->GetPlayingQuestInfoByQuestID(q));
    if (info && m_pQuestSystem->CanReward(static_cast<int>(info->questKind))) {
        m_pPopupMessagePtr(static_cast<HelpKind>(17));
    }
}

void cltHelpSystem::Check_DoLessonEachSubject(std::uint16_t mapKind) {
    if (!m_pPopupMessagePtr || !m_pQuestSystem || !m_pLessonSystem) {
        return;
    }
    if (mapKind == cltMapInfo::TranslateKindCode("T0004") || mapKind == cltMapInfo::TranslateKindCode("V0032")) {
        const auto q = cltQuestKindInfo::TranslateKindCode("Q0002");
        auto* info = reinterpret_cast<stPlayingQuestInfo*>(m_pQuestSystem->GetPlayingQuestInfoByQuestID(q));
        if (!info || info->completed) {
            return;
        }
        if (m_pLessonSystem->CanTraningLesson(10) && m_pLessonSystem->CanTraningLesson(11)) {
            if (m_pLessonSystem->CanTraningLesson(20) && m_pLessonSystem->CanTraningLesson(21)) {
                if (m_pLessonSystem->CanTraningLesson(30) && m_pLessonSystem->CanTraningLesson(31)) {
                    if (!m_pLessonSystem->CanTraningLesson(40) || !m_pLessonSystem->CanTraningLesson(41)) {
                        m_pPopupMessagePtr(static_cast<HelpKind>(21));
                    }
                } else {
                    m_pPopupMessagePtr(static_cast<HelpKind>(20));
                }
            } else {
                m_pPopupMessagePtr(static_cast<HelpKind>(19));
            }
        } else {
            m_pPopupMessagePtr(static_cast<HelpKind>(18));
        }
    }
}

void cltHelpSystem::Check_GoToRoraForRewar() {
    if (!m_pPopupMessagePtr || !m_pQuestSystem || !m_pLessonSystem) {
        return;
    }
    const auto q = cltQuestKindInfo::TranslateKindCode("Q0002");
    auto* info = reinterpret_cast<stPlayingQuestInfo*>(m_pQuestSystem->GetPlayingQuestInfoByQuestID(q));
    if (info && !info->completed
        && m_pLessonSystem->GetTotalSwordLessonPt()
        && m_pLessonSystem->GetTotalBowLessonPt()
        && m_pLessonSystem->GetTotalTheologyLessonPt()
        && m_pLessonSystem->GetTotalMagicLessonPt()) {
        m_pPopupMessagePtr(static_cast<HelpKind>(22));
    }
}

void cltHelpSystem::Check_SkillHelper(std::uint16_t) {}
void cltHelpSystem::Check_MajorHelper(std::uint16_t) {}
