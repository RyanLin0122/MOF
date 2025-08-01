#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Take
 * @brief 代表技能發動或詠唱時的視覺特效。
 *
 * 繼承自 CEffectBase，是一個跟隨角色播放的一次性動畫特效。
 */
class CEffect_Skill_Take : public CEffectBase {
public:
    CEffect_Skill_Take();
    virtual ~CEffect_Skill_Take();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的跟隨目標。
    /// @param pOwner 施法角色。
    void SetEffect(ClientCharacter* pOwner);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者（施法角色）。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};