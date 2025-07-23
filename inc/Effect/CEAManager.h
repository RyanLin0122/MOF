#pragma once
#include "Effect/CCAEffect.h" // �]�t CCAEffect ���w�q

/**
 * @class CEAManager
 * @brief �S�İʵe (.ea) �ƾڪ��޲z���C
 * * �ϥγ�ҼҦ��A�@���@�ӥ��쪺�֨��t�ΡA�t�d�i�~�����J�B
 * �x�s�ô��ѩҦ��S�Ī��ʵe�ƾڡC
 */
class CEAManager {
public:
    /// @brief ���o�ߤ@���޲z����ҡC
    static CEAManager* GetInstance();

    /// @brief �Ѻc�禡�A�|����Ҧ��w���J���S�ļƾڡC
    ~CEAManager();

    /// @brief ������w���S�İʵe�ƾڡC
    /// �p�G�ƾک|�����J�A���禡�|Ĳ�o�ɮ�Ū���M�ѪR�C
    /// @param effectID �S�Ī��ߤ@ ID�C
    /// @param szFileName �S�Ī��ɮצW�١C
    /// @param pEffect �n�����ƾڪ� CCAEffect ������СC
    void GetEAData(int effectID, const char* szFileName, CCAEffect* pEffect);

    /// @brief ���]�޲z���A�M�ũҦ��w���J���S�ļƾڡC
    void Reset();

private:
    // --- �p���禡 ---

    CEAManager();
    CEAManager(const CEAManager&) = delete;
    CEAManager& operator=(const CEAManager&) = delete;

    /// @brief �q�W���ɮ׸��J .ea �ƾڡC
    void LoadEA(int effectID, const char* szFileName);

    /// @brief �q�ʸ��� (mof.pak) ���J .ea �ƾڡC
    void LoadEAInPack(int effectID, char* szFileName);

    // --- �p������ ---
    static CEAManager* s_pInstance;

    // �����禡����� (�ѽsĶ���޲z)
    // void* m_pVftable; // �첾 +0

    // ���а}�C�A�@���S�ļƾڪ��֨��C
    EADATALISTINFO* m_pEaData[65535]; // �첾 +4
};