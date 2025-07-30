#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_Field_Warp
 * @brief �t�d��ܶǰe���ζǰe�k�}����ı�S�ġC
 *
 * �~�Ӧ� CEffectBase�A�i�H�ھڶǤJ��������ܤ��P�~�[�A
 * �åB�i�H��ܸ��H����ΰ��d�b�T�w��m�C
 */
class CEffect_Field_Warp : public CEffectBase {
public:
    // ���M��l�X�����ѡA�����F����ʦӲK�[
    CEffect_Field_Warp();
    virtual ~CEffect_Field_Warp();

    // --- �����禡�мg ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---
    /// @brief �]�w�ǰe���S�ġC
    /// @param warpType �S�Ī������A�M�w�F��~�[�C
    /// @param x ��l X �y�� (�p�G pOwner ���šA�h���T�w�y��)�C
    /// @param y ��l Y �y�� (�p�G pOwner ���šA�h���T�w�y��)�C
    /// @param pOwner �n���H�����⪫����СA�i�� nullptr�C
    void SetEffect(unsigned short warpType, float x, float y, ClientCharacter* pOwner);

private:
    // --- �����ܼ� ---
    ClientCharacter* m_pOwnerCharacter; // �첾 +132 (0x84): �n���H������
};