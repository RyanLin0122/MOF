#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Mon_DarkSpark
 * @brief 代表怪物黑暗攻擊命中時的火花特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的一次性動畫特效。
 */
class CEffect_Mon_DarkSpark : public CEffectBase {
public:
    CEffect_Mon_DarkSpark();
    virtual ~CEffect_Mon_DarkSpark();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的跟隨目標。
    /// @param pTarget 特效要附加到的目標角色。
    void SetEffect(ClientCharacter* pTarget);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者（被擊中的角色）。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};