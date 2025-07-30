#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip

// �e�V�ŧi
class GameImage;

/**
 * @class CEffect_MiniGame_Archer_String
 * @brief �b�p�C������ܤ}�b��������ܤ�r���{�Ǥưʵe�S�ġC
 */
class CEffect_MiniGame_Archer_String : public CEffectBase {
public:
    CEffect_MiniGame_Archer_String();
    virtual ~CEffect_MiniGame_Archer_String();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�S�ġC
    /// @param cFrameIndex �n��ܪ���r�b���ɤ����v����ޡC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    void SetEffect(char cFrameIndex, float x, float y);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005384D0 ���_) ---
    GameImage* m_pImage;              // �첾 +132 (0x84)

    // �ʵe���A
    char          m_cFrameIndex;     // �첾 +136 (0x88): �n��ܪ���r�v��
    unsigned char m_ucState;         // �첾 +137 (0x89): �ʵe���A��
    float         m_fAlpha;          // �첾 +140 (0x8C): ��e�z����
    float         m_fScale;          // �첾 +144 (0x90): ��e�Y��/�C���

    // ��m
    float         m_fInitialPosX;    // �첾 +152 (0x98): ��l X �y��
    float         m_fCurrentPosY;    // �첾 +156 (0x9C): ��e Y �y��

    // �p�ɾ�
    FrameSkip m_FrameSkip;           // �첾 +160 (0xA0)
};