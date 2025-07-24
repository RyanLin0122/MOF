#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Skill_Type_Directed
 * @brief 代表在目標角色位置播放一次的指向性技能特效。
 *
 * 繼承自 CEffectBase，用於實現需要精確命中在目標身上的視覺效果。
 * 特效的位置會持續追蹤目標角色，其生命週期由動畫本身長度決定。
 */
class CEffect_Skill_Type_Directed : public CEffectBase {
public:
    CEffect_Skill_Type_Directed();
    virtual ~CEffect_Skill_Type_Directed();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，主要工作是同步目標的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效。
    /// @param pCaster 施法者角色，用於決定特效方向。
    /// @param pTarget 目標角色，特效將跟隨此角色的位置。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    /// @param ucDirectionFlag 方向旗標，用於決定翻轉邏輯。
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, unsigned char ucDirectionFlag);

protected:
    // --- 成員變數 ---

    /// @brief 指向施法者。
    ClientCharacter* m_pCasterCharacter; // 位移 +132 (0x84)

    /// @brief 指向目標。特效的位置會根據此角色的位置動態更新。
    ClientCharacter* m_pTargetCharacter; // 位移 +136 (0x88)
};