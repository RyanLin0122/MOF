#include "Logic/CPlayerSpirit.h"

#include "global.h"
#include "System/CSpiritSystem.h"

CPlayerSpirit::CPlayerSpirit()
{
    Initialize();
}

void CPlayerSpirit::Initialize()
{
    m_wSpiritAttribute = 0;
    m_wIncStr = 0;
    m_wIncInt = 0;
    m_wIncDex = 0;
    m_wIncVit = 0;
    m_wSpiritKind = 0;
}

int CPlayerSpirit::UpdatePlayerSpiritInfo(std::uint16_t spiritKind, int equipFlag)
{
    stSpiritInfo* info = g_clSpiritSystem.GetSpiritInfo(spiritKind);
    if (info)
    {
        if (equipFlag == 1)
        {
            SetSpiritInfo(info);
            return 1;
        }
        Initialize();
    }
    return 1;
}

int CPlayerSpirit::UpdatePlayerSpiritInfo(std::uint16_t spiritKind, std::uint16_t level, int equipFlag)
{
    stSpiritInfo* info = g_clSpiritSystem.GetSpiritInfo(spiritKind, level);
    if (info)
    {
        if (equipFlag == 1)
        {
            m_wSpiritKind = spiritKind;
            SetSpiritInfo(info);
            return 1;
        }
        Initialize();
    }
    return 1;
}

int CPlayerSpirit::UpdatePlayerSpiritInfo(std::uint16_t level)
{
    stSpiritInfo* info = g_clSpiritSystem.GetSpiritInfo(m_wSpiritKind, level);
    if (info)
        SetSpiritInfo(info);
    return 1;
}

void CPlayerSpirit::SetSpiritInfo(stSpiritInfo* info)
{
    m_wSpiritAttribute = info->wSpiritAttribute;
    m_wIncStr = info->wIncStr;
    m_wIncInt = info->wIncInt;
    m_wIncDex = info->wIncDex;
    m_wIncVit = info->wIncVit;
}
