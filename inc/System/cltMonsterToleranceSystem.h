#pragma once

#include <cstdint>

class cltMonsterToleranceKindInfo;
class cltSkillSystem;
struct strMonsterToleranceKindInfo;

class cltMonsterToleranceSystem {
public:
    static void InitializeStaticVariable(
        cltMonsterToleranceKindInfo* kindInfo,
        int (*getKillMonsterNum)(void* userData, std::uint16_t charKind),
        void (*updatedCallback)(void* userData, std::uint16_t charKind, struct strMonsterToleranceKindInfo* kindInfo));

    cltMonsterToleranceSystem() = default;
    ~cltMonsterToleranceSystem();

    void Initialize(void* userData, cltSkillSystem* skillSystem);
    void Free();

    int GetAPowerAdvantage(std::uint16_t charKind);
    int GetDPowerAdvantage(std::uint16_t charKind);
    int GetSkillAPowerAdvantage(std::uint16_t charKind);
    int GetHitRateAdvantage(std::uint16_t charKind);
    int GetMissRateAdvantage(std::uint16_t charKind);
    int GetCriticalHitRateAdvantage(std::uint16_t charKind);

    void OnKilledMonster(std::uint16_t charKind);

private:
    cltSkillSystem* m_pSkillSystem = nullptr;
    void* m_pUserData = nullptr;

    static cltMonsterToleranceKindInfo* m_pclMonsterToleranceKindInfo;
    static int (*m_pMonsterToleranceGetKillMonsterNumFuncPtr)(void* userData, std::uint16_t charKind);
    static void (*m_pMonsterToleranceUpdatedFuncPtr)(void* userData, std::uint16_t charKind, struct strMonsterToleranceKindInfo* kindInfo);
};
