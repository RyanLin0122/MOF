#include "System/cltEmblemSystem.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>

#include "Info/cltCharKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include "Info/cltEmblemKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltClassSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltMoneySystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltTitleSystem.h"

cltEmblemKindInfo* cltEmblemSystem::m_pclEmblemKindInfo = nullptr;
cltClassKindInfo* cltEmblemSystem::m_pclClassKindInfo = nullptr;
int (*cltEmblemSystem::m_pExternIsKillMonsterFuncPtr)(unsigned int, std::uint16_t, int) = nullptr;
int (*cltEmblemSystem::m_pExternIsKillBossMonsterFuncPtr)(unsigned int, int) = nullptr;
void (*cltEmblemSystem::m_pExternOnAcquiredEmblemFuncPtr)(unsigned int, std::uint16_t) = nullptr;

namespace {
void TryAcquireByRandom(cltEmblemSystem* self, strEmblemKindInfo** infos, int count, std::uint32_t strEmblemKindInfo::*field) {
    if (!self || !infos) return;
    for (int i = 0; i < count; ++i) {
        strEmblemKindInfo* info = infos[i];
        if (!info || self->IsAcquiredEmblem(info->wEmblemId) == 1) continue;
        if ((std::rand() % 10000) < static_cast<int>(info->*field)) {
            self->AcquireEmblem(info->wEmblemId);
            if (cltEmblemSystem::m_pExternOnAcquiredEmblemFuncPtr) {
                cltEmblemSystem::m_pExternOnAcquiredEmblemFuncPtr(self->GetUserData0(), info->wEmblemId);
            }
        }
    }
}
}

void cltEmblemSystem::InitializeStaticVariable(
    cltEmblemKindInfo* emblemKindInfo,
    cltClassKindInfo* classKindInfo,
    int (*externIsKillMonster)(unsigned int, std::uint16_t, int),
    int (*externIsKillBossMonster)(unsigned int, int),
    void (*externOnAcquiredEmblem)(unsigned int, std::uint16_t)) {
    m_pclEmblemKindInfo = emblemKindInfo;
    m_pclClassKindInfo = classKindInfo;
    m_pExternIsKillMonsterFuncPtr = externIsKillMonster;
    m_pExternIsKillBossMonsterFuncPtr = externIsKillBossMonster;
    m_pExternOnAcquiredEmblemFuncPtr = externOnAcquiredEmblem;
}

cltEmblemSystem::cltEmblemSystem() {
    m_acquiredEmblemKinds.fill(0);
    m_acquiredEmblemNum = 0;
    m_usingEmblemKind = 0;
    m_pUsingEmblemInfo = nullptr;
    m_isUsingEmblemValid = 0;
}


cltEmblemSystem::~cltEmblemSystem() {
    Free();
}

void cltEmblemSystem::Initialize(cltLevelSystem* levelSystem, cltClassSystem* classSystem, cltQuestSystem* questSystem,
                                 cltMoneySystem* moneySystem, cltTitleSystem* titleSystem,
                                 std::uint16_t emblemNum, const std::uint16_t* emblemKinds,
                                 std::uint16_t usingEmblemKind, unsigned int userData0) {
    m_pLevelSystem = levelSystem;
    m_pQuestSystem = questSystem;
    m_pMoneySystem = moneySystem;
    m_pClassSystem = classSystem;
    m_pTitleSystem = titleSystem;

    if (emblemNum > 0 && emblemKinds) {
        std::memcpy(m_acquiredEmblemKinds.data(), emblemKinds, emblemNum * sizeof(std::uint16_t));
    }
    m_acquiredEmblemNum = emblemNum;
    m_usingEmblemKind = usingEmblemKind;

    if (usingEmblemKind) {
        m_pUsingEmblemInfo = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(usingEmblemKind) : nullptr;
        UpdateValidity(this);
    } else {
        m_pUsingEmblemInfo = nullptr;
        m_isUsingEmblemValid = 0;
    }

    m_userData0 = userData0;
}

void cltEmblemSystem::Free() {
    m_acquiredEmblemKinds.fill(0);
    m_acquiredEmblemNum = 0;
    m_usingEmblemKind = 0;
    m_pUsingEmblemInfo = nullptr;
    m_isUsingEmblemValid = 0;
}

