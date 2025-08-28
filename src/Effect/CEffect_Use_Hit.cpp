#include "Effect/CEffect_Use_Hit.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>

// 對應反組譯碼: 0x0052F870
CEffect_Use_Hit::CEffect_Use_Hit()
{
    m_pImage = nullptr;
    m_fAlpha = 250.0f;        // 1133903872 (0x437A0000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    m_fRotation = 0.0f;
    m_cCurrentFrame = 0;
    m_cDirectionType = 0;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fScaleX_Rate = 1.0f;    // 1065353216 (0x3F800000)
    m_fAlpha_Rate = 1.0f;     // 1065353216 (0x3F800000)
    m_dwResourceID = 0;
    m_fFrameCounter = 0.0f;

    // 原始碼: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

// 對應反組譯碼: 0x0052F920
CEffect_Use_Hit::~CEffect_Use_Hit()
{
}

// 對應反組譯碼: 0x0052F940
void CEffect_Use_Hit::SetEffect(float x, float y, unsigned char directionType)
{
    m_fFrameCounter = 0.0f;
    m_cDirectionType = directionType;
    m_fCurrentPosY = y - 55.0f;
    m_dwResourceID = 184550916; // 0xB0005A4

    switch (directionType)
    {
    case 1:
        m_fCurrentPosX = x - 15.0f;
        break;
    case 0:
        m_fCurrentPosX = x - 30.0f;
        break;
    case 2:
        m_fCurrentPosX = x - 20.0f;
        break;
    default:
        // 在原始碼中，如果 directionType 不是 0, 1, 2，
        // m_fCurrentPosX 會使用一個未初始化的值，然後再減 8。
        // 這可能是一個 bug，或者依賴於特定的呼叫順序。
        // 為了安全，我們給一個預設值。
        m_fCurrentPosX = x - 8.0f;
        break;
    }
}

// 對應反組譯碼: 0x0052F9D0
bool CEffect_Use_Hit::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    // 更新旋轉 (在原始碼中，此成員被用作縮放)
    m_fScale += fFrameCount * 0.5f;

    // 更新透明度
    m_fAlpha -= fFrameCount * m_fAlpha_Rate * 1.4f;

    // 更新浮點數影格計數器
    m_fFrameCounter += fFrameCount * m_fScaleX_Rate * 0.04f;

    // 如果計數器超過3，表示動畫序列結束，且如果透明度已很低則銷毀
    if (m_fFrameCounter >= 3.0f) {
        m_fFrameCounter = 3.0f;
        if (m_fAlpha < 45.0f) {
            return true;
        }
    }

    // 更新要顯示的整數影格
    m_cCurrentFrame = static_cast<char>(m_fFrameCounter);

    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x0052FAF0
void CEffect_Use_Hit::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_pImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

        if (m_pImage) {
            float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

            m_pImage->SetPosition(screenX, screenY);
            m_pImage->SetBlockID(m_cCurrentFrame);

            float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
            m_pImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
            // 原始碼將 m_fAlpha 也設定給了 Color
            m_pImage->SetColor(static_cast<unsigned int>(clampedAlpha));
            m_pImage->SetScale(static_cast<int>(m_fScale));

            m_pImage->Process();
        }
    }
}

// 對應反組譯碼: 0x0052FC40
void CEffect_Use_Hit::Draw()
{
    if (m_bIsVisible && m_pImage && m_pImage->IsInUse())
    {
        // 設定加亮混合模式
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 0xA -> 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // 2

        m_pImage->Draw();

        // 恢復預設混合模式
        CDeviceManager::GetInstance()->ResetRenderState();
    }
}