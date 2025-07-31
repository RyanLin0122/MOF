#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Portal
 * @brief �b���w�y����ܤ@�Ӷǰe������ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м��񪺰ʵe�S�ġA
 * ��~�[�i�ѥ~���ǤJ���W�٨M�w�A�ͩR�g���]�i�ѥ~������C
 */
class CEffect_Portal : public CEffectBase {
public:
    CEffect_Portal();
    virtual ~CEffect_Portal();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�ǰe���S�ġC
    /// @param szEffectName �S�Ī��W�٥N�X�A�Ω�d�߯S�ĸ귽�C
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    void SetEffect(char* szEffectName, float x, float y);

    /// @brief �]�w�S�Ī������X�СC
    void SetFinished(bool bFinished) { m_bIsFinished = bFinished; }

private:
    // --- �����ܼ� ---
    bool m_bIsFinished; // �첾 +132 (0x84): �Ω�~������ͩR�g�����X��
};