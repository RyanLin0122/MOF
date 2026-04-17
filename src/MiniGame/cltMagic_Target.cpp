// cltMagic_Target — cltMini_Magic_2 用的飛行目標物件，對齊 mofclient.c
// 0x5BD630..0x5BDA90 還原。

#include "MiniGame/cltMagic_Target.h"

#include <cstdlib>

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
cltMagic_Target::cltMagic_Target()
{
}

cltMagic_Target::~cltMagic_Target()
{
}

// ---------------------------------------------------------------------------
// Initialize
// ---------------------------------------------------------------------------
void cltMagic_Target::Initialize(std::uint8_t a2, std::uint8_t a3,
                                  std::uint8_t a4, int a5)
{
    m_type = a2;
    m_active = a5;
    m_kind = a3;
    m_direction = a4;
    m_flip = 0;

    // 速度依 type
    switch (a2) {
    case 0: m_speed = 7.0f;  break;   // 1088421888
    case 1: m_speed = 10.0f; break;   // 1092616192
    case 2: m_speed = 13.0f; break;   // 1095761920
    case 3: m_speed = 15.0f; break;   // 1097859072
    default: break;
    }

    // 資源/動畫依 kind
    switch (a3) {
    case 0:
        m_resID = 0x0C00029Fu;
        m_frameCount = 4;
        m_startFrame = 0;
        m_halfSizeX = 30.0f;   // 1106247680
        m_halfSizeY = 30.0f;
        break;
    case 1:
        m_resID = 0x0C00029Fu;
        m_frameCount = 4;
        m_startFrame = 7;
        m_halfSizeX = 30.0f;
        m_halfSizeY = 30.0f;
        break;
    case 2:
        m_resID = 0x0C00029Fu;
        m_frameCount = 2;
        m_startFrame = 14;
        m_halfSizeX = 20.0f;   // 1099431936
        m_halfSizeY = 25.0f;   // 1102053376
        break;
    }

    m_screenBaseX = (g_Game_System_Info.ScreenWidth - 800) / 2;
    m_screenBaseY = (g_Game_System_Info.ScreenHeight - 600) / 2;

    if (a4) {
        m_flip = 1;
        m_posX = static_cast<float>(m_screenBaseX - 30);
    } else {
        m_posX = static_cast<float>(m_screenBaseX + 810);
    }

    m_live = 1;
    m_posY = static_cast<float>(std::rand() % 200 + 150) + static_cast<float>(m_screenBaseY);
    m_animCounter = static_cast<float>(m_startFrame);
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------
void cltMagic_Target::SetActive(int a2) { m_active = a2; }
int  cltMagic_Target::GetActive()       { return m_active; }
int  cltMagic_Target::GetLive()         { return m_live; }

int cltMagic_Target::GetPoint()
{
    switch (m_kind) {
    case 0: return 1;
    case 1: return 3;
    case 2: return -5;
    }
    return 0;
}

void cltMagic_Target::SetDie()
{
    switch (m_kind) {
    case 0:
        m_frameCount = 3;
        m_startFrame = 4;
        break;
    case 1:
        m_frameCount = 3;
        m_startFrame = 11;
        break;
    case 2:
        m_frameCount = 3;
        m_startFrame = 16;
        break;
    }
    m_speed = 0.0f;
    m_live = 0;
}

void cltMagic_Target::SetPos(float a2, float a3)
{
    m_posX = a2;
    m_posY = a3;
}

void cltMagic_Target::GetPos(float* a2, float* a3)
{
    *a2 = m_posX;
    *a3 = m_posY;
}

float cltMagic_Target::GetPosY()
{
    return m_posY;
}

void cltMagic_Target::GetHSize(float* a2, float* a3)
{
    *a2 = m_halfSizeX;
    *a3 = m_halfSizeY;
}

std::uint8_t cltMagic_Target::GetDirection()
{
    return m_direction;
}

void cltMagic_Target::Release()
{
    m_active = 0;
}

// ---------------------------------------------------------------------------
// Poll
// ---------------------------------------------------------------------------
void cltMagic_Target::Poll()
{
    if (!m_active)
        return;

    if (m_direction) {
        // 左→右
        m_posX = m_speed + m_posX;
        if (m_posX > static_cast<float>(m_screenBaseX + 810))
            Release();
    } else {
        // 右→左
        m_posX = m_posX - m_speed;
        if (m_posX < static_cast<float>(m_screenBaseX - 10))
            Release();
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing / Draw
// ---------------------------------------------------------------------------
void cltMagic_Target::PrepareDrawing()
{
    if (!m_active)
        return;

    if (static_cast<float>(m_startFrame + m_frameCount) < m_animCounter) {
        if (!m_live)
            Release();
        m_currentFrame = m_startFrame;
        m_animCounter = static_cast<float>(m_startFrame);
    }

    GameImage* img = cltImageManager::GetInstance()->GetGameImage(9u, m_resID, 0, 1);
    m_pImage = img;
    *((float*)img + 83) = m_posX;
    *((__int16*)img + 186) = m_currentFrame;
    *((std::uint8_t*)img + 447) = 1;
    *((float*)img + 84) = m_posY;
    *((std::uint8_t*)img + 446) = 1;
    *((std::uint8_t*)img + 444) = 0;
    *(int*)((char*)m_pImage + 392) = m_flip;

    m_animCounter += 0.40000001f;
    m_currentFrame = static_cast<std::int16_t>(static_cast<__int64>(m_animCounter));
}

void cltMagic_Target::Draw()
{
    if (m_active && m_pImage)
        m_pImage->Draw();
}
