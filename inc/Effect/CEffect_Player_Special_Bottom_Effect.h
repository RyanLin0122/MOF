#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Player_Special_Bottom_Effect
 * @brief 代表玩家特殊狀態的、位於底層的持續性光環特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的動畫特效，
 * 其生命週期由外部邏輯動態控制。
 */
class CEffect_Player_Special_Bottom_Effect : public CEffectBase {
public:
    CEffect_Player_Special_Bottom_Effect();
    virtual ~CEffect_Player_Special_Bottom_Effect();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據擁有者指標是否有效來決定是否提前終止特效。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pOwner 要附加特效的角色。
    /// @param effectType 特效的類型，決定了其外觀。
    void SetEffect(ClientCharacter* pOwner, unsigned char effectType);

protected:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
    unsigned char    m_ucEffectType;    // 位移 +136 (0x88)
};