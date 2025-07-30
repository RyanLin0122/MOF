#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Mon_Die_Ghost
 * @brief �N��Ǫ����`�ɥX�{���F��S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӧb�T�w�y�м���@�����ʵe�S�ġC
 */
class CEffect_Mon_Die_Ghost : public CEffectBase {
public:
    CEffect_Mon_Die_Ghost();
    virtual ~CEffect_Mon_Die_Ghost();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥΩT�w����l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī���ܦ�m�C
    /// @param pDeadMonster ���`���Ǫ����⪫��C
    void SetEffect(ClientCharacter* pDeadMonster);

private:
    // �����O�S���B�~�������ܼơA�Ҧ���m��T���x�s�b CEffectBase ���C
};