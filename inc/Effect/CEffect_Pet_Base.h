#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class cltPetObject;

/**
 * @class CEffect_Pet_Base
 * @brief �N����[�b�d�����W���@���ʯS�Ī������O�C
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H�d�����񪺰ʵe�S�ġC
 */
class CEffect_Pet_Base : public CEffectBase {
public:
    CEffect_Pet_Base();
    virtual ~CEffect_Pet_Base();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̡]�d���^����m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī����H�ؼЩM��ı�귽�C
    /// @param pPetOwner �n���H���d������C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(cltPetObject* pPetOwner, unsigned short effectKindID, char* szFileName);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡]�d������^�C
    cltPetObject* m_pOwnerPet; // �첾 +132 (0x84)
};