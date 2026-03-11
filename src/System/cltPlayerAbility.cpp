#include "System/cltPlayerAbility.h"

#include <algorithm>

#include "System/cltLevelSystem.h"
#include "System/cltSkillSystem.h"

class cltPartySystem {
public:
    int GetStrAdvantage(int, int, int);
    int GetDexAdvantage(int, int, int);
    int GetIntAdvantage(int, int, int);
    int GetVitAdvantage(int, int, int);
    int GetAPowerRateAdvantage(int, int, int);
    int GetDPowerRateAdvantage(int, int, int);
    int GetHitRateAdvantage(int, int, int);
    int GetExpAdvantage(int, int, int);
};

class cltEmblemSystem {
public:
    int GetAPowerAdvantage(int);
    int GetDPowerAdvantage();
    int GetShopItemPriceAdvantage();
    int GetSellingAgencyTaxAdvantage();
    int GetDeadPenaltyExpAdvantage();
    int GetExpAdvantage();
    int GetFaintingInfo(int*, int*);
    int GetConfusionInfo(int*, int*);
    int GetFreezingInfo(int*, int*);
};

class cltMonsterToleranceSystem {
public:
    int GetAPowerAdvantage(std::uint16_t);
    int GetDPowerAdvantage(std::uint16_t);
    int GetSkillAPowerAdvantage(std::uint16_t);
};

class cltRestBonusSystem {
public:
    int GetExpAdvantage();
};

class cltPetSystem {
public:
    int GetSTRAdvantage();
    int GetDEXAdvantage();
    int GetINTAdvantage();
    int GetVITAdvantage();
    int GetAPowerAdvantage();
    int GetDPowerAdvantage();
    int GetAttackSpeedAdvantage();
    int GetDropRateAdvantage();
    int GetSkillAPowerAdvantage(std::uint16_t);
};

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
    CPlayerSpirit* playerSpirit,
    cltEmblemSystem* emblemSystem,
    cltMonsterToleranceSystem* monsterToleranceSystem,
    cltRestBonusSystem* restBonusSystem,
    cltPetSystem* petSystem,
    cltMyItemSystem* myItemSystem,
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
    m_pPlayerSpirit = playerSpirit;
    m_pEmblemSystem = emblemSystem;
    m_pMonsterToleranceSystem = monsterToleranceSystem;
    m_pRestBonusSystem = restBonusSystem;
    m_pPetSystem = petSystem;
    m_pMyItemSystem = myItemSystem;

    m_baseStr = m_initialStr = str;
    m_baseDex = m_initialDex = dex;
    m_baseInt = m_initialInt = intel;
    m_baseVit = m_initialVit = vit;
    m_bonusPoint = bonusPoint;
    m_hp = max(1, hp);
    m_mana = max(0, mp);
}