std::uint16_t cltEmblemSystem::GetUsingEmblemKind() { return m_usingEmblemKind; }

int cltEmblemSystem::CanAcquireEmblem(std::uint16_t emblemKind) {
    if (m_acquiredEmblemNum >= 100) return 0;
    if (IsAcquiredEmblem(emblemKind) == 1) return 0;

    strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(emblemKind) : nullptr;
    if (!info) return 0;
    if (!m_pMoneySystem || !m_pMoneySystem->CanDecreaseMoney(static_cast<int>(info->dwPrice))) return 0;
    if (!m_pLevelSystem || m_pLevelSystem->GetLevel() < info->wAcquireConditionLevel) return 0;

    if (info->AcquireConditionMonsterType[0] != '0' && info->dwAcquireConditionMonsterKillCount != 0
        && !IsKillMonster(info->AcquireConditionMonsterType, static_cast<int>(info->dwAcquireConditionMonsterKillCount))) {
        return 0;
    }

    if (info->qwAcquireConditionJobChangeAtb != 0) {
        const std::uint16_t classKind = m_pClassSystem ? m_pClassSystem->GetClass() : 0;
        strClassKindInfo* classInfo = m_pclClassKindInfo ? m_pclClassKindInfo->GetClassKindInfo(classKind) : nullptr;
        if (!classInfo || ((info->qwAcquireConditionJobChangeAtb & classInfo->qwClassAtb) == 0)) return 0;
    }

    if (info->wAcquireConditionQuest != 0 && m_pQuestSystem->IsCompleteQuest(info->wAcquireConditionQuest) == 0) {
        return 0;
    }

    return 1;
}

void cltEmblemSystem::AcquireEmblem(std::uint16_t emblemKind) {
    strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(emblemKind) : nullptr;
    m_acquiredEmblemKinds[m_acquiredEmblemNum++] = emblemKind;
    m_pMoneySystem->DecreaseMoney(static_cast<int>(info->dwPrice));
}

int cltEmblemSystem::IsAcquiredEmblem(std::uint16_t emblemKind) {
    for (std::uint16_t i = 0; i < m_acquiredEmblemNum; ++i) {
        if (m_acquiredEmblemKinds[i] == emblemKind) return 1;
    }
    return 0;
}

int cltEmblemSystem::CanSetUsingEmblem(std::uint16_t emblemKind) {
    if (emblemKind == 0) return m_usingEmblemKind != 0;
    if (!IsAcquiredEmblem(emblemKind)) return 0;

    strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(emblemKind) : nullptr;
    if (!info) return 0;

    if (info->wEquipConditionLevelFrom && m_pLevelSystem->GetLevel() < info->wEquipConditionLevelFrom) return 0;
    if (info->wEquipConditionLevelTo && m_pLevelSystem->GetLevel() > info->wEquipConditionLevelTo) return 0;
    return 1;
}

void cltEmblemSystem::SetUsingEmblem(std::uint16_t emblemKind) {
    m_usingEmblemKind = emblemKind;
    if (emblemKind) {
        m_pUsingEmblemInfo = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(emblemKind) : nullptr;
        UpdateValidity(this);
        m_pTitleSystem->OnEvent_setemblem(m_usingEmblemKind);
    } else {
        m_pUsingEmblemInfo = nullptr;
        m_isUsingEmblemValid = 0;
    }
}

int cltEmblemSystem::IsKillMonster(char* monsterTypeList, int killCount) {
    char buffer[1024]{};
    std::strcpy(buffer, monsterTypeList);

    const char* delim = "|";
    char* tok = std::strtok(buffer, delim);
    if (!tok) return 1;

    while (tok) {
        std::uint16_t kind = cltCharKindInfo::TranslateKindCode(tok);
        if (!kind || !m_pExternIsKillMonsterFuncPtr(m_userData0, kind, killCount)) {
            return 0;
        }
        tok = std::strtok(nullptr, delim);
    }

    return 1;
}

int cltEmblemSystem::GetUsingValue(std::uint32_t strEmblemKindInfo::*field) {
    int result = IsUsingEmblemValidity();
    if (result) result = m_pUsingEmblemInfo ? static_cast<int>(m_pUsingEmblemInfo->*field) : 0;
    return result;
}

