#include "Logic/CMap_Item_Climate.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Util/cltTimer.h"
#include "global.h"

#include <cstring>

// mofclient.c 0x4E4760
CMap_Item_Climate::CMap_Item_Climate()
    : m_iActive(0), m_iActiveStatic(0), m_iSubFlag(0),
      m_wUnitCount(0), m_wKind(0), m_wUseItemKind(0),
      m_dwIconResId(0), m_dwRemainTime(0), m_dwIconBlockId(0),
      m_dwField4009(0), m_dwTimerID(0) {
    std::memset(m_szUserName, 0, sizeof(m_szUserName));
}

// mofclient.c 0x4E4830
void CMap_Item_Climate::Free() {
    for (unsigned short i = 0; i < m_wUnitCount; ++i) m_units[i].Free();
    m_iActive = 0;
    m_wUnitCount = 0;
    if (m_dwTimerID) {
        g_clTimerManager.ReleaseTimer(m_dwTimerID);
        m_dwTimerID = 0;
    }
}

// mofclient.c 0x4E4890
int CMap_Item_Climate::InitMapItemClimate(unsigned short itemID) {
    auto* info = g_clMapUseItemInfoKindInfo.GetMapUseItemInfoKindInfo(itemID);
    if (!info) return 0;
    m_iActive       = 1;
    m_iActiveStatic = 0;
    m_wUseItemKind  = info->ID;                 // *((_WORD *)v3 + 0) = byte 0
    // *((_WORD *)v3 + 16) = byte 32 = low16(UnitCount)
    unsigned short n = static_cast<unsigned short>(info->UnitCount);
    if (n > kMaxUnits) n = kMaxUnits;
    m_wUnitCount    = n;
    m_dwIconResId   = info->WeatherIconResID;   // *((_DWORD *)v3 + 6) = byte 24
    m_dwIconBlockId = info->BlockID2;           // *((_DWORD *)v3 + 15) = byte 60
    for (unsigned short i = 0; i < n; ++i) {
        m_units[i].Init(m_wUseItemKind);
    }
    return 1;
}

// mofclient.c 0x4E4920
void CMap_Item_Climate::Poll() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) m_units[i].Poll();
}

// mofclient.c 0x4E4960
void CMap_Item_Climate::PrepareDrawing() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) m_units[i].PrepareDrawing();
}

// mofclient.c 0x4E49A0
void CMap_Item_Climate::Draw() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) m_units[i].Draw();
}

void CMap_Item_Climate::SetActiveStaticMapClimate(int v) { m_iActiveStatic = v; }
void CMap_Item_Climate::SetActiveMapItemClimate(int v)   { m_iActive = v; }
void CMap_Item_Climate::SetSubMapItemClimate(int v)      { m_iSubFlag = v; }
void CMap_Item_Climate::SetMapItemClimateRemainTime(unsigned int t) { m_dwRemainTime = t; }
void CMap_Item_Climate::SetMapItemClimateUserName(char* a2) {
    if (a2) std::strcpy(m_szUserName, a2);
}
int CMap_Item_Climate::IsActiveMapItemClimate()  const { return m_iActive; }
int CMap_Item_Climate::IsSubMapItemClimate()     const { return m_iSubFlag; }
int CMap_Item_Climate::IsActiveStaticMapClimate() const { return m_iActiveStatic; }
unsigned short CMap_Item_Climate::GetUseMapItemClimateKind() const { return m_wUseItemKind; }
char* CMap_Item_Climate::GetMapItemClimateUserName()         { return m_szUserName; }
unsigned int CMap_Item_Climate::GetMapItemClimateRemainTime() const { return m_dwRemainTime; }
unsigned int CMap_Item_Climate::GetMapItemClimateIconResourceID() const { return m_dwIconResId; }
unsigned int CMap_Item_Climate::GetMapItemClimateIconResourceBlockID() const { return m_dwIconBlockId; }
void CMap_Item_Climate::SetMapItemClimateKind(unsigned short kind) { m_wKind = kind; }
unsigned short CMap_Item_Climate::GetMapItemClimateKind() const { return m_wKind; }

void CMap_Item_Climate::SetTimerID() {
    m_dwTimerID = g_clTimerManager.CreateTimer(
        m_dwRemainTime, reinterpret_cast<std::uintptr_t>(this),
        0, 1, nullptr, nullptr, nullptr, nullptr, nullptr);
}

unsigned int CMap_Item_Climate::GetTimerID() const { return m_dwTimerID; }
