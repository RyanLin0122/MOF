#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h"

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_UI_HP_MP_Number
 * @brief �b UI �h��� HP �� MP �ܤƼƦr���{�Ǥưʵe�S�ġC
 */
class CEffect_UI_HP_MP_Number : public CEffectBase {
public:
    CEffect_UI_HP_MP_Number();
    virtual ~CEffect_UI_HP_MP_Number();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param value ��ܪ��ƭȡC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    /// @param type ���� (0: HP-���, 1: MP-�Ŧ�)�C
    void SetEffect(int value, float x, float y, int type);

private:
    static const int MAX_DIGITS = 10; // �̦h���10���

    // --- �����ܼ� (�ھ� Effectall.c @ 0x005329C0 ���_) ---
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