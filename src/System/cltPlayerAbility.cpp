#include "System/cltPlayerAbility.h"

#include <algorithm>

#include "System/cltLevelSystem.h"
#include "System/cltSkillSystem.h"

cltCharKindInfo* cltPlayerAbility::m_pclCharKindInfo = nullptr;
cltItemKindInfo* cltPlayerAbility::m_pclItemKindInfo = nullptr;

void cltPlayerAbility::InitializeStaticVariable(cltCharKindInfo* charKindInfo, cltItemKindInfo* itemKindInfo) {
    m_pclCharKindInfo = charKindInfo;
    m_pclItemKindInfo = itemKindInfo;
}

void cltPlayerAbility::Initialize(
    cltLevelSystem* levelSystem,
    cltSkillSystem* skillSystem,
    cltEquipmentSystem* equipmentSystem,
    cltClassSystem* classSystem,
    cltUsingItemSystem* usingItemSystem,
    cltUsingSkillSystem* usingSkillSystem,
    cltWorkingPassiveSkillSystem* workingPassiveSkillSystem,
    int hp,
    int mp,
    std::uint16_t str,
    std::uint16_t dex,
    std::uint16_t intel,
    std::uint16_t vit,
    std::uint16_t bonusPoint) {
    m_pLevelSystem = levelSystem;
    m_pSkillSystem = skillSystem;
    m_pEquipmentSystem = equipmentSystem;
    m_pClassSystem = classSystem;
    m_pUsingItemSystem = usingItemSystem;
    m_pUsingSkillSystem = usingSkillSystem;
    m_pWorkingPassiveSkillSystem = workingPassiveSkillSystem;

    m_baseStr = m_initialStr = str;
    m_baseDex = m_initialDex = dex;
    m_baseInt = m_initialInt = intel;
    m_baseVit = m_initialVit = vit;
    m_bonusPoint = bonusPoint;
    m_hp = std::max(1, hp);
    m_mana = std::max(0, mp);
}

void cltPlayerAbility::Free() {
    m_pLevelSystem = nullptr;
    m_pSkillSystem = nullptr;
    m_pEquipmentSystem = nullptr;
    m_pClassSystem = nullptr;
    m_pUsingItemSystem = nullptr;
    m_pUsingSkillSystem = nullptr;
    m_pWorkingPassiveSkillSystem = nullptr;
}

void cltPlayerAbility::IncreaseBonusPoint(cltPlayerAbility* self, int value) {
    if (!self || value <= 0) {
        return;
    }
    const int next = static_cast<int>(self->m_bonusPoint) + value;
    self->m_bonusPoint = static_cast<std::uint16_t>(std::clamp(next, 0, 0xFFFF));
}

void cltPlayerAbility::ResetAbility(cltPlayerAbility* self) {
    if (!self) {
        return;
    }
    self->m_baseStr = self->m_initialStr;
    self->m_baseDex = self->m_initialDex;
    self->m_baseInt = self->m_initialInt;
    self->m_baseVit = self->m_initialVit;
}

void cltPlayerAbility::DecreaseBonusPoint(std::uint16_t value) {
    if (value >= m_bonusPoint) {
        m_bonusPoint = 0;
    } else {
        m_bonusPoint = static_cast<std::uint16_t>(m_bonusPoint - value);
    }
}

std::uint16_t cltPlayerAbility::GetBaseStr() { return m_baseStr; }
std::uint16_t cltPlayerAbility::GetBaseDex() { return m_baseDex; }
std::uint16_t cltPlayerAbility::GetBaseInt() { return m_baseInt; }
std::uint16_t cltPlayerAbility::GetBaseVit() { return m_baseVit; }

void cltPlayerAbility::SetMana(int mana) { m_mana = std::max(mana, 0); }
int cltPlayerAbility::GetMP() const { return m_mana; }

void cltPlayerAbility::DecreaseMP(int value) {
    if (value <= 0) {
        return;
    }
    m_mana = std::max(0, m_mana - value);
}

std::uint16_t cltPlayerAbility::GetBonusPoint() const { return m_bonusPoint; }

