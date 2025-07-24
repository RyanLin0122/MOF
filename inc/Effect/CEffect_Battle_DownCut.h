#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Battle_DownCut
 * @brief �N��U�A�������M���μC��S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м���@����²��ʵe�S�ġC
 */
class CEffect_Battle_DownCut : public CEffectBase {
public:
    CEffect_Battle_DownCut();
    virtual ~CEffect_Battle_DownCut();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī���ܦ�m�M��V�C
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    /// @param bFlip �O�_����½��C
    /// @param a5 ���ϥΪ��ѼơA�O�d�H�ŦX��l�X�C
    void SetEffect(float x, float y, bool bFlip, unsigned char a5);
};