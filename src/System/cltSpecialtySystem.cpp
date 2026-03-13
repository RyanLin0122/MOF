#include "System/cltSpecialtySystem.h"

#include <algorithm>
#include <cstring>

#include "Info/cltSpecialtyKindInfo.h"
#include "Network/CMofMsg.h"
#include "System/cltGradeSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltSkillSystem.h"

cltSpecialtyKindInfo* cltSpecialtySystem::m_pclSpecialtyKindInfo = nullptr;

void cltSpecialtySystem::InitializeStaticVariable(cltSpecialtyKindInfo* specialtyKindInfo) {
    m_pclSpecialtyKindInfo = specialtyKindInfo;
}

cltSpecialtySystem::cltSpecialtySystem() {
    m_pGradeSystem = nullptr;
    m_pLevelSystem = nullptr;
    m_specialtyPt = 0;
    std::memset(m_acquiredSpecialtyKinds.data(), 0, sizeof(std::uint16_t) * m_acquiredSpecialtyKinds.size());
    m_acquiredSpecialtyNum = 0;
}

cltSpecialtySystem::~cltSpecialtySystem() {
    Free();
}

void cltSpecialtySystem::Initialize(cltGradeSystem* gradeSystem, cltSkillSystem* skillSystem, cltLevelSystem* levelSystem,
                                    cltQuestSystem* questSystem, std::int16_t specialtyPt,
                                    std::uint16_t acquiredSpecialtyNum, const std::uint16_t* acquiredSpecialtyKind) {
    m_pGradeSystem = gradeSystem;
    m_pLevelSystem = levelSystem;
    m_specialtyPt = specialtyPt;
    m_acquiredSpecialtyNum = acquiredSpecialtyNum;
    m_pSkillSystem = skillSystem;
    m_pQuestSystem = questSystem;
    std::memcpy(m_acquiredSpecialtyKinds.data(), acquiredSpecialtyKind,
                sizeof(std::uint16_t) * std::min<std::size_t>(acquiredSpecialtyNum, m_acquiredSpecialtyKinds.size()));
}

void cltSpecialtySystem::Free() {
    m_pGradeSystem = nullptr;
    m_pLevelSystem = nullptr;
    m_specialtyPt = 0;
    std::memset(m_acquiredSpecialtyKinds.data(), 0, sizeof(std::uint16_t) * m_acquiredSpecialtyKinds.size());
    m_acquiredSpecialtyNum = 0;
}

std::int16_t cltSpecialtySystem::GetSpecialtyPt() { return m_specialtyPt; }
void cltSpecialtySystem::IncreaseSpecialtyPt(std::int16_t value) { m_specialtyPt = static_cast<std::int16_t>(m_specialtyPt + value); }
void cltSpecialtySystem::DecreaseSpecialtyPt(std::int16_t value) { m_specialtyPt = static_cast<std::int16_t>(m_specialtyPt - value); }

std::uint16_t cltSpecialtySystem::GetAcquiredSpecialtyNum() { return m_acquiredSpecialtyNum; }
std::uint16_t* cltSpecialtySystem::GetAcquiredSpecialtyKind() { return m_acquiredSpecialtyKinds.data(); }

int cltSpecialtySystem::CanAcquireSpecialty(std::uint16_t specialtyKind) {
    if (m_acquiredSpecialtyNum >= 30) return 0;

    auto* info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(specialtyKind) : nullptr;
    if (!info) return 0;

    if (IsAcquiredSpecialty(specialtyKind) == 1) return 0;
    if (info->byRequiredSpecialtyPt > m_specialtyPt) return 0;
    if (m_pGradeSystem && info->byRequiredGrade > m_pGradeSystem->GetGrade()) return 0;
    if (m_pLevelSystem && info->dwRequiredLevel > m_pLevelSystem->GetLevel()) return 0;

    const std::uint16_t req = info->wRequiredSpecialtyKind;
    if (!req || IsAcquiredSpecialty(req)) return 1;
    return 0;
}

int cltSpecialtySystem::IsAcquiredSpecialty(std::uint16_t specialtyKind) {
    if (!m_acquiredSpecialtyNum) return 0;

    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (m_acquiredSpecialtyKinds[i] == specialtyKind) return 1;

        auto* info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(m_acquiredSpecialtyKinds[i]) : nullptr;
        if (!info) continue;

        for (std::uint16_t parent = info->wRequiredSpecialtyKind; parent != 0;) {
            auto* parentInfo = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(parent) : nullptr;
            if (!parentInfo) break;
            if (parentInfo->wKind == specialtyKind) return 1;
            parent = parentInfo->wRequiredSpecialtyKind;
        }
    }
    return 0;
}

