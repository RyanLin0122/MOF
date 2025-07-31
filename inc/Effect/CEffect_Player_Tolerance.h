#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_Tolerance
 * @brief �N���a��o�ܩʮɡA���W�h����ı�S�ġC
 */
class CEffect_Player_Tolerance : public CEffectBase {
public:
    CEffect_Player_Tolerance();
    virtual ~CEffect_Player_Tolerance();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @param toleranceType �ܩʺ��� (1-3)�A�M�w�S�ĥ~�[�C
    void SetEffect(ClientCharacter* pOwner, unsigned char toleranceType);

protected:
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};


/**
 * @class CEffect_Player_Tolerance_Sub
 * @brief �N���a��o�ܩʮɡA��󩳼h����ı�S�ġC
 */
class CEffect_Player_Tolerance_Sub : public CEffectBase {
public:
    CEffect_Player_Tolerance_Sub();
    virtual ~CEffect_Player_Tolerance_Sub();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @param toleranceType �ܩʺ��� (1-3)�A�M�w�S�ĥ~�[�C
    void SetEffect(ClientCharacter* pOwner, unsigned char toleranceType);

protected:
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};