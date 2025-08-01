#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Heal
 * @brief 代表治療技能的視覺特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的一次性動畫特效，
 * 其外觀會根據技能等級變化。
 */
class CEffect_Skill_Heal : public CEffectBase {
public:
    CEffect_Skill_Heal();
    virtual ~CEffect_Skill_Heal();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pTarget 接受治療的目標角色。
    /// @param skillLevel 技能等級 (0-2)，決定特效的外觀。
    void SetEffect(ClientCharacter* pTarget, unsigned char skillLevel);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者（被治療的角色）。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};