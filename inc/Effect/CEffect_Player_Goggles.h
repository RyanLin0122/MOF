#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Player_Goggles
 * @brief �b�e���W����@����������{�Ǥưʵe�S�ġC
 */
class CEffect_Player_Goggles : public CEffectBase {
public:
    CEffect_Player_Goggles();
    virtual ~CEffect_Player_Goggles();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param x ��l X �y�СC
    /// @param y ��l Y �y�СC
    /// @param bFlip �O�_����½��C
    /// @param resourceID �n�ϥΪ��Ϥ��귽 ID�C
    /// @param isUiEffect �O�_�� UI �S�� (�M�w�y�ЬO�_�ഫ)�C
    void SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int isUiEffect);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005318B0 ���_) ---
    GameImage* m_pImage;              // �첾 +132 (0x84)

    // �ʵe���A
    float   m_fAlpha;            // �첾 +136 (0x88)
    float   m_fScale;            // �첾 +140 (0x8C)
    float   m_fRotation;         // �첾 +144 (0x90)
    char    m_cInitialFrame;     // �첾 +152 (0x98)

    // ��m�P���z
    float   m_fCurrentPosX;      // �첾 +156 (0x9C)
    float   m_fCurrentPosY;      // �첾 +160 (0xA0)
    float   m_fScaleX_Rate;      // �첾 +180 (0xB4): �Y�� X �ܤƲv
    float   m_fAlpha_Rate;       // �첾 +184 (0xB8): Alpha �ܤƲv

    FrameSkip m_FrameSkip;       // �첾 +164 (0xA4)

    unsigned int m_dwResourceID; // �첾 +176 (0xB0)
    int          m_isUiEffect;   // �첾 +188 (0xBC): �O�_�� UI �S��
};