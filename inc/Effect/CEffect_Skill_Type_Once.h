#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Skill_Type_Once
 * @brief �N��b�I�k�̦�m����@�����ޯ�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�Ω��{�Ҧp buff �ҥ������B��a�I�k��
 * ���ݭn�l�ܥؼЩέ��檺��ı�ĪG�C
 * ��ͩR�g���ѯS�İʵe���� (.ea �ɮ�) �����רM�w�C
 */
class CEffect_Skill_Type_Once : public CEffectBase {
public:
    CEffect_Skill_Type_Once();
    virtual ~CEffect_Skill_Type_Once();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�D�n�u�@�O�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pCaster �I�k�̨���A�S�ıN���H�����⪺��m�C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�I�k�̩ίS�Ī��֦��̡C
    /// �S�Ī���m�|�ھڦ����⪺��m�ʺA��s�C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};