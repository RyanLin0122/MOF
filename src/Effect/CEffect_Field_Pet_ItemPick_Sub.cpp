#include "Effect/CEffect_Field_Pet_ItemPick_Sub.h"
#include "Effect/CEAManager.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

// ... ���]�������ܼ� ...

// =======================================================================
// CEffect_Field_Pet_ItemPick_Sub_Complete
// =======================================================================

CEffect_Field_Pet_ItemPick_Sub_Complete::CEffect_Field_Pet_ItemPick_Sub_Complete()
{
    CEAManager::GetInstance()->GetEAData(106, "MoFData/Effect/Pet-ItemPick.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false);
}

// ... (�Ѻc, Process, Draw �禡�P��L "Once" �����S������) ...

// =======================================================================
// CEffect_Field_Pet_ItemPick_Sub_Light
// =======================================================================

// �����ϲ�Ķ�X: 0x00536EE0
CEffect_Field_Pet_ItemPick_Sub_Light::CEffect_Field_Pet_ItemPick_Sub_Light()
{
    m_pLightImage = nullptr;
    m_fAlpha = 100.0f; // 1120403456
    m_FrameSkip.m_fTimePerFrame = 1.0f / 30.0f; // 995783694
}

// ... (�Ѻc�禡) ...

// �����ϲ�Ķ�X: 0x00536F20
void CEffect_Field_Pet_ItemPick_Sub_Light::SetEffect(float x, float y, bool bFlip)
{
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x00536F40
bool CEffect_Field_Pet_ItemPick_Sub_Light::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (m_FrameSkip.Update(fElapsedTime, frameCount)) {
        m_fAlpha -= static_cast<float>(frameCount);
    }
    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x00536FD0
void CEffect_Field_Pet_ItemPick_Sub_Light::Process()
{
    m_pLightImage = cltImageManager::GetInstance()->GetGameImage(7, 0x1D00011A, 0, 1);
    if (m_pLightImage)
    {
        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

        m_pLightImage->SetPosition(screenX, screenY);
        m_pLightImage->SetColor(static_cast<unsigned int>(m_fAlpha)); // ��l�X�N Alpha �]�� Color
        m_pLightImage->Process();
    }
}

// �����ϲ�Ķ�X: 0x00537070
void CEffect_Field_Pet_ItemPick_Sub_Light::Draw()
{
    if (m_pLightImage && m_pLightImage->IsInUse()) {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 0xA -> 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);     // 2
        m_pLightImage->Draw();
    }
}