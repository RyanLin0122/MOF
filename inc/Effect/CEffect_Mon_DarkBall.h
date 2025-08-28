#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip
#include <d3dx9math.h>

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Mon_DarkBall
 * @brief 代表怪物施放的黑暗球投射物特效。
 */
class CEffect_Mon_DarkBall : public CEffectBase {
public:
    CEffect_Mon_DarkBall();
    virtual ~CEffect_Mon_DarkBall();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效 (角色對角色)。
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, float fFlip, int hitInfoID);

    /// @brief 設定特效 (座標對座標)。
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x00530770 推斷) ---

    float   m_fTraveledDistance; // 位移 +132 (0x84)
    float   m_fTotalDistance;    // 位移 +136 (0x88)
    ClientCharacter* m_pTargetCharacter;  // 位移 +140 (0x8C)

    // 專用於移動和生命週期的 FrameSkip 物件
    FrameSkip m_MovementFrameSkip; // 位移 +144 (0x90)

    float   m_fAngle;            // 位移 +156 (0x9C)
    int     m_nHitInfoID;        // 位移 +160 (0xA0)
};