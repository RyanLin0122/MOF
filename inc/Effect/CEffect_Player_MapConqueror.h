#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Player_MapConqueror
 * @brief 代表玩家作為地圖佔領者的持續性光環特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的動畫特效，
 * 其生命週期由角色是否仍為地圖佔領者動態控制。
 */
class CEffect_Player_MapConqueror : public CEffectBase {
public:
    CEffect_Player_MapConqueror();
    virtual ~CEffect_Player_MapConqueror();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據角色是否仍為地圖佔領者來決定是否提前終止特效。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pOwner 要附加特效的角色。
    void SetEffect(ClientCharacter* pOwner);

protected:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};