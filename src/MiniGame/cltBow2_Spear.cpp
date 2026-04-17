// cltBow2_Spear / stBowSpear — 對齊 mofclient.c 的飛矛投射物實作。

#include "MiniGame/cltBow2_Spear.h"

#include <cmath>

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"

// ---------------------------------------------------------------------------
// stBowSpear
// ---------------------------------------------------------------------------
stBowSpear::stBowSpear()
{
    // GT: 只呼叫 cltBow2_Spear 建構子（C++ 自動處理），不初始化 m_active
}

stBowSpear::~stBowSpear()
{
    // cltBow2_Spear 由其自身解構子清理
}

// ---------------------------------------------------------------------------
// cltBow2_Spear
// ---------------------------------------------------------------------------
cltBow2_Spear::cltBow2_Spear()
    : m_active(0)
{
    // GT: 只設 vftable（自動）+ *((_DWORD *)this + 10) = 0 即 m_active
}

cltBow2_Spear::~cltBow2_Spear()
{
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B79D0
// ---------------------------------------------------------------------------
void cltBow2_Spear::Create(std::uint8_t posIndex, std::uint8_t direction,
                            float moveType, float speed)
{
    // 螢幕中心偏移（對齊 mofclient.c 的計算）
    m_centerX = static_cast<float>((g_Game_System_Info.ScreenWidth - 800) / 2) + 403.0f;
    m_centerY = static_cast<float>((g_Game_System_Info.ScreenHeight - 600) / 2) + 272.0f;

    m_radius    = 216.0f;    // IEEE 1129840640
    m_maxDist   = 46656.0f;  // IEEE 1194737664 (= 216^2)
    m_intercept = 0.0f;
    m_moveType  = moveType;

    // 從全域位移表取得偏移量（17 組 [x, y]）
    float xOff = g_fBowSpearTable[2 * posIndex];
    float yOff = g_fBowSpearTable[2 * posIndex + 1];

    float sqr8 = sqrtf(8.0f);
    m_speedX = sqr8;
    m_speedY = sqr8;

    switch (direction)
    {
    case 1: // 從右上方
        m_posX = xOff + m_centerX;
        m_posY = yOff + m_centerY;
        m_dirX = -1.0f;
        m_dirY = -1.0f;
        break;
    case 2: // 從左上方
        m_posX = m_centerX - xOff;
        m_posY = yOff + m_centerY;
        m_dirX = 1.0f;
        m_dirY = -1.0f;
        break;
    case 3: // 從左下方
        m_posX = m_centerX - xOff;
        m_posY = m_centerY - yOff;
        m_dirX = 1.0f;
        m_dirY = 1.0f;
        break;
    case 4: // 從右下方
        m_posX = xOff + m_centerX;
        m_posY = m_centerY - yOff;
        m_dirX = -1.0f;
        m_dirY = 1.0f;
        break;
    default:
        break;
    }

    // moveType 100/101 保持原值（垂直/水平移動）
    // 其他值計算斜線軌跡
    if (moveType != 100.0f && moveType != 101.0f)
    {
        if (direction & 1) // 奇數方向
            m_moveType = 1.0f;
        else
            m_moveType = -1.0f;

        m_intercept = m_posY - m_posX * m_moveType;
        m_speedX    = speed * m_dirX;
    }

    m_active = 1;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7B60
// ---------------------------------------------------------------------------
void cltBow2_Spear::Release()
{
    m_active = 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7B70
// ---------------------------------------------------------------------------
int cltBow2_Spear::GetActive()
{
    return m_active;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7B80
// ---------------------------------------------------------------------------
void cltBow2_Spear::Poll()
{
    if (!m_active)
        return;

    float dx = fabsf(m_centerX - m_posX);
    float dy = fabsf(m_centerY - m_posY);

    if (m_maxDist >= dy * dy + dx * dx)
    {
        if (m_moveType == 100.0f)
        {
            // 垂直移動
            m_posY = m_dirY * m_speedY + m_posY;
        }
        else if (m_moveType == 101.0f)
        {
            // 水平移動
            m_posX = m_dirX * m_speedX + m_posX;
        }
        else
        {
            // 斜線移動（y = slope * x + intercept）
            m_posX = m_speedX + m_posX;
            m_posY = m_posX * m_moveType + m_intercept;
        }
    }
    else
    {
        Release();
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7C00
// ---------------------------------------------------------------------------
void cltBow2_Spear::PrepareDrawing()
{
    if (!m_active)
        return;

    // 動畫 frame 限制在 0..1
    if (m_frame > 1)
    {
        m_frame       = 0;
        m_animCounter = 0.0f;
    }

    GameImage* pImg = cltMoF_BaseMiniGame::m_pclImageMgr->GetGameImage(
        9u, 0x0C00029Eu, 0, 1);

    // GT: 無 nullptr 防護；直接依序寫入 GameImage 欄位（mofclient.c 361958-361968）。
    m_pImage = pImg;
    pImg->m_fPosX = m_posX;
    pImg->SetBlockID(static_cast<unsigned short>(m_frame));
    pImg->m_bFlag_447 = true;
    pImg->m_fPosY = m_posY;
    pImg->m_bFlag_446 = true;
    pImg->m_bVertexAnimation = false;

    m_animCounter += 0.2f;
    m_frame = static_cast<int>(m_animCounter);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7C80
// ---------------------------------------------------------------------------
void cltBow2_Spear::Draw()
{
    if (m_active && m_pImage)
        m_pImage->Draw();
}