int cltEmblemSystem::GetExpAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraExperienceRate); }
int cltEmblemSystem::GetAPowerAdvantage(int monsterType) {
    int result = IsUsingEmblemValidity();
    if (!result) return result;
    if (!m_pUsingEmblemInfo) return 0;

    result = static_cast<int>(m_pUsingEmblemInfo->dwExtraAttackPowerRate);
    switch (monsterType) {
        case 0: result += static_cast<int>(m_pUsingEmblemInfo->dwVsUndeadExtraAttackPowerRate); break;
        case 4: result += static_cast<int>(m_pUsingEmblemInfo->dwVsOtherMonsterExtraAttackPowerRate); break;
        case 5: result += static_cast<int>(m_pUsingEmblemInfo->dwVsDemonExtraAttackPowerRate); break;
        case 6: result += static_cast<int>(m_pUsingEmblemInfo->dwVsAnimalExtraAttackPowerRate); break;
        default: break;
    }
    return result;
}
int cltEmblemSystem::GetDPowerAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraDefenseRate); }
int cltEmblemSystem::GetMaxHPAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraHpRate); }
int cltEmblemSystem::GetMaxManaAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraMpRate); }
int cltEmblemSystem::GetHitRateAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraHitRate); }
int cltEmblemSystem::GetMissRateAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraEvasionRate); }
int cltEmblemSystem::GetCriticalHitRateAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwExtraCriticalRate); }
int cltEmblemSystem::GetDeadPenaltyExpAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwDeathPenaltyChangeRate); }
int cltEmblemSystem::GetItemRecoverHPAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwHpPotionEffectChangeRate); }
int cltEmblemSystem::GetItemRecoverManaAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwMpPotionEffectChangeRate); }
int cltEmblemSystem::GetAutoRecoverHPAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwHpAutoRecoveryChangeRate); }
int cltEmblemSystem::GetAutoRecoverManaAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwMpAutoRecoveryChangeRate); }
int cltEmblemSystem::GetShopItemPriceAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwShopPurchasePriceChangeRate); }
int cltEmblemSystem::GetSellingAgencyTaxAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwRunePayFeeChange); }
int cltEmblemSystem::GetTeleportDragonCostAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwDragonUsageFeeChangeRate); }
int cltEmblemSystem::GetMeritoriousAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwHuntingPublicQuestExtraPoints); }
int cltEmblemSystem::GetSwordLessonPtAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwSwordClassExtraPointsRate); }
int cltEmblemSystem::GetBowLessonPtAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwArcheryClassExtraPointsRate); }
int cltEmblemSystem::GetMagicLessonPtAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwMagicClassExtraPointsRate); }
int cltEmblemSystem::GetTheologyLessonPtAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwTheologyClassExtraPointsRate); }
int cltEmblemSystem::GetEnchantAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwEnchantSuccessRate); }
int cltEmblemSystem::GetSkillRangeAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwSkillAreaAttackIncreaseRate); }
std::uint16_t cltEmblemSystem::GetTraningCardItemKind() {
    if (!IsUsingEmblemValidity() || !m_pUsingEmblemInfo) return 0;
    return m_pUsingEmblemInfo->wTrainingCard;
}
int cltEmblemSystem::GetWarMetoriousMonsterKillNumAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwPublicQuestMonsterCountDiscountRate); }
int cltEmblemSystem::GetSellItemPriceAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwItemSalePriceDiscountRate); }
int cltEmblemSystem::GetCircleQuestRewardPtAdvantage() { return GetUsingValue(&strEmblemKindInfo::dwCircleEvaluationScore); }

void cltEmblemSystem::FillOutEmblemInfo(CMofMsg* msg) {
    msg->Put_BYTE(static_cast<unsigned __int8>(m_acquiredEmblemNum));
    msg->Put(reinterpret_cast<char*>(m_acquiredEmblemKinds.data()), 2 * m_acquiredEmblemNum);
    msg->Put_WORD(m_usingEmblemKind);
}

