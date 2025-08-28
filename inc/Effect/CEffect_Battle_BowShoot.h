#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>

/**
 * @class CEffect_Battle_BowShoot
 * @brief 代表弓箭射擊的投射物特效。
 */
class CEffect_Battle_BowShoot : public CEffectBase {
public:
    CEffect_Battle_BowShoot();
    virtual ~CEffect_Battle_BowShoot();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效 (角色對角色)。
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool a4, int hitInfoID);

    /// @brief 設定特效 (座標對座標)。
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x0052D770 推斷) ---

    // CEffectBase 已包含 m_ccaEffect (位移+36)

    ClientCharacter* m_pTargetCharacter;  // 位移 +140 (0x8C)
    float   m_fTotalDistance;    // 位移 +136 (0x88)
    float   m_fTraveledDistance; // 位移 +132 (0x84)

    // 專用於移動和生命週期的 FrameSkip 物件
    FrameSkip m_MovementFrameSkip; // 位移 +144 (0x90)

    float   m_fAngle;            // 位移 +156 (0x9C)
    int     m_nHitInfoID;        // 位移 +160 (0xA0)

    // 為了還原 SetEffect 中複雜的臨時變數計算
    float m_fTempCalculated[10]; // 位移 +164 (0xA4) 到 +200 (0xC8)
};