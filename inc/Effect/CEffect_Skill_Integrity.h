#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Integrity
 * @brief 代表角色處於格擋或減傷狀態時的持續性視覺特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放的動畫特效，
 * 其生命週期由目標角色的特殊狀態旗標動態控制。
 */
class CEffect_Skill_Integrity : public CEffectBase {
public:
    CEffect_Skill_Integrity();
    virtual ~CEffect_Skill_Integrity();

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
    /// @param a3 未使用的參數，保留以符合原始碼。
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
    int              m_nUnknown_a3;     // 位移 +136 (0x88)
    float            m_fInitialPosX;    // 位移 +144 (0x90)
    float            m_fInitialPosY;    // 位移 +148 (0x94)
};