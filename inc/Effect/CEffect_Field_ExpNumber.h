#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Field_ExpNumber
 * @brief �t�d�����o�g��ȼƦr���{�Ǥưʵe�S�ġC
 */
class CEffect_Field_ExpNumber : public CEffectBase {
public:
    CEffect_Field_ExpNumber();
    virtual ~CEffect_Field_ExpNumber();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param expValue ��ܪ��g��ȼƭȡC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    void SetEffect(int expValue, float x, float y);

private:
    static const int MAX_DIGITS = 10; // �̦h���10���

    // --- �����ܼ� (�ھ� Effectall.c @ 0x00535210 ���_) ---
    GameImage** m_pNumberImages;     // �첾 +132 (0x84): ���V�Ʀr GameImage ���а}�C

    char  m_cDigitCount;             // �첾 +146 (0x92): �Ʀr�����
    unsigned char m_ucState;         // �첾 +147 (0x93): �ʵe���A (0:�H�J, 1:�H�X�W�})

    float m_fAlpha;                  // �첾 +148 (0x94): ����z����
    float m_fScale;                  // �첾 +152 (0x98): �����Y��/�C���

    unsigned char m_ucDigitFrames[MAX_DIGITS]; // �첾 +136 (0x88): �x�s�C�ӼƦr���v��ID

    float m_fInitialPosX;            // �첾 +160 (0xA0): ��l X �y��
    float m_fCurrentPosY;            // �첾 +164 (0xA4): ��e Y �y��
    float m_fTotalWidth;             // �첾 +168 (0xA8): �Ʀr�`�e�סA�Ω�~��

    FrameSkip m_FrameSkip;           // �첾 +172 (0xAC)
};