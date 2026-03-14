#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

class cltTutorialSystem {
public:
    cltTutorialSystem();
    ~cltTutorialSystem();

    int InitalizeTutorialSystem(std::uint8_t tutorialType);
    int Poll();
    void AttackMonster();
    void PickUpItem();
    void UseItem();
    int ExitTutorialMap();
    void SendTutorialMsg(std::uint8_t msgType);
    void MoveCharacterMission(std::uint8_t missionType);

    static void OnTimer_EndUseItem();
    static void OnTimer_StartExitMap();

private:
    struct TutorialEquipSlot {
        std::uint16_t itemKind{};
        std::uint16_t itemQty{};
        int value0{};
        int value1{};
    };

    struct TutorialCharacter {
        int account{};
        int x{};
        int y{};
        int mapKind{};
        int hp{};
        int maxHp{};
        int attackPower{};
        int classKind{};
        int isMonster{};
        int dead{};
        std::string name;
    };

    struct TutorialItemDrop {
        std::uint32_t owner{};
        std::uint32_t dropId{};
        std::uint16_t itemKind{};
        std::uint16_t itemQty{};
        int x{};
        int y{};
        int picked{};
    };

    struct TutorialTimer {
        unsigned int id{};
        unsigned int dueMs{};
        int enabled{};
        enum class Callback : std::uint8_t {
            None = 0,
            EndUseItem,
            StartExitMap
        } callback{Callback::None};
    };

    struct TutorialProfile {
        std::uint8_t nation{};
        std::uint8_t classKind{};
        std::uint8_t sex{};
        std::uint16_t bodyKind{};
        std::uint16_t hairKind{};
        int mapKind{};
        std::string charName;
        std::array<std::uint16_t, 11> equipPrimary{};
        std::array<std::uint16_t, 11> equipSecondary{};
    };

    void ResetRuntimeState();
    void ResetProfileCaches();
    void BuildProfileFromType(std::uint8_t tutorialType, TutorialProfile& outProfile);
    void ApplyProfileToCharacter(const TutorialProfile& profile);
    void BuildInitialInventories();
    void BuildInitialEquipment(const TutorialProfile& profile);
    void BuildInitialWorld(const TutorialProfile& profile);

    void InitializeInputAndMessageLayer();
    void InitializeQuestAndMeritoriousLayer();
    void InitializeHelpAndUIState();

    void AddTutorialStep(int stepId);
    void PushSystemMessage(const std::string& msg);

    TutorialCharacter* GetMyCharacter();
    TutorialCharacter* GetCharacterByAccount(std::uint32_t account);
    std::uint32_t FindNearestAliveMonsterAccount() const;

    void IssueAttackOrder(TutorialCharacter& me, TutorialCharacter& target, int damage);
    void SpawnTrainingMonsterIfNeeded();
    void SpawnPotionDropIfNeeded();

    TutorialItemDrop* FindPickupCandidate();
    void PickupCandidate(TutorialItemDrop& drop);

    void ProcessTimers();
    unsigned int CreateTimer(unsigned int dueMs, TutorialTimer::Callback callback);
    void ReleaseTimer(unsigned int timerId);

    bool IsInsideExitPortal(const TutorialCharacter& me) const;
    int ShouldAdvanceMissionByMovement(std::uint8_t missionType) const;

private:
    int playerPtr_{};
    unsigned int timerId_{};
    float startX_{};
    float startY_{};
    int tutorialState_{};

    int monsterHp_{};
    unsigned int lastAttackTick_{};
    int waitingUseItemResult_{};

    int mapKind_{};
    int helpWasVisible_{};
    int tutorialActive_{};

    std::array<TutorialEquipSlot, 11> equipPrimary_{};
    std::array<TutorialEquipSlot, 11> equipSecondary_{};
    std::array<TutorialEquipSlot, 16> inventory_{};

    std::vector<TutorialCharacter> characters_;
    std::vector<TutorialItemDrop> drops_;

    std::array<TutorialTimer, 8> timers_{};
    unsigned int timerSerial_{};
    unsigned int fakeNowTick_{};

    static cltTutorialSystem* s_activeTutorial;
};
