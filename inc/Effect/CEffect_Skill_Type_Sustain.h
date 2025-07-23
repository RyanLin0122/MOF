#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_Type_Sustain
 * @brief �N��b�I�k�̨��W����s�b���ޯ�S�ġ]�Ҧp Buff/Debuff�^�C
 *
 * �~�Ӧ� CEffectBase�A�Ω��{�P����ޯબ�A�P�B������ʵ�ı�ĪG�C
 * ��ͩR�g�����̿��ʵe���סA�ӬO�ѬI�k�̨��W���ޯબ�A�O�_�s�b�M�w�C
 */
class CEffect_Skill_Type_Sustain : public CEffectBase {
public:
    CEffect_Skill_Type_Sustain();
    virtual ~CEffect_Skill_Type_Sustain();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�ˬd�֦��̪�����ޯબ�A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pCaster �I�k�̨���A�S�ıN���H������C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    /// @param sustainSkillID ���S�Ĺ���������ʧޯ� ID�A�Ω��ˬd�ͩR�g���C
    void SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName, unsigned short sustainSkillID);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)

    /// @brief ���S�ĸj�w������ʧޯ� ID�C
    unsigned short m_sSustainSkillID;   // �첾 +136 (0x88)
};