#pragma once
//
// strUIHelperMenu — mofclient.c 還原（位址 0x42E8E0）
//
// CUIHelper 把多個 helper window 用 vector<strUIHelperMenu> 管理；
// 每個 strUIHelperMenu 又含一個 vector<strUIHelperBallon>。
//
// 20-byte layout：
//   +0   uint16_t  m_wWindowID         // 對應 UI window kind
//   +2   uint16_t  m_wPad
//   +4   uint32_t  m_dwUnknown         // mofclient.c 寫 0
//   +8   ptr       m_vBalloonBegin     ← destructor 釋放
//   +12  ptr       m_vBalloonEnd
//   +16  ptr       m_vBalloonCap
//
// destructor 會 operator delete( *(this+2) ) 並清空 +2/+3/+4。本還原直接以
// std::vector<strUIHelperBallon> 作為內含 member，由 STL 自行處理。
//
#include <cstdint>
#include <vector>

struct strUIHelperBallon {
    int   a;   // 16 bytes total
    int   b;
    int   c;
    int   d;
};

struct strUIHelperMenu {
    uint16_t m_wWindowID;     // +0
    uint16_t m_wPad;          // +2
    uint32_t m_dwUnknown;     // +4
    std::vector<strUIHelperBallon> m_vBalloons;  // +8 (begin/end/cap, 12 bytes)

    strUIHelperMenu() : m_wWindowID(0), m_wPad(0), m_dwUnknown(0) {}
    //----- (0042E8E0) ----------------------------------------------------
    ~strUIHelperMenu() = default;  // STL vector 自動釋放
};
