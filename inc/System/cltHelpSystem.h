#pragma once

#include <array>
#include <cstdint>

class cltLevelSystem;
class cltQuestSystem;
class cltSkillSystem;
class cltEquipmentSystem;
class cltPartySystem;
class cltLessonSystem;
class cltCharKindInfo;
class cltSkillKindInfo;

enum HelpKind : int {
    HELP_RECEIPT_QUEST = 0,
    HELP_SET_SCHEDULE = 1,
    HELP_DO_LESSON = 2,
    HELP_ACQUIRE_RUN_SKILL = 3,
    HELP_DO_RUN = 4,
    HELP_ATTACK_MONSTER = 5,
    HELP_EQUIP_ITEM = 6,
    HELP_USE_BONUS_POINT = 7,
    HELP_QUICKSLOT_SKILL = 8,
    HELP_VIEW_WORLDMAP = 9,
};

class cltHelpSystem {
public:
    static void InitializeStaticVariable(void (*popup)(HelpKind), cltCharKindInfo* charKindInfo, cltSkillKindInfo* skillKindInfo);

    void Initialize(cltLevelSystem* levelSystem, cltQuestSystem* questSystem, cltSkillSystem* skillSystem,
        cltEquipmentSystem* equipmentSystem, cltPartySystem* partySystem, cltLessonSystem* lessonSystem);

    void OnReceiptedQuest(std::uint16_t questKind);
    void OnMeetNPC(std::uint16_t questKind);
    void OnMapEntered(std::uint16_t mapKind);
    void OnCompletedQuest(std::uint16_t questKind);
    void OnSkillAcquired(std::uint16_t skillKind);
    void OnLevelUp();
    void OnOpenCharInfoWindow();
    void OnOpenSkillWindow();
    void OnOpenWorldMapWindow();
    void OnKillMonster(std::uint16_t charKind);
    void OnAttack();
    void OnLeaveNPC();
    void OnMapEnteredHelpWindow(std::uint16_t mapKind);

    void Check_ReceiptQuest();
    void Check_SetSchedule(std::uint16_t questKind);
    void Check_DoLesson(std::uint16_t questKind);
    void Check_AcquireRunSkill(std::uint16_t questKind);
    void Check_DoRun(std::uint16_t skillKind);
    void Check_GoTo_Curuno(std::uint16_t questKind);
    void Check_GoTo_Rora_For_Reward(std::uint16_t questKind);
    void Check_AttackMonster(std::uint16_t mapKind);
    void Check_EquipItem();
    void Check_UseBonusPoint();
    void Check_QuickslotSkill(std::uint16_t skillKind);
    void Check_ViewWorldMap(std::uint16_t mapKind);
    void Check_Party_Caution1(std::uint16_t charKind);
    void Check_Can_Acquire_ActiveSkill();
    void Check_Can_Upgrade_Weapon();
    void Check_Can_Upgrade_Class();
    void Check_See_Diary(std::uint16_t mapKind);
    void Check_SetScheduleQuestComplete(std::uint16_t);
    void Check_DoLessonEachSubject(std::uint16_t mapKind);
    void Check_GoToRoraForRewar();
    void Check_SkillHelper(std::uint16_t mapKind);
    void Check_MajorHelper(std::uint16_t mapKind);

private:
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltQuestSystem* m_pQuestSystem = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    cltEquipmentSystem* m_pEquipmentSystem = nullptr;
    cltPartySystem* m_pPartySystem = nullptr;
    cltLessonSystem* m_pLessonSystem = nullptr;

    int m_needReceiptQuestCheck = 1;
    std::array<int, 10> m_popupOpened{};

    static void (*m_pPopupMessagePtr)(HelpKind);
    static cltCharKindInfo* m_pclCharKindInfo;
    static cltSkillKindInfo* m_pclSkillKindInfo;
};
