#include "System/cltSkillSystem.h"

#include <algorithm>

#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "System/cltClassSystem.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltLessonSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltQuickSlotSystem.h"
#include "System/cltTitleSystem.h"

cltItemKindInfo* cltSkillSystem::m_pclItemKindInfo = nullptr;
cltSkillKindInfo* cltSkillSystem::m_pclSkillKindInfo = nullptr;
cltClassKindInfo* cltSkillSystem::m_pclClassKindInfo = nullptr;
void (*cltSkillSystem::m_pOnAcquireCircleSkillFuncPtr)(unsigned int) = nullptr;

void cltSkillSystem::InitializeStaticVariable(
    cltSkillKindInfo* skillKindInfo,
    cltClassKindInfo* classKindInfo,
    cltItemKindInfo* itemKindInfo,
    void (*onAcquireCircleSkill)(unsigned int)) {
    m_pclSkillKindInfo = skillKindInfo;
    m_pclClassKindInfo = classKindInfo;
    m_pclItemKindInfo = itemKindInfo;
    m_pOnAcquireCircleSkillFuncPtr = onAcquireCircleSkill;
}

void cltSkillSystem::Initialize(
    cltQuickSlotSystem* quickSlotSystem,
    cltLessonSystem* lessonSystem,
    cltLevelSystem* levelSystem,
    cltClassSystem* classSystem,
    cltEquipmentSystem* equipmentSystem,
    cltPlayerAbility* playerAbility,
    cltTitleSystem* titleSystem) {
    m_pQuickSlotSystem = quickSlotSystem;
    m_pLessonSystem = lessonSystem;
    m_pLevelSystem = levelSystem;
    m_pClassSystem = classSystem;
    m_pEquipmentSystem = equipmentSystem;
    m_pPlayerAbility = playerAbility;
    m_pTitleSystem = titleSystem;
    m_userData1 = 0;
}

void cltSkillSystem::Initialize(
    cltQuickSlotSystem* quickSlotSystem,
    cltLessonSystem* lessonSystem,
    cltLevelSystem* levelSystem,
    cltClassSystem* classSystem,
    cltEquipmentSystem* equipmentSystem,
    cltPlayerAbility* playerAbility,
    cltTitleSystem* titleSystem,
    std::uint16_t skillNum,
    const std::uint16_t* skillKinds,
    unsigned int userData1) {
    Initialize(quickSlotSystem, lessonSystem, levelSystem, classSystem, equipmentSystem, playerAbility, titleSystem);
    m_userData1 = userData1;
    m_skillKinds.clear();
    m_skillValidity.clear();
    if (skillKinds && skillNum) {
        m_skillKinds.assign(skillKinds, skillKinds + skillNum);
        m_skillValidity.assign(skillNum, 0);
    }
    UpdateValidity();
}

void cltSkillSystem::Free() {
    m_skillKinds.clear();
    m_skillValidity.clear();
}

std::uint16_t cltSkillSystem::GetSkillNum() const { return static_cast<std::uint16_t>(m_skillKinds.size()); }

std::uint16_t cltSkillSystem::GetSkillKind(unsigned int index) const {
    if (index >= m_skillKinds.size()) {
        return 0;
    }
    return m_skillKinds[index];
}

int cltSkillSystem::GetClassActiveSkill(std::uint16_t* outSkillKinds, int* outAcquiredFlags) const {
    if (!m_pclSkillKindInfo || !m_pClassSystem || !outSkillKinds) {
        return 0;
    }
    const int count = m_pclSkillKindInfo->GetClassActiveSkill(m_pClassSystem->GetClass(), outSkillKinds);
    for (int i = 0; i < count; ++i) {
        if (outAcquiredFlags) {
            std::uint16_t acquired = 0;
            outAcquiredFlags[i] = IsAcquiredSkill(outSkillKinds[i], &acquired);
            if (outAcquiredFlags[i]) {
                outSkillKinds[i] = acquired;
            }
        }
    }
    return count;
}

