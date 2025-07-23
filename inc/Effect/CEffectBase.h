#pragma once

#include "Effect/CCAEffect.h" // CEffectBase �]�t�@�� CCAEffect ����ӳB�z��V

/**
 * @class CEffectBase
 * @brief �Ҧ��C���S�Ī���H�����O�C
 *
 * �o�����O���ѤF�@�ӲΤ@�������Ӻ޲z�S�Ī��ͩR�g���]FrameProcess�^�B
 * �޿��s�]Process�^�Mø�s�]Draw�^�C������������ø�s�A�ӬO�]�t�@��
 * CCAEffect ����ӳB�z�Ҧ��P�ʵe�M��V���������h�u�@�C
 * �C�Ө��骺�S�ġ]�p���y�B�v���������^�������~�Ӧ۳o�����O�C
 */
class CEffectBase {
public:
    /// @brief �غc�禡�A��l�ư�¦�ݩʡC
    CEffectBase();

    /// @brief �����Ѻc�禡�A�T�O�l�����O��Q���T�P���C
    virtual ~CEffectBase();

    // --- �֤ߵ����禡 (�ѭl�����O��@) ---

    /**
     * @brief �B�z�S�Ī��C�v���s�A�D�n�t�d��s��ͩR�g���C
     * @param fElapsedTime �ۤW�@�v��H�Ӹg�L���ɶ��]��^�C
     * @return �p�G�S�����ӳQ�P���A�h�^�� true�F�_�h�^�� false�C
     */
    virtual bool FrameProcess(float fElapsedTime);

    /**
     * @brief �ǳ�ø�s�e���޿��s�C
     * �q�`�b�o�̭p��S�Ī���m�B�i���ʡA�ç�s CCAEffect �����A�C
     */
    virtual void Process() = 0; // �µ����禡�A�j��l�����O��@

    /**
     * @brief ø�s�S�ġC
     * �q�`�u�O�I�s���� m_ccaEffect �� Draw �禡�C
     */
    virtual void Draw() = 0; // �µ����禡�A�j��l�����O��@


protected:
    /**
     * @brief �ˬd�S�ĬO�_�b�i���d�򤺡]²�������ŧP�_�^�C
     * @param x �S�Ī� X �y�СC
     * @param y �S�Ī� Y �y�СC
     * @return �p�G�b�i���d�򤺡A��^ TRUE�F�_�h��^ FALSE�C
     */
    bool IsCliping(float x, float y);

protected:

    void* m_pOwner;      // �첾 +4:  ���V�֦��̪��󪺫��С]�Ҧp�I�k�̡^

    // ��m�P��V
    float           m_fCurrentPosX; // �첾 +8:  �S�ķ�e�� X �y��
    float           m_fCurrentPosY; // �첾 +12: �S�ķ�e�� Y �y��
    float           m_fDirectionX;  // �첾 +16: X ��V�V�q/�t��
    float           m_fDirectionY;  // �첾 +20: Y ��V�V�q/�t��
    float           m_fSpeed;       // �첾 +24: ���ʳt��

    bool            m_bIsFlip;     // �첾 +28: �O�_����½��

    // ø�s����
    bool            m_bIsVisible;  // �첾 +32: �ھڵ��ŧP�_�A�S�ĬO�_�i��

    // �֤ߴ�V����
    CCAEffect       m_ccaEffect;   // �첾 +36: ��ڭt�d�ʵe����Mø�s������
};