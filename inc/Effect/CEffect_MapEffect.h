#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_MapEffect
 * @brief 在地圖的固定座標播放，且生命週期由外部條件控制的特效。
 */
class CEffect_MapEffect : public CEffectBase {
public:
    CEffect_MapEffect();
    virtual ~CEffect_MapEffect();

    // --- 虛擬函式覆寫 ---

    /// @brief 根據綁定角色的狀態 ID 來判斷是否應該結束特效。
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---

    /// @brief 設定特效 (透過名稱)。
    void SetEffect(char* szEffectName, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y);

    /// @brief 設定特效 (透過 ID)。
    void SetEffect(unsigned short effectKindID, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y);

private:
    // --- 成員變數 (根據 Effectall.c @ 0x00537810 推斷) ---
    ClientCharacter* m_pStateOwner;     // 位移 +132 (0x84): 其狀態決定特效生命週期的角色
    unsigned short   m_wRequiredStateID;  // 位移 +136 (0x88): 維持特效存在所需的狀態 ID
};