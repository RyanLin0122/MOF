#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Type_Sustain
 * @brief 代表在施法者身上持續存在的技能特效（例如 Buff/Debuff）。
 *
 * 繼承自 CEffectBase，用於實現與角色技能狀態同步的持續性視覺效果。
 * 其生命週期不依賴於動畫長度，而是由施法者身上的技能狀態是否存在決定。
 */
class CEffect_Skill_Type_Sustain : public CEffectBase {
public:
    CEffect_Skill_Type_Sustain();
    virtual ~CEffect_Skill_Type_Sustain();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，檢查擁有者的持續技能狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pCaster 施法者角色，特效將跟隨此角色。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    /// @param sustainSkillID 此特效對應的持續性技能 ID，用於檢查生命週期。
    void SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName, unsigned short sustainSkillID);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)

    /// @brief 此特效綁定的持續性技能 ID。
    unsigned short m_sSustainSkillID;   // 位移 +136 (0x88)
};