#pragma once
//
// CMatchTeamInfo — mofclient.c 還原（位址 0x57CF20~0x57D0B0）
//
// PvP / Match 中一支隊伍的容器。內含：
//   - 隊伍 metadata（teamID + 人數 + flags）
//   - CMatchUserInfo[7]：固定 7 格，每格放一名隊員的 account ID
//
// Layout（對齊 mofclient.c）：
//   +0   uint16_t  m_wTeamID
//   +2   uint8_t   m_byMemberCount     // AddCharacter 時 ++、DelCharacter 時 --
//   +3   uint8_t   m_pad3
//   +4   uint32_t  m_dwFlags
//   +8..+35  CMatchUserInfo m_aUsers[7]   // stride 4 bytes
//
// CMatchUserInfo 本身是單一 uint32_t (account ID) 的 wrapper，原始碼用
// `*((DWORD*)this + slot + 2) = account` 直接寫入。本還原把這個語意以
// CMatchUserInfo::SetAccount/GetAccount 露出。
//
#include <cstdint>
#include "Logic/CMatchUserInfo.h"

class CMatchTeamInfo {
public:
    static constexpr int kMaxMembers = 7;

    CMatchTeamInfo();
    ~CMatchTeamInfo() = default;     // 對齊 mofclient.c：直接走 eh vector dtor

    // -----------------------------------------------------------------
    // mofclient.c 0x57CF60
    // -----------------------------------------------------------------
    void InitTeamInfo();

    // -----------------------------------------------------------------
    // mofclient.c 0x57CF90 / 0x57CFB0
    // -----------------------------------------------------------------
    int  IsExistEmptyCharSlot();
    int  IsExistSameChar(uint32_t account);

    // -----------------------------------------------------------------
    // mofclient.c 0x57CFE0 / 0x57D000
    // 找新空格 / 找已存在 char 的 slot index；找不到回 -1。
    // -----------------------------------------------------------------
    int16_t GetNewCharSlot();
    int16_t GetCharSlot(uint32_t account);

    // -----------------------------------------------------------------
    // mofclient.c 0x57D020 / 0x57D050 / 0x57D070 / 0x57D090
    // -----------------------------------------------------------------
    bool CanAddCharacter(uint32_t account);
    void AddCharacter(uint32_t account);
    bool CanDelCharacter(uint32_t account);
    void DelCharacter(uint32_t account);

    // 直接欄位存取（封裝 +0..+7 metadata）
    uint16_t GetTeamID()       const { return m_wTeamID; }
    uint8_t  GetMemberCount()  const { return m_byMemberCount; }
    void     SetTeamID(uint16_t id)  { m_wTeamID = id; }

private:
    uint16_t       m_wTeamID;          // +0
    uint8_t        m_byMemberCount;    // +2
    uint8_t        m_pad3;             // +3
    uint32_t       m_dwFlags;          // +4
    CMatchUserInfo m_aUsers[kMaxMembers];  // +8..+35
};
