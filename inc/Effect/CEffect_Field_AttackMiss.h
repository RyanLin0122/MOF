#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

// 前向宣告
class GameImage;

/**
 * @class CEffect_Field_AttackMiss
 * @brief 在畫面上顯示 "MISS" 字樣的程序化動畫特效。
 *
 * 繼承自 CEffectBase，用於顯示攻擊未命中的視覺回饋。
 * 其動畫（淡入、上漂、淡出）完全由程式碼控制。
 */
class CEffect_Field_AttackMiss : public CEffectBase {
public:
    CEffect_Field_AttackMiss();
    virtual ~CEffect_Field_AttackMiss();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效的初始位置。
    void SetEffect(float x, float y);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x005341C0 推斷) ---
    GameImage* m_pMissImage;        // 位移 +132 (0x84): 指向 "MISS" 字樣的 GameImage

    // 動畫狀態
    unsigned char m_ucState;        // 位移 +136 (0x88): 0=淡入, 1=淡出上漂
    float         m_fAlpha;         // 位移 +140 (0x8C): 當前透明度
    float         m_fColorValue;    // 位移 +144 (0x90): 當前顏色值

    // 位置
    float         m_fInitialPosX;   // 位移 +152 (0x98): 初始 X 座標
    float         m_fCurrentPosY;   // 位移 +156 (0x9C): 當前 Y 座標

    // 計時器
    FrameSkip m_FrameSkip;          // 位移 +160 (0xA0)
};