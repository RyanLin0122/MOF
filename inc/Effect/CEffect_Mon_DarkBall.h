#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip
#include <d3dx9math.h>

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Mon_DarkBall
 * @brief �N��Ǫ��I�񪺶·t�y��g���S�ġC
 */
class CEffect_Mon_DarkBall : public CEffectBase {
public:
    CEffect_Mon_DarkBall();
    virtual ~CEffect_Mon_DarkBall();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�� (����﨤��)�C
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, float fFlip, int hitInfoID);

    /// @brief �]�w�S�� (�y�й�y��)�C
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00530770 ���_) ---

    float   m_fTraveledDistance; // �첾 +132 (0x84)
    float   m_fTotalDistance;    // �첾 +136 (0x88)
    ClientCharacter* m_pTargetCharacter;  // �첾 +140 (0x8C)

    // �M�Ω󲾰ʩM�ͩR�g���� FrameSkip ����
    FrameSkip m_MovementFrameSkip; // �첾 +144 (0x90)

    float   m_fAngle;            // �첾 +156 (0x9C)
    int     m_nHitInfoID;        // �첾 +160 (0xA0)
};