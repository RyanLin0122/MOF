#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Field_Miss
 * @brief �b�e���W��� "MISS" �r�˪��{�Ǥưʵe�S�ġA�a���H���}���ĪG�C
 */
class CEffect_Field_Miss : public CEffectBase {
public:
    CEffect_Field_Miss();
    virtual ~CEffect_Field_Miss();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�Ī���l��m�C
    void SetEffect(float x, float y);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00537A40 ���_) ---
    GameImage* m_pMissImage;        // �첾 +132 (0x84): ���V "MISS" �r�˪� GameImage

    // �ʵe���A
    float   m_fAlpha;            // �첾 +136 (0x88)
    float   m_fScale;            // �첾 +140 (0x8C)
    float   m_fRotation;         // �첾 +144 (0x90)
    char    m_cInitialFrame;     // �첾 +152 (0x98)

    // ��m�P���z
    float   m_fCurrentPosX;      // �첾 +156 (0x9C)
    float   m_fCurrentPosY;      // �첾 +160 (0xA0)
    float   m_fVelX;             // �첾 +188 (0xBC): X �b�t��/��V
    float   m_fVelY;             // �첾 +192 (0xC0): Y �b�t��/��V
    float   m_fSpeedFactor;      // �첾 +196 (0xC4): �t�׫Y��

    FrameSkip m_FrameSkip;       // �첾 +164 (0xA4)

    unsigned int m_dwResourceID; // �첾 +176 (0xB0)
};