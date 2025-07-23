#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Battle_GunSpark
 * @brief 代表槍手射擊時在槍口產生的火花特效。
 *
 * 繼承自 CEffectBase，是一個由 CEffect_Battle_GunShoot 觸發的一次性視覺效果。
 */
class CEffect_Battle_GunSpark : public CEffectBase {
public:
    CEffect_Battle_GunSpark();
    virtual ~CEffect_Battle_GunSpark();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param x 槍口火花的 X 座標。
    /// @param y 槍口火花的 Y 座標。
    /// @param bFlip 是否根據角色朝向水平翻轉。
    void SetEffect(float x, float y, bool bFlip);
};