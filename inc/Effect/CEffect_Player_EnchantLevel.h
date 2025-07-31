#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_EnchantLevel
 * @brief �N���a�˳Ʊj�Ƶ��Ū�����ʥ����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺰ʵe�S�ġA
 * ��ͩR�g���Ѩ��⪬�A�ʺA����C
 */
class CEffect_Player_EnchantLevel : public CEffectBase {
public:
    CEffect_Player_EnchantLevel();
    virtual ~CEffect_Player_EnchantLevel();

    // --- �����禡�мg ---

    /// @brief �ھڨ��⪬�A�M�w�O�_���e�פ�S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pOwner �n���[�S�Ī�����C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName);

protected:
    // --- �����ܼ� ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};