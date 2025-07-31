#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Player_Dead
 * @brief 代表玩家角色死亡時出現的靈魂特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放的持續性特效。
 * 其生命週期由玩家是否復活或特定計時器決定。
 */
class CEffect_Player_Dead : public CEffectBase {
public:
    CEffect_Player_Dead();
    virtual ~CEffect_Player_Dead();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據玩家復活狀態或計時器來判斷是否應該結束特效。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pDeadChar 死亡的角色物件。
    /// @param x 角色死亡時的 X 座標。
    /// @param y 角色死亡時的 Y 座標。
    /// @param isPkMode 是否為 PK 模式（影響超時邏輯）。
    void SetEffect(ClientCharacter* pDeadChar, float x, float y, int isPkMode);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x005313A0 推斷) ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84): 指向死亡的角色
    unsigned int     m_dwStartTime;     // 位移 +136 (0x88): 特效創建時的時間戳
    int              m_bIsPK_Mode;      // 位移 +140 (0x8C): 是否為 PK 模式
};