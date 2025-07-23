#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_Walkdust
 * @brief 負責在地面上產生煙塵、塵土飛揚效果的類別。
 * @note 此類別繼承自 CEffectBase，是一個由其他特效觸發的一次性效果。
 */
class CEffect_Field_Walkdust : public CEffectBase {
public:
    CEffect_Field_Walkdust();
    virtual ~CEffect_Field_Walkdust();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    /// @brief 設定煙塵特效的屬性。
    void SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int a6, char frameID, bool bMoveRight = false);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x005371D0 推斷) ---
    // m_fCurrentPosX, m_fCurrentPosY, m_bIsVisible 等已從 CEffectBase 繼承

    GameImage* m_pEffectImage;     // 位移 +132 (0x84)

    // 特效狀態
    float   m_fAlpha;            // 位移 +136 (0x88)
    float   m_fScale;            // 位移 +140 (0x8C)
    float   m_fRotation;         // 位移 +144 (0x90)
    char    m_cInitialFrame;     // 位移 +152 (0x98)
    // m_bFlipX 在 CEffectBase 中已定義 (位移+28)

    unsigned int m_dwResourceID; // 位移 +176 (0xB0)
    float   m_fScaleX_Rate;      // 位移 +180 (0xB4)
    float   m_fAlpha_Rate;       // 位移 +184 (0xB8)

    // 計時器
    FrameSkip m_FrameSkip;       // 位移 +164 (0xA4)

    bool m_bMoveRight;           // 位移 +188 (0xBC), 用於控制漂移方向
};