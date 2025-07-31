#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Player_EnchantLevel
 * @brief 代表玩家裝備強化等級的持續性光環特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的動畫特效，
 * 其生命週期由角色狀態動態控制。
 */
class CEffect_Player_EnchantLevel : public CEffectBase {
public:
    CEffect_Player_EnchantLevel();
    virtual ~CEffect_Player_EnchantLevel();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據角色狀態決定是否提前終止特效。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pOwner 要附加特效的角色。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName);

protected:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};