#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>


/**
 * @class CEffect_Battle_MagicBook
 * @brief �N��q�]�k�Ѥ��g�X����g���S�ġ]�Ҧp�����^�C
 */
class CEffect_Battle_MagicBook : public CEffectBase {
public:
    CEffect_Battle_MagicBook();
    virtual ~CEffect_Battle_MagicBook();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�� (����﨤��)�C
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool bFlip, int hitInfoID);

    /// @brief �]�w�S�� (�y�й�y��)�C
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052EBA0 ���_) ---

    ClientCharacter* m_pTargetCharacter;  // �첾 +140 (0x8C)
    float   m_fTotalDistance;    // �첾 +136 (0x88)
    float   m_fTraveledDistance; // �첾 +132 (0x84)

    // �M�Ω󲾰ʩM�ͩR�g���� FrameSkip ����
    FrameSkip m_MovementFrameSkip; // �첾 +144 (0x90)

    float   m_fAngle;            // �첾 +156 (0x9C)
    int     m_nHitInfoID;        // �첾 +160 (0xA0)
};