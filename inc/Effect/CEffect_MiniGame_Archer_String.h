#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip

// 前向宣告
class GameImage;

/**
 * @class CEffect_MiniGame_Archer_String
 * @brief 在小遊戲中顯示弓箭手相關提示文字的程序化動畫特效。
 */
class CEffect_MiniGame_Archer_String : public CEffectBase {
public:
    CEffect_MiniGame_Archer_String();
    virtual ~CEffect_MiniGame_Archer_String();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定特效。
    /// @param cFrameIndex 要顯示的文字在圖檔中的影格索引。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    void SetEffect(char cFrameIndex, float x, float y);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x005384D0 推斷) ---
    GameImage* m_pImage;              // 位移 +132 (0x84)

    // 動畫狀態
    char          m_cFrameIndex;     // 位移 +136 (0x88): 要顯示的文字影格
    unsigned char m_ucState;         // 位移 +137 (0x89): 動畫狀態機
    float         m_fAlpha;          // 位移 +140 (0x8C): 當前透明度
    float         m_fScale;          // 位移 +144 (0x90): 當前縮放/顏色值

    // 位置
    float         m_fInitialPosX;    // 位移 +152 (0x98): 初始 X 座標
    float         m_fCurrentPosY;    // 位移 +156 (0x9C): 當前 Y 座標

    // 計時器
    FrameSkip m_FrameSkip;           // 位移 +160 (0xA0)
};