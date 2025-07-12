#pragma once

#include <windows.h> // for DWORD

// �e�V�ŧi�A�����Y�ɮץi�H�W�߳Q�ޥΡA�Ӥ��ݭn���D ImageResourceListData ������w�q
struct ImageResourceListData;

/// @class cltBaseResource
/// @brief �x�Ϊ��귽�޲z�������O�C
///
/// �o�����O�޲z�@�Ӹ귽�֨����A�]�t�Ѧҭp�ƩM���ɶ����۰��������C
/// ���w�q�F�@�Ӯج[�A�ӹ�ڪ��귽���J�P����ѭl�����O (�p cltGIResource)
/// �z�L��{�µ����禡�ӧ����C
class cltBaseResource {
public:
    /// @brief �غc�禡�G��l�ƩҦ��������w�]�ȡC
    cltBaseResource();

    /// @brief �����Ѻc�禡�G�T�O�b�R�������O���ЮɡA�l�����O���Ѻc�禡��Q���T�I�s�C
    virtual ~cltBaseResource();

    /// @brief ��l�Ƹ귽�޲z���C
    /// @param capacity �֨������̤j�e�q�C
    /// @param timeout �귽�b�L�H�ޥΫ�A�۰ʳQ�^�����O�ɮɶ� (�@��)�C0 ��ܤ��^���C
    void Initialize(unsigned int capacity, unsigned int timeout);

    /// @brief ����Ҧ��귽�òM�z�֨����C
    void Free();

    /// @brief ���o�@�Ӹ귽�C�p�G�귽���b�֨����A�|Ĳ�o���J�y�{�C
    /// @param id �귽���ߤ@ID�C
    /// @param a3 �ǻ������J�禡���ѼơC
    /// @param a4 �ǻ������J�禡���ѼơC
    /// @return ���V�귽��ƪ����� (void*)�A���ѫh��^ nullptr�C
    void* Get(unsigned int id, int a3 = 0, int a4 = 0);

    /// @brief �Ȩ��o�w�s�b��֨������귽�A�p�G���s�b�h���|Ĳ�o���J�C
    /// @param id �귽���ߤ@ID�C
    /// @return ���V�귽��ƪ����� (void*)�A�Y���s�b�h��^ nullptr�C
    void* Get1(unsigned int id, int a3 = 0, unsigned char a4 = 0);

    /// @brief �����@�Ӹ귽���ޥ� (�N�Ѧҭp�ƴ� 1)�C
    /// @param id �귽��ID�C
    /// @return ���\��^ 1�A���Ѫ�^ 0�C
    int Release(unsigned int id);

    /// @brief �j��q�֨������R���@�Ӹ귽�A�L�ר�Ѧҭp�Ƭ���C
    /// @param id �귽��ID�C
    /// @return ���\��^ 1�A���Ѫ�^ 0�C
    int Delete(unsigned int id);

    /// @brief ���ߨ禡�A�Ω��ˬd�æ^���W�ɥB�L�H�ޥΪ��귽�C
    void Poll();

    /// @brief �j��R���Ҧ��w���J���귽�C
    void DeleteAllResource();

    /// @brief ���o���w�귽�ثe���Ѧҭp�ơC
    /// @param id �귽��ID�C
    /// @return �귽���Ѧҭp�ơA�Y�귽���s�b�h��^ 0�C
    int GetRefCount(unsigned int id);

protected:
    /// @struct ResourceEntry
    /// @brief �b�귽�֨������A�Ω�y�z��@�귽���A�����c�C
    struct ResourceEntry {
        unsigned int id;        // �귽���ߤ@ID
        void* pData;     // ���V��ڸ귽��ƪ����� (�Ҧp ImageResourceListData*)
        int          refCount;  // �Ѧҭp��
        DWORD        lastAccess;// �̫�s���ɶ��W (timeGetTime() �����G)�A�Ω�۰ʦ^��
    };

    // --- �����ܼ� ---
    ResourceEntry* m_pResourcePool;   // ���V�귽�֨����}�C������
    unsigned int   m_nCapacity;       // �֨������̤j�e�q
    unsigned int   m_nItemCount;      // �ثe�֨����������ؼƶq
    unsigned int   m_nTimeout;        // �귽�۰ʦ^�����O�ɮɶ� (�@��)
    bool           m_bUnkFlag;        // �������X�� (�b�첾+20)

    /// @brief �����禡�A�Ω�B�z�귽���s�W�޿�A�|�I�s�������J�禡�C
    /// @return ���A�X (0:����, 1:�w��, 2:�w�s�b, 3:���\�[�J)�C
    int Add(unsigned int id, int a3, int a4);

    // --- �µ����禡 (Pure Virtual Functions) ---
    // �H�U�禡�w�q�F�l�����O������{�������C

    /// @brief �q�W���ɮ׸��J�귽����@�C
    virtual ImageResourceListData* LoadResource(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief �q�ʸ��ɸ��J�귽����@�C
    virtual ImageResourceListData* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief �����@�귽�Ҧ��Ϊ��O����M�귽�C
    virtual void FreeResource(void* pResourceData) = 0;
};