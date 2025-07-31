#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Player_WeatherPreView
 * @brief 在 UI 層顯示天氣或地圖環境預覽的特效。
 *
 * 繼承自 CEffectBase，是一個在固定螢幕座標播放、
 * 且生命週期由外部邏輯控制的動畫特效。
 */
class CEffect_Player_WeatherPreView : public CEffectBase {
public:
    CEffect_Player_WeatherPreView();
    virtual ~CEffect_Player_WeatherPreView();

    // --- 虛擬函式覆寫 ---

    /// @brief 更新動畫，但永不回傳 true，使特效持續存在。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用固定的螢幕位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param x 螢幕 X 座標。
    /// @param y 螢幕 Y 座標。
    /// @param baseEffectID 特效的基礎 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(float x, float y, int baseEffectID, char* szFileName);

private:
    // --- 成員變數 ---
    float m_fScreenPosX; // 位移 +132 (0x84)
    float m_fScreenPosY; // 位移 +136 (0x88)
};