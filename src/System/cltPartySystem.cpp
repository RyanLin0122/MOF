#include "System/cltPartySystem.h"

#include <algorithm>
#include <cstring>

void (*cltPartySystem::m_pOnPartyFreeFuncPtr)(cltPartySystem*) = nullptr;
void (*cltPartySystem::m_pOnPartyLeaderChangedFuncPtr)(cltPartySystem*) = nullptr;
cltUsingSkillSystem* (*cltPartySystem::m_pPartyGetUsingSkillFuncPtr)(void*) = nullptr;
cltWorkingPassiveSkillSystem* (*cltPartySystem::m_pPartyGetWPSSystemFuncPtr)(void*) = nullptr;
int (*cltPartySystem::m_pPartyGetAvrgLevelFuncPtr)(cltPartySystem*) = nullptr;
int (*cltPartySystem::m_pPartyGetLevelFuncPtr)(void*) = nullptr;
void (*cltPartySystem::m_pPartyJoinedNewPartyMemberFuncPtr)(cltPartySystem*) = nullptr;
void (*cltPartySystem::m_pPartyLeftPartyMemeberFuncPtr)(cltPartySystem*) = nullptr;

void cltPartySystem::InitializeStaticVariable(
    void (*a1)(cltPartySystem*),
    void (*a2)(cltPartySystem*),
    cltUsingSkillSystem* (*a3)(void*),
    cltWorkingPassiveSkillSystem* (*a4)(void*),
    int (*a5)(cltPartySystem*),
    int (*a6)(void*),
    void (*a7)(cltPartySystem*),
    void (*a8)(cltPartySystem*)) {
    m_pOnPartyFreeFuncPtr = a1;
    m_pOnPartyLeaderChangedFuncPtr = a2;
    m_pPartyGetUsingSkillFuncPtr = a3;
    m_pPartyGetWPSSystemFuncPtr = a4;
    m_pPartyGetAvrgLevelFuncPtr = a5;
    m_pPartyGetLevelFuncPtr = a6;
    m_pPartyJoinedNewPartyMemberFuncPtr = a7;
    m_pPartyLeftPartyMemeberFuncPtr = a8;
}

cltPartySystem::cltPartySystem() = default;
cltPartySystem::~cltPartySystem() = default;
cltPartySystem* cltPartySystem::vector_deleting_destructor(char) { return this; }

void cltPartySystem::Create(void* a2, void* a3) {
    members_[0] = a2;
    members_[1] = a3;
    members_[2] = nullptr;
    members_[3] = nullptr;
    members_[4] = nullptr;
    leaderIndex_ = 0;
    memberCount_ = 2;
    if (m_pOnPartyLeaderChangedFuncPtr) {
        m_pOnPartyLeaderChangedFuncPtr(this);
    }
}

void cltPartySystem::Create(void* const* a2, std::uint8_t a3) {
    std::memcpy(members_.data(), a2, 0x14u);
    leaderIndex_ = a3;
    memberCount_ = 0;
    for (void* m : members_) {
        if (m) ++memberCount_;
    }
    if (m_pOnPartyLeaderChangedFuncPtr) {
        m_pOnPartyLeaderChangedFuncPtr(this);
    }
}

int cltPartySystem::IsCreated() { return memberCount_ != 0; }

void cltPartySystem::Free() {
    members_.fill(nullptr);
    memberCount_ = 0;
    userData1_ = 0;
    apAdv_ = 0;
    dpAdv_ = 0;
    expAdv_ = 0;
}

unsigned int cltPartySystem::CanJoin(void* a2) {
    if (memberCount_ >= 5) {
        return 900;
    }
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        if (members_[i] == a2) {
            return 902;
        }
    }
    return 0;
}

void cltPartySystem::Join(void* a2) {
    if (memberCount_ < 5) {
        members_[memberCount_++] = a2;
        if (m_pPartyJoinedNewPartyMemberFuncPtr) {
            m_pPartyJoinedNewPartyMemberFuncPtr(this);
        }
    }
}

int cltPartySystem::CanLeave(void* a2) {
    for (std::uint8_t i = 0; i < 5; ++i) {
        if (members_[i] == a2) {
            return 1;
        }
    }
    return 0;
}

void cltPartySystem::Leave(void* a2) {
    const bool wasLeader = GetLeadInstance() == a2;
    int found = -1;
    for (int i = 0; i < memberCount_; ++i) {
        if (members_[i] == a2) {
            found = i;
            break;
        }
    }
    if (found < 0) return;

    for (int i = found; i + 1 < memberCount_; ++i) {
        members_[i] = members_[i + 1];
    }
    members_[memberCount_ - 1] = nullptr;
    --memberCount_;

    if (memberCount_ > 1) {
        if (wasLeader) {
            if (m_pOnPartyLeaderChangedFuncPtr) {
                m_pOnPartyLeaderChangedFuncPtr(this);
            }
        }
        if (m_pPartyLeftPartyMemeberFuncPtr) {
            m_pPartyLeftPartyMemeberFuncPtr(this);
        }
    } else {
        if (m_pOnPartyFreeFuncPtr) {
            m_pOnPartyFreeFuncPtr(this);
        }
        Free();
    }
}

