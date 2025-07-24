#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Skill_Type_Once
 * @brief 代表在施法者位置播放一次的技能特效。
 *
 * 繼承自 CEffectBase，用於實現例如 buff 啟用瞬間、原地施法等
 * 不需要追蹤目標或飛行的視覺效果。
 * 其生命週期由特效動畫本身 (.ea 檔案) 的長度決定。
 */
class CEffect_Skill_Type_Once : public CEffectBase {
public:
    CEffect_Skill_Type_Once();
    virtual ~CEffect_Skill_Type_Once();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，主要工作是同步擁有者的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pCaster 施法者角色，特效將跟隨此角色的位置。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName);

protected:
    // --- 成員變數 ---

    /// @brief 指向施法者或特效的擁有者。
    /// 特效的位置會根據此角色的位置動態更新。
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};