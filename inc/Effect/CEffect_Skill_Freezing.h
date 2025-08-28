#pragma once

#include "Effect/CEffectBase.h"

// 前向宣告
class ClientCharacter;

/**
 * @class CEffect_Skill_Freezing
 * @brief 代表角色被冰凍時，位於上層的視覺特效。
 */
class CEffect_Skill_Freezing : public CEffectBase {
public:
    CEffect_Skill_Freezing();
    virtual ~CEffect_Skill_Freezing();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
    int              m_nUnknown_a3;     // 位移 +136 (0x88)
    float            m_fInitialPosX;    // 位移 +144 (0x90)
    float            m_fInitialPosY;    // 位移 +148 (0x94)
};


/**
 * @class CEffect_Skill_Freezing_Sub
 * @brief 代表角色被冰凍時，位於底層的視覺特效。
 */
class CEffect_Skill_Freezing_Sub : public CEffectBase {
public:
    CEffect_Skill_Freezing_Sub();
    virtual ~CEffect_Skill_Freezing_Sub();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    ClientCharacter* m_pOwnerCharacter; // 位移 +132 (0x84)
    int              m_nUnknown_a3;     // 位移 +136 (0x88)
    float            m_fInitialPosX;    // 位移 +144 (0x90)
    float            m_fInitialPosY;    // 位移 +148 (0x94)
};