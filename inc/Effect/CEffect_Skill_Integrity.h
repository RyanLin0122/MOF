#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Skill_Integrity
 * @brief �N����B���שδ�˪��A�ɪ�����ʵ�ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м��񪺰ʵe�S�ġA
 * ��ͩR�g���ѥؼШ��⪺�S���A�X�аʺA����C
 */
class CEffect_Skill_Integrity : public CEffectBase {
public:
    CEffect_Skill_Integrity();
    virtual ~CEffect_Skill_Integrity();

    // --- �����禡�мg ---

    /// @brief �ھڥؼШ��⪺���A�M�w�O�_���e�פ�S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pTarget �n���[�S�Ī��ؼШ���C
    /// @param a3 ���ϥΪ��ѼơA�O�d�H�ŦX��l�X�C
    void SetEffect(ClientCharacter* pTarget, int a3);

protected:
    // --- �����ܼ� ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
    int              m_nUnknown_a3;     // �첾 +136 (0x88)
    float            m_fInitialPosX;    // �첾 +144 (0x90)
    float            m_fInitialPosY;    // �첾 +148 (0x94)
};