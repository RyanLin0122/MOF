#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class cltPetObject;

/**
 * @class CEffect_Pet_Base
 * @brief 代表附加在寵物身上的一次性特效的基底類別。
 *
 * 繼承自 CEffectBase，是一個跟隨寵物播放的動畫特效。
 */
class CEffect_Pet_Base : public CEffectBase {
public:
    CEffect_Pet_Base();
    virtual ~CEffect_Pet_Base();

    // --- 虛擬函式覆寫 ---

    /// @brief 處理特效的生命週期，直接依賴 CCAEffect 的動畫播放狀態。
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief 準備繪製前的邏輯更新，同步擁有者（寵物）的位置。
    virtual void Process() override;

    /// @brief 繪製特效。
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效的跟隨目標和視覺資源。
    /// @param pPetOwner 要跟隨的寵物物件。
    /// @param effectKindID 特效的種類 ID。
    /// @param szFileName 特效資源 (.ea) 的檔案名稱。
    void SetEffect(cltPetObject* pPetOwner, unsigned short effectKindID, char* szFileName);

protected:
    // --- 成員變數 ---

    /// @brief 指向特效的擁有者（寵物物件）。
    cltPetObject* m_pOwnerPet; // 位移 +132 (0x84)
};