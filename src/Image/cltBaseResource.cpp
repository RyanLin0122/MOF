#include "Image/cltBaseResource.h"
#include <windows.h> // for timeGetTime, MessageBoxA, etc.
#include <cstdio>    // for wsprintfA
#include <string.h>  // for memmove
#include <mmsystem.h>

// --- �~���̿઺�����ܼ� ---
extern int IsInMemory; // ����O�q�W���ɮ�(0)�٬O�ʸ���(1)���J


// --- cltBaseResource ���O����{ ---

cltBaseResource::cltBaseResource()
{
    // ��l�ƩҦ������ܼ�
    m_pResourcePool = nullptr;
    m_nCapacity = 0;
    m_nItemCount = 0;
    m_nTimeout = 0;
    m_bUnkFlag = false;
}

cltBaseResource::~cltBaseResource()
{
    // �b�Ѻc�ɽT�O�Ҧ��귽�Q����
    Free();
}

void cltBaseResource::Initialize(unsigned int capacity, unsigned int timeout)
{
    m_nCapacity = capacity;
    m_nTimeout = timeout;

    // �t�m�귽�����O����
    // �ϥ� new[] ����ŦX C++ ����A�ýT�O���T���Ѻc
    m_pResourcePool = new ResourceEntry[m_nCapacity];
    memset(m_pResourcePool, 0, sizeof(ResourceEntry) * m_nCapacity);

    m_nItemCount = 0;
}

void cltBaseResource::Free()
{
    if (m_pResourcePool)
    {
        // �R���Ҧ��٦b�������귽
        DeleteAllResource();

        // ����귽���������O����
        delete[] m_pResourcePool;
        m_pResourcePool = nullptr;
    }
    m_nCapacity = 0;
}

void* cltBaseResource::Get(unsigned int id, int a3, int a4)
{
    // �o�O�@�ӵL���j��A����귽�Q���Ϋإߥ���
    while (true)
    {
        // 1. �b�{�����֨������M��귽
        for (unsigned int i = 0; i < m_nItemCount; ++i)
        {
            if (m_pResourcePool[i].id == id)
            {
                // ���F�A�W�[�Ѧҭp�ƨê�^�귽����
                m_pResourcePool[i].refCount++;
                return m_pResourcePool[i].pData;
            }
        }

        // 2. �p�G�S���A�N�I�s Add �禡���ո��J�s�귽
        // Add �禡�|�I�s�l�����O���������J�禡 (LoadResource / LoadResourceInPack)
        int addResult = this->Add(id, a3, a4);

        // �p�G Add ���� (�Ҧp�ϺФ��L���ɮ�)�A�h���X�j��ê�^����
        if (addResult != 3) // 3 �N���\�[�J
        {
            // OutputDebugStringA("Resource Add failed\n");
            return nullptr;
        }

        // �p�G Add ���\�Awhile(true) �j��|���s�}�l�A
        // �b�U�@�����N���A�귽�N�|�b�B�J 1 ���Q���C
        // �o����a�٭�F�ϽsĶ�{���X���޿�C
    }
}

int cltBaseResource::Release(unsigned int id)
{
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            // ���F�A�Ѧҭp�ƴ� 1
            if (m_pResourcePool[i].refCount > 0)
            {
                m_pResourcePool[i].refCount--;
            }
            // ��s�̫�s���ɶ��A�Ω� Poll ����
            m_pResourcePool[i].lastAccess = timeGetTime();
            return 1; // ���\
        }
    }
    return 0; // �S���
}

// �o�O�����禡�A�� Get �I�s
int cltBaseResource::Add(unsigned int id, int a3, int a4)
{
    // �ˬd�귽�O�_�w�s�b (���M Get �禡�w�g�ˬd�L�A���o�O��l�޿�)
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            return 2; // �w�s�b
        }
    }

    // �ˬd�֨����O�_�w��
    if (m_nItemCount >= m_nCapacity)
    {
        char szText[256];
        wsprintfA(szText, "Error! image buffer over: %0x", id);
        MessageBoxA(NULL, szText, "BaseResource::Add", 0);
        return 1; // ����
    }

    // �I�s�ѭl�����O��{�������禡�Ӹ��J�귽
    void* pNewResource = nullptr;
    if (IsInMemory) // �ھڥ���X�ШM�w���J�覡
    {
        pNewResource = this->LoadResourceInPack(id, a3, a4);
    }
    else
    {
        pNewResource = this->LoadResource(id, a3, a4);
    }

    if (!pNewResource)
    {
        return 0; // ���J����
    }

    // �N�s���J���귽�[�J��֨�����
    ResourceEntry& newEntry = m_pResourcePool[m_nItemCount];
    newEntry.id = id;
    newEntry.pData = pNewResource;
    newEntry.refCount = 0; // Get �禡����|�⥦�[�� 1
    newEntry.lastAccess = timeGetTime();

    m_nItemCount++;

    return 3; // ���\�[�J
}

// �o�O�����禡�A�� Poll �� DeleteAllResource �I�s
int cltBaseResource::Delete(unsigned int id)
{
    if (!m_nItemCount || id == 0) return 0;

    int nIndex = -1;
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            nIndex = i;
            break;
        }
    }

    if (nIndex == -1) return 0; // �S���

    // �I�s�l�����O�������禡������귽����ڸ��
    this->FreeResource(m_pResourcePool[nIndex].pData);

    // �q�}�C�������ӱ��ءA�ñN�᭱���������e����
    if (nIndex < (int)m_nItemCount - 1)
    {
        memmove(&m_pResourcePool[nIndex],
            &m_pResourcePool[nIndex + 1],
            sizeof(ResourceEntry) * (m_nItemCount - nIndex - 1));
    }

    m_nItemCount--;
    return 1; // ���\�R��
}

void cltBaseResource::Poll()
{
    if (!m_nTimeout || !m_nItemCount) return;

    DWORD currentTime = timeGetTime();

    // �q�᩹�e�M���H�w���a�R������
    for (int i = m_nItemCount - 1; i >= 0; --i)
    {
        if (m_pResourcePool[i].refCount == 0 &&
            (currentTime - m_pResourcePool[i].lastAccess > m_nTimeout))
        {
            // �p�G�귽�L�H�ޥΥB�w�O�ɡA�h�R����
            this->Delete(m_pResourcePool[i].id);
        }
    }
}

void cltBaseResource::DeleteAllResource()
{
    // ���ЧR���Ĥ@�Ӥ����A����֨�������
    while (m_nItemCount > 0)
    {
        this->Delete(m_pResourcePool[0].id);
    }
}

int cltBaseResource::GetRefCount(unsigned int id)
{
    for (unsigned int i = 0; i < m_nItemCount; ++i)
    {
        if (m_pResourcePool[i].id == id)
        {
            return m_pResourcePool[i].refCount;
        }
    }
    return 0; // �S���
}