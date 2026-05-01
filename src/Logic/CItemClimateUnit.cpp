#include "Logic/CItemClimateUnit.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "global.h"

#include <cstdlib>

// mofclient.c 0x4E4380 — ctor
CItemClimateUnit::CItemClimateUnit()
    : m_iX(0), m_iY(0), m_iX2(0), m_iLandY(0),
      m_iAirSpeed(0), m_iPattern(0), m_iAnimSpeed(0),
      m_wField16(0), m_wField17(0), m_wField18(0),
      m_wCurFrame(0), m_wMaxFrame(0),
      m_dwResourceID(0), m_iStateFlag(0), m_pImage(nullptr),
      m_fAnimSpeedF(0.0f), m_fFrame(0.0f) {}

// mofclient.c 0x4E4400
void CItemClimateUnit::Free() {
    m_iX = 0;
    m_iY = 0;
    m_iX2 = 0;
    m_iLandY = 0;
    m_iAirSpeed = 0;
    m_iPattern = 0;
    m_iAnimSpeed = 0;
    m_wField16 = 0;
    m_wField17 = 0;
    m_wField18 = 0;
    m_wCurFrame = 0;
    m_wMaxFrame = 0;
    m_dwResourceID = 0;
    m_iStateFlag = 0;
    m_fFrame = 0.0f;
}

// mofclient.c 0x4E4440：依 itemID 從 g_clMapUseItemInfoKindInfo 取得 info 後 Init。
void CItemClimateUnit::Init(unsigned short itemID) {
    auto* info = g_clMapUseItemInfoKindInfo.GetMapUseItemInfoKindInfo(itemID);
    if (info) Init(info);
}

// mofclient.c 0x4E44B0
void CItemClimateUnit::Init(const strMapUseItemInfoKindInfo* a2) {
    if (!a2) return;
    m_iPattern   = static_cast<int>(a2->Pattern);
    m_iAirSpeed  = static_cast<int>(a2->AirSpeed);
    m_iAnimSpeed = static_cast<int>(a2->AnimSpeed);
    m_wField16   = static_cast<unsigned short>(a2->StartBlock);
    m_wField18   = static_cast<unsigned short>(a2->AnimStartBlockID);
    m_wMaxFrame  = static_cast<unsigned short>(a2->MaxBlocks);
    m_wField17   = static_cast<unsigned short>(m_wMaxFrame - 1);
    m_dwResourceID = a2->ResourceID;
    m_fAnimSpeedF = static_cast<float>(static_cast<double>(m_iAnimSpeed) * 0.0099999998);
    m_wCurFrame  = 0;
    m_iStateFlag = 0;
    m_fFrame     = 0.0f;
    Reset();
}

// mofclient.c 0x4E4510
void CItemClimateUnit::Reset() {
    const int sysW = g_Game_System_Info.ScreenWidth;
    const int sysH = g_Game_System_Info.ScreenHeight;
    if (m_iPattern == 1 || m_iPattern == 2) {
        m_iX = (std::rand() % sysW) + dword_A73088;
        m_iY = dword_A7308C - (std::rand() % sysH);
        m_iX2 = dword_A73088 + m_iX;
        m_iLandY = (std::rand() % sysH) - 80 + dword_A7308C;
        m_wField17 = m_wField18;
        m_fFrame = 0.0f;
        m_wCurFrame = 0;
        m_wField16 = 0;
        m_iStateFlag = 0;
    }
    if (m_iLandY < dword_A7308C + 100) {
        m_iLandY += 520;
    }
    if (m_iLandY > dword_A7308C + 520) {
        m_iLandY = dword_A7308C + 520;
    }
}

// mofclient.c 0x4E45C0
void CItemClimateUnit::Poll() {
    const float v2 = m_fAnimSpeedF + m_fFrame;
    m_fFrame = v2;
    m_iY += m_iAirSpeed;
    m_wCurFrame = static_cast<unsigned short>(static_cast<int>(v2));

    const int v4 = m_iStateFlag;

    if (v4 == 1 && m_iPattern == 2) {
        m_iY = m_iLandY;
    }
    if (!v4 && m_wCurFrame >= m_wField17) {
        m_fFrame = 0.0f;
        m_wCurFrame = 0;
    }

    if (m_iPattern == 1 && !v4) {
        const int distance = m_iAirSpeed * static_cast<int>(m_wField18);
        if (m_iY >= m_iLandY - distance) {
            m_wField16 = m_wField18;
            m_wField17 = m_wMaxFrame;
            m_iStateFlag = 1;
            m_fFrame = static_cast<float>(m_wField18);
        }
    } else if (m_iPattern == 2 && !v4) {
        if (m_iY >= m_iLandY) {
            m_wField16 = m_wField18;
            m_wField17 = m_wMaxFrame;
            m_iStateFlag = 1;
            m_iY = m_iLandY;
            m_fFrame = static_cast<float>(m_wField18);
        }
    }

    if (m_wCurFrame >= m_wField17) Reset();

    const int sysW = g_Game_System_Info.ScreenWidth;
    if (m_iX - dword_A73088 >= 0) {
        if (sysW < m_iX - dword_A73088) {
            m_iX -= sysW;
        }
    } else {
        m_iX += sysW;
    }
}

// mofclient.c 0x4E46D0
void CItemClimateUnit::PrepareDrawing() {
    cltImageManager* im = cltImageManager::GetInstance();
    if (!im) return;
    GameImage* img = im->GetGameImage(7u, m_dwResourceID, 0, 1);
    m_pImage = img;
    if (!img) return;
    img->m_wBlockID = m_wCurFrame;
    img->m_bFlag_447 = true;
    img->m_bFlag_446 = true;
    img->m_bVertexAnimation = false;
    img->m_fPosX = static_cast<float>(m_iX - dword_A73088);
    img->m_fPosY = static_cast<float>(m_iY - dword_A7308C);
}

// mofclient.c 0x4E4750
void CItemClimateUnit::Draw() {
    if (m_pImage) m_pImage->Draw();
}
