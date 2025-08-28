#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_DamageNumber
 * @brief 負責顯示普通傷害數字的程序化動畫特效。
 */
class CEffect_Field_DamageNumber : public CEffectBase {
public:
    CEffect_Field_DamageNumber();
    virtual ~CEffect_Field_DamageNumber();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param damageValue 顯示的傷害數值。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    /// @param type 類型 (0: 怪物對玩家-紅色, 1: 玩家對怪物-白色)。
    void SetEffect(int damageValue, float x, float y, int type);

private:
    static const int MAX_DIGITS = 10; // 最多顯示10位數

    // --- 成員變數 (根據 Effectall.c @ 0x00534CA0 推斷) ---
    GameImage** m_pNumberImages;     // 位移 +132 (0x84): 指向數字 GameImage 指標陣列

    char  m_cDigitCount;             // 位移 +146 (0x92): 數字的位數
    unsigned char m_ucState;         // 位移 +147 (0x93): 動畫狀態 (0:放大, 1:縮小, 2:淡出)

    float m_fAlpha;                  // 位移 +148 (0x94): 整體透明度
    float m_fScale;                  // 位移 +152 (0x98): 整體縮放值

    unsigned char m_ucDigitFrames[MAX_DIGITS]; // 位移 +136 (0x88): 儲存每個數字的影格ID

    float m_fInitialPosX;            // 位移 +160 (0xA0): 初始 X 座標
    float m_fCurrentPosY;            // 位移 +164 (0xA4): 當前 Y 座標
    float m_fTotalWidth;             // 位移 +168 (0xA8): 數字總寬度，用於居中

    FrameSkip m_FrameSkip;           // 位移 +172 (0xAC)
};