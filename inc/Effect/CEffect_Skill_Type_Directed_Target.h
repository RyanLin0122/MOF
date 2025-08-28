#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Type_Directed_Target
 * @brief 代表一個跟隨目標的指向性技能特效。
 *
 * 繼承自 CEffectBase，用於實現在目標身上播放，但方向由施法者決定的視覺效果。
 */
class CEffect_Skill_Type_Directed_Target : public CEffectBase {
public:
    CEffect_Skill_Type_Directed_Target();
    virtual ~CEffect_Skill_Type_Directed_Target();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步目標的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pCaster 施法者角色，用於決定特效方向。
    /// @param pTarget 目標角色，特效將跟隨此角色的位置。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName);

protected:
    // --- 成員變數 ---

    /// @brief 指向施法者。
    ClientCharacter* m_pCasterCharacter; // 位移 +132 (0x84)

    /// @brief 指向目標。
    ClientCharacter* m_pTargetCharacter; // 位移 +136 (0x88)
};