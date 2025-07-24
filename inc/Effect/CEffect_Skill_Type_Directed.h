#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Skill_Type_Directed
 * @brief �N��b�ؼШ����m����@�������V�ʧޯ�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�Ω��{�ݭn��T�R���b�ؼШ��W����ı�ĪG�C
 * �S�Ī���m�|����l�ܥؼШ���A��ͩR�g���Ѱʵe�������רM�w�C
 */
class CEffect_Skill_Type_Directed : public CEffectBase {
public:
    CEffect_Skill_Type_Directed();
    virtual ~CEffect_Skill_Type_Directed();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�D�n�u�@�O�P�B�ؼЪ���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pCaster �I�k�̨���A�Ω�M�w�S�Ĥ�V�C
    /// @param pTarget �ؼШ���A�S�ıN���H�����⪺��m�C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    /// @param ucDirectionFlag ��V�X�СA�Ω�M�w½���޿�C
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, unsigned char ucDirectionFlag);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�I�k�̡C
    ClientCharacter* m_pCasterCharacter; // �첾 +132 (0x84)

    /// @brief ���V�ؼСC�S�Ī���m�|�ھڦ����⪺��m�ʺA��s�C
    ClientCharacter* m_pTargetCharacter; // �첾 +136 (0x88)
};