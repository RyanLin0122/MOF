#include "System/cltMonsterToleranceSystem.h"

#include "Info/cltMonsterToleranceKindInfo.h"
#include "System/cltSkillSystem.h"

cltMonsterToleranceKindInfo* cltMonsterToleranceSystem::m_pclMonsterToleranceKindInfo = nullptr;
int (*cltMonsterToleranceSystem::m_pMonsterToleranceGetKillMonsterNumFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltMonsterToleranceSystem::m_pMonsterToleranceUpdatedFuncPtr)(void*, std::uint16_t, strMonsterToleranceKindInfo*) = nullptr;

cltMonsterToleranceSystem::~cltMonsterToleranceSystem() {
    Free();
}

void cltMonsterToleranceSystem::InitializeStaticVariable(
    cltMonsterToleranceKindInfo* kindInfo,
    int (*getKillMonsterNum)(void* userData, std::uint16_t charKind),
    void (*updatedCallback)(void* userData, std::uint16_t charKind, strMonsterToleranceKindInfo* kindInfo)) {
    m_pclMonsterToleranceKindInfo = kindInfo;
    m_pMonsterToleranceGetKillMonsterNumFuncPtr = getKillMonsterNum;
    m_pMonsterToleranceUpdatedFuncPtr = updatedCallback;
}

void cltMonsterToleranceSystem::Initialize(void* userData, cltSkillSystem* skillSystem) {
    m_pUserData = userData;
    m_pSkillSystem = skillSystem;
}

void cltMonsterToleranceSystem::Free() {
    m_pUserData = nullptr;
    m_pSkillSystem = nullptr;
}

int cltMonsterToleranceSystem::GetAPowerAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    if (!info || !info->attack_up_100) return 0;
    return info->attack_up_100 + (m_pSkillSystem ? m_pSkillSystem->MT_GetAPowerAdvantage() : 0);
}

int cltMonsterToleranceSystem::GetDPowerAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    if (!info || !info->defense_up_100) return 0;
    return info->defense_up_100 + (m_pSkillSystem ? m_pSkillSystem->MT_GetDPowerAdvantage() : 0);
}

int cltMonsterToleranceSystem::GetSkillAPowerAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    return info ? info->skill_attack_up_100 : 0;
}

int cltMonsterToleranceSystem::GetHitRateAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    if (!info || !info->hit_up_1000) return 0;
    return info->hit_up_1000 + (m_pSkillSystem ? m_pSkillSystem->MT_GetHitRateAdvantage() : 0);
}

int cltMonsterToleranceSystem::GetMissRateAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    if (!info || !info->dodge_up_1000) return 0;
    return info->dodge_up_1000 + (m_pSkillSystem ? m_pSkillSystem->MT_GetMissRateAdvantage() : 0);
}

int cltMonsterToleranceSystem::GetCriticalHitRateAdvantage(std::uint16_t charKind) {
    if (!charKind || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return 0;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    if (!killCount) return 0;
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    if (!info || !info->crit_up_1000) return 0;
    return info->crit_up_1000 + (m_pSkillSystem ? m_pSkillSystem->MT_GetCriticalHitRateAdvantage() : 0);
}

void cltMonsterToleranceSystem::OnKilledMonster(std::uint16_t charKind) {
    if (!m_pMonsterToleranceUpdatedFuncPtr || !m_pMonsterToleranceGetKillMonsterNumFuncPtr || !m_pclMonsterToleranceKindInfo) return;
    const int killCount = m_pMonsterToleranceGetKillMonsterNumFuncPtr(m_pUserData, charKind);
    strMonsterToleranceKindInfo* info = m_pclMonsterToleranceKindInfo->GetMonsterToleranceKindInfo(static_cast<unsigned int>(killCount));
    m_pMonsterToleranceUpdatedFuncPtr(m_pUserData, charKind, info);
}
