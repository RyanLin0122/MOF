#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Skill_Trap_Explosion
 * @brief �N����Ĳ�o�ɪ��z����ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м��񪺤@���ʰʵe�S�ġA
 * ��~�[�|�ھڧޯ൥���ܤơC
 */
class CEffect_Skill_Trap_Explosion : public CEffectBase {
public:
    CEffect_Skill_Trap_Explosion();
    virtual ~CEffect_Skill_Trap_Explosion();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    /// @param skillLevel �ޯ൥�� (0-2)�A�M�w�S�Ī��~�[�C
    void SetEffect(float x, float y, unsigned char skillLevel);

protected:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005340B0 ���_) ---

    /// @brief �S�ĳЫخɪ� X �y�ЧַӡC
    float m_fInitialPosX;       // �첾 +132 (0x84)

    /// @brief �S�ĳЫخɪ� Y �y�ЧַӡC
    float m_fInitialPosY;       // �첾 +136 (0x88)
};