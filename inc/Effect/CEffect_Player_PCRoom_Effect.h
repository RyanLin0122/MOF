#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_PCRoom_Effect
 * @brief �N���a�b���@(PC Room)���ҤU������ʥ����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺰ʵe�S�ġA
 * ��ͩR�g���Ѩ���O�_���B�� PC Room ���A�ʺA����C
 */
class CEffect_Player_PCRoom_Effect : public CEffectBase {
public:
    CEffect_Player_PCRoom_Effect();
    virtual ~CEffect_Player_PCRoom_Effect();

    // --- �����禡�мg ---

    /// @brief �ھڨ���O�_���ɦ� PC Room ���A�ӨM�w�O�_���e�פ�S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pOwner �n���[�S�Ī�����C
    void SetEffect(ClientCharacter* pOwner);

protected:
    // --- �����ܼ� ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};