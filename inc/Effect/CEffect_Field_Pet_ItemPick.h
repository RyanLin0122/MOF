#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>


class CEffect_Field_Pet_ItemPick : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick();
    virtual ~CEffect_Field_Pet_ItemPick();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID);

private:
    ClientCharacter* m_pOwnerPet;
    unsigned int     m_dwOwnerAccountID;
    unsigned int m_dwTotalFrame;
    unsigned char m_ucState;
    float m_fSpeedFactor;
    float m_fCurrentPosX;
    float m_fCurrentPosY;
    float m_fTargetPosX;
    float m_fTargetPosY;
    FrameSkip m_FrameSkip;
    int m_nSubEffectTimer;
};