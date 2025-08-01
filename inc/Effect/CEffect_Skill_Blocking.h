#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Blocking
 * @brief 代表角色被定身或阻礙時的持續性視覺特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放的動畫特效，
 * 其生命週期由目標角色的特殊狀態旗標動態控制。
 */
class CEffect_Skill_Blocking : public CEffectBase {
public:
    CEffect_Skill_Blocking();
    virtual ~CEffect_Skill_Blocking();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據目標角色的狀態決定是否提前終止特效。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pTarget 要附加特效的目標角色。
    void SetEffect(ClientCharacter* pTarget);

protected:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
    float m_fInitialPosX;               // 位移 +136 (0x88)
    float m_fInitialPosY;               // 位移 +140 (0x8C)
};