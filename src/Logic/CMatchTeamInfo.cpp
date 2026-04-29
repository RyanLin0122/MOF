// CMatchTeamInfo — 1:1 還原 mofclient.c 0x57CF20~0x57D0B0
#include "Logic/CMatchTeamInfo.h"

//----- (0057CF20) --------------------------------------------------------
CMatchTeamInfo::CMatchTeamInfo()
    : m_wTeamID(0), m_byMemberCount(0), m_pad3(0), m_dwFlags(0)
{
    // m_aUsers[7] 由 CMatchUserInfo::CMatchUserInfo() 預設填 0
}

//----- (0057CF60) --------------------------------------------------------
void CMatchTeamInfo::InitTeamInfo()
{
    m_wTeamID       = 0;
    m_byMemberCount = 0;
    m_dwFlags       = 0;
    for (int i = 0; i < kMaxMembers; ++i) {
        m_aUsers[i].InitUserInfo();
    }
}

//----- (0057CF90) --------------------------------------------------------
int CMatchTeamInfo::IsExistEmptyCharSlot()
{
    for (int i = 0; i < kMaxMembers; ++i) {
        if (m_aUsers[i].GetAccount() == 0) {
            return 1;
        }
    }
    return 0;
}

//----- (0057CFB0) --------------------------------------------------------
int CMatchTeamInfo::IsExistSameChar(uint32_t account)
{
    for (int i = 0; i < kMaxMembers; ++i) {
        if (m_aUsers[i].GetAccount() == account) {
            return 1;
        }
    }
    return 0;
}

//----- (0057CFE0) --------------------------------------------------------
int16_t CMatchTeamInfo::GetNewCharSlot()
{
    for (int i = 0; i < kMaxMembers; ++i) {
        if (m_aUsers[i].GetAccount() == 0) {
            return (int16_t)i;
        }
    }
    return -1;
}

//----- (0057D000) --------------------------------------------------------
int16_t CMatchTeamInfo::GetCharSlot(uint32_t account)
{
    for (int i = 0; i < kMaxMembers; ++i) {
        if (m_aUsers[i].GetAccount() == account) {
            return (int16_t)i;
        }
    }
    return -1;
}

//----- (0057D020) --------------------------------------------------------
bool CMatchTeamInfo::CanAddCharacter(uint32_t account)
{
    if (!IsExistEmptyCharSlot()) return false;
    return IsExistSameChar(account) != 1;
}

//----- (0057D050) --------------------------------------------------------
void CMatchTeamInfo::AddCharacter(uint32_t account)
{
    int16_t slot = GetNewCharSlot();
    if (slot >= 0) {
        m_aUsers[slot].SetAccount(account);
        ++m_byMemberCount;
    }
}

//----- (0057D070) --------------------------------------------------------
bool CMatchTeamInfo::CanDelCharacter(uint32_t account)
{
    return IsExistSameChar(account) != 0;
}

//----- (0057D090) --------------------------------------------------------
void CMatchTeamInfo::DelCharacter(uint32_t account)
{
    int16_t slot = GetCharSlot(account);
    if (slot >= 0) {
        m_aUsers[slot].SetAccount(0);
        --m_byMemberCount;
    }
}
