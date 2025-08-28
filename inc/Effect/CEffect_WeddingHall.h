#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_WeddingHall
 * @brief 在婚禮地圖的固定座標播放，且生命週期由玩家是否在地圖內決定的特效。
 */
class CEffect_WeddingHall : public CEffectBase {
public:
    CEffect_WeddingHall();
    virtual ~CEffect_WeddingHall();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據綁定角色的地圖ID來判斷是否應該結束特效。
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 初始化特效資源。
    void Init();

    /// @brief 設定特效。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    /// @param pPlayer 玩家角色，用於檢查地圖ID。
    void SetEffect(float x, float y, ClientCharacter* pPlayer);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x00537ED0 推斷) ---
    ClientCharacter* m_pOwnerPlayer; // 位移 +132 (0x84): 其地圖ID決定特效生命週期的角色
};