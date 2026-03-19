#include "Object/CEffectObject.h"
#include "Effect/CEffect_MapEffect.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"

CEffectObject::CEffectObject()
    : CBaseObject()
{
    // Ground truth constructor only sets vftable, does not initialize m_pEffect
    // m_pEffect is zeroed in InitEffectObject before use
}

CEffectObject::~CEffectObject()
{
}

int CEffectObject::InitEffectObject(unsigned short wEffectKindID, unsigned short wPlacementType,
                                     unsigned short wObjectKind, ClientCharacter* pCharacter,
                                     int nPosX, int nPosY)
{
    m_pEffect = nullptr;

    CEffect_MapEffect* pEffect = new CEffect_MapEffect();
    m_pEffect = pEffect;

    pEffect->SetEffect(wEffectKindID, wObjectKind, pCharacter, nPosX, nPosY);

    if (wPlacementType == 3)
    {
        // TOP type: add to bullet list (managed by effect manager), not by us
        CEffectManager::GetInstance()->BulletAdd(m_pEffect);
        return 0;
    }
    else
    {
        // NORMAL type: this object manages the effect
        m_nPosY = nPosY;
        m_nPosX = nPosX;
        m_siType = wPlacementType;
        return 1;
    }
}

void CEffectObject::FrameProcess(float fElapsedTime)
{
    if (m_pEffect)
        m_pEffect->FrameProcess(fElapsedTime);
}

void CEffectObject::Process()
{
    if (m_pEffect)
        m_pEffect->Process();
}

void CEffectObject::Draw()
{
    if (m_pEffect)
    {
        m_pEffect->Draw();
        CDeviceManager::GetInstance()->ResetRenderState();
    }
}
