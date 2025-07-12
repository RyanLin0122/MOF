#pragma once

#include "ImageResource.h" // ���] ImageResource.h �w�Q�w�q�B�i�Q�]�t

// �e�V�ŧi
struct ImageResourceListData;

/// @class ImageResourceListDataMgr
/// @brief �޲z ImageResourceListData �`�I�����V�쵲��C�C
///
/// �o�Ӻ޲z���t�d�إߡB�R���M�l�ܩҦ����Ϥ��귽�C
/// �����欰�O�q�ϽsĶ�� C �{���X���٭쪺�C
class ImageResourceListDataMgr {
public:
    /// @brief �غc�禡
    ImageResourceListDataMgr();

    /// @brief �Ѻc�禡
    ~ImageResourceListDataMgr();

    /// @brief �b�쵲��C�������s�W�@�ӷs���귽�`�I�C
    /// @return ���V�s�إߪ� ImageResourceListData �`�I�����СC
    ImageResourceListData* Add();

    /// @brief �q�쵲��C���R�����w���귽�`�I�C
    /// @param pNode �n�R�����`�I���СC
    void Delete(ImageResourceListData* pNode);

    /// @brief �R���쵲��C�����Ҧ��`�I�C
    void DeleteAll();

    /// @brief �� Direct3D �˸m�� (Device Lost) �ɩI�s�C
    /// �M���Ҧ��귽�����񥦭̪����z�A���O�d�O���餤����ơC
    void DeviceLostToRelease();

    /// @brief �� Direct3D �˸m���] (Reset) ��I�s�C
    /// �M���Ҧ��귽�îھګO�d���ɮ׸�T���s���J���̡C
    void DeviceLostToReLoad();

private:
    ImageResourceListData* m_pHead; // ���V�쵲��C���Ĥ@�Ӹ`�I
    ImageResourceListData* m_pTail; // ���V�쵲��C���̫�@�Ӹ`�I
    int m_nCount;                   // �쵲��C���`�I���`��
};

/// @struct ImageResourceListData
/// @brief ���V�쵲��C�����`�I�A�Ω�s��@�ӹϤ��귽�Ψ䤸�ƾڡC
///
/// ���c�O�ھڤϽsĶ�{���X�����O����G���M�j�p (0x138 bytes) ���_�X�Ӫ��C
struct ImageResourceListData {
    ImageResourceListData* pPrev;       // ���V�e�@�Ӹ`�I
    ImageResourceListData* pNext;       // ���V��@�Ӹ`�I
    ImageResource m_Resource;           // �O�J���Ϥ��귽����

    // �q Device_Reset_Manager::CreateImageResource ���_�X������
    char m_szFileName[255];             // �귽���ɮצW��
    char m_cFlag;                       // ���ϥΪ��X�� (���� a3)
    unsigned char m_ucPackerType;       // �ʸ������� (���� a4)
    char m_padding[3];                  // �O����������R�줸��

    /// @brief �`�I���غc�禡
    ImageResourceListData();

    /// @brief �`�I���Ѻc�禡
    // �b��l�X���A���@�ө��T���Ѻc�禡�Q�I�s�A
    // �D�n�ت��OĲ�o ImageResource ���Ѻc�C�b C++ ���A�o�O�۰ʳB�z���C
    ~ImageResourceListData();
};