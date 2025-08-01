#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Skill_Trap_Explosion
 * @brief 代表陷阱觸發時的爆炸視覺特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放的一次性動畫特效，
 * 其外觀會根據技能等級變化。
 */
class CEffect_Skill_Trap_Explosion : public CEffectBase {
public:
    CEffect_Skill_Trap_Explosion();
    virtual ~CEffect_Skill_Trap_Explosion();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    /// @param skillLevel 技能等級 (0-2)，決定特效的外觀。
    void SetEffect(float x, float y, unsigned char skillLevel);

protected:
    // --- 成員變數 (根據 Effectall.c @ 0x005340B0 推斷) ---

    /// @brief 特效創建時的 X 座標快照。
    float m_fInitialPosX;       // 位移 +132 (0x84)

    /// @brief 特效創建時的 Y 座標快照。
    float m_fInitialPosY;       // 位移 +136 (0x88)
};