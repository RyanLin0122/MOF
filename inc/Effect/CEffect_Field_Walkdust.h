#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Field_Walkdust
 * @brief �t�d�b�a���W���ͷϹСB�Фg�����ĪG�����O�C
 * @note �����O�~�Ӧ� CEffectBase�A�O�@�ӥѨ�L�S��Ĳ�o���@���ʮĪG�C
 */
class CEffect_Field_Walkdust : public CEffectBase {
public:
    CEffect_Field_Walkdust();
    virtual ~CEffect_Field_Walkdust();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    /// @brief �]�w�ϹЯS�Ī��ݩʡC
    void SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int a6, char frameID, bool bMoveRight = false);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005371D0 ���_) ---
    // m_fCurrentPosX, m_fCurrentPosY, m_bIsVisible ���w�q CEffectBase �~��

    GameImage* m_pEffectImage;     // �첾 +132 (0x84)

    // �S�Ī��A
    float   m_fAlpha;            // �첾 +136 (0x88)
    float   m_fScale;            // �첾 +140 (0x8C)
    float   m_fRotation;         // �첾 +144 (0x90)
    char    m_cInitialFrame;     // �첾 +152 (0x98)
    // m_bFlipX �b CEffectBase ���w�w�q (�첾+28)

    unsigned int m_dwResourceID; // �첾 +176 (0xB0)
    float   m_fScaleX_Rate;      // �첾 +180 (0xB4)
    float   m_fAlpha_Rate;       // �첾 +184 (0xB8)

    // �p�ɾ�
    FrameSkip m_FrameSkip;       // �첾 +164 (0xA4)

    bool m_bMoveRight;           // �첾 +188 (0xBC), �Ω󱱨�}����V
};