#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h"

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Use_Hit
 * @brief �b�e���W��ܴ��q�����R�����᪺�{�Ǥưʵe�S�ġC
 */
class CEffect_Use_Hit : public CEffectBase {
public:
    CEffect_Use_Hit();
    virtual ~CEffect_Use_Hit();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param x ��l X �y�СC
    /// @param y ��l Y �y�СC
    /// @param directionType ��V���� (0-2)�A�v�T��l��m�����C
    void SetEffect(float x, float y, unsigned char directionType);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052F870 ���_) ---
    GameImage* m_pImage;              // �첾 +132 (0x84)

    // �ʵe���A
    float   m_fAlpha;            // �첾 +136 (0x88)
    float   m_fScale;            // �첾 +140 (0x8C)
    float   m_fRotation;         // �첾 +144 (0x90)
    char    m_cCurrentFrame;     // �첾 +152 (0x98)
    char    m_cDirectionType;    // �첾 +153 (0x99)

    // ��m�P���z
    float   m_fCurrentPosX;      // �첾 +156 (0x9C)
    float   m_fCurrentPosY;      // �첾 +160 (0xA0)
    float   m_fScaleX_Rate;      // �첾 +180 (0xB4): �Y�� X �ܤƲv
    float   m_fAlpha_Rate;       // �첾 +184 (0xB8): Alpha �ܤƲv

    FrameSkip m_FrameSkip;       // �첾 +164 (0xA4)

    unsigned int m_dwResourceID; // �첾 +176 (0xB0)
    float        m_fFrameCounter;  // �첾 +192 (0xC0): �B�I�Ƽv��p�ƾ�
};