int cltPlayerAbility::GetReqBPForIncreaseStat(std::uint16_t baseStat, int amount) {
    int need = 0;
    for (int i = 0; i < amount; ++i, ++baseStat) {
        if (baseStat < 0x13) {
            need += 1;
        } else if (baseStat < 0x27) {
            need += 2;
        } else if (baseStat < 0x3B) {
            need += 3;
        } else if (baseStat < 0x4F) {
            need += 4;
        } else {
            need += 5;
        }
    }
    return need;
}

void cltPlayerAbility::IncreaseStr(int value, int byBonusPoint) {
    if (value <= 0) return;
    if (byBonusPoint == 1) {
        const int req = GetReqBPForIncreaseStat(GetBaseStr(), value);
        if (GetBonusPoint() < req) {
            return;
        }
        DecreaseBonusPoint(static_cast<std::uint16_t>(req));
    }
    m_baseStr = static_cast<std::uint16_t>(std::min(0xFFFF, static_cast<int>(m_baseStr) + value));
}

void cltPlayerAbility::IncreaseDex(int value, int byBonusPoint) {
    if (value <= 0) return;
    if (byBonusPoint == 1) {
        const int req = GetReqBPForIncreaseStat(GetBaseDex(), value);
        if (GetBonusPoint() < req) {
            return;
        }
        DecreaseBonusPoint(static_cast<std::uint16_t>(req));
    }
    m_baseDex = static_cast<std::uint16_t>(std::min(0xFFFF, static_cast<int>(m_baseDex) + value));
}

void cltPlayerAbility::IncreaseInt(int value, int byBonusPoint) {
    if (value <= 0) return;
    if (byBonusPoint == 1) {
        const int req = GetReqBPForIncreaseStat(GetBaseInt(), value);
        if (GetBonusPoint() < req) {
            return;
        }
        DecreaseBonusPoint(static_cast<std::uint16_t>(req));
    }
    m_baseInt = static_cast<std::uint16_t>(std::min(0xFFFF, static_cast<int>(m_baseInt) + value));
}

void cltPlayerAbility::IncreaseVit(int value, int byBonusPoint) {
    if (value <= 0) return;
    if (byBonusPoint == 1) {
        const int req = GetReqBPForIncreaseStat(GetBaseVit(), value);
        if (GetBonusPoint() < req) {
            return;
        }
        DecreaseBonusPoint(static_cast<std::uint16_t>(req));
    }
    m_baseVit = static_cast<std::uint16_t>(std::min(0xFFFF, static_cast<int>(m_baseVit) + value));
}

int cltPlayerAbility::GetMaxHP(std::uint16_t vit) {
    if (!m_pLevelSystem || !m_pClassSystem) {
        return 1;
    }
    return 16 * vit + 10 * m_pLevelSystem->GetLevel() + 36;
}

int cltPlayerAbility::GetMaxMP(std::uint16_t intel) {
    if (!m_pLevelSystem || !m_pClassSystem) {
        return 1;
    }
    return 12 * intel + 10 * m_pLevelSystem->GetLevel();
}

int cltPlayerAbility::CanResetAbility() {
    return m_baseStr != m_initialStr
        || m_baseDex != m_initialDex
        || m_baseInt != m_initialInt
        || m_baseVit != m_initialVit;
}

int cltPlayerAbility::GetNeedManaForUsingSkill(int baseSkillMana) const {
    if (!m_pSkillSystem) {
        return baseSkillMana;
    }
    // Requires cltSkillSystem::GetSkillManaAdvantage full restoration.
    return std::max(1, baseSkillMana);
}

int cltPlayerAbility::GetBuffNum() const { return m_buffNum; }
int cltPlayerAbility::GetMaxBuffNum() const { return m_maxBuffNum; }
bool cltPlayerAbility::CanAddBuff() const { return m_buffNum < m_maxBuffNum; }


std::uint16_t cltPlayerAbility::GetStr(int, void*) const { return m_baseStr; }
std::uint16_t cltPlayerAbility::GetDex(int, void*) const { return m_baseDex; }
std::uint16_t cltPlayerAbility::GetInt(int, void*) const { return m_baseInt; }
std::uint16_t cltPlayerAbility::GetVit(int, void*) const { return m_baseVit; }

void cltPlayerAbility::IncreaseMP(int value, int, void*) {
    if (value <= 0) return;
    m_mana += value;
}

