#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_UI_SkillSet
 * @brief �b UI �W��ܧޯ�]�m���\�ɪ���ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�ù��y�м��񪺤@���ʰʵe�S�ġC
 */
class CEffect_UI_SkillSet : public CEffectBase {
public:
    CEffect_UI_SkillSet();
    virtual ~CEffect_UI_SkillSet();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī���ܦ�m�C
    /// @param x �ù� X �y�СC
    /// @param y �ù� Y �y�СC
    void SetEffect(float x, float y);
};