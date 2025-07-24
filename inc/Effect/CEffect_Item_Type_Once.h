#pragma once

#include "Effect/CEffectBase.h"
#include "Character/ClientCharacter.h"

/**
 * @class CEffect_Item_Type_Once
 * @brief �N��b����ϥΪ��~�ɡA��T�w��m����@�����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�P CEffect_Skill_Type_Once ���D�n�ϧO�b��A
 * ���S�Ī���m�b�ЫخɴN�w�T�w�A���|�H���Ⲿ�ʦӧ�s�C
 */
class CEffect_Item_Type_Once : public CEffectBase {
public:
    CEffect_Item_Type_Once();
    virtual ~CEffect_Item_Type_Once();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�ϥγЫخɪ���l��m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�ġC
    /// @param pUser �ϥΪ��~������C
    /// @param effectKindID �S�Ī����� ID�C
    /// @param szFileName �S�ĸ귽 (.ea) ���ɮצW�١C
    void SetEffect(ClientCharacter* pUser, unsigned short effectKindID, char* szFileName);

protected:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00539200 ���_) ---

    /// @brief ���V�ϥΪ��~������C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)

    /// @brief �S�ĳЫخɡA�֦��̪� X �y�ЧַӡC
    float m_fInitialPosX;       // �첾 +136 (0x88)

    /// @brief �S�ĳЫخɡA�֦��̪� Y �y�ЧַӡC
    float m_fInitialPosY;       // �첾 +140 (0x8C)
};