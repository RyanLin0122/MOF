#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h"

// 前向宣告
class GameImage;

/**
 * @class CEffect_Use_Hit
 * @brief 在畫面上顯示普通攻擊命中火花的程序化動畫特效。
 */
class CEffect_Use_Hit : public CEffectBase {
public:
    CEffect_Use_Hit();
    virtual ~CEffect_Use_Hit();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param x 初始 X 座標。
    /// @param y 初始 Y 座標。
    /// @param directionType 方向類型 (0-2)，影響初始位置偏移。
    void SetEffect(float x, float y, unsigned char directionType);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x0052F870 推斷) ---
    GameImage* m_pImage;              // 位移 +132 (0x84)

    // 動畫狀態
    float   m_fAlpha;            // 位移 +136 (0x88)
    float   m_fScale;            // 位移 +140 (0x8C)
    float   m_fRotation;         // 位移 +144 (0x90)
    char    m_cCurrentFrame;     // 位移 +152 (0x98)
    char    m_cDirectionType;    // 位移 +153 (0x99)

    // 位置與物理
    float   m_fCurrentPosX;      // 位移 +156 (0x9C)
    float   m_fCurrentPosY;      // 位移 +160 (0xA0)
    float   m_fScaleX_Rate;      // 位移 +180 (0xB4): 縮放 X 變化率
    float   m_fAlpha_Rate;       // 位移 +184 (0xB8): Alpha 變化率

    FrameSkip m_FrameSkip;       // 位移 +164 (0xA4)

    unsigned int m_dwResourceID; // 位移 +176 (0xB0)
    float        m_fFrameCounter;  // 位移 +192 (0xC0): 浮點數影格計數器
};