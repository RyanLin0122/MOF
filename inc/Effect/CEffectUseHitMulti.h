#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���FFrameSkip ���ϥ�

// �e�V�ŧi
class GameImage;

struct Particle {
    GameImage* pImage;         // �첾 +0: ���V���ɤl������ GameImage
    float       fAlpha;         // �첾 +4: ��e�z����/�ͩR�g��
    float       fScale;         // �첾 +8: ��e�Y��/�C���
    char        cFrameIndex;    // �첾 +12: �ɤl���ʵe�v��
    char        ucOwnerDirection; // �첾 +13: �Ыخɾ֦��̪���V
    char        padding[2];     // �첾 +14
    float       fPosX;          // �첾 +16
    float       fPosY;          // �첾 +20
    float       fAngle;         // �첾 +28: ���ਤ��
    float       fVelX;          // �첾 +32: X �b�t��/��V
    float       fVelY;          // �첾 +36: Y �b�t��/��V
    float       fRotation_Unused; // �첾 +40 (��l�X v13[9])
    float       fSpeed;         // �첾 +48: �t�׫Y��
    bool        bIsActive;      // �첾 +52
    bool        bIsVisible;     // �첾 +53
    char        padding2[10];
};

/**
 * @class CEffectUseHitMulti
 * @brief �@�Ӱ��ɤl�t�Ϊ��ƦX�S�ġA�Ω󲣥��z���B�����i���h�ɤl�ĪG�C
 */
class CEffectUseHitMulti : public CEffectBase {
public:
    CEffectUseHitMulti();
    virtual ~CEffectUseHitMulti();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�ê�l�Ʋɤl�t�ίS�ġC
    /// @param x �S�Ĥ����I X �y�СC
    /// @param y �S�Ĥ����I Y �y�СC
    /// @param ucOwnerDirection �֦��̪���V (0: ��, 1: �k)�C
    /// @param effectType �S�Ī����� (1-4)�A�M�w�F�ɤl���欰�M�~�[�C
    void SetEffect2(float x, float y, unsigned char ucOwnerDirection, int effectType);

private:
    static const int MAX_PARTICLES = 15;

    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052FCD0 ���_) ---
    Particle m_Particles[MAX_PARTICLES]; // �첾 +132 (0x84), 15 * 64 = 960 bytes

    unsigned int m_dwResourceID; // �첾 +1092 (0x444), �S�ĨϥΪ� GameImage �귽 ID
    unsigned int m_uParticleCount; // �첾 +1096 (0x448), ���S�Ĺ�ڲ��ͪ��ɤl�ƶq
    int          m_nEffectType;    // �첾 +1100 (0x44C), �x�s SetEffect2 �ǤJ������

    FrameSkip    m_FrameSkip;      // �첾 +1104 (0x450)

    bool         m_bUseAdditiveBlend; // �첾 +1116 (0x45C), �Ω�M�w Draw �ɪ��V�X�Ҧ�
};