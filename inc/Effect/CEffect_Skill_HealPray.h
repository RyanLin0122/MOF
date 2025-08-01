#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_HealPray
 * @brief �N��v����ë�ޯ઺���۵�ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺤@���ʰʵe�S�ġA
 * ��~�[�|�ھڧޯ൥���ܤơC
 */
class CEffect_Skill_HealPray : public CEffectBase {
public:
    CEffect_Skill_HealPray();
    virtual ~CEffect_Skill_HealPray();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pCaster �I�k����C
    /// @param skillLevel �ޯ൥�� (0-2)�A�M�w�S�Ī��~�[�C
    void SetEffect(ClientCharacter* pCaster, unsigned char skillLevel);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡]�I�k����^�C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};