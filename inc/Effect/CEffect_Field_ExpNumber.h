#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_ExpNumber
 * @brief 負責顯示獲得經驗值數字的程序化動畫特效。
 */
class CEffect_Field_ExpNumber : public CEffectBase {
public:
    CEffect_Field_ExpNumber();
    virtual ~CEffect_Field_ExpNumber();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param expValue 顯示的經驗值數值。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    void SetEffect(int expValue, float x, float y);

private:
    static const int MAX_DIGITS = 10; // 最多顯示10位數

    // --- 成員變數 (根據 Effectall.c @ 0x00535210 推斷) ---
    GameImage** m_pNumberImages;     // 位移 +132 (0x84): 指向數字 GameImage 指標陣列

    char  m_cDigitCount;             // 位移 +146 (0x92): 數字的位數
    unsigned char m_ucState;         // 位移 +147 (0x93): 動畫狀態 (0:淡入, 1:淡出上漂)

    float m_fAlpha;                  // 位移 +148 (0x94): 整體透明度
    float m_fScale;                  // 位移 +152 (0x98): 整體縮放/顏色值

    unsigned char m_ucDigitFrames[MAX_DIGITS]; // 位移 +136 (0x88): 儲存每個數字的影格ID

    float m_fInitialPosX;            // 位移 +160 (0xA0): 初始 X 座標
    float m_fCurrentPosY;            // 位移 +164 (0xA4): 當前 Y 座標
    float m_fTotalWidth;             // 位移 +168 (0xA8): 數字總寬度，用於居中

    FrameSkip m_FrameSkip;           // 位移 +172 (0xAC)
};