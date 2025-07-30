#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Hit_Critical
 * @brief �N������R���ɪ���ı�ĪG�C
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м���@�����B�񴶳q�R������R���ʵe�S�ġC
 */
class CEffect_Hit_Critical : public CEffectBase {
public:
    CEffect_Hit_Critical();
    virtual ~CEffect_Hit_Critical();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī���ܦ�m�C
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    void SetEffect(float x, float y);
};