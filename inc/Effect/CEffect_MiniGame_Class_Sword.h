#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_MiniGame_Class_Sword
 * @brief 代表小遊戲中劍士的斬擊特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放一次的簡單動畫特效，
 * 通常用於 UI 層或無攝影機滾動的場景。
 */
class CEffect_MiniGame_Class_Sword : public CEffectBase {
public:
    CEffect_MiniGame_Class_Sword();
    virtual ~CEffect_MiniGame_Class_Sword();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的顯示位置。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    void SetEffect(float x, float y);
};