void cltEmblemSystem::UpdateValidity(cltEmblemSystem* self) {
    if (!self || !self->m_pUsingEmblemInfo) return;

    if (self->m_pUsingEmblemInfo->wEquipConditionLevelFrom
        && self->m_pLevelSystem
        && self->m_pLevelSystem->GetLevel() < self->m_pUsingEmblemInfo->wEquipConditionLevelFrom) {
        self->m_isUsingEmblemValid = 0;
    } else {
        self->m_isUsingEmblemValid = !self->m_pUsingEmblemInfo->wEquipConditionLevelTo
            || (self->m_pLevelSystem && self->m_pLevelSystem->GetLevel() <= self->m_pUsingEmblemInfo->wEquipConditionLevelTo);
    }
}

int cltEmblemSystem::IsUsingEmblemValidity() { return m_isUsingEmblemValid; }

int cltEmblemSystem::GetGenericEmblem(std::uint16_t* outEmblemKinds) {
    int outCount = 0;
    for (std::uint16_t i = 0; i < m_acquiredEmblemNum; ++i) {
        strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(m_acquiredEmblemKinds[i]) : nullptr;
        if (info && info->dwRarityType != 1U) {
            outEmblemKinds[outCount++] = m_acquiredEmblemKinds[i];
        }
    }
    return outCount;
}

strEmblemKindInfo* cltEmblemSystem::GetGenericEmblemByID(std::uint16_t emblemKind) {
    for (std::uint16_t i = 0; i < m_acquiredEmblemNum; ++i) {
        strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(m_acquiredEmblemKinds[i]) : nullptr;
        if (info && info->dwRarityType != 1U && info->wEmblemId == emblemKind) return info;
    }
    return nullptr;
}

int cltEmblemSystem::GetRareEmblem(std::uint16_t* outEmblemKinds) {
    int outCount = 0;
    for (std::uint16_t i = 0; i < m_acquiredEmblemNum; ++i) {
        strEmblemKindInfo* info = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo(m_acquiredEmblemKinds[i]) : nullptr;
        if (info && info->dwRarityType) {
            outEmblemKinds[outCount++] = m_acquiredEmblemKinds[i];
        }
    }
    return outCount;
}

void cltEmblemSystem::GetFaintingInfo(int* outRate, int* outDuration) {
    if (m_pUsingEmblemInfo) {
        *outRate = static_cast<int>(m_pUsingEmblemInfo->dwStunRate);
        *outDuration = static_cast<int>(m_pUsingEmblemInfo->dwStunDurationMs);
    } else {
        *outRate = 0; *outDuration = 0;
    }
}
void cltEmblemSystem::GetConfusionInfo(int* outRate, int* outDuration) {
    if (m_pUsingEmblemInfo) {
        *outRate = static_cast<int>(m_pUsingEmblemInfo->dwBlockRate);
        *outDuration = static_cast<int>(m_pUsingEmblemInfo->dwBlockDurationMs);
    } else {
        *outRate = 0; *outDuration = 0;
    }
}
void cltEmblemSystem::GetFreezingInfo(int* outRate, int* outDuration) {
    if (m_pUsingEmblemInfo) {
        *outRate = static_cast<int>(m_pUsingEmblemInfo->dwFreezeRate);
        *outDuration = static_cast<int>(m_pUsingEmblemInfo->dwFreezeDurationMs);
    } else {
        *outRate = 0; *outDuration = 0;
    }
}

