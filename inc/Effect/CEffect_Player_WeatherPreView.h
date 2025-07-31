#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Player_WeatherPreView
 * @brief �b UI �h��ܤѮ�Φa�����ҹw�����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�ù��y�м���B
 * �B�ͩR�g���ѥ~���޿豱��ʵe�S�ġC
 */
class CEffect_Player_WeatherPreView : public CEffectBase {
public:
    CEffect_Player_WeatherPreView();
    virtual ~CEffect_Player_WeatherPreView();

    // --- �����禡�мg ---

    /// @brief ��s�ʵe�A���ä��^�� true�A�ϯS�ī���s�b�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w���ù���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param x �ù� X �y�СC
    /// @param y �ù� Y �y�СC
    /// @param baseEffectID �S�Ī���¦ ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(float x, float y, int baseEffectID, char* szFileName);

private:
    // --- �����ܼ� ---
    float m_fScreenPosX; // �첾 +132 (0x84)
    float m_fScreenPosY; // �첾 +136 (0x88)
};