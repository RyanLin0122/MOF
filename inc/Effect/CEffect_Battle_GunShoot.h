#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // �]�t FrameSkip ���w�q
#include <d3dx9math.h>

// �e�V�ŧi
class ClientCharacter;
class CEffect_Field_Walkdust;
class CEffect_Battle_GunSpark;

class CEffect_Battle_GunShoot : public CEffectBase {
public:
    CEffect_Battle_GunShoot();
    virtual ~CEffect_Battle_GunShoot();

    // ... (��L�禡�ŧi����) ...
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short a4, int hitInfoID);
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip, unsigned short a5, int hitInfoID);

private:
    // --- �����ܼ� (�ץ��P�s�W) ---

    // ��l�X *((_DWORD *)this + 33) ����m�A�{�b���T��γ~
    GameImage* m_pBulletImage;      // �첾 +132 (0x84): �N��l�u/�u�D�� GameImage

    ClientCharacter* m_pTargetCharacter;  // �첾 +136 (0x88): �ؼШ���

    float   m_fTotalDistance;    // �첾 +140 (0x8C): �`�Z��
    float   m_fTraveledDistance; // �첾 +144 (0x90): �w����Z��

    FrameSkip m_MovementFrameSkip; // �첾 +148 (0x94)

    int     m_nHitInfoID;        // �첾 +160 (0xA0)
    int     m_dwCasterAccountID; // �첾 +164 (0xA4)
    unsigned short m_usUnk_a4;   // �첾 +168 (0xA8): SetEffect �ǤJ�������Ѽ� a4
};