#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_UI_WorldMap_Light
 * @brief �b UI �@�ɦa�ϤW��ܪ�����`�����I�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�ù��y�м��񪺰ʵe�S�ġA
 * ��ͩR�g���ѥ~���޿豱��C
 */
class CEffect_UI_WorldMap_Light : public CEffectBase {
public:
    CEffect_UI_WorldMap_Light();
    virtual ~CEffect_UI_WorldMap_Light();

    // --- �����禡�мg ---

    /// @brief ��s�ʵe�A�ͩR�g���� m_bIsFinished �X�б���C
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

    /// @brief �]�w�S�Ī������X�СC
    void SetFinished(bool bFinished) { m_bIsFinished = bFinished; }

    /// @brief �]�w�@�ӥ������T�κX�СC
    void SetDisable(int disableFlag);

private:
    // --- �����ܼ� ---
    bool m_bIsFinished; // �첾 +132 (0x84): �Ω�~������ͩR�g�����X��
    int  m_nDisableFlag; // �첾 +136 (0x88): �����γ~���X��
};