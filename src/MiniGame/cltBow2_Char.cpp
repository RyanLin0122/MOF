// cltBow2_Char — 對齊 mofclient.c 的 Bow2 角色實作。

#include "MiniGame/cltBow2_Char.h"

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7CA0 — 建構子
// ---------------------------------------------------------------------------
cltBow2_Char::cltBow2_Char()
    : m_initPosX(0), m_initPosY(0), m_curPosX(0), m_curPosY(0),
      m_resID(0), m_frame(0), m_direction(0), m_frameCount(0),
      m_animCounter(0), m_radius(0), m_blockIDBase(0),
      m_pImage(nullptr), m_active(0)
{
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7CD0
// ---------------------------------------------------------------------------
cltBow2_Char::~cltBow2_Char()
{
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7CE0
// ---------------------------------------------------------------------------
void cltBow2_Char::Initalize(float radius, std::uint8_t degree,
                              int screenX, int screenY)
{
    m_radius = radius;

    float cx = static_cast<float>(screenX + 403);
    m_initPosX = cx;
    m_curPosX  = cx;

    float cy = static_cast<float>(screenY + 272);
    m_initPosY = cy;
    m_curPosY  = cy;

    switch (degree)
    {
    case 1:
        m_resID = 0x0C00029Du;
        break;
    case 2:
        m_resID = 0x0B00073Bu;  // 184551227
        break;
    case 4:
        m_resID = 0x0B00073Cu;  // 184551228
        break;
    }

    m_frame       = 0;
    m_frameCount  = 3;
    m_direction   = 0;
    m_animCounter = 0;
    m_blockIDBase = 0;
    SetDirection(4);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7D60
// ---------------------------------------------------------------------------
void cltBow2_Char::SetActive(int active)
{
    m_active = active;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7D70
// ---------------------------------------------------------------------------
void cltBow2_Char::SetDirection(std::uint8_t direction)
{
    m_direction = direction;
    switch (direction)
    {
    case 0:
        m_blockIDBase = 5;
        break;
    case 1:
    case 7:
        m_blockIDBase = 14;
        break;
    case 2:
    case 6:
        m_blockIDBase = 18;
        break;
    case 3:
    case 5:
        m_blockIDBase = 22;
        break;
    case 4:
        m_blockIDBase = 10;
        break;
    case 8:
        m_blockIDBase = 4;
        m_frameCount  = 0;
        break;
    default:
        return;
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7DF0
// ---------------------------------------------------------------------------
void cltBow2_Char::SetPos(float x, float y)
{
    if (m_active)
    {
        float dx = x - m_initPosX;
        float dy = y - m_initPosY;
        if (dx * dx + dy * dy <= m_radius * m_radius)
        {
            m_curPosX = x;
            m_curPosY = y;
        }
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7E40
// ---------------------------------------------------------------------------
double cltBow2_Char::GetPosX()
{
    return m_curPosX;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7E50
// ---------------------------------------------------------------------------
double cltBow2_Char::GetPosY()
{
    return m_curPosY;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7E70 — Poll（角色本身無獨立邏輯，空實作）
// ---------------------------------------------------------------------------
void cltBow2_Char::Poll()
{
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7E70
// ---------------------------------------------------------------------------
void cltBow2_Char::PrepareDrawing()
{
    if (!m_active)
        return;

    int baseBlock = m_blockIDBase;
    if (m_frame > baseBlock + m_frameCount)
    {
        m_frame       = baseBlock;
        m_animCounter = 0;
    }

    GameImage* pImg = cltMoF_BaseMiniGame::m_pclImageMgr->GetGameImage(
        9u, m_resID, 0, 1);

    m_pImage = pImg;
    if (pImg)
    {
        pImg->m_fPosX           = m_curPosX;
        pImg->SetBlockID(static_cast<unsigned short>(m_frame));
        pImg->m_bFlag_447       = true;
        pImg->m_fPosY           = m_curPosY;
        pImg->m_bFlag_446       = true;
        pImg->m_bVertexAnimation = false;

        if (m_direction > 4)
            pImg->m_bFlipX = true;
    }

    m_animCounter += 0.5f;
    m_frame = m_blockIDBase + static_cast<int>(m_animCounter);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7F10
// ---------------------------------------------------------------------------
void cltBow2_Char::Draw()
{
    if (m_active && m_pImage)
        m_pImage->Draw();
}
