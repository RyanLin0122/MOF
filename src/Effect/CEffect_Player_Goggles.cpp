#include "Effect/CEffect_Player_Goggles.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>

// 對應反組譯碼: 0x005318B0
CEffect_Player_Goggles::CEffect_Player_Goggles()
{
    m_pImage = nullptr;
    m_fAlpha = 250.0f;        // 1137180672 (0x437A0000)
    m_fScale = 50.0f;         // 1092616192 (0x41200000)
    m_fRotation = 0.0f;
    m_cInitialFrame = rand() % 4;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fScaleX_Rate = 1.0f;    // 1065353216 (0x3F800000)
    m_fAlpha_Rate = 1.0f;     // 1065353216 (0x3F800000)
    m_dwResourceID = 0;
    m_isUiEffect = 0;

    // 原始碼: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

// 對應反組譯碼: 0x00531960
CEffect_Player_Goggles::~CEffect_Player_Goggles()
{
}

// 對應反組譯碼: 0x00531980
void CEffect_Player_Goggles::SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int isUiEffect)
{
    m_isUiEffect = isUiEffect;
    m_bIsFlip = bFlip;
    m_fScaleX_Rate = 0.0f;
    m_fCurrentPosY = y - 100.0f; // 初始 Y 座標有較大偏移
    m_fAlpha_Rate = 0.5f;       // 1058642330 (0x3F000000)

    if (bFlip) {
        m_fCurrentPosX = x - 3.0f;
    }
    else {
        m_fCurrentPosX = x - 28.0f;
    }

    m_dwResourceID = resourceID;
}

// 對應反組譯碼: 0x00531A00
bool CEffect_Player_Goggles::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    // 更新旋轉 (在原始碼中，此成員被用作縮放)
    m_fScale += fFrameCount * 0.5f;

    // 更新顏色值 (在原始碼中，此成員被用作旋轉)
    m_fRotation += fFrameCount * m_fScaleX_Rate;

    // 更新透明度
    m_fAlpha -= fFrameCount * m_fAlpha_Rate;

    // 更新 Y 座標 (上漂)
    m_fCurrentPosY -= fFrameCount * 0.1f;

    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00531AF0
void CEffect_Player_Goggles::Process()
{
    float screenX = m_fCurrentPosX;
    if (!m_isUiEffect) {
        screenX -= static_cast<float>(g_Game_System_Info.ScreenX);
    }

    m_bIsVisible = IsCliping(screenX, 0.0f);
    if (!m_bIsVisible) return;

    m_pImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

    if (m_pImage) {
        float screenY = m_fCurrentPosY;
        if (!m_isUiEffect) {
            screenY -= static_cast<float>(g_Game_System_Info.ScreenY);
        }

        m_pImage->SetPosition(screenX, screenY);
        m_pImage->SetBlockID(m_cInitialFrame);

        float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
        m_pImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
        m_pImage->SetColor(static_cast<unsigned int>(m_fScale));
        m_pImage->SetRotation(static_cast<int>(m_fRotation));

        m_pImage->Process();
    }
}

// 對應反組譯碼: 0x00531C70
void CEffect_Player_Goggles::Draw()
{
    if (m_bIsVisible && m_pImage && m_pImage->IsInUse())
    {
        CDeviceManager::GetInstance()->ResetRenderState();
        m_pImage->Draw();
    }
}