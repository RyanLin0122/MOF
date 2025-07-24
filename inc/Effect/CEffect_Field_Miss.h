#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_Miss
 * @brief 在畫面上顯示 "MISS" 字樣的程序化動畫特效，帶有隨機漂移效果。
 */
class CEffect_Field_Miss : public CEffectBase {
public:
    CEffect_Field_Miss();
    virtual ~CEffect_Field_Miss();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效的初始位置。
    void SetEffect(float x, float y);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x00537A40 推斷) ---
    GameImage* m_pMissImage;        // 位移 +132 (0x84): 指向 "MISS" 字樣的 GameImage

    // 動畫狀態
    float   m_fAlpha;            // 位移 +136 (0x88)
    float   m_fScale;            // 位移 +140 (0x8C)
    float   m_fRotation;         // 位移 +144 (0x90)
    char    m_cInitialFrame;     // 位移 +152 (0x98)

    // 位置與物理
    float   m_fCurrentPosX;      // 位移 +156 (0x9C)
    float   m_fCurrentPosY;      // 位移 +160 (0xA0)
    float   m_fVelX;             // 位移 +188 (0xBC): X 軸速度/方向
    float   m_fVelY;             // 位移 +192 (0xC0): Y 軸速度/方向
    float   m_fSpeedFactor;      // 位移 +196 (0xC4): 速度係數

    FrameSkip m_FrameSkip;       // 位移 +164 (0xA4)

    unsigned int m_dwResourceID; // 位移 +176 (0xB0)
};