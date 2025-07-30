#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Item_Use_HPPotion
 * @brief �N��ϥ� HP �Ĥ��ɪ��v���S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺤@���ʰʵe�S�ġC
 */
class CEffect_Item_Use_HPPotion : public CEffectBase {
public:
    CEffect_Item_Use_HPPotion();
    virtual ~CEffect_Item_Use_HPPotion();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī����H�ؼСC
    /// @param pUser �ϥ��Ĥ�������C
    void SetEffect(ClientCharacter* pUser);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡]�ϥ��Ĥ�������^�C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};