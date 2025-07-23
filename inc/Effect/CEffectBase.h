#pragma once

#include "Effect/CCAEffect.h" // CEffectBase 包含一個 CCAEffect 物件來處理渲染

/**
 * @class CEffectBase
 * @brief 所有遊戲特效的抽象基底類別。
 *
 * 這個類別提供了一個統一的介面來管理特效的生命週期（FrameProcess）、
 * 邏輯更新（Process）和繪製（Draw）。它本身不直接繪製，而是包含一個
 * CCAEffect 物件來處理所有與動畫和渲染相關的底層工作。
 * 每個具體的特效（如火球、治療光環等）都應該繼承自這個類別。
 */
class CEffectBase {
public:
    /// @brief 建構函式，初始化基礎屬性。
    CEffectBase();

    /// @brief 虛擬解構函式，確保衍生類別能被正確銷毀。
    virtual ~CEffectBase();

    // --- 核心虛擬函式 (由衍生類別實作) ---

    /**
     * @brief 處理特效的每影格更新，主要負責更新其生命週期。
     * @param fElapsedTime 自上一影格以來經過的時間（秒）。
     * @return 如果特效應該被銷毀，則回傳 true；否則回傳 false。
     */
    virtual bool FrameProcess(float fElapsedTime);

    /**
     * @brief 準備繪製前的邏輯更新。
     * 通常在這裡計算特效的位置、可見性，並更新 CCAEffect 的狀態。
     */
    virtual void Process() = 0; // 純虛擬函式，強制衍生類別實作

    /**
     * @brief 繪製特效。
     * 通常只是呼叫內部 m_ccaEffect 的 Draw 函式。
     */
    virtual void Draw() = 0; // 純虛擬函式，強制衍生類別實作


protected:
    /**
     * @brief 檢查特效是否在可視範圍內（簡易的裁剪判斷）。
     * @param x 特效的 X 座標。
     * @param y 特效的 Y 座標。
     * @return 如果在可視範圍內，返回 TRUE；否則返回 FALSE。
     */
    bool IsCliping(float x, float y);

protected:

    void* m_pOwner;      // 位移 +4:  指向擁有者物件的指標（例如施法者）

    // 位置與方向
    float           m_fCurrentPosX; // 位移 +8:  特效當前的 X 座標
    float           m_fCurrentPosY; // 位移 +12: 特效當前的 Y 座標
    float           m_fDirectionX;  // 位移 +16: X 方向向量/速度
    float           m_fDirectionY;  // 位移 +20: Y 方向向量/速度
    float           m_fSpeed;       // 位移 +24: 移動速度

    bool            m_bIsFlip;     // 位移 +28: 是否水平翻轉

    // 繪製相關
    bool            m_bIsVisible;  // 位移 +32: 根據裁剪判斷，特效是否可見

    // 核心渲染物件
    CCAEffect       m_ccaEffect;   // 位移 +36: 實際負責動畫播放和繪製的物件
};