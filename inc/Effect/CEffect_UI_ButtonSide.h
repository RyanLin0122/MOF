#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_UI_ButtonSide
 * @brief 在 UI 按鈕側面顯示的持續循環特效。
 *
 * 繼承自 CEffectBase，是一個在固定螢幕座標播放的動畫特效，
 * 其生命週期由外部邏輯控制。
 */
class CEffect_UI_ButtonSide : public CEffectBase {
public:
    CEffect_UI_ButtonSide();
    virtual ~CEffect_UI_ButtonSide();

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

private:
    // --- 成員變數 ---
    bool m_bIsFinished; // 位移 +132 (0x84): 用於外部控制生命週期的旗標
};