void cltEmblemSystem::OnEvent_Dead(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnDead(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionDeathRate);
}
void cltEmblemSystem::OnEvent_UsedRecoverHPItem() {
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnUsingRecoverHPItem(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionHpPotionRate);
}
void cltEmblemSystem::OnEvent_UsedRecoverManaItem() {
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnUsingRecoverManaItem(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionMpPotionRate);
}
void cltEmblemSystem::OnEvent_BeAttackedCritically(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnBeAttackedCritically(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionCritReceivedRate);
}
void cltEmblemSystem::OnEvent_AttackCritically(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnAttackCritically(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionCritDealtRate);
}
void cltEmblemSystem::OnEvent_RegistrySellingAgency(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnRegistrySellingAgency(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionRunePayRate);
}
void cltEmblemSystem::OnEvent_BuyItemFromNPC(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnBuyItemFromNPC(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionShopUsageRate);
}
void cltEmblemSystem::OnEvent_CompleteQuest(std::uint16_t questKind) {
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteQuest(&infos) : 0;
    for (int i = 0; i < count; ++i) {
        strEmblemKindInfo* info = infos[i];
        if (!info || IsAcquiredEmblem(info->wEmblemId) == 1) continue;
        if (info->dwAcquireConditionQuestImmediateAcquire == questKind) {
            AcquireEmblem(info->wEmblemId);
            if (m_pExternOnAcquiredEmblemFuncPtr) m_pExternOnAcquiredEmblemFuncPtr(m_userData0, info->wEmblemId);
        }
    }
}
void cltEmblemSystem::OnEvent_KillBossMonster() {
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnKillBossMonster(&infos) : 0;
    for (int i = 0; i < count; ++i) {
        strEmblemKindInfo* info = infos[i];
        if (!info || IsAcquiredEmblem(info->wEmblemId) == 1) continue;
        if (m_pExternIsKillBossMonsterFuncPtr(m_userData0, static_cast<int>(info->dwAcquireConditionAllBossKill))) {
            AcquireEmblem(info->wEmblemId);
            if (m_pExternOnAcquiredEmblemFuncPtr) m_pExternOnAcquiredEmblemFuncPtr(m_userData0, info->wEmblemId);
        }
    }
}
void cltEmblemSystem::OnEvent_TeleportDragon(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnTeleportDragon(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionDragonUsageRate);
}
void cltEmblemSystem::OnEvent_CompleteMeritous(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteMeritous(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionPublicQuestCompletionRate);
}
void cltEmblemSystem::OnEvent_CompleteSwordLesson(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteSwordLesson(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionSwordClassCompletionRate);
}
void cltEmblemSystem::OnEvent_CompleteBowLesson(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteBowLesson(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionArcheryClassCompletionRate);
}
void cltEmblemSystem::OnEvent_CompleteMagicLesson(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteMagicLesson(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionMagicClassCompletionRate);
}
void cltEmblemSystem::OnEvent_CompleteTheologyLesson(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteTheologyLesson(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionTheologyClassCompletionRate);
}
void cltEmblemSystem::OnEvent_EnchantItem() {
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnEnchantItem(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionEnchantRate);
}
void cltEmblemSystem::OnEvent_CompleteCircleQuest(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnCompleteCircleQuest(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionCircleTaskCompletion);
}
void cltEmblemSystem::OnEvent_SellItemToNpc(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnSellItemToNPC(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionItemSale);
}
void cltEmblemSystem::OnEvent_MultiAttack(unsigned int seed) {
    std::srand(seed);
    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnMultiAttack(&infos) : 0;
    TryAcquireByRandom(this, infos, count, &strEmblemKindInfo::dwAcquireConditionAreaAttack);
}

void cltEmblemSystem::OnEvent_ChangeClass() {
    const std::uint16_t classKind = m_pClassSystem ? m_pClassSystem->GetClass() : 0;
    strClassKindInfo* classInfo = m_pclClassKindInfo ? m_pclClassKindInfo->GetClassKindInfo(classKind) : nullptr;
    if (!classInfo) return;

    strEmblemKindInfo** infos = nullptr;
    const int count = m_pclEmblemKindInfo ? m_pclEmblemKindInfo->GetEmblemKindInfo_OnChangeClass(&infos) : 0;

    for (int i = 0; i < count; ++i) {
        strEmblemKindInfo* info = infos[i];
        if (!info || IsAcquiredEmblem(info->wEmblemId) == 1) continue;
        if (info->dwAcquireConditionJobChangeQuestCompletion == 0) continue;
        if (m_pQuestSystem->IsGiveupQuestPermanently() == 1) continue;
        if (classInfo->bTransferStage == static_cast<std::uint8_t>(info->dwAcquireConditionJobChangeQuestCompletion)) {
            AcquireEmblem(info->wEmblemId);
            if (m_pExternOnAcquiredEmblemFuncPtr) m_pExternOnAcquiredEmblemFuncPtr(m_userData0, info->wEmblemId);
        }
    }
}
