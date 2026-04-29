#pragma once
//
// CMatchUserInfo — mofclient.c 還原（位址 0x57CEF0~0x57CF10）
//
// CMatchTeamInfo 內含 7 × CMatchUserInfo（stride 4 bytes），用來儲存 PvP 隊員的
// account ID。Init 把欄位設為 0；析構為 no-op。
//
#include <cstdint>

class CMatchUserInfo {
public:
    CMatchUserInfo() = default;
    ~CMatchUserInfo() = default;

    //----- (0057CF10) ----------------------------------------------------
    void InitUserInfo() { m_dwAccount = 0; }

    void SetAccount(uint32_t account) { m_dwAccount = account; }
    uint32_t GetAccount() const { return m_dwAccount; }

private:
    uint32_t m_dwAccount = 0;   // +0 (整個物件就 4 bytes)
};

static_assert(sizeof(CMatchUserInfo) == 4, "CMatchUserInfo must be 4 bytes (mofclient.c stride)");
