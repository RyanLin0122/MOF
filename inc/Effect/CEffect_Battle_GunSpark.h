#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Battle_GunSpark
 * @brief �N��j��g���ɦb�j�f���ͪ�����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�ӥ� CEffect_Battle_GunShoot Ĳ�o���@���ʵ�ı�ĪG�C
 */
class CEffect_Battle_GunSpark : public CEffectBase {
public:
    CEffect_Battle_GunSpark();
    virtual ~CEffect_Battle_GunSpark();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param x �j�f���᪺ X �y�СC
    /// @param y �j�f���᪺ Y �y�СC
    /// @param bFlip �O�_�ھڨ���¦V����½��C
    void SetEffect(float x, float y, bool bFlip);
};