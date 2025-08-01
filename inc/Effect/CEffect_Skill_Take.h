#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_Take
 * @brief �N��ޯ�o�ʩε��ۮɪ���ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺤@���ʰʵe�S�ġC
 */
class CEffect_Skill_Take : public CEffectBase {
public:
    CEffect_Skill_Take();
    virtual ~CEffect_Skill_Take();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī����H�ؼСC
    /// @param pOwner �I�k����C
    void SetEffect(ClientCharacter* pOwner);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡]�I�k����^�C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};