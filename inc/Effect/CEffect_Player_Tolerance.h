#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Player_Tolerance
 * @brief 代表玩家獲得抗性時，位於上層的視覺特效。
 */
class CEffect_Player_Tolerance : public CEffectBase {
public:
    CEffect_Player_Tolerance();
    virtual ~CEffect_Player_Tolerance();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @param toleranceType 抗性種類 (1-3)，決定特效外觀。
    void SetEffect(ClientCharacter* pOwner, unsigned char toleranceType);

protected:
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};


/**
 * @class CEffect_Player_Tolerance_Sub
 * @brief 代表玩家獲得抗性時，位於底層的視覺特效。
 */
class CEffect_Player_Tolerance_Sub : public CEffectBase {
public:
    CEffect_Player_Tolerance_Sub();
    virtual ~CEffect_Player_Tolerance_Sub();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @param toleranceType 抗性種類 (1-3)，決定特效外觀。
    void SetEffect(ClientCharacter* pOwner, unsigned char toleranceType);

protected:
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
};