#include "System/cltClientPartySystem.h"

#include <cstring>

#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "global.h"

cltClientPartySystem::cltClientPartySystem() : cltPartySystem() {
    std::memset(m_members.data(), 0, sizeof(m_members));
}
cltClientPartySystem::~cltClientPartySystem() = default;

void cltClientPartySystem::Create(std::uint8_t count, strPartyMemberInfo* members) {
    std::array<void*, 5> ptrs{};
    std::memset(m_members.data(), 0, sizeof(m_members));

    std::memcpy(m_members.data(), members, sizeof(strPartyMemberInfo) * count);
    for (std::uint8_t i = 0; i < count; ++i) {
        ptrs[i] = &m_members[i];
    }

    m_incStr = m_incDex = m_incVit = m_incInt = 0;
    cltPartySystem::Create(ptrs.data(), 0);
}

void cltClientPartySystem::Join(unsigned int accountId, char* name, char classCode, int a5, int a6, int a7, int a8, std::int64_t a9) {
    for (auto& m : m_members) {
        if (m.dwAccountId != 0) continue;
        m.dwAccountId = accountId;
        std::strcpy(m.szName, name);
        m.byClass = static_cast<std::uint8_t>(classCode);
        m.nInfo0 = a5;
        m.nInfo1 = a7;
        m.nHP = a6;
        m.nInfo3 = a8;
        m.nInfo4 = a9;
        cltPartySystem::Join(&m);
        return;
    }
}

void cltClientPartySystem::Leave(unsigned int accountId) {
    ClientCharacter* chr = g_ClientCharMgr.GetCharByAccount(accountId);
    if (chr) {
        chr->SetClassCode(reinterpret_cast<unsigned short*>(chr)[4856], 0);
    }

    auto* member = GetPartyMemberInstance(accountId);
    if (!member) return;

    if (accountId == g_dwMyAccountID) {
        int idx = 0;
        const int memberNum = cltPartySystem::GetPartyMemberNum();
        while (idx < memberNum) {
            auto* partyMember = GetPartyMemberInfo(static_cast<std::uint8_t>(idx));
            if (partyMember) {
                ClientCharacter* memberChar = g_ClientCharMgr.GetCharByAccount(partyMember->dwAccountId);
                if (memberChar) {
                    memberChar->SetClassCode(reinterpret_cast<unsigned short*>(memberChar)[4856], 0);
                }
            }
            ++idx;
        }
        this->Free();
    } else {
        cltPartySystem::Leave(member);
    }

    std::memset(member, 0, sizeof(*member));

    if (cltPartySystem::GetPartyMemberNum() == 0) {
        ClientCharacter* myChar = g_ClientCharMgr.GetCharByAccount(g_dwMyAccountID);
        if (myChar) {
            myChar->SetClassCode(reinterpret_cast<unsigned short*>(myChar)[4856], 0);
        }
    }
}

strPartyMemberInfo* cltClientPartySystem::GetPartyMemberInstance(unsigned int accountId) {
    for (auto& m : m_members) {
        if (m.dwAccountId == accountId) return &m;
    }
    return nullptr;
}

strPartyMemberInfo* cltClientPartySystem::GetPartyMemberInfo(std::uint8_t index) {
    return static_cast<strPartyMemberInfo*>(cltPartySystem::GetPartyInstance(index));
}

unsigned int* cltClientPartySystem::GetLeaderAccount() {
    auto* leader = static_cast<unsigned int**>(cltPartySystem::GetLeadInstance());
    if (leader) {
        return *leader;
    }
    return nullptr;
}

void cltClientPartySystem::UpdatePartyMemberInfo(unsigned int accountId, char classCode, int a4, int a5, int a6, int a7, std::int64_t a8) {
    auto* v = GetPartyMemberInstance(accountId);
    if (!v) return;
    v->byClass = static_cast<std::uint8_t>(classCode);
    v->nInfo0 = a4;
    v->nHP = a5;
    v->nInfo1 = a6;
    v->nInfo3 = a7;
    v->nInfo4 = a8;
}

void cltClientPartySystem::UpdatePartyMemberSkill(unsigned int accountId, std::uint16_t a3, std::uint16_t a4, std::uint16_t a5) {
    auto* v = GetPartyMemberInstance(accountId);
    if (!v) return;
    v->wSkill0 = a3;
    v->wSkill1 = a4;
    v->wSkill2 = a5;
}

void cltClientPartySystem::UpdateNewHP(unsigned int accountId, int hp) {
    auto* v = GetPartyMemberInstance(accountId);
    if (v) v->nHP = hp;
}

void cltClientPartySystem::SetIncSTR(int v) { m_incStr = v; }
void cltClientPartySystem::SetIncDEX(int v) { m_incDex = v; }
void cltClientPartySystem::SetIncVIT(int v) { m_incVit = v; }
void cltClientPartySystem::SetIncINT(int v) { m_incInt = v; }

int cltClientPartySystem::GetIncSTR(void**, int*, std::uint16_t*) { return m_incStr; }
int cltClientPartySystem::GetIncDEX(void**, int*, std::uint16_t*) { return m_incDex; }
int cltClientPartySystem::GetIncVIT(void**, int*, std::uint16_t*) { return m_incVit; }
int cltClientPartySystem::GetIncINT(void**, int*, std::uint16_t*) { return m_incInt; }
