#pragma once

#include "Effect/CEffectBase.h"

/**
 * @class CEffect_Field_Recall_Potal
 * @brief 在指定座標顯示一個傳送門的視覺特效。
 *
 * 繼承自 CEffectBase，是一個在固定座標播放的動畫特效，
 * 其外觀可由外部傳入的名稱決定。
 */
class CEffect_Field_Recall_Potal : public CEffectBase {
public:
    CEffect_Field_Recall_Potal();
    virtual ~CEffect_Field_Recall_Potal();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定傳送門特效。
    /// @param szEffectName 特效的名稱代碼，用於查詢特效資源。
    /// @param x 特效中心點的 X 座標。
    /// @param y 特效中心點的 Y 座標。
    void SetEffect(char* szEffectName, float x, float y);

private:
    // --- 成員變數 ---
    bool m_bIsFinished; // 位移 +132 (0x84): 用於外部控制生命週期的旗標
};