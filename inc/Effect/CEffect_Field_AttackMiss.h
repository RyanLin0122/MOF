#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Field_AttackMiss
 * @brief �b�e���W��� "MISS" �r�˪��{�Ǥưʵe�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�Ω���ܧ������R������ı�^�X�C
 * ��ʵe�]�H�J�B�W�}�B�H�X�^�����ѵ{���X����C
 */
class CEffect_Field_AttackMiss : public CEffectBase {
public:
    CEffect_Field_AttackMiss();
    virtual ~CEffect_Field_AttackMiss();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�Ī���l��m�C
    void SetEffect(float x, float y);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005341C0 ���_) ---
    GameImage* m_pMissImage;        // �첾 +132 (0x84): ���V "MISS" �r�˪� GameImage

    // �ʵe���A
    unsigned char m_ucState;        // �첾 +136 (0x88): 0=�H�J, 1=�H�X�W�}
    float         m_fAlpha;         // �첾 +140 (0x8C): ��e�z����
    float         m_fColorValue;    // �첾 +144 (0x90): ��e�C���

    // ��m
    float         m_fInitialPosX;   // �첾 +152 (0x98): ��l X �y��
    float         m_fCurrentPosY;   // �첾 +156 (0x9C): ��e Y �y��

    // �p�ɾ�
    FrameSkip m_FrameSkip;          // �첾 +160 (0xA0)
};