void cltPlayerAbility::IncreaseMPPercent(int percent, unsigned char, void*) {
    if (percent <= 0) return;
    m_mana += GetMaxMP(GetBaseInt()) * percent / 100;
}

void cltPlayerAbility::DecreaseMPPercent(int percent, unsigned char, void*) {
    if (percent <= 0) return;
    DecreaseMP(GetMaxMP(GetBaseInt()) * percent / 100);
}

int cltPlayerAbility::GetHPRate(void*) const {
    const int maxHp = const_cast<cltPlayerAbility*>(this)->GetMaxHP(m_baseVit);
    if (maxHp <= 0) return 0;
    return (100 * m_hp) / maxHp;
}

int cltPlayerAbility::CanMultiAttack() const { return 0; }
int cltPlayerAbility::GetAttackSpeedAdvantage() const { return 0; }
int cltPlayerAbility::GetDamageHP2ManaRate() const { return 0; }
int cltPlayerAbility::GetDropRateAdvantage() const { return 0; }
int cltPlayerAbility::GetDeadPenaltyExpAdvantage() const { return 0; }
int cltPlayerAbility::GetExpAdvantage(void*) const { return 0; }
int cltPlayerAbility::IsActiveFastRun() const { return 0; }
int cltPlayerAbility::IsActiveNonDelayAttack() const { return 0; }
int cltPlayerAbility::GetAttackAtb(int) const { return 0; }

int cltPlayerAbility::GetItemRecoverHPAdvantage() const { return 0; }
int cltPlayerAbility::GetItemRecoverManaAdvantage() const { return 0; }
int cltPlayerAbility::GetMagicResistRateAdvantage() const { return 0; }
int cltPlayerAbility::GetMagicResistRate() const { return 0; }
int cltPlayerAbility::GetPartyAPowerAdvantage(void*) const { return 0; }
int cltPlayerAbility::GetPartyDPowerAdvantage(void*) const { return 0; }
int cltPlayerAbility::GetPartyHitRateAdvantage(void*) const { return 0; }

bool cltPlayerAbility::GetMaxFaintingInfo(int* a2, int* a3, int, std::uint16_t*) const {
    if (a2) *a2 = 0;
    if (a3) *a3 = 0;
    return false;
}

bool cltPlayerAbility::GetMaxConfusionInfo(int* a2, int* a3, int, std::uint16_t*) const {
    if (a2) *a2 = 0;
    if (a3) *a3 = 0;
    return false;
}

bool cltPlayerAbility::GetMaxFreezingInfo(int* a2, int* a3, int, std::uint16_t*) const {
    if (a2) *a2 = 0;
    if (a3) *a3 = 0;
    return false;
}


int cltPlayerAbility::GetHitRateAdvantage(std::uint16_t, int a3, void*) const { return a3; }
int cltPlayerAbility::GetHitRate(std::uint16_t a2, void* party) const { return GetHitRateAdvantage(0, 3 * a2, party); }
int cltPlayerAbility::GetCriticalHitRateAdvantage(std::uint16_t) const { return 0; }
int cltPlayerAbility::GetCriticalHitRate(std::uint16_t a2) const { return a2 + GetCriticalHitRateAdvantage(a2); }
int cltPlayerAbility::GetMissRateAdvantage(std::uint16_t) const { return 0; }
int cltPlayerAbility::GetMissRate(std::uint16_t a2) const { return a2 + GetMissRateAdvantage(a2); }
int cltPlayerAbility::GetSkillAPowerAdvantage(std::uint16_t) const { return 0; }
int cltPlayerAbility::GetShopItemPriceAdvangtage() const { return 0; }
int cltPlayerAbility::GetSellingAgencyTaxAdvantage() const { return 0; }

void cltPlayerAbility::GetAPower(std::uint16_t* outMin, std::uint16_t* outMax, std::uint16_t, std::uint16_t, int, void*, int, int, std::uint16_t*, int) const {
    if (outMin) *outMin = static_cast<std::uint16_t>(m_baseStr);
    if (outMax) *outMax = static_cast<std::uint16_t>(m_baseStr);
}

int cltPlayerAbility::GetDPower(std::uint16_t, void*, int a4) const {
    return m_baseVit + a4;
}
