// cltMagic_Box — cltMini_Magic_2 用的閃爍方塊，對齊 mofclient.c
// 0x5BDAA0..0x5BDC30 還原。

#include "MiniGame/cltMagic_Box.h"

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
cltMagic_Box::cltMagic_Box()
{
}

cltMagic_Box::~cltMagic_Box()
{
}

// ---------------------------------------------------------------------------
// Initailize (保持原始拼寫)
// ---------------------------------------------------------------------------
void cltMagic_Box::Initailize(float a2, float a3)
{
    m_posX = a2;
    m_posY = a3;
    m_resID = 0x20000031u;   // 536870961
    m_frame = 0;
    m_active = 1;
    m_alpha = static_cast<std::uint8_t>(-1);  // 0xFF
    m_fadeDir = 1;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------
float cltMagic_Box::GetPosX()               { return m_posX; }
float cltMagic_Box::GetPosY()               { return m_posY; }
void  cltMagic_Box::SetPosY(float a2)       { m_posY = a2; }
void  cltMagic_Box::SetActive(int a2)       { m_active = a2; }
int   cltMagic_Box::GetActive()             { return m_active; }

// ---------------------------------------------------------------------------
// Poll — alpha 淡入/淡出循環
// ---------------------------------------------------------------------------
void cltMagic_Box::Poll()
{
    if (!m_active)
        return;

    if (m_fadeDir == 1) {
        m_alpha = static_cast<std::uint8_t>(m_alpha - 10);
        if (static_cast<std::uint8_t>(m_alpha) < 0x7Fu)
            m_fadeDir = 0;
    } else {
        m_alpha = static_cast<std::uint8_t>(m_alpha + 10);
        if (m_alpha == 0xFF)
            m_fadeDir = (m_fadeDir == 0) ? 1 : 0;
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing / Draw
// ---------------------------------------------------------------------------
void cltMagic_Box::PrepareDrawing()
{
    if (!m_active)
        return;

    GameImage* img = cltImageManager::GetInstance()->GetGameImage(9u, m_resID, 0, 1);
    m_pImage = img;
    *((float*)img + 83) = m_posX;
    *((__int16*)img + 186) = m_frame;
    *((std::uint8_t*)img + 447) = 1;
    *((float*)img + 84) = m_posY;
    *((std::uint8_t*)img + 446) = 1;
    *((std::uint8_t*)img + 444) = 0;

    *(int*)((char*)m_pImage + 380) = static_cast<int>(m_alpha);
    *((std::uint8_t*)((char*)m_pImage + 450)) = 1;
    *((std::uint8_t*)((char*)m_pImage + 444)) = 0;
}

void cltMagic_Box::Draw()
{
    if (m_active && m_pImage)
        m_pImage->Draw();
}
