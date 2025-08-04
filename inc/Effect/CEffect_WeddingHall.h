#pragma once

#include "Effect/CEffectBase.h"

// �e�V�ŧi
class ClientCharacter;

/**
 * @class CEffect_WeddingHall
 * @brief �b�B§�a�Ϫ��T�w�y�м���A�B�ͩR�g���Ѫ��a�O�_�b�a�Ϥ��M�w���S�ġC
 */
class CEffect_WeddingHall : public CEffectBase {
public:
    CEffect_WeddingHall();
    virtual ~CEffect_WeddingHall();

    // --- �����禡�мg ---

    /// @brief �ھڸj�w���⪺�a��ID�ӧP�_�O�_���ӵ����S�ġC
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- �M���禡 ---

    /// @brief ��l�ƯS�ĸ귽�C
    void Init();

    /// @brief �]�w�S�ġC
    /// @param x �S�Ĥ����I�� X �y�СC
    /// @param y �S�Ĥ����I�� Y �y�СC
    /// @param pPlayer ���a����A�Ω��ˬd�a��ID�C
    void SetEffect(float x, float y, ClientCharacter* pPlayer);

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00537ED0 ���_) ---
    ClientCharacter* m_pOwnerPlayer; // �첾 +132 (0x84): ��a��ID�M�w�S�ĥͩR�g��������
};