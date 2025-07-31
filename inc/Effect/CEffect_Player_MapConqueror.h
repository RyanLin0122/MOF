#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_MapConqueror
 * @brief �N���a�@���a�Ϧ���̪�����ʥ����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺰ʵe�S�ġA
 * ��ͩR�g���Ѩ���O�_�����a�Ϧ���̰ʺA����C
 */
class CEffect_Player_MapConqueror : public CEffectBase {
public:
    CEffect_Player_MapConqueror();
    virtual ~CEffect_Player_MapConqueror();

    // --- �����禡�мg ---

    /// @brief �ھڨ���O�_�����a�Ϧ���̨ӨM�w�O�_���e�פ�S�ġC
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