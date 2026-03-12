#pragma once

#include <array>
#include <cstdint>

struct cltUsingSkillSystem;
struct cltWorkingPassiveSkillSystem;
enum AttackAtb : int;

class cltPartySystem {
public:
    static void InitializeStaticVariable(
        void (*)(cltPartySystem*),
        void (*)(cltPartySystem*),
        cltUsingSkillSystem* (*)(void*),
        cltWorkingPassiveSkillSystem* (*)(void*),
        int (*)(cltPartySystem*),
        int (*)(void*),
        void (*)(cltPartySystem*),
        void (*)(cltPartySystem*));

    cltPartySystem();
    ~cltPartySystem();
    cltPartySystem* vector_deleting_destructor(char);

    void Create(void*, void*);
    void Create(void* const*, std::uint8_t);
    int IsCreated();
    void Free();
    unsigned int CanJoin(void*);
    void Join(void*);
    int CanLeave(void*);
    void Leave(void*);
    void* GetLeadInstance();
    std::uint8_t GetPartyMemberNum();
    void* GetPartyInstance(int);
    void SetUserData1(unsigned int);
    unsigned int GetUserData1();
    void SetUserData2(unsigned int);
    unsigned int GetUserData2();
    void SetAdvantages(int, int, int);
    int GetAPowerRateAdvantage(void**, int*, std::uint16_t*);
    int GetDPowerRateAdvantage(void**, int*, std::uint16_t*);
    int GetExpAdvantage(void**, int*, std::uint16_t*);
    int GetHitRateAdvantage();
    int GetIncSTR(void**, int*, std::uint16_t*);
    int GetIncDEX(void**, int*, std::uint16_t*);
    int GetIncVIT(void**, int*, std::uint16_t*);
    int GetIncINT(void**, int*, std::uint16_t*);
    int GetAttackAtb(AttackAtb*, void**, std::uint16_t*);
    void GetPartyableLevelRange(int*, int*);

private:
    std::array<void*, 5> members_{};
    std::uint8_t leaderIndex_{};
    std::uint8_t memberCount_{};
    unsigned int userData1_{};
    unsigned int userData2_{};
    int apAdv_{};
    int dpAdv_{};
    int expAdv_{};

    static void (*m_pOnPartyFreeFuncPtr)(cltPartySystem*);
    static void (*m_pOnPartyLeaderChangedFuncPtr)(cltPartySystem*);
    static cltUsingSkillSystem* (*m_pPartyGetUsingSkillFuncPtr)(void*);
    static cltWorkingPassiveSkillSystem* (*m_pPartyGetWPSSystemFuncPtr)(void*);
    static int (*m_pPartyGetAvrgLevelFuncPtr)(cltPartySystem*);
    static int (*m_pPartyGetLevelFuncPtr)(void*);
    static void (*m_pPartyJoinedNewPartyMemberFuncPtr)(cltPartySystem*);
    static void (*m_pPartyLeftPartyMemeberFuncPtr)(cltPartySystem*);
};
