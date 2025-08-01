#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_SpeedUP
 * @brief �N��[�t�ޯ�ҥήɪ���ı�S�ġA�æb������Ĳ�o���⪬�A�C
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺤@���ʰʵe�S�ġC
 */
class CEffect_Skill_SpeedUP : public CEffectBase {
public:
    CEffect_Skill_SpeedUP();
    virtual ~CEffect_Skill_SpeedUP();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�æb�ʵe������Ĳ�o���⪬�A�C
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

    /// @brief ���V�S�Ī��֦��̡C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};