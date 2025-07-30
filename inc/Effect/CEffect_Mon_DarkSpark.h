#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Mon_DarkSpark
 * @brief �N��Ǫ��·t�����R���ɪ�����S�ġC
 *
 * �~�Ӧ� CEffectBase�A�O�@�Ӹ��H���⼽�񪺤@���ʰʵe�S�ġC
 */
class CEffect_Mon_DarkSpark : public CEffectBase {
public:
    CEffect_Mon_DarkSpark();
    virtual ~CEffect_Mon_DarkSpark();

    // --- �����禡�мg ---

    /// @brief �B�z�S�Ī��ͩR�g���A�����̿� CCAEffect ���ʵe���񪬺A�C
    virtual bool FrameProcess(float fElapsedTime) override;

    /// @brief �ǳ�ø�s�e���޿��s�A�P�B�֦��̪���m�C
    virtual void Process() override;

    /// @brief ø�s�S�ġC
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�Ī����H�ؼСC
    /// @param pTarget �S�ĭn���[�쪺�ؼШ���C
    void SetEffect(ClientCharacter* pTarget);

protected:
    // --- �����ܼ� ---

    /// @brief ���V�S�Ī��֦��̡]�Q����������^�C
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84)
};