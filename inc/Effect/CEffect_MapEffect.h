#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_MapEffect
 * @brief �b�a�Ϫ��T�w�y�м���A�B�ͩR�g���ѥ~�����󱱨�S�ġC
 */
class CEffect_MapEffect : public CEffectBase {
public:
    CEffect_MapEffect();
    virtual ~CEffect_MapEffect();

    // --- �����禡�мg ---

    /// @brief �ھڸj�w���⪺���A ID �ӧP�_�O�_���ӵ����S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief �]�w�S�� (�z�L�W��)�C
    void SetEffect(char* szEffectName, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y);

    /// @brief �]�w�S�� (�z�L ID)�C
    void SetEffect(unsigned short effectKindID, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00537810 ���_) ---
    ClientCharacter* m_pStateOwner;     // �첾 +132 (0x84): �䪬�A�M�w�S�ĥͩR�g��������
    unsigned short   m_wRequiredStateID;  // �첾 +136 (0x88): �����S�Ħs�b�һݪ����A ID
};