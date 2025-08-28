#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Battle_HorizonCut
 * @brief 代表水平斬擊的刀光或劍氣特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放一次的簡單動畫特效。
 */
class CEffect_Battle_HorizonCut : public CEffectBase {
public:
    CEffect_Battle_HorizonCut();
    virtual ~CEffect_Battle_HorizonCut();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的顯示位置和方向。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    /// @param bFlip 是否水平翻轉。
    /// @param a5 未使用的參數，保留以符合原始碼。
    void SetEffect(float x, float y, bool bFlip, unsigned char a5);
};