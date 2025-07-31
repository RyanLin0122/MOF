#include "Effect/CEffect_Field_ItemPick_Sub.h"
#include "Effect/CEAManager.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

// ... 假設的全域變數 ...

// =======================================================================
// CEffect_Field_ItemPick_Sub_Complete
// =======================================================================

// 對應反組譯碼: 0x005362F0
CEffect_Field_ItemPick_Sub_Complete::CEffect_Field_ItemPick_Sub_Complete()
{
    CEAManager::GetInstance()->GetEAData(105, "Effect/efn_ItemPick_01.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false);
}
CEffect_Field_ItemPick_Sub_Complete::~CEffect_Field_ItemPick_Sub_Complete() {}

// 對應反組譯碼: 0x00536390
void CEffect_Field_ItemPick_Sub_Complete::SetEffect(float x, float y, bool bFlip)
{
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFlip = bFlip;
}

// 對應反組譯碼: 0x005363B0
bool CEffect_Field_ItemPick_Sub_Complete::FrameProcess(float fElapsedTime)
{
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x005363C0
void CEffect_Field_ItemPick_Sub_Complete::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00536400
void CEffect_Field_ItemPick_Sub_Complete::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}

// =======================================================================
// CEffect_Field_ItemPick_Sub_Light
// =======================================================================

// 對應反組譯碼: 0x00536410
CEffect_Field_ItemPick_Sub_Light::CEffect_Field_ItemPick_Sub_Light()
{
    m_pLightImage = nullptr;
    m_fAlpha = 100.0f; // 1120403456
    m_FrameSkip.m_fTimePerFrame = 1.0f / 30.0f; // 995783694
}

CEffect_Field_ItemPick_Sub_Light::~CEffect_Field_ItemPick_Sub_Light() {}

// 對應反組譯碼: 0x005364B0
void CEffect_Field_ItemPick_Sub_Light::SetEffect(float x, float y, bool bFlip)
{
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFlip = bFlip;
}

// 對應反組譯碼: 0x005364D0
bool CEffect_Field_ItemPick_Sub_Light::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (m_FrameSkip.Update(fElapsedTime, frameCount)) {
        m_fAlpha -= static_cast<float>(frameCount);
    }
    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00536560
void CEffect_Field_ItemPick_Sub_Light::Process()
{
    // 資源 ID: 0x1D00011A
    m_pLightImage = cltImageManager::GetInstance()->GetGameImage(7, 0x1D00011A, 0, 1);
    if (m_pLightImage)
    {
        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

        m_pLightImage->SetPosition(screenX, screenY);
        m_pLightImage->SetColor(static_cast<unsigned int>(m_fAlpha));
        m_pLightImage->Process();
    }
}

// 對應反組譯碼: 0x00536600
void CEffect_Field_ItemPick_Sub_Light::Draw()
{
    if (m_pLightImage && m_pLightImage->IsInUse()) {
        // 使用加亮混合模式
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLEND_ZERO);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        m_pLightImage->Draw();
    }
}