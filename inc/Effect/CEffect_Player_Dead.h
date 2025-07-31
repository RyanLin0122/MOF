#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Player_Dead
 * @brief �N���a���⦺�`�ɥX�{���F��S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м��񪺫���ʯS�ġC
 * ��ͩR�g���Ѫ��a�O�_�_���ίS�w�p�ɾ��M�w�C
 */
class CEffect_Player_Dead : public CEffectBase {
public:
    CEffect_Player_Dead();
    virtual ~CEffect_Player_Dead();

    // --- �����禡�мg ---

    /// @brief �ھڪ��a�_�����A�έp�ɾ��ӧP�_�O�_���ӵ����S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pDeadChar ���`�����⪫��C
    /// @param x ���⦺�`�ɪ� X �y�СC
    /// @param y ���⦺�`�ɪ� Y �y�СC
    /// @param isPkMode �O�_�� PK �Ҧ��]�v�T�W���޿�^�C
    void SetEffect(ClientCharacter* pDeadChar, float x, float y, int isPkMode);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x005313A0 ���_) ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84): ���V���`������
    unsigned int     m_dwStartTime;     // �첾 +136 (0x88): �S�ĳЫخɪ��ɶ��W
    int              m_bIsPK_Mode;      // �첾 +140 (0x8C): �O�_�� PK �Ҧ�
};