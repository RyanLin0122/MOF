#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip

// 前向宣告
class GameImage;

/**
 * @class CEffect_Player_Goggles
 * @brief 在畫面上顯示護目鏡相關的程序化動畫特效。
 */
class CEffect_Player_Goggles : public CEffectBase {
public:
    CEffect_Player_Goggles();
    virtual ~CEffect_Player_Goggles();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param x 初始 X 座標。
    /// @param y 初始 Y 座標。
    /// @param bFlip 是否水平翻轉。
    /// @param resourceID 要使用的圖片資源 ID。
    /// @param isUiEffect 是否為 UI 特效 (決定座標是否轉換)。
    void SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int isUiEffect);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x005318B0 推斷) ---
    GameImage* m_pImage;              // 位移 +132 (0x84)

    // 動畫狀態
    float   m_fAlpha;            // 位移 +136 (0x88)
    float   m_fScale;            // 位移 +140 (0x8C)
    float   m_fRotation;         // 位移 +144 (0x90)
    char    m_cInitialFrame;     // 位移 +152 (0x98)

    // 位置與物理
    float   m_fCurrentPosX;      // 位移 +156 (0x9C)
    float   m_fCurrentPosY;      // 位移 +160 (0xA0)
    float   m_fScaleX_Rate;      // 位移 +180 (0xB4): 縮放 X 變化率
    float   m_fAlpha_Rate;       // 位移 +184 (0xB8): Alpha 變化率

    FrameSkip m_FrameSkip;       // 位移 +164 (0xA4)

    unsigned int m_dwResourceID; // 位移 +176 (0xB0)
    int          m_isUiEffect;   // 位移 +188 (0xBC): 是否為 UI 特效
};