#include "Logic/cltMap_Climate.h"
#include "Logic/Map.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "global.h"

#include <cstdlib>

// mofclient.c 0x4E3D80 — cltClimateUnit ctor
cltClimateUnit::cltClimateUnit()
    : m_iX(0), m_iY(0), m_dwResId(0),
      m_wStartBlock(0), m_wRandRange(0), m_wMaxBlock(0), m_wSnowSpeed(0),
      m_dwHasDrawBG(0), m_dwIsSnowing(0), m_pImage(nullptr),
      m_wCurFrame(0), m_fFrame(0.0f), m_fAnimSpeed(0.0f), m_iRangeY(0) {}

// mofclient.c 0x4E3DC0
void cltClimateUnit::Free() {
    m_dwResId      = 0;
    m_wStartBlock  = 0;
    m_wRandRange   = 0;
    m_iX           = 0;
    m_iY           = 0;
    m_wCurFrame    = 0;
    m_fAnimSpeed   = 0.0f;
}

// mofclient.c 0x4E3DE0
void cltClimateUnit::Init(int hasDrawBG, unsigned short climateKind, int rangeY) {
    m_dwHasDrawBG = static_cast<unsigned int>(hasDrawBG);
    auto* info = g_Map.GetClimateKindByClimateKind(climateKind);
    if (!info) return;
    m_dwResId      = info->resourceId;
    m_wStartBlock  = info->startBlock;
    m_wRandRange   = info->maxBlockCount;
    m_wMaxBlock    = static_cast<unsigned short>(m_wStartBlock + m_wRandRange);
    m_dwIsSnowing  = static_cast<unsigned int>(info->isSnowing);
    m_fAnimSpeed   = static_cast<float>(static_cast<double>(info->animation) * 0.0099999998);
    m_wSnowSpeed   = info->snowSpeed;
    m_fFrame       = 0.0f;
    m_iRangeY      = rangeY;
    Reset();
}

// mofclient.c 0x4E3E60
void cltClimateUnit::Reset() {
    const int sysW = g_Game_System_Info.ScreenWidth;
    const int sysH = g_Game_System_Info.ScreenHeight;
    if (m_dwIsSnowing) {
        m_iX = (std::rand() % (sysW + 200)) - 100 + dword_A73088;
        m_iY = (std::rand() % (sysH + 200)) - 100 + dword_A7308C;
    } else {
        m_iX = (std::rand() % (sysW > 0 ? sysW : 1)) + dword_A73088;
        m_iY = (std::rand() % (sysH > 0 ? sysH : 1)) + dword_A7308C;
    }
    m_wCurFrame = m_wStartBlock;
    if (m_dwIsSnowing) {
        m_fFrame = static_cast<float>(m_wStartBlock + (std::rand() & 0xFFFF) % 5);
    } else {
        const int range = (m_wRandRange > 0) ? m_wRandRange : 1;
        m_fFrame = static_cast<float>(m_wStartBlock + (std::rand() & 0xFFFF) % range);
    }
}

// mofclient.c 0x4E3F50
void cltClimateUnit::Poll() {
    m_fFrame   += m_fAnimSpeed;
    m_wCurFrame = static_cast<unsigned short>(static_cast<int>(m_fFrame));
    if (m_dwIsSnowing) {
        m_iY += m_wSnowSpeed;
    } else if (m_iY < m_iRangeY
               && m_wCurFrame >= static_cast<unsigned short>(m_wMaxBlock - 4)) {
        Reset();
    }
    if (m_wCurFrame >= m_wMaxBlock) {
        Reset();
    }
}

// mofclient.c 0x4E3FC0
void cltClimateUnit::PrepareDrawing() {
    cltImageManager* im = cltImageManager::GetInstance();
    if (!im) return;
    GameImage* img = im->GetGameImage(6u, m_dwResId, 0, 1);
    m_pImage = img;
    if (!img) return;
    if (m_dwIsSnowing) {
        // 雪花路徑：依 frame 索引把 X 微調（mofclient.c v10[] 表）。
        static const int kOffsets[18] = {
             0, -1, -2, -3, -4, -3, -2, -1,
             0,  0,  1,  2,  3,  4,  3,  2,  1,  0
        };
        const int idx = static_cast<int>(m_fFrame);
        const int safeIdx = (idx >= 0 && idx < 18) ? idx : 0;
        const int sx = m_iX + kOffsets[safeIdx] - dword_A73088;
        const int sy = m_iY - dword_A7308C;
        img->m_wBlockID = m_wCurFrame;
        img->m_bFlag_447 = true;
        img->m_bFlag_446 = true;
        img->m_bVertexAnimation = false;
        img->m_fPosX = static_cast<float>(sx);
        img->m_fPosY = static_cast<float>(sy);
    } else {
        const int sx = m_iX - dword_A73088;
        const int sy = m_iY - dword_A7308C;
        img->m_wBlockID = m_wCurFrame;
        img->m_bFlag_447 = true;
        img->m_bFlag_446 = true;
        img->m_bVertexAnimation = false;
        img->m_fPosX = static_cast<float>(sx);
        img->m_fPosY = static_cast<float>(sy);
    }
    // mofclient.c：*(_DWORD *)(img + 392) = g_bDir。GameImage+392 = m_bFlipX。
    img->m_bFlipX = (g_bDir != 0);
}

// mofclient.c 0x4E4140
void cltClimateUnit::Draw() {
    if (m_pImage) m_pImage->Draw();
}

// ----------------------------------------------------------------------------
// cltMap_Climate
// ----------------------------------------------------------------------------
cltMap_Climate::cltMap_Climate()
    : m_iActive(0), m_wUnitCount(0), m_wClimateKind(0) {}

// mofclient.c 0x4E41D0
void cltMap_Climate::Free() {
    for (unsigned short i = 0; i < m_wUnitCount; ++i) {
        m_units[i].Free();
    }
    m_iActive    = 0;
    m_wUnitCount = 0;
}

// mofclient.c 0x4E4230
void cltMap_Climate::InitClimate(int hasDrawBG, unsigned short climateKind, int rangeY) {
    m_wClimateKind = climateKind;
    g_bDir = std::rand() & 1;
    auto* info = g_Map.GetClimateKindByClimateKind(climateKind);
    if (!info) return;
    m_iActive    = 1;
    unsigned short n = info->unitCount;
    if (n > kMaxUnits) n = kMaxUnits;
    m_wUnitCount = n;
    for (unsigned short i = 0; i < n; ++i) {
        m_units[i].Init(hasDrawBG, climateKind, rangeY);
    }
}

// mofclient.c 0x4E42C0
void cltMap_Climate::Poll() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) {
        m_units[i].Poll();
    }
}

// mofclient.c 0x4E4300
void cltMap_Climate::PrepareDrawing() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) {
        m_units[i].PrepareDrawing();
    }
}

// mofclient.c 0x4E4340
void cltMap_Climate::Draw() {
    if (!m_iActive) return;
    for (unsigned short i = 0; i < m_wUnitCount; ++i) {
        m_units[i].Draw();
    }
}