void cltPlayerAbility::Free() {
    m_pLevelSystem = nullptr;
    m_pSkillSystem = nullptr;
    m_pEquipmentSystem = nullptr;
    m_pClassSystem = nullptr;
    m_pUsingItemSystem = nullptr;
    m_pUsingSkillSystem = nullptr;
    m_pWorkingPassiveSkillSystem = nullptr;
    m_pPlayerSpirit = nullptr;
    m_pEmblemSystem = nullptr;
    m_pMonsterToleranceSystem = nullptr;
    m_pRestBonusSystem = nullptr;
    m_pPetSystem = nullptr;
    m_pMyItemSystem = nullptr;
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

std::uint16_t cltPlayerAbility::GetBaseStr() const { return m_baseStr; }
std::uint16_t cltPlayerAbility::GetBaseDex() const { return m_baseDex; }
std::uint16_t cltPlayerAbility::GetBaseInt() const { return m_baseInt; }
std::uint16_t cltPlayerAbility::GetBaseVit() const { return m_baseVit; }

void cltPlayerAbility::SetMana(int mana) { m_mana = max(mana, 0); }
int cltPlayerAbility::GetMP() const { return m_mana; }

void cltPlayerAbility::DecreaseMP(int value) {
    if (value <= 0) {
        return;
    }
    m_mana = max(0, m_mana - value);
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
    m_baseStr = static_cast<std::uint16_t>(min(0xFFFF, static_cast<int>(m_baseStr) + value));
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
    m_baseDex = static_cast<std::uint16_t>(min(0xFFFF, static_cast<int>(m_baseDex) + value));
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
    m_baseInt = static_cast<std::uint16_t>(min(0xFFFF, static_cast<int>(m_baseInt) + value));
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
    m_baseVit = static_cast<std::uint16_t>(min(0xFFFF, static_cast<int>(m_baseVit) + value));
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

int cltPlayerAbility::CanResetAbility() const {
    return m_baseStr != m_initialStr
        || m_baseDex != m_initialDex
        || m_baseInt != m_initialInt
        || m_baseVit != m_initialVit;
}

int cltPlayerAbility::GetNeedManaForUsingSkill(int baseSkillMana) const {
    if (!m_pSkillSystem) {
        return baseSkillMana;
    }
    const int adv = m_pSkillSystem->GetSkillManaAdvantage();
    if (!adv) {
        return baseSkillMana;
    }
    const int need = baseSkillMana * adv / -100 + baseSkillMana;
    return max(1, need);
}

int cltPlayerAbility::GetBuffNum() const { return m_buffNum; }
int cltPlayerAbility::GetMaxBuffNum() const { return m_maxBuffNum; }
bool cltPlayerAbility::CanAddBuff() const { return m_buffNum < m_maxBuffNum; }


std::uint16_t cltPlayerAbility::GetStr(int, void* party) const {
    if (!m_pEquipmentSystem || !m_pUsingItemSystem || !m_pUsingSkillSystem) {
        return 0;
    }
    const auto p = reinterpret_cast<cltPartySystem*>(party);
    const int partyAdv = p ? p->GetStrAdvantage(0, 0, 0) : 0;
    const int petAdv = m_pPetSystem ? m_pPetSystem->GetSTRAdvantage() : 0;
    return static_cast<std::uint16_t>(m_baseStr + m_pEquipmentSystem->GetTotalStr() + m_pUsingItemSystem->GetTotalStrOfUsingItem()
        + m_pUsingSkillSystem->GetTotalStrOfUsingSkill() + partyAdv + petAdv);
}
std::uint16_t cltPlayerAbility::GetDex(int, void* party) const {
    if (!m_pEquipmentSystem || !m_pUsingItemSystem || !m_pUsingSkillSystem) {
        return 0;
    }
    const auto p = reinterpret_cast<cltPartySystem*>(party);
    const int partyAdv = p ? p->GetDexAdvantage(0, 0, 0) : 0;
    const int petAdv = m_pPetSystem ? m_pPetSystem->GetDEXAdvantage() : 0;
    return static_cast<std::uint16_t>(m_baseDex + m_pEquipmentSystem->GetTotalDex() + m_pUsingItemSystem->GetTotalDexOfUsingItem()
        + m_pUsingSkillSystem->GetTotalDexOfUsingSkill() + partyAdv + petAdv);
}
std::uint16_t cltPlayerAbility::GetInt(int, void* party) const {
    if (!m_pEquipmentSystem || !m_pUsingItemSystem || !m_pUsingSkillSystem) {
        return 0;
    }
    const auto p = reinterpret_cast<cltPartySystem*>(party);
    const int partyAdv = p ? p->GetIntAdvantage(0, 0, 0) : 0;
    const int petAdv = m_pPetSystem ? m_pPetSystem->GetINTAdvantage() : 0;
    return static_cast<std::uint16_t>(m_baseInt + m_pEquipmentSystem->GetTotalInt() + m_pUsingItemSystem->GetTotalIntOfUsingItem()
        + m_pUsingSkillSystem->GetTotalIntOfUsingSkill() + partyAdv + petAdv);
}
std::uint16_t cltPlayerAbility::GetVit(int, void* party) const {
    if (!m_pEquipmentSystem || !m_pUsingItemSystem || !m_pUsingSkillSystem) {
        return 0;
    }
    const auto p = reinterpret_cast<cltPartySystem*>(party);
    const int partyAdv = p ? p->GetVitAdvantage(0, 0, 0) : 0;
    const int petAdv = m_pPetSystem ? m_pPetSystem->GetVITAdvantage() : 0;
    return static_cast<std::uint16_t>(m_baseVit + m_pEquipmentSystem->GetTotalVit() + m_pUsingItemSystem->GetTotalVitOfUsingItem()
        + m_pUsingSkillSystem->GetTotalVitOfUsingSkill() + partyAdv + petAdv);
}

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
int cltPlayerAbility::GetAttackSpeedAdvantage() const {
    if (!m_pUsingSkillSystem || !m_pWorkingPassiveSkillSystem) {
        return 0;
    }
    return m_pUsingSkillSystem->GetAttackSpeedAdvantage() + m_pWorkingPassiveSkillSystem->GetAttackSpeedAdvantage()
        + (m_pPetSystem ? m_pPetSystem->GetAttackSpeedAdvantage() : 0);
}
int cltPlayerAbility::GetDamageHP2ManaRate() const {
    if (m_pUsingSkillSystem && m_pUsingSkillSystem->IsActiveManashieldSkill() == 1) {
        return 100;
    }
    return m_pWorkingPassiveSkillSystem ? m_pWorkingPassiveSkillSystem->GetDamageHP2ManaRate() : 0;
}
int cltPlayerAbility::GetDropRateAdvantage() const { return m_pPetSystem ? m_pPetSystem->GetDropRateAdvantage() : 0; }
int cltPlayerAbility::GetDeadPenaltyExpAdvantage() const { return m_pEmblemSystem ? m_pEmblemSystem->GetDeadPenaltyExpAdvantage() : 0; }
int cltPlayerAbility::GetExpAdvantage(void* party) const {
    int v = 0;
    if (m_pEquipmentSystem) v += m_pEquipmentSystem->GetExpAdvantageByEquipItem();
    if (m_pEmblemSystem) v += m_pEmblemSystem->GetExpAdvantage();
    if (m_pRestBonusSystem) v += m_pRestBonusSystem->GetExpAdvantage();
    if (m_pUsingSkillSystem) v += m_pUsingSkillSystem->GetExpAdvantage(party != nullptr ? 1 : 0);
    if (party) v += reinterpret_cast<cltPartySystem*>(party)->GetExpAdvantage(0, 0, 0);
    return v;
}
int cltPlayerAbility::IsActiveFastRun() const {
    if (m_pUsingItemSystem && m_pUsingItemSystem->IsActiveFastRun() == 1) {
        return 1;
    }
    return m_pUsingSkillSystem && m_pUsingSkillSystem->IsActiveFastRunSkill() == 1;
}
int cltPlayerAbility::IsActiveNonDelayAttack() const { return m_pUsingSkillSystem && m_pUsingSkillSystem->IsActiveNonDelayAttack() == 1; }
int cltPlayerAbility::GetAttackAtb(int) const { return m_pEquipmentSystem ? m_pEquipmentSystem->GetWeaponAttackAtb() : 0; }

int cltPlayerAbility::GetItemRecoverHPAdvantage() const { return 0; }
int cltPlayerAbility::GetItemRecoverManaAdvantage() const { return 0; }
int cltPlayerAbility::GetMagicResistRateAdvantage() const { return 0; }
int cltPlayerAbility::GetMagicResistRate() const { return 0; }
int cltPlayerAbility::GetPartyAPowerAdvantage(void*) const { return m_pSkillSystem ? m_pSkillSystem->GetPartyAPowerAdvantage(nullptr) : 0; }
int cltPlayerAbility::GetPartyDPowerAdvantage(void*) const { return m_pSkillSystem ? m_pSkillSystem->GetPartyDPowerAdvantage(nullptr) : 0; }
int cltPlayerAbility::GetPartyHitRateAdvantage(void*) const { return m_pSkillSystem ? m_pSkillSystem->GetPartyHitRateAdvantage(nullptr) : 0; }

bool cltPlayerAbility::GetMaxFaintingInfo(int* a2, int* a3, int, std::uint16_t*) const {
    int v1 = 0, d1 = 0, v2 = 0, d2 = 0, v3 = 0, d3 = 0;
    if (m_pSkillSystem) m_pSkillSystem->GetMaxFaintingInfo(&v1, &d1, 0, nullptr);
    if (m_pUsingSkillSystem) m_pUsingSkillSystem->GetMaxFaintingInfo(&v2, &d2);
    if (m_pEmblemSystem) m_pEmblemSystem->GetFaintingInfo(&v3, &d3);
    int bestV = v1, bestD = d1;
    if (bestV < v2) { bestV = v2; bestD = d2; }
    if (bestV < v3) { bestV = v3; bestD = d3; }
    if (a2) *a2 = bestV;
    if (a3) *a3 = bestD;
    return bestV != 0;
}

bool cltPlayerAbility::GetMaxConfusionInfo(int* a2, int* a3, int, std::uint16_t*) const {
    int v1 = 0, d1 = 0, v2 = 0, d2 = 0, v3 = 0, d3 = 0;
    if (m_pSkillSystem) m_pSkillSystem->GetMaxConfusionInfo(&v1, &d1, 0, nullptr);
    if (m_pUsingSkillSystem) m_pUsingSkillSystem->GetMaxConfusionInfo(&v2, &d2);
    if (m_pEmblemSystem) m_pEmblemSystem->GetConfusionInfo(&v3, &d3);
    int bestV = v1, bestD = d1;
    if (bestV < v2) { bestV = v2; bestD = d2; }
    if (bestV < v3) { bestV = v3; bestD = d3; }
    if (a2) *a2 = bestV;
    if (a3) *a3 = bestD;
    return bestV != 0;
}

bool cltPlayerAbility::GetMaxFreezingInfo(int* a2, int* a3, int, std::uint16_t*) const {
    int v1 = 0, d1 = 0, v2 = 0, d2 = 0, v3 = 0, d3 = 0;
    if (m_pSkillSystem) m_pSkillSystem->GetMaxFreezingInfo(&v1, &d1, 0, nullptr);
    if (m_pUsingSkillSystem) m_pUsingSkillSystem->GetMaxFreezingInfo(&v2, &d2);
    if (m_pEmblemSystem) m_pEmblemSystem->GetFreezingInfo(&v3, &d3);
    int bestV = v1, bestD = d1;
    if (bestV < v2) { bestV = v2; bestD = d2; }
    if (bestV < v3) { bestV = v3; bestD = d3; }
    if (a2) *a2 = bestV;
    if (a3) *a3 = bestD;
    return bestV != 0;
}


int cltPlayerAbility::GetHitRateAdvantage(std::uint16_t, int a3, void*) const { return a3; }
int cltPlayerAbility::GetHitRate(std::uint16_t a2, void* party) const { return GetHitRateAdvantage(0, 3 * a2, party); }
int cltPlayerAbility::GetCriticalHitRateAdvantage(std::uint16_t) const {
    return m_pSkillSystem ? m_pSkillSystem->GetCriticalRateAdvantage() : 0;
}
int cltPlayerAbility::GetCriticalHitRate(std::uint16_t a2) const { return a2 + GetCriticalHitRateAdvantage(a2); }
int cltPlayerAbility::GetMissRateAdvantage(std::uint16_t) const { return m_pSkillSystem ? m_pSkillSystem->GetMissRateAdvantage() : 0; }
int cltPlayerAbility::GetMissRate(std::uint16_t a2) const { return a2 + GetMissRateAdvantage(a2); }
int cltPlayerAbility::GetSkillAPowerAdvantage(std::uint16_t charKind) const {
    int v = 0;
    if (m_pEquipmentSystem) v += m_pEquipmentSystem->GetSkillAPowerAdvantage();
    if (m_pMonsterToleranceSystem) v += m_pMonsterToleranceSystem->GetSkillAPowerAdvantage(charKind);
    if (m_pPetSystem) v += m_pPetSystem->GetSkillAPowerAdvantage(charKind);
    return v;
}
int cltPlayerAbility::GetShopItemPriceAdvangtage() const { return m_pEmblemSystem ? m_pEmblemSystem->GetShopItemPriceAdvantage() : 0; }
int cltPlayerAbility::GetSellingAgencyTaxAdvantage() const { return m_pEmblemSystem ? m_pEmblemSystem->GetSellingAgencyTaxAdvantage() : 0; }

void cltPlayerAbility::GetAPower(std::uint16_t* outMin, std::uint16_t* outMax, std::uint16_t, std::uint16_t, int, void*, int, int, std::uint16_t*, int) const {
    std::uint16_t eMin = 0, eMax = 0;
    if (m_pEquipmentSystem) {
        m_pEquipmentSystem->GetAPower(&eMin, &eMax);
    }
    if (outMin) *outMin = eMin;
    if (outMax) *outMax = eMax;
}

int cltPlayerAbility::GetDPower(std::uint16_t, void*, int a4) const {
    const int base = m_pEquipmentSystem ? m_pEquipmentSystem->GetDPower() : 0;
    return (base + base * a4 / 1000) / 3;
}
