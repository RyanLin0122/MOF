#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip ���ϥ�

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_Field_CriticalNumber
 * @brief �t�d��ܼ����ˮ`�Ʀr���{�Ǥưʵe�S�ġC
 */
class CEffect_Field_CriticalNumber : public CEffectBase {
public:
    CEffect_Field_CriticalNumber();
    virtual ~CEffect_Field_CriticalNumber();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param damageValue ��ܪ��ˮ`�ƭȡC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    /// @param type ���� (0: ���a��Ǫ�, 1: �Ǫ��缾�a)�A�M�w�Ʀr�C��C
    void SetEffect(int damageValue, float x, float y, int type);

private:
    static const int MAX_DIGITS = 10; // �̦h���10���

    // --- �����ܼ� (�ھ� Effectall.c @ 0x005344C0 ���_) ---
    GameImage** m_pNumberImages;     // �첾 +132 (0x84): ���V�Ʀr GameImage ���а}�C
    GameImage* m_pFlashImage;       // �첾 +136 (0x88): �I���{��
    GameImage* m_pTextImage;        // �첾 +140 (0x8C): �e����r (CRITICAL)

    float m_fScale;                  // �첾 +144 (0x90): �����Y���
    float m_fAlpha;                  // �첾 +148 (0x94): ����z����

    char  m_cDigitCount;             // �첾 +166 (0xA6): �Ʀr�����
    unsigned char m_ucState;         // �첾 +167 (0xA7): �ʵe���A (0:��j, 1:�Y�p, 2:�H�X)
    unsigned short m_usTextImageFrame; // �첾 +204 (0xCC): �e����r���v�� ID

    float m_fInitialPosX;            // �첾 +180 (0xB4): ��l X �y��
    float m_fCurrentPosY;            // �첾 +184 (0xB8): ��e Y �y��
    float m_fTotalWidth;             // �첾 +188 (0xBC): �Ʀr�`�e�סA�Ω�~��

    unsigned char m_ucDigitFrames[MAX_DIGITS]; // �첾 +156 (0x9C): �x�s�C�ӼƦr���v��ID

    FrameSkip m_FrameSkip;           // �첾 +192 (0xC0)
};