#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�
#include <d3dx9math.h>

class ClientCharacter;

/**
 * @class CEffect_Field_Pet_ItemPick
 * @brief �d���B�����~���޿豱��P�y��ͦ����C
 */
class CEffect_Field_Pet_ItemPick : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick();
    virtual ~CEffect_Field_Pet_ItemPick();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override; // ���禡���šA�]������������i��

    /// @brief �]�w�S�Ī��_�I�M�ؼСC
    void SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID);

private:
    ClientCharacter* m_pOwnerPet;      // �첾 +132 (0x84): ���V�ؼ��d��
    unsigned int     m_dwOwnerAccountID; // �첾 +136 (0x88): �d���֦��̪��b�� ID

    unsigned int m_dwTotalFrame;       // �첾 +140 (0x8C): �`�p�ɾ�
    unsigned char m_ucState;           // �첾 +144 (0x90): ���檬�A�� (0-4)

    // ����y�����
    float m_fSpeedFactor;        // �첾 +148 (0x94)
    float m_fCurrentPosX;        // �첾 +156 (0x9C)
    float m_fCurrentPosY;        // �첾 +160 (0xA0)
    float m_fTargetPosX;         // �첾 +172 (0xAC)
    float m_fTargetPosY;         // �첾 +176 (0xB0)

    FrameSkip m_FrameSkip;       // �첾 +180 (0xB4)

    int m_nSubEffectTimer;       // �첾 +192 (0xC0): �ͦ��l�S�Ī��p�ɾ�
};