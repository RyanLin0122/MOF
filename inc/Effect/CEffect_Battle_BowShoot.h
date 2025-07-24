#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�
#include <d3dx9math.h>

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Battle_BowShoot
 * @brief �N��}�b�g������g���S�ġC
 */
class CEffect_Battle_BowShoot : public CEffectBase {
public:
    CEffect_Battle_BowShoot();
    virtual ~CEffect_Battle_BowShoot();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�� (����﨤��)�C
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool a4, int hitInfoID);

    /// @brief �]�w�S�� (�y�й�y��)�C
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052D770 ���_) ---

    // CEffectBase �w�]�t m_ccaEffect (�첾+36)

    ClientCharacter* m_pTargetCharacter;  // �첾 +140 (0x8C)
    float   m_fTotalDistance;    // �첾 +136 (0x88)
    float   m_fTraveledDistance; // �첾 +132 (0x84)

    // �M�Ω󲾰ʩM�ͩR�g���� FrameSkip ����
    FrameSkip m_MovementFrameSkip; // �첾 +144 (0x90)

    float   m_fAngle;            // �첾 +156 (0x9C)
    int     m_nHitInfoID;        // �첾 +160 (0xA0)

    // ���F�٭� SetEffect ���������{���ܼƭp��
    float m_fTempCalculated[10]; // �첾 +164 (0xA4) �� +200 (0xC8)
};