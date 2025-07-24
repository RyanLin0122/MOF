#pragma once

#include "Effect/CEffectBase.h"
#include <d3dx9math.h>

/**
 * @class CEffect_Battle_StaffShoot
 * @brief �N��k���g�X���]�k��g���S�ġC
 *
 * �~�Ӧ� CEffectBase�A�Ω��{���u���檺�]�k�u�ĪG�C
 */
class CEffect_Battle_StaffShoot : public CEffectBase {
public:
    CEffect_Battle_StaffShoot();
    virtual ~CEffect_Battle_StaffShoot();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param pStartPos ��g�����_�l�@�ɮy�СC
    /// @param pEndPos ��g�����ؼХ@�ɮy�СC
    /// @param bFlip �O�_����½��C
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052F420 ���_) ---

    // CEffectBase �w�]�t m_ccaEffect (�첾+36), m_fSpeed (�첾+24) ��

    float m_fTraveledDistance; // �첾 +132 (0x84): �w���檺�Z��
    float m_fTotalDistance;    // �첾 +136 (0x88): �_�I����I���`�Z��
};