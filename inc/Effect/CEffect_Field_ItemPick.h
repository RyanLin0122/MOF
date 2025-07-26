#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>


/**
 * @class CEffect_Field_ItemPick
 * @brief ���a�B�����~���޿豱��P�y��ͦ����C
 */
class CEffect_Field_ItemPick : public CEffectBase {
public:
    CEffect_Field_ItemPick();
    virtual ~CEffect_Field_ItemPick();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    /// @brief �]�w�S�Ī��_�I�M�ؼЪ��a�C
    void SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID);

private:
    ClientCharacter* m_pOwnerCharacter;   // �첾 +132 (0x84): ���V�ؼЪ��a
    unsigned int     m_dwOwnerAccountID;  // �첾 +136 (0x88): ���a���b�� ID

    unsigned int m_dwTotalFrame;        // �첾 +140 (0x8C): �`�p�ɾ�
    unsigned char m_ucState;            // �첾 +144 (0x90): ���檬�A�� (0-4)

    float m_fSpeedFactor;         // �첾 +148 (0x94)
    // char m_cUnk152;            // �첾 +152 (0x98)

    float m_fCurrentPosX;         // �첾 +156 (0x9C)
    float m_fCurrentPosY;         // �첾 +160 (0xA0)
    float m_fTargetPosX;          // �첾 +172 (0xAC)
    float m_fTargetPosY;          // �첾 +176 (0xB0)

    FrameSkip m_FrameSkip;        // �첾 +180 (0xB4)

    int m_nSubEffectTimer;        // �첾 +192 (0xC0): �ͦ��l�S�Ī��p�ɾ�
};