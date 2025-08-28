#include "Effect/CEffect_Field_Walkdust.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib> // for rand()

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x005371D0
CEffect_Field_Walkdust::CEffect_Field_Walkdust() : CEffectBase() // 正確呼叫基底類別建構函式
{
    m_pEffectImage = nullptr;
    m_fAlpha = 250.0f;       // 1137180672
    m_fScale = 50.0f;        // 1092616192
    m_fRotation = 0.0f;
    m_cInitialFrame = rand() % 4; // 隨機選擇 0-3 的初始影格
    m_bIsFlip = false;
    m_dwResourceID = 0;
    m_fScaleX_Rate = 1.0f;   // 1065353216
    m_fAlpha_Rate = 1.0f;    // 1065353216
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_bMoveRight = false;
    m_bIsVisible = FALSE;

    // 原始碼: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

CEffect_Field_Walkdust::~CEffect_Field_Walkdust()
{
    // GameImage 由 cltImageManager 管理，此處不需 delete
}

void CEffect_Field_Walkdust::SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int a6, char frameID, bool bMoveRight)
{
    // --- 關鍵修正 ---
    // 將對未定義成員 m_bFlipX 的賦值，
    // 改為對從 CEffectBase 繼承來的成員 m_bIsFlip 賦值。
    m_bIsFlip = bFlip;

    m_fCurrentPosY = y - 13.0f;

    // ... 函式的其餘部分不變 ...
    if (a6 != 0) {
        m_fScaleX_Rate = 0.0f;
        m_cInitialFrame = frameID;
        m_fAlpha_Rate = 0.5f;
    }

    if (bFlip) {
        m_fCurrentPosX = x + 7.0f;
    }
    else {
        m_fCurrentPosX = x - 35.0f;
    }

    m_dwResourceID = resourceID;
    if (resourceID == 0) {
        m_dwResourceID = 184550807; //0xB000597
    }

    m_bMoveRight = bMoveRight;
}

// 對應反組譯碼: 0x00537330
bool CEffect_Field_Walkdust::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false; // 如果沒有影格推進，則不做任何事
    }

    // 更新旋轉
    m_fRotation += static_cast<float>(frameCount) * 0.5f;

    // 更新縮放
    m_fScale += static_cast<float>(frameCount) * m_fScaleX_Rate;

    // 更新透明度
    m_fAlpha -= static_cast<float>(frameCount) * m_fAlpha_Rate;

    // 更新 Y 座標 (上升)
    m_fCurrentPosY -= static_cast<float>(frameCount) * 0.1f;

    // 檢查生命週期是否結束
    if (m_fAlpha < 0.0f) {
        return true;
    }

    // 如果設定了水平漂移
    if (m_bMoveRight) {
        m_fCurrentPosX += 0.25f;
    }

    return false;
}

// 對應反組譯碼: 0x00537430
void CEffect_Field_Walkdust::Process()
{
    float screenX = m_fCurrentPosX;
    if (!m_bMoveRight) {
        screenX -= static_cast<float>(g_Game_System_Info.ScreenWidth);
    }

    // --- 關鍵修正 ---
    // 因為現在繼承自 CEffectBase，可以直接呼叫 IsCliping 成員函式。
    m_bIsVisible = IsCliping(screenX, 0.0f);
    if (!m_bIsVisible) return;

    // 獲取 GameImage
    m_pEffectImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

    if (m_pEffectImage) {
        float screenY = m_bMoveRight ? m_fCurrentPosY : m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

        m_pEffectImage->SetPosition(screenX, screenY);
        m_pEffectImage->SetBlockID(m_cInitialFrame);

        float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
        m_pEffectImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));

        m_pEffectImage->SetColor(static_cast<unsigned int>(m_fScale));
        m_pEffectImage->SetRotation(static_cast<int>(m_fRotation));

        m_pEffectImage->Process();
    }
}

// 對應反組譯碼: 0x005375B0
void CEffect_Field_Walkdust::Draw()
{
    if (m_bIsVisible && m_pEffectImage && m_pEffectImage->IsInUse())
    {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        m_pEffectImage->Draw();
    }
}