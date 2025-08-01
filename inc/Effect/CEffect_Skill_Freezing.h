#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_Freezing
 * @brief �N����Q�B��ɡA���W�h����ı�S�ġC
 */
class CEffect_Skill_Freezing : public CEffectBase {
public:
    CEffect_Skill_Freezing();
    virtual ~CEffect_Skill_Freezing();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
    int              m_nUnknown_a3;     // �첾 +136 (0x88)
    float            m_fInitialPosX;    // �첾 +144 (0x90)
    float            m_fInitialPosY;    // �첾 +148 (0x94)
};


/**
 * @class CEffect_Skill_Freezing_Sub
 * @brief �N����Q�B��ɡA��󩳼h����ı�S�ġC
 */
class CEffect_Skill_Freezing_Sub : public CEffectBase {
public:
    CEffect_Skill_Freezing_Sub();
    virtual ~CEffect_Skill_Freezing_Sub();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
    int              m_nUnknown_a3;     // �첾 +136 (0x88)
    float            m_fInitialPosX;    // �첾 +144 (0x90)
    float            m_fInitialPosY;    // �첾 +148 (0x94)
};