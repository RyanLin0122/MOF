#pragma once
#include <cstdint>

class ClientCharacter;

class cltTutorialSystem {
public:
    cltTutorialSystem();
    virtual ~cltTutorialSystem();

    int  InitalizeTutorialSystem(std::uint8_t tutorialType);
    int  Poll();
    void AttackMonster();
    void PickUpItem();
    void UseItem();
    int  ExitTutorialMap();
    void SendTutorialMsg(std::uint8_t msgType);
    void MoveCharacterMission(std::uint8_t missionType);

    static void OnTimer_EndUseItem();
    static void OnTimer_StartExitMap();

private:
    ClientCharacter* m_pMyCharacter;      // +1 (word offset)
    unsigned int     m_nTimerID;          // +2
    float            m_fStartX;           // +3
    float            m_fStartY;           // +4
    int              m_nMonsterHP;        // +5
    unsigned int     m_nLastAttackTime;   // +6
    int              m_nWaitingUseItemResult; // +7
};