int cltSkillSystem::GetClassPassiveSkill(std::uint16_t* outSkillKinds, int* outAcquiredFlags) const {
    if (!m_pclSkillKindInfo || !m_pClassSystem || !outSkillKinds) {
        return 0;
    }
    const int count = m_pclSkillKindInfo->GetClassPassiveSkill(m_pClassSystem->GetClass(), outSkillKinds);
    for (int i = 0; i < count; ++i) {
        if (outAcquiredFlags) {
            std::uint16_t acquired = 0;
            outAcquiredFlags[i] = IsAcquiredSkill(outSkillKinds[i], &acquired);
            if (outAcquiredFlags[i]) {
                outSkillKinds[i] = acquired;
            }
        }
    }
    return count;
}

int cltSkillSystem::FindSkillIndex(std::uint16_t skillKind) const {
    const auto it = std::find(m_skillKinds.begin(), m_skillKinds.end(), skillKind);
    if (it == m_skillKinds.end()) {
        return -1;
    }
    return static_cast<int>(it - m_skillKinds.begin());
}

void cltSkillSystem::AddSkill(std::uint16_t skillKind, int* replaced, std::uint16_t* replacedSkillKind) {
    int localReplaced = 0;
    auto* info = m_pclSkillKindInfo ? m_pclSkillKindInfo->GetSkillKindInfo(skillKind) : nullptr;
    if (!info) {
        if (replaced) {
            *replaced = 0;
        }
        return;
    }

    const auto prevSkill = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(info) + 70);
    if (prevSkill != 0) {
        const int idx = FindSkillIndex(prevSkill);
        if (idx >= 0) {
            localReplaced = 1;
            if (replacedSkillKind) {
                *replacedSkillKind = m_skillKinds[idx];
            }
            m_skillKinds[idx] = skillKind;
        } else {
            goto notify_only;
        }
    } else {
        if (!IsExistSkill(skillKind) && m_skillKinds.size() < 100) {
            m_skillKinds.push_back(skillKind);
            m_skillValidity.push_back(0);
        }
    }

notify_only:
    if (replaced) {
        *replaced = localReplaced;
    }
    if (m_pQuickSlotSystem) {
        m_pQuickSlotSystem->OnSkillAdded(skillKind);
    }
    if (cltSkillKindInfo::IsPassiveSkill(skillKind)) {
        const auto circleKind = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 256);
        if (circleKind != 0 && m_pOnAcquireCircleSkillFuncPtr) {
            m_pOnAcquireCircleSkillFuncPtr(static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(this)));
        }
    }
    if (m_pTitleSystem) {
        m_pTitleSystem->OnEvent_acquireskill(skillKind);
    }
    UpdateValidity();
}

bool cltSkillSystem::IsSkillCountOver() const {
    return m_skillKinds.size() >= 100;
}

int cltSkillSystem::IsExistSkill(std::uint16_t skillKind) const { return FindSkillIndex(skillKind) >= 0 ? 1 : 0; }

std::uint16_t cltSkillSystem::GetActiveSkill(std::uint16_t* out, int onlyValid) const {
    if (!out) return 0;
    std::uint16_t count = 0;
    for (std::size_t i = 0; i < m_skillKinds.size(); ++i) {
        const auto skill = m_skillKinds[i];
        if (!cltSkillKindInfo::IsActiveSkill(skill)) continue;
        if (onlyValid && (i >= m_skillValidity.size() || !m_skillValidity[i])) continue;
        out[count++] = skill;
    }
    return count;
}

std::uint16_t cltSkillSystem::GetPassiveSkill(std::uint16_t* out, int onlyValid, int workingOnly) const {
    if (!out) return 0;
    std::uint16_t count = 0;
    for (std::size_t i = 0; i < m_skillKinds.size(); ++i) {
        const auto skill = m_skillKinds[i];
        if (cltSkillKindInfo::IsActiveSkill(skill)) continue;
        if (onlyValid && (i >= m_skillValidity.size() || !m_skillValidity[i])) continue;
        if (!workingOnly) {
            auto* info = m_pclSkillKindInfo ? m_pclSkillKindInfo->GetSkillKindInfo_P(skill) : nullptr;
            if (!info) {
                continue;
            }
            if (*reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 196) != 0) {
                continue;
            }
        }
        out[count++] = skill;
    }
    return count;
}

