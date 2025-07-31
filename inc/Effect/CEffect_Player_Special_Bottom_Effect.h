#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_Special_Bottom_Effect
 * @brief �N���a�S���A���B��󩳼h������ʥ����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺰ʵe�S�ġA
 * ��ͩR�g���ѥ~���޿�ʺA����C
 */
class CEffect_Player_Special_Bottom_Effect : public CEffectBase {
public:
    CEffect_Player_Special_Bottom_Effect();
    virtual ~CEffect_Player_Special_Bottom_Effect();

    // --- �����禡�мg ---

    /// @brief �ھھ֦��̫��ЬO�_���ĨӨM�w�O�_���e�פ�S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pOwner �n���[�S�Ī�����C
    /// @param effectType �S�Ī������A�M�w�F��~�[�C
    void SetEffect(ClientCharacter* pOwner, unsigned char effectType);

protected:
    // --- �����ܼ� ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
    unsigned char    m_ucEffectType;    // �첾 +136 (0x88)
};