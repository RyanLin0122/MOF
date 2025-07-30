#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_MiniGame_Priest_Light
 * @brief �N��p�C�������v���t���S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м���@����²��ʵe�S�ġA
 * �q�`�Ω� UI �h�εL��v���u�ʪ������C
 */
class CEffect_MiniGame_Priest_Light : public CEffectBase {
public:
    CEffect_MiniGame_Priest_Light();
    virtual ~CEffect_MiniGame_Priest_Light();

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