std::uint16_t cltSkillSystem::GetPassiveSkill_Workingable(std::uint16_t* out) const {
    if (!out || !m_pclSkillKindInfo) {
        return 0;
    }
    std::uint16_t count = 0;
    for (std::size_t i = 0; i < m_skillKinds.size(); ++i) {
        const auto skill = m_skillKinds[i];
        if (cltSkillKindInfo::IsActiveSkill(skill)) {
            continue;
        }
        if (!m_skillValidity.empty() && !m_skillValidity[i]) {
            continue;
        }
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo_P(skill);
        if (!info) {
            continue;
        }
        const auto workingType = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 196);
        if (workingType != 0) {
            out[count++] = skill;
        }
    }
    return count;
}

int cltSkillSystem::IsValidSkill(std::uint16_t skillKind) const {
    const int idx = FindSkillIndex(skillKind);
    if (idx < 0 || static_cast<std::size_t>(idx) >= m_skillValidity.size()) {
        return 0;
    }
    return m_skillValidity[idx];
}

int cltSkillSystem::IsAcquiredSkill(std::uint16_t skillKind, std::uint16_t* acquiredKind) const {
    if (!m_pclSkillKindInfo) {
        return 0;
    }
    for (const auto ownedSkill : m_skillKinds) {
        if (ownedSkill == skillKind) {
            if (acquiredKind) {
                *acquiredKind = ownedSkill;
            }
            return 1;
        }

        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(ownedSkill);
        if (!info) {
            continue;
        }
        std::uint16_t prev = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(info) + 70);
        while (prev != 0) {
            stSkillKindInfo* prevInfo = m_pclSkillKindInfo->GetSkillKindInfo(prev);
            if (!prevInfo) {
                break;
            }
            if (*reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(prevInfo) + 0) == skillKind) {
                if (acquiredKind) {
                    *acquiredKind = ownedSkill;
                }
                return 1;
            }
            prev = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(prevInfo) + 70);
        }
    }
    return 0;
}

int cltSkillSystem::CanAcquireSkill(std::uint16_t skillKind) const {
    if (!m_pclSkillKindInfo || !m_pClassSystem) {
        return 0;
    }
    if (IsAcquiredSkill(skillKind, nullptr)) {
        return 0;
    }
    auto* info = m_pclSkillKindInfo->GetSkillKindInfo(skillKind);
    if (!info) {
        return 0;
    }
    const std::uint16_t reqLevel = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(info) + 68);
    if (m_pLevelSystem && m_pLevelSystem->GetLevel() < reqLevel) {
        return 0;
    }

    const std::uint16_t prevCode = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(info) + 70);
    if (prevCode != 0 && !IsAcquiredSkill(prevCode, nullptr)) {
        return 0;
    }

    if (m_pclClassKindInfo && m_pClassSystem) {
        strClassKindInfo* classInfo = m_pclClassKindInfo->GetClassKindInfo(m_pClassSystem->GetClass());
        const std::uint64_t reqClassMask = *reinterpret_cast<std::uint64_t*>(reinterpret_cast<unsigned char*>(info) + 40);
        if (reqClassMask != 0 && classInfo) {
            bool classMatched = false;
            for (strClassKindInfo* cur = classInfo; cur; cur = m_pclClassKindInfo->GetClassKindInfo(cur->from_class)) {
                if ((reqClassMask & cur->atb) != 0) {
                    classMatched = true;
                    break;
                }
            }
            if (!classMatched) {
                return 0;
            }
        }
    }

    return 1;
}

