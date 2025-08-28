#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_CriticalNumber
 * @brief 負責顯示暴擊傷害數字的程序化動畫特效。
 */
class CEffect_Field_CriticalNumber : public CEffectBase {
public:
    CEffect_Field_CriticalNumber();
    virtual ~CEffect_Field_CriticalNumber();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param damageValue 顯示的傷害數值。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    /// @param type 類型 (0: 玩家對怪物, 1: 怪物對玩家)，決定數字顏色。
    void SetEffect(int damageValue, float x, float y, int type);

private:
    static const int MAX_DIGITS = 10; // 最多顯示10位數

    // --- 成員變數 (根據 Effectall.c @ 0x005344C0 推斷) ---
    GameImage** m_pNumberImages;     // 位移 +132 (0x84): 指向數字 GameImage 指標陣列
    GameImage* m_pFlashImage;       // 位移 +136 (0x88): 背景閃光
    GameImage* m_pTextImage;        // 位移 +140 (0x8C): 前景文字 (CRITICAL)

    float m_fScale;                  // 位移 +144 (0x90): 整體縮放值
    float m_fAlpha;                  // 位移 +148 (0x94): 整體透明度

    char  m_cDigitCount;             // 位移 +166 (0xA6): 數字的位數
    unsigned char m_ucState;         // 位移 +167 (0xA7): 動畫狀態 (0:放大, 1:縮小, 2:淡出)
    unsigned short m_usTextImageFrame; // 位移 +204 (0xCC): 前景文字的影格 ID

    float m_fInitialPosX;            // 位移 +180 (0xB4): 初始 X 座標
    float m_fCurrentPosY;            // 位移 +184 (0xB8): 當前 Y 座標
    float m_fTotalWidth;             // 位移 +188 (0xBC): 數字總寬度，用於居中

    unsigned char m_ucDigitFrames[MAX_DIGITS]; // 位移 +156 (0x9C): 儲存每個數字的影格ID

    FrameSkip m_FrameSkip;           // 位移 +192 (0xC0)
};