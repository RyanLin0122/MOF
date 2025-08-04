#pragma once

#include <d3dx9math.h> // ���F D3DXVECTOR2

/**
 * @class CBulletBase
 * @brief ²���g���]�l�u�^����H�����O�C
 *
 * ���C�����W�ߩ� CEffectManager �޲z�����檫�鴣�Ѥ@�ӲΤ@�������C
 */
class CBulletBase {
public:
    CBulletBase();
    virtual ~CBulletBase();

    // --- �֤ߵ����禡 ---

    /// @brief �Ыبó]�w��g������l���A�C
    /// @param dwOwnerID �o�g�̪��ߤ@ ID�C
    /// @param pStartPos �_�l��m�C
    /// @param pEndPos �ؼЦ�m�C
    /// @param fSpeed ����t�סC
    virtual void Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed) = 0; // �µ���

    /// @brief ��s��g�������A�]�Ҧp��m�^�C
    /// @param fElapsedTime �g�L���ɶ��C
    /// @return �p�G��g�����Q�P���A�h�^�� true�C
    virtual bool Process(float fElapsedTime);

    /// @brief ø�s��g���C
    virtual void Draw() = 0; // �µ���

protected:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052D700 ���_) ---
    void* m_pVftable;        // �첾 +0

    unsigned int m_dwOwnerID;  // �첾 +4: �o�g�̪� ID

    // ��m�P��V
    D3DXVECTOR2 m_vecPos;      // �첾 +8, +12
    D3DXVECTOR2 m_vecDir;      // �첾 +16, +20

    float m_fSpeed;            // �첾 +24
};