void* cltPartySystem::GetLeadInstance() {
    if (!memberCount_) return nullptr;
    return members_[leaderIndex_];
}

std::uint8_t cltPartySystem::GetPartyMemberNum() { return memberCount_; }

void* cltPartySystem::GetPartyInstance(int a2) {
    if (a2 >= memberCount_) return nullptr;
    return members_[a2];
}

void cltPartySystem::SetUserData1(unsigned int a2) { userData1_ = a2; }
unsigned int cltPartySystem::GetUserData1() { return userData1_; }
void cltPartySystem::SetUserData2(unsigned int a2) { userData2_ = a2; }
unsigned int cltPartySystem::GetUserData2() { return userData2_; }

void cltPartySystem::SetAdvantages(int a2, int a3, int a4) {
    apAdv_ = a2;
    dpAdv_ = a3;
    expAdv_ = a4;
}

int cltPartySystem::GetAPowerRateAdvantage(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr || !m_pPartyGetWPSSystemFuncPtr) return apAdv_;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        auto* wps = m_pPartyGetWPSSystemFuncPtr(members_[i]);
        if (!use || !wps) continue;
        int nUse = 0, nWps = 0;
        std::uint16_t idsUse[10]{}, idsWps[10]{};
        const int v = use->GetPartyMemberAPowerAdvantage(&nUse, idsUse) + wps->GetPartyMemberAPowerAdvantage(&nWps, idsWps);
        if (v > best) {
            best = v;
            if (a2) *a2 = members_[i];
            if (a4) {
                if (a3) *a3 = nUse + nWps;
                std::memcpy(a4, idsUse, nUse * sizeof(std::uint16_t));
                std::memcpy(a4 + nUse, idsWps, nWps * sizeof(std::uint16_t));
            }
        }
    }
    return best + apAdv_;
}

int cltPartySystem::GetDPowerRateAdvantage(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return dpAdv_;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyMemberDPowerAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best + dpAdv_;
}

int cltPartySystem::GetExpAdvantage(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return 0;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyExpAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best;
}

int cltPartySystem::GetHitRateAdvantage() { return expAdv_; }

int cltPartySystem::GetIncSTR(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return 0;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyMemberStrAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best;
}

int cltPartySystem::GetIncDEX(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return 0;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyMemberDexAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best;
}

int cltPartySystem::GetIncVIT(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return 0;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyMemberVitAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best;
}

int cltPartySystem::GetIncINT(void** a2, int* a3, std::uint16_t* a4) {
    int best = 0;
    if (!m_pPartyGetUsingSkillFuncPtr) return 0;
    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;
        int n = 0; std::uint16_t ids[10]{};
        const int v = use->GetPartyMemberIntAdvantage(&n, ids);
        if (v > best) { best = v; if (a2) *a2 = members_[i]; if (a4) { if (a3) *a3 = n; std::memcpy(a4, ids, n * sizeof(std::uint16_t)); } }
    }
    return best;
}

int cltPartySystem::GetAttackAtb(AttackAtb* a2, void** a3, std::uint16_t* a4) {
    unsigned int found = 0;
    AttackAtb best{};

    if (!m_pPartyGetUsingSkillFuncPtr || !memberCount_) {
        return 0;
    }

    for (std::uint8_t i = 0; i < memberCount_; ++i) {
        auto* use = m_pPartyGetUsingSkillFuncPtr(members_[i]);
        if (!use) continue;

        unsigned int localFound = 0;
        std::uint16_t atbCode = 0;
        AttackAtb local{};
        if (use->GetPartyMemberAttackAtb(&local, &localFound, &atbCode)) {
            best = local;
            found = localFound;
            if (a3) *a3 = members_[i];
            if (a4) *a4 = atbCode;
        }
    }

    if (!found) return 0;
    *a2 = best;
    return 1;
}

void cltPartySystem::GetPartyableLevelRange(int* a2, int* a3) {
    int maxLv = 0;
    int minLv = 0x7FFFFFFF;

    if (memberCount_ && m_pPartyGetLevelFuncPtr) {
        for (std::uint8_t i = 0; i < memberCount_; ++i) {
            const int lv = m_pPartyGetLevelFuncPtr(members_[i]);
            if (lv < minLv) minLv = lv;
            if (lv > maxLv) maxLv = lv;
        }
    }

    *a2 = (maxLv - 10 <= 0) ? 0 : maxLv - 10;
    *a3 = minLv + 10;
}