int cltSkillSystem::CanBuySkill(std::uint16_t skillKind) const {
    if (m_skillKinds.size() >= 100 || !CanAcquireSkill(skillKind) || !m_pclSkillKindInfo || !m_pLessonSystem || !m_pClassSystem) {
        return 0;
    }

    stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(skillKind);
    if (!info) {
        return 0;
    }

    const std::uint32_t needFig = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 52);
    const std::uint32_t needArc = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 56);
    const std::uint32_t needMag = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 60);
    const std::uint32_t needPri = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 64);

    return m_pLessonSystem->GetSwordLessonPt() >= needFig
        && m_pLessonSystem->GetBowLessonPt() >= needArc
        && m_pLessonSystem->GetTheologyLessonPt() >= needPri
        && m_pLessonSystem->GetMagicLessonPt() >= needMag;
}

void cltSkillSystem::BuySkill(std::uint16_t skillKind) {
    auto* info = m_pclSkillKindInfo ? m_pclSkillKindInfo->GetSkillKindInfo(skillKind) : nullptr;
    if (!info) {
        return;
    }
    if (!m_pLessonSystem) {
        return;
    }
    const auto needFig = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 52);
    const auto needArc = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 56);
    const auto needMag = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 60);
    const auto needPri = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 64);
    m_pLessonSystem->DecLessonPt_Sword(needFig);
    m_pLessonSystem->DecLessonPt_Bow(needArc);
    m_pLessonSystem->DecLessonPt_Theology(needPri);
    m_pLessonSystem->DecLessonPt_Magic(needMag);
    AddSkill(skillKind, nullptr, nullptr);
}

std::uint16_t cltSkillSystem::GetBuyAbleSkillList(unsigned int skillClass, std::uint16_t* outSkillKinds, int highClassOnly) const {
    if (!outSkillKinds || !m_pclSkillKindInfo) {
        return 0;
    }

    std::uint16_t writeCount = 0;
    const bool checkUpperClassOnly = (highClassOnly != 0);

    auto processList = [&](stSkillKindInfo* list, std::uint16_t count) {
        for (std::uint16_t i = 0; i < count; ++i) {
            stSkillKindInfo* rec = list + i;
            const auto buyType = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(rec) + 48);
            if (buyType != skillClass) {
                continue;
            }

            const auto reqClassMask = *reinterpret_cast<std::uint64_t*>(reinterpret_cast<unsigned char*>(rec) + 40);
            bool passClassRange = true;
            if (checkUpperClassOnly) {
                passClassRange = m_pclSkillKindInfo->IsTwoLevelClassSkill(reqClassMask) == 1
                    || m_pclSkillKindInfo->IsThreeLevelClassSkill(reqClassMask) == 1
                    || m_pclSkillKindInfo->IsFourLevelClassSkill(reqClassMask) == 1;
            } else if (reqClassMask != 0) {
                passClassRange = m_pclSkillKindInfo->IsUnderTwoLevelClassSkill(reqClassMask) == 1;
            }
            if (!passClassRange) {
                continue;
            }

            const auto skillKind = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(rec) + 0);
            const auto prevSkill = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(rec) + 70);
            if (prevSkill != 0 && !IsAcquiredSkill(prevSkill, nullptr)) {
                continue;
            }

            if (!m_pclSkillKindInfo->IsLastLevelSkill(skillKind) && IsAcquiredSkill(skillKind, nullptr)) {
                continue;
            }

            outSkillKinds[writeCount++] = skillKind;
        }
    };

    processList(m_pclSkillKindInfo->GetSkillKindInfo_P(), m_pclSkillKindInfo->GetPassiveSkillNum());
    processList(m_pclSkillKindInfo->GetSkillKindInfo_A(), m_pclSkillKindInfo->GetActiveSkillNum());

    return static_cast<std::uint16_t>(writeCount);
}

bool cltSkillSystem::IsAcquireSkill_Run() const {
    if (!m_pclSkillKindInfo) {
        return false;
    }
    return IsAcquiredSkill(cltSkillKindInfo::TranslateKindCode("P00001"), nullptr) == 1;
}

bool cltSkillSystem::IsActiveSkill(std::uint16_t skillKind) const {
    (void)this;
    return cltSkillKindInfo::IsActiveSkill(skillKind);
}

int cltSkillSystem::CanCreateCircle(int* outCircleKind) const {
    if (!outCircleKind || !m_pclSkillKindInfo) {
        return 0;
    }
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) {
            continue;
        }
        const auto circleKind = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 256);
        if (circleKind != 0) {
            *outCircleKind = static_cast<int>(circleKind);
            return 1;
        }
    }
    return 0;
}

unsigned int cltSkillSystem::GetUserData1() const {
    return m_userData1;
}

void cltSkillSystem::DeleteSkill(std::uint16_t skillKind) {
    const int idx = FindSkillIndex(skillKind);
    if (idx < 0) {
        return;
    }
    m_skillKinds.erase(m_skillKinds.begin() + idx);
    if (static_cast<std::size_t>(idx) < m_skillValidity.size()) {
        m_skillValidity.erase(m_skillValidity.begin() + idx);
    }
    if (m_pQuickSlotSystem) {
        m_pQuickSlotSystem->OnSkillDeleted(skillKind);
    }
    UpdateValidity();
}

static int sum_passive_valid(const cltSkillSystem* self, std::size_t dwordIndex) {
    if (!cltSkillSystem::m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t passives[100]{};
    const auto count = self->GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = cltSkillSystem::m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        sum += static_cast<int>(dw[dwordIndex]);
    }
    return sum;
}

std::uint16_t cltSkillSystem::GetMaxHPAdvantage_P() const { return static_cast<std::uint16_t>(sum_passive_valid(this, 28)); }
std::uint16_t cltSkillSystem::GetMaxHPAdvantage_A() const { return 0; }
std::uint16_t cltSkillSystem::GetMaxMPAdvantage_P() const { return static_cast<std::uint16_t>(sum_passive_valid(this, 29)); }
std::uint16_t cltSkillSystem::GetMaxMPAdvantage_A() const { return 0; }
std::uint16_t cltSkillSystem::GetMaxHPAdvantage() const { return static_cast<std::uint16_t>(GetMaxHPAdvantage_P() + GetMaxHPAdvantage_A()); }
std::uint16_t cltSkillSystem::GetMaxMPAdvantage() const { return static_cast<std::uint16_t>(GetMaxMPAdvantage_P() + GetMaxMPAdvantage_A()); }

int cltSkillSystem::GetAPowerAdvantage_P(int atb, int extraCount, std::uint16_t* extraKinds, int) const {
    if (!m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t kinds[200]{};
    auto count = GetPassiveSkill(kinds, 1, 0);
    if (extraKinds && extraCount > 0) {
        for (int i = 0; i < extraCount && count < 200; ++i) {
            stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(extraKinds[i]);
            if (info && *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(info) + 248) == 1) {
                kinds[count++] = extraKinds[i];
            }
        }
    }
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(kinds[i]);
        if (!info) {
            continue;
        }
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        sum += static_cast<int>(dw[6]);
        switch (atb) {
        case 0: sum += static_cast<int>(dw[33]); break;
        case 1: sum += static_cast<int>(dw[34]); break;
        case 2: sum += static_cast<int>(dw[35]); break;
        case 3: sum += static_cast<int>(dw[36]); break;
        case 4: sum += static_cast<int>(dw[37]); break;
        case 5: sum += static_cast<int>(dw[38]); break;
        case 6: sum += static_cast<int>(dw[39]); break;
        case 7: sum += static_cast<int>(dw[40]); break;
        default: break;
        }
    }
    return sum;
}
std::uint16_t cltSkillSystem::GetAPowerAdvantage_A() const { return 0; }
int cltSkillSystem::GetAPowerAdvantage(int a2) const { return GetAPowerAdvantage_P(a2, 0, nullptr, 0) + GetAPowerAdvantage_A(); }
std::uint16_t cltSkillSystem::GetDPowerAdvantage_P(int atb) const {
    if (!m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) {
            continue;
        }
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        sum += static_cast<int>(dw[7]);
        switch (atb) {
        case 0: sum += static_cast<int>(dw[41]); break;
        case 1: sum += static_cast<int>(dw[42]); break;
        case 2: sum += static_cast<int>(dw[43]); break;
        case 3: sum += static_cast<int>(dw[44]); break;
        case 4: sum += static_cast<int>(dw[45]); break;
        case 5: sum += static_cast<int>(dw[46]); break;
        case 6: sum += static_cast<int>(dw[47]); break;
        case 7: sum += static_cast<int>(dw[48]); break;
        default: break;
        }
    }
    return static_cast<std::uint16_t>(sum);
}
std::uint16_t cltSkillSystem::GetDPowerAdvantage() const { return GetDPowerAdvantage_P(-1); }
int cltSkillSystem::GetHitRateAdvantage() const { return sum_passive_valid(this, 32); }
int cltSkillSystem::GetCriticalRateAdvantage() const { return sum_passive_valid(this, 33); }
int cltSkillSystem::GetSkillAPowerAdvantage() const { return sum_passive_valid(this, 34); }
int cltSkillSystem::GetMissRateAdvantage() const {
    if (!m_pclSkillKindInfo) {
        return 0;
    }
    int value = 0;
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[67]) {
            value = static_cast<int>(dw[67]);
        }
    }
    return value;
}
int cltSkillSystem::GetPartyAPowerAdvantage(void* party) const {
    if (!party || !m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[68] || dw[71] || dw[74] || dw[77]) {
            sum += static_cast<int>(dw[71]);
        }
    }
    return sum;
}
int cltSkillSystem::GetPartyDPowerAdvantage(void* party) const {
    if (!party || !m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[69] || dw[72] || dw[75] || dw[78]) {
            sum += static_cast<int>(dw[72]);
        }
    }
    return sum;
}
int cltSkillSystem::GetPartyHitRateAdvantage(void* party) const {
    if (!party || !m_pclSkillKindInfo) {
        return 0;
    }
    int sum = 0;
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[70] || dw[73] || dw[76] || dw[79]) {
            sum += static_cast<int>(dw[73]);
        }
    }
    return sum;
}
int cltSkillSystem::GetAutoRecoverHPAdvantage() const { return sum_passive_valid(this, 65); }
int cltSkillSystem::GetAutoRecoverManaAdvantage() const { return sum_passive_valid(this, 66); }
int cltSkillSystem::GetSkillManaAdvantage() const { return sum_passive_valid(this, 90); }
int cltSkillSystem::GetAddBuffNum() const { return sum_passive_valid(this, 91); }

static bool get_max_state_info(const cltSkillSystem* self, std::size_t typeIdx, std::size_t chanceIdx, std::size_t durIdx,
    int* a2, int* a3, int a4, std::uint16_t* a5) {
    int bestChance = 0;
    int bestDur = 0;
    std::uint16_t kinds[200]{};
    auto count = self->GetPassiveSkill(kinds, 1, 0);
    for (int i = 0; i < a4 && i < 100; ++i) {
        kinds[count++] = a5[i];
    }
    for (std::uint16_t i = 0; i < count; ++i) {
        stSkillKindInfo* info = cltSkillSystem::m_pclSkillKindInfo ? cltSkillSystem::m_pclSkillKindInfo->GetSkillKindInfo(kinds[i]) : nullptr;
        if (!info) continue;
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[typeIdx] == 1 && static_cast<int>(dw[chanceIdx]) >= bestChance && dw[chanceIdx] > 0 && dw[durIdx] > 0) {
            bestChance = static_cast<int>(dw[chanceIdx]);
            bestDur = static_cast<int>(dw[durIdx]);
        }
    }
    if (a2) *a2 = bestChance;
    if (a3) *a3 = bestDur;
    return bestChance != 0;
}

bool cltSkillSystem::GetMaxFaintingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const { return get_max_state_info(this, 62, 82, 83, a2, a3, a4, a5); }
bool cltSkillSystem::GetMaxConfusionInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const { return get_max_state_info(this, 62, 84, 85, a2, a3, a4, a5); }
bool cltSkillSystem::GetMaxFreezingInfo(int* a2, int* a3, int a4, std::uint16_t* a5) const { return get_max_state_info(this, 62, 86, 87, a2, a3, a4, a5); }
std::uint16_t cltSkillSystem::GetGeneralPartyAdvantageSkillKind() const {
    if (!m_pclSkillKindInfo) {
        return 0;
    }
    std::uint16_t passives[100]{};
    const auto count = GetPassiveSkill(passives, 1, 0);
    for (std::uint16_t i = 0; i < count; ++i) {
        auto* info = m_pclSkillKindInfo->GetSkillKindInfo(passives[i]);
        if (!info) {
            continue;
        }
        const auto* dw = reinterpret_cast<const std::uint32_t*>(reinterpret_cast<const unsigned char*>(info));
        if (dw[68] || dw[71] || dw[74] || dw[77] || dw[69] || dw[72] || dw[75] || dw[78]) {
            return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(info) + 0);
        }
    }
    return 0;
}
int cltSkillSystem::MT_GetDPowerAdvantage() const { return GetDPowerAdvantage(); }
int cltSkillSystem::MT_GetAPowerAdvantage() const { return GetAPowerAdvantage(0); }
int cltSkillSystem::MT_GetHitRateAdvantage() const { return GetHitRateAdvantage(); }
int cltSkillSystem::MT_GetMissRateAdvantage() const { return GetMissRateAdvantage(); }
int cltSkillSystem::MT_GetCriticalHitRateAdvantage() const { return GetCriticalRateAdvantage(); }

void cltSkillSystem::UpdateValidity() {
    m_skillValidity.assign(m_skillKinds.size(), 0);
    if (!m_pclSkillKindInfo || !m_pclClassKindInfo || !m_pClassSystem) {
        return;
    }

    strClassKindInfo* classInfo = m_pclClassKindInfo->GetClassKindInfo(m_pClassSystem->GetClass());

    for (std::size_t i = 0; i < m_skillKinds.size(); ++i) {
        stSkillKindInfo* skillInfo = m_pclSkillKindInfo->GetSkillKindInfo(m_skillKinds[i]);
        if (!skillInfo) {
            continue;
        }

        bool classValid = true;
        const std::uint64_t reqClassMask = *reinterpret_cast<std::uint64_t*>(reinterpret_cast<unsigned char*>(skillInfo) + 40);
        if (reqClassMask != 0 && classInfo) {
            classValid = false;
            for (strClassKindInfo* cur = classInfo; cur; cur = m_pclClassKindInfo->GetClassKindInfo(cur->from_class)) {
                if ((reqClassMask & cur->atb) != 0) {
                    classValid = true;
                    break;
                }
            }
        }

        if (!classValid) {
            continue;
        }

        bool weaponValid = true;
        const std::uint32_t reqWeaponCount = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<unsigned char*>(skillInfo) + 88);
        if (reqWeaponCount && m_pEquipmentSystem && m_pclItemKindInfo) {
            weaponValid = false;
            const unsigned int refEquipKind[16] = { 4, 5, 6, 7, 8, 9, 10, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
            for (int reqIdx = 0; reqIdx < 16; ++reqIdx) {
                if (*reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(skillInfo) + 72 + reqIdx) == 0) {
                    continue;
                }
                const auto slot = refEquipKind[reqIdx];
                const auto itemKind = m_pEquipmentSystem->GetEquipItem(1, slot);
                if (!itemKind || !m_pEquipmentSystem->IsEquipItemValidity(1, slot)) {
                    continue;
                }
                auto* itemInfo = m_pclItemKindInfo->GetItemKindInfo(itemKind);
                if (!itemInfo) {
                    continue;
                }
                const auto itemTypeAt84 = *reinterpret_cast<std::uint16_t*>(reinterpret_cast<unsigned char*>(itemInfo) + 84);
                if (itemTypeAt84 == reqIdx) {
                    weaponValid = true;
                    break;
                }
            }
        }

        m_skillValidity[i] = weaponValid ? 1 : 0;
    }
}
