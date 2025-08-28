#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Hit_Critical
 * @brief 代表暴擊命中時的視覺效果。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放一次的、比普通命中更華麗的動畫特效。
 */
class CEffect_Hit_Critical : public CEffectBase {
public:
    CEffect_Hit_Critical();
    virtual ~CEffect_Hit_Critical();

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