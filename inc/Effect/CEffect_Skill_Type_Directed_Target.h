#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_Type_Directed_Target
 * @brief �N��@�Ӹ��H�ؼЪ����V�ʧޯ�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�Ω��{�b�ؼШ��W����A����V�ѬI�k�̨M�w����ı�ĪG�C
 */
class CEffect_Skill_Type_Directed_Target : public CEffectBase {
public:
    CEffect_Skill_Type_Directed_Target();
    virtual ~CEffect_Skill_Type_Directed_Target();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�ؼЪ���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pCaster �I�k�̨���A�Ω�M�w�S�Ĥ�V�C
    /// @param pTarget �ؼШ���A�S�ıN���H�����⪺��m�C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�I�k�̡C
    ClientCharacter* m_pCasterCharacter; // �첾 +132 (0x84)

    /// @brief ���V�ؼСC
    ClientCharacter* m_pTargetCharacter; // �첾 +136 (0x88)
};