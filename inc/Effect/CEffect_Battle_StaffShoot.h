#pragma once

#include "Effect/CEffectBase.h"
#include <d3dx9math.h>

/**
 * @class CEffect_Battle_StaffShoot
 * @brief 代表法杖射出的魔法投射物特效。
 *
 * 繼承自 CEffectBase，用於實現直線飛行的魔法彈效果。
 */
class CEffect_Battle_StaffShoot : public CEffectBase {
public:
    CEffect_Battle_StaffShoot();
    virtual ~CEffect_Battle_StaffShoot();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param pStartPos 投射物的起始世界座標。
    /// @param pEndPos 投射物的目標世界座標。
    /// @param bFlip 是否水平翻轉。
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x0052F420 推斷) ---

    // CEffectBase 已包含 m_ccaEffect (位移+36), m_fSpeed (位移+24) 等

    float m_fTraveledDistance; // 位移 +132 (0x84): 已飛行的距離
    float m_fTotalDistance;    // 位移 +136 (0x88): 起點到終點的總距離
};