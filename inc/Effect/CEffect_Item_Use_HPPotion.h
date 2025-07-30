#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Item_Use_HPPotion
 * @brief 代表使用 HP 藥水時的治療特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的一次性動畫特效。
 */
class CEffect_Item_Use_HPPotion : public CEffectBase {
public:
    CEffect_Item_Use_HPPotion();
    virtual ~CEffect_Item_Use_HPPotion();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的跟隨目標。
    /// @param pUser 使用藥水的角色。
    void SetEffect(ClientCharacter* pUser);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者（使用藥水的角色）。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};