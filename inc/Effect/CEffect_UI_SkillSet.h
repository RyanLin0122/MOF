#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_UI_SkillSet
 * @brief 在 UI 上顯示技能設置成功時的視覺特效。
 *
 * 繼承自 CEffectBase，是一個在固定螢幕座標播放的一次性動畫特效。
 */
class CEffect_UI_SkillSet : public CEffectBase {
public:
    CEffect_UI_SkillSet();
    virtual ~CEffect_UI_SkillSet();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
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
};