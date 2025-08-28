#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Field_Warp
 * @brief 負責顯示傳送門或傳送法陣的視覺特效。
 *
 * 繼承自 CEffectBase，可以根據傳入的類型顯示不同外觀，
 * 並且可以選擇跟隨角色或停留在固定位置。
 */
class CEffect_Field_Warp : public CEffectBase {
public:
    // 雖然原始碼未提供，但為了完整性而添加
    CEffect_Field_Warp();
    virtual ~CEffect_Field_Warp();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定傳送門特效。
    /// @param warpType 特效的類型，決定了其外觀。
    /// @param x 初始 X 座標 (如果 pOwner 為空，則為固定座標)。
    /// @param y 初始 Y 座標 (如果 pOwner 為空，則為固定座標)。
    /// @param pOwner 要跟隨的角色物件指標，可為 nullptr。
    void SetEffect(unsigned short warpType, float x, float y, ClientCharacter* pOwner);

private:
    // --- 成員變數 ---
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84): 要跟隨的角色
};