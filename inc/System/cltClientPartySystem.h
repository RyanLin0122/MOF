#pragma once

#include <array>
#include <cstdint>

#include "System/cltPartySystem.h"

#pragma pack(push, 1)
struct strPartyMemberInfo {
    std::uint32_t dwAccountId = 0; // +0
    char szName[32]{};             // +4
    std::uint8_t byClass = 0;      // +36
    std::uint8_t pad0[3]{};        // +37
    int nInfo0 = 0;                // +40
    int nInfo1 = 0;                // +44
    int nHP = 0;                   // +48
    int nInfo3 = 0;                // +52
    std::int64_t nInfo4 = 0;       // +56
    std::uint16_t wSkill0 = 0;     // +64
    std::uint16_t wSkill1 = 0;     // +66
    std::uint16_t wSkill2 = 0;     // +68
    std::uint16_t pad1 = 0;        // +70
};
#pragma pack(pop)
static_assert(sizeof(strPartyMemberInfo) == 72, "strPartyMemberInfo must be 72 bytes");

class cltClientPartySystem : public cltPartySystem {
public:
    cltClientPartySystem();
    virtual ~cltClientPartySystem();

    void Create(std::uint8_t count, strPartyMemberInfo* members);
    void Join(unsigned int accountId, char* name, char classCode, int a5, int a6, int a7, int a8, std::int64_t a9);
    void Leave(unsigned int accountId);

    strPartyMemberInfo* GetPartyMemberInstance(unsigned int accountId);
    strPartyMemberInfo* GetPartyMemberInfo(std::uint8_t index);
    unsigned int* GetLeaderAccount();

    void UpdatePartyMemberInfo(unsigned int accountId, char classCode, int a4, int a5, int a6, int a7, std::int64_t a8);
    void UpdatePartyMemberSkill(unsigned int accountId, std::uint16_t a3, std::uint16_t a4, std::uint16_t a5);
    void UpdateNewHP(unsigned int accountId, int hp);

    void SetIncSTR(int v);
    void SetIncDEX(int v);
    void SetIncVIT(int v);
    void SetIncINT(int v);

    int GetIncSTR(void**, int*, std::uint16_t*);
    int GetIncDEX(void**, int*, std::uint16_t*);
    int GetIncVIT(void**, int*, std::uint16_t*);
    int GetIncINT(void**, int*, std::uint16_t*);

private:
    std::array<strPartyMemberInfo, 5> m_members{};
    int m_incStr = 0;
    int m_incDex = 0;
    int m_incVit = 0;
    int m_incInt = 0;
};
