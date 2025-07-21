#pragma once

#include <windows.h> // �ϥ� timeGetTime, MessageBoxA ��
#include <cstdio>    // �ϥ� NULL

/**
 * @struct ResourceInfo
 * @brief �x�s��@�귽�����~��ơC
 */
struct ResourceInfo {
    unsigned int id;         // �귽���ߤ@ID
    void* pData;      // ���V��ڸ귽��ƪ�����
    int          refCount;   // �ޥέp��
    DWORD        timestamp;  // �̫�@���Q���񪺮ɶ��W
};

/**
 * @class cltBaseResource
 * @brief �귽�޲z����H�����O�C
 *
 * �w�q�F�@�M�q�Ϊ��귽�޲z�ج[�A�]�A���J�B����B�ޥέp�ƩM�۰ʦ^���C
 * ���骺�귽���J�P�����޿�ѭl�����O�z�L��@�µ����禡�ӧ����C
 */
class cltBaseResource {
public:
    /// @brief �غc�禡
    cltBaseResource();

    /// @brief �����Ѻc�禡
    virtual ~cltBaseResource();

    /// @brief ��l�Ƹ귽�޲z���C
    /// @param capacity �귽�}�C���̤j�e�q�C
    /// @param timeout ���m�귽�Q�۰ʦ^�����W�ɮɶ�(�@��)�C0��ܤ��^���C
    void Initialize(unsigned int capacity, unsigned int timeout);

    /// @brief ����Ҧ��w���t�������O����C
    void Free();

    /// @brief ���o�귽�C�p�G�귽���s�b�A�|���ո��J�C
    /// @return ���\�h��^���V�귽��ƪ����СA�_�h��^ nullptr�C
    void* Get(unsigned int id, int a3, int a4);

    /// @brief ���o�귽�A���p�G���s�b�h�����J�C
    /// @return ���\�h��^���V�귽��ƪ����СA�_�h��^ nullptr�C
    void* Get1(unsigned int id, int a3, unsigned char a4);

    /// @brief �����@�Ӹ귽���ޥΡC
    /// @return ���\��^1�A�귽���s�b��^0�C
    int Release(unsigned int id);

    /// @brief ���o���w�귽���ޥέp�ơC
    /// @return ��^�ޥέp�ƭȡA�Y�귽���s�b�h��^0�C
    int GetRefCount(unsigned int id);

    /// @brief ���ߨòM�z���m���귽�C
    virtual void Poll();

    /// @brief �R���Ҧ��w���J���귽�C
    void DeleteAllResource();

protected:
    /// @brief ���շs�W�@�Ӹ귽��޲z�����]�q�`��Get�����I�s�^�C
    /// @return 1:���~(Buffer��), 2:�w�s�b, 3:�s�W���\
    virtual int Add(unsigned int id, int a3, int a4);

    /// @brief �q�޲z�����R���@�Ӹ귽�C
    /// @return ���\��^1�A���Ѫ�^0�C
    virtual int Delete(unsigned int id);

    // --- �µ����禡 (�ѭl�����O��@) ---

    /// @brief �q�ɮ׸��J�귽�C
    virtual void* LoadResource(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief �q�ʸ��ɸ��J�귽�C
    virtual void* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief ����귽��ơC
    virtual void FreeResource(void* pResourceData) = 0;

protected:
    ResourceInfo* m_pResourceArray;           // �귽��T�}�C
    unsigned int  m_uResourceArrayCapacity;   // �}�C�e�q
    unsigned int  m_uResourceCount;           // �ثe�귽�ƶq
    DWORD         m_dwTimeout;                // ���m�W�ɮɶ�(ms)
    bool          m_bInitialized;             // �O�_�w��l��
};