void cltSpecialtySystem::AcquireSpecialty(std::uint16_t specialtyKind, std::uint16_t* outSkillNum,
                                          std::uint16_t* outSkillKinds, std::uint16_t* outQuestKinds,
                                          unsigned int* outQuestValues) {
    std::uint16_t skillCount = 0;
    auto* info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(specialtyKind) : nullptr;
    if (!info) {
        if (outSkillNum) *outSkillNum = 0;
        return;
    }

    const std::uint16_t req = info->wRequiredSpecialtyKind;
    if (req) {
        for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
            if (m_acquiredSpecialtyKinds[i] == req) {
                m_acquiredSpecialtyKinds[i] = specialtyKind;
                break;
            }
        }
    } else if (m_acquiredSpecialtyNum < m_acquiredSpecialtyKinds.size()) {
        m_acquiredSpecialtyKinds[m_acquiredSpecialtyNum++] = specialtyKind;
    }

    DecreaseSpecialtyPt(info->byRequiredSpecialtyPt);

    for (int i = 0; i < 5; ++i) {
        const std::uint16_t skillKind = info->wAcquiredSkillKinds[i];
        if (!skillKind) break;
        if (m_pSkillSystem) m_pSkillSystem->AddSkill(skillKind, nullptr, nullptr);
        if (outSkillKinds) outSkillKinds[skillCount] = skillKind;
        ++skillCount;
    }

    if (outSkillNum) *outSkillNum = skillCount;
    if (m_pQuestSystem) m_pQuestSystem->CompleteFunctionQuest(16, outQuestKinds, outQuestValues);
}

int cltSpecialtySystem::GetAcquireAbleSpecialtyList(char category, std::uint16_t* outSpecialtyKindList) {
    int available = 0;
    const int count = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyList(category, outSpecialtyKindList) : 0;
    if (count <= 0) return 0;

    std::uint16_t* write = outSpecialtyKindList;
    for (int i = 0; i < count; ++i) {
        auto* info = m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(outSpecialtyKindList[i]);
        if (!info) continue;

        const std::uint16_t req = info->wRequiredSpecialtyKind;
        const bool reqOk = (!req || IsAcquiredSpecialty(req));
        const bool notAcquiredOrLast = (m_pclSpecialtyKindInfo->IsLastLevelSpecialty(info->wKind) || IsAcquiredSpecialty(info->wKind) != 1);
        if (reqOk && notAcquiredOrLast) {
            *write++ = info->wKind;
            ++available;
        }
    }

    return available;
}

void cltSpecialtySystem::FillOutSpecialtyInfo(CMofMsg* msg) {
    msg->Put_SHORT(m_specialtyPt);
    msg->Put_WORD(m_acquiredSpecialtyNum);
    msg->Put(reinterpret_cast<char*>(m_acquiredSpecialtyKinds.data()), 2 * m_acquiredSpecialtyNum);
}

std::uint16_t cltSpecialtySystem::GetAcquiredGenericSpecialty(std::uint16_t* outSpecialtyKinds) {
    std::uint16_t count = 0;
    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsGenericSpeciatly(m_acquiredSpecialtyKinds[i])) {
            outSpecialtyKinds[count++] = m_acquiredSpecialtyKinds[i];
        }
    }
    return count;
}

std::uint16_t cltSpecialtySystem::GetAcquiredMakingItemSpecialty(std::uint16_t* outSpecialtyKinds) {
    std::uint16_t count = 0;
    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsMakingItemSpecialty(m_acquiredSpecialtyKinds[i])) {
            outSpecialtyKinds[count++] = m_acquiredSpecialtyKinds[i];
        }
    }
    return count;
}

std::uint16_t cltSpecialtySystem::GetAcquiredTransformSpecialty(std::uint16_t* outSpecialtyKinds) {
    std::uint16_t count = 0;
    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsTransformSpecialty(m_acquiredSpecialtyKinds[i])) {
            outSpecialtyKinds[count++] = m_acquiredSpecialtyKinds[i];
        }
    }
    return count;
}

int cltSpecialtySystem::CanResetSpecialty(int includeCircle) {
    const std::int16_t n = m_acquiredSpecialtyNum;
    if (!n) return 0;
    return includeCircle || n != 1 || (m_pclSpecialtyKindInfo && !m_pclSpecialtyKindInfo->IsCircleSpecialty(m_acquiredSpecialtyKinds[0]));
}

int cltSpecialtySystem::CanResetCircleSpecialty(int allow) {
    if (!allow) return 0;
    if (!m_acquiredSpecialtyNum) return 0;

    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsCircleSpecialty(m_acquiredSpecialtyKinds[i])) return 1;
    }
    return 0;
}

void cltSpecialtySystem::ResetSpecialty(int includeCircle, int* outDeletedSpecialtyNum, std::uint16_t* outDeletedSpecialty,
                                        int* outDeletedSkillNum, std::uint16_t* outDeletedSkillKinds) {
    int circleIndex = -1;
    std::uint16_t preserve = 0;

    if (outDeletedSpecialtyNum) *outDeletedSpecialtyNum = 0;
    if (outDeletedSkillNum) *outDeletedSkillNum = 0;

    if (!includeCircle && m_acquiredSpecialtyNum) {
        for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
            if (m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsCircleSpecialty(m_acquiredSpecialtyKinds[i])) {
                circleIndex = i;
                preserve = m_acquiredSpecialtyKinds[i];
                break;
            }
        }
    }

    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        const auto specialty = m_acquiredSpecialtyKinds[i];
        if (specialty == preserve) continue;

        if (outDeletedSpecialtyNum) outDeletedSpecialty[(*outDeletedSpecialtyNum)++] = specialty;
        DeleteSpecialtySkill(specialty, outDeletedSkillNum, outDeletedSkillKinds);

        if (m_pclSpecialtyKindInfo) {
            const std::int16_t back = m_pclSpecialtyKindInfo->GetSpecialtyTotalPoint(specialty);
            IncreaseSpecialtyPt(back);
        }
    }

    if (preserve) {
        m_acquiredSpecialtyKinds[0] = preserve;
        m_acquiredSpecialtyNum = 1;
    } else {
        m_acquiredSpecialtyNum = 0;
    }
}

void cltSpecialtySystem::ResetCircleSpecialty(int* outDeletedSpecialtyNum, std::uint16_t* outDeletedSpecialty,
                                              int* outDeletedSkillNum, std::uint16_t* outDeletedSkillKinds) {
    if (outDeletedSpecialtyNum) *outDeletedSpecialtyNum = 0;
    if (outDeletedSkillNum) *outDeletedSkillNum = 0;

    if (!m_acquiredSpecialtyNum) return;

    for (std::uint16_t i = 0; i < m_acquiredSpecialtyNum; ++i) {
        if (!(m_pclSpecialtyKindInfo && m_pclSpecialtyKindInfo->IsCircleSpecialty(m_acquiredSpecialtyKinds[i]))) continue;

        const auto specialty = m_acquiredSpecialtyKinds[i];
        if (outDeletedSpecialtyNum) outDeletedSpecialty[(*outDeletedSpecialtyNum)++] = specialty;
        DeleteSpecialtySkill(specialty, outDeletedSkillNum, outDeletedSkillKinds);

        if (m_pclSpecialtyKindInfo) {
            const std::int16_t back = m_pclSpecialtyKindInfo->GetSpecialtyTotalPoint(specialty);
            IncreaseSpecialtyPt(back);
        }

        std::memmove(&m_acquiredSpecialtyKinds[i], &m_acquiredSpecialtyKinds[i + 1],
                     sizeof(std::uint16_t) * (m_acquiredSpecialtyNum - i - 1));
        --m_acquiredSpecialtyNum;
        return;
    }
}

void cltSpecialtySystem::DeleteSpecialtySkill(std::uint16_t specialtyKind, int* outDeletedSkillNum,
                                              std::uint16_t* outDeletedSkillKinds) {
    auto* info = m_pclSpecialtyKindInfo ? m_pclSpecialtyKindInfo->GetSpecialtyKindInfo(specialtyKind) : nullptr;
    if (!info) return;

    for (int i = 0; i < 5; ++i) {
        const std::uint16_t skillKind = info->wAcquiredSkillKinds[i];
        if (!skillKind) continue;

        if (outDeletedSkillNum) outDeletedSkillKinds[(*outDeletedSkillNum)++] = skillKind;
        if (m_pSkillSystem) m_pSkillSystem->DeleteSkill(skillKind);
    }
}
