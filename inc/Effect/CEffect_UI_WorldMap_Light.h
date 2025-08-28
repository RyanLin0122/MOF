#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_UI_WorldMap_Light
 * @brief 在 UI 世界地圖上顯示的持續循環光點特效。
 *
 * 繼承自 CEffectBase，是一個在固定螢幕座標播放的動畫特效，
 * 其生命週期由外部邏輯控制。
 */
class CEffect_UI_WorldMap_Light : public CEffectBase {
public:
    CEffect_UI_WorldMap_Light();
    virtual ~CEffect_UI_WorldMap_Light();

    // --- 虛擬函式覆寫 ---

    /// @brief 更新動畫，生命週期由 m_bIsFinished 旗標控制。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的顯示位置。
    /// @param x 螢幕 X 座標。
    /// @param y 螢幕 Y 座標。
    void SetEffect(float x, float y);

    /// @brief 設定特效的結束旗標。
    void SetFinished(bool bFinished) { m_bIsFinished = bFinished; }

    /// @brief 設定一個未知的禁用旗標。
    void SetDisable(int disableFlag);

private:
    // --- 成員變數 ---
    bool m_bIsFinished; // 位移 +132 (0x84): 用於外部控制生命週期的旗標
    int  m_nDisableFlag; // 位移 +136 (0x88): 未知用途的旗標
};