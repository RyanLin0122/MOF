#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Item_Type_Once
 * @brief 代表在角色使用物品時，於固定位置播放一次的特效。
 *
 * 繼承自 CEffectBase，與 CEffect_Skill_Type_Once 的主要區別在於，
 * 此特效的位置在創建時就已固定，不會隨角色移動而更新。
 */
class CEffect_Item_Type_Once : public CEffectBase {
public:
    CEffect_Item_Type_Once();
    virtual ~CEffect_Item_Type_Once();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，使用創建時的初始位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pUser 使用物品的角色。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(ClientCharacter* pUser, unsigned short effectKindID, char* szFileName);

protected:
    // --- 成員變數 (根據 Effectall.c @ 0x00539200 推斷) ---

    /// @brief 指向使用物品的角色。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)

    /// @brief 特效創建時，擁有者的 X 座標快照。
    float m_fInitialPosX;       // 位移 +136 (0x88)

    /// @brief 特效創建時，擁有者的 Y 座標快照。
    float m_fInitialPosY;       // 位移 +140 (0x8C)
};