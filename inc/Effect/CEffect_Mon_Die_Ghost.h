#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Mon_Die_Ghost
 * @brief 代表怪物死亡時出現的靈魂特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放一次的動畫特效。
 */
class CEffect_Mon_Die_Ghost : public CEffectBase {
public:
    CEffect_Mon_Die_Ghost();
    virtual ~CEffect_Mon_Die_Ghost();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的顯示位置。
    /// @param pDeadMonster 死亡的怪物角色物件。
    void SetEffect(ClientCharacter* pDeadMonster);

private:
    // 此類別沒有額外的成員變數，所有位置資訊都儲存在 CEffectBase 中。
};