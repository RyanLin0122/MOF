#include "Image/cltBaseResource.h"
#include <cstring> // �ϥ� memmove, memset

// �����ϽsĶ�X: 0x005442C0
cltBaseResource::cltBaseResource() {
    // ��l�ƩҦ������ܼ�
    m_pResourceArray = nullptr;
    m_uResourceArrayCapacity = 0;
    m_uResourceCount = 0;
    m_dwTimeout = 0;
    m_bInitialized = false; // ���� *((_BYTE *)this + 20) = 0;
}

// �����ϽsĶ�X: 0x005442F0
cltBaseResource::~cltBaseResource() {
    Free();
}

// �����ϽsĶ�X: 0x00544300
void cltBaseResource::Initialize(unsigned int capacity, unsigned int timeout) {
    m_uResourceArrayCapacity = capacity;
    m_dwTimeout = timeout;
    m_uResourceCount = 0;

    // ���t�귽�}�C�O����
    m_pResourceArray = new ResourceInfo[capacity];
    // �N�O����M�s
    memset(m_pResourceArray, 0, sizeof(ResourceInfo) * capacity);
}

// �����ϽsĶ�X: 0x00544350
void cltBaseResource::Free() {
    if (m_pResourceArray) {
        // ������Ҧ��귽
        DeleteAllResource();
        // �A����}�C����
        delete[] m_pResourceArray;
        m_pResourceArray = nullptr;
    }
    m_uResourceArrayCapacity = 0;
}

// �����ϽsĶ�X: 0x00544380
void* cltBaseResource::Get(unsigned int id, int a3, int a4) {
    // �ϥΤ@�ӵL���j��ӽT�O�b�귽�Q�s�W��ন�\���o
    while (true) {
        for (unsigned int i = 0; i < m_uResourceCount; ++i) {
            if (m_pResourceArray[i].id == id) {
                m_pResourceArray[i].refCount++; // �W�[�ޥέp��
                return m_pResourceArray[i].pData; // ��^�귽����
            }
        }

        // �p�G�䤣��A�h�I�sAdd�禡���ո��J
        if (Add(id, a3, a4) != 3) {
            //OutputDebugStringA("Resource Add failed\n");
            return nullptr; // �s�W����
        }
        // �p�GAdd���\�A�j��|�~��A�æb�U�@�����N������s�W���귽
    }
}

// �����ϽsĶ�X: 0x005443F0
void* cltBaseResource::Get1(unsigned int id, int a3, unsigned char a4) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            m_pResourceArray[i].refCount++;
            return m_pResourceArray[i].pData;
        }
    }
    return nullptr; // �䤣�쪽����^
}

// �����ϽsĶ�X: 0x00544440
int cltBaseResource::Release(unsigned int id) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            m_pResourceArray[i].refCount--; // ��֤ޥέp��
            m_pResourceArray[i].timestamp = timeGetTime(); // ��s�ɶ��W
            return 1;
        }
    }
    return 0; // �䤣��귽
}

// �����ϽsĶ�X: 0x00544680
int cltBaseResource::GetRefCount(unsigned int id) {
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            return m_pResourceArray[i].refCount;
        }
    }
    return 0;
}

// �����ϽsĶ�X: 0x00544600
void cltBaseResource::Poll() {
    if (!m_dwTimeout || m_uResourceCount == 0) {
        return; // �p�G�W�ɳ]��0�ΨS���귽�A�h������
    }

    DWORD currentTime = timeGetTime();
    // �q�᩹�e�M���A�]���R���ާ@�|���ʤ���
    for (int i = m_uResourceCount - 1; i >= 0; --i) {
        if (m_pResourceArray[i].refCount <= 0 && (currentTime - m_pResourceArray[i].timestamp > m_dwTimeout)) {
            // �I�s������Delete�禡�Ӳ����W�ɪ��귽
            this->Delete(m_pResourceArray[i].id);
        }
    }
}

// �����ϽsĶ�X: 0x00544660
void cltBaseResource::DeleteAllResource() {
    // ���ƧR���Ĥ@�Ӥ����A����}�C����
    while (m_uResourceCount > 0) {
        this->Delete(m_pResourceArray[0].id);
    }
}

// �����ϽsĶ�X: 0x00544490
int cltBaseResource::Add(unsigned int id, int a3, int a4) {
    // �ˬd�귽�O�_�w�g�s�b
    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            return 2; // �귽�w�s�b
        }
    }

    // �ˬd�}�C�e�q�O�_�w��
    if (m_uResourceCount >= m_uResourceArrayCapacity) {
        char text[256];
        wsprintfA(text, "Error! image buffer over: %0x", id);
        MessageBoxA(nullptr, text, "BaseResource::Add", MB_OK);
        return 1; // �}�C�w��
    }

    // �I�s�µ����禡���J�귽
    // ��l�X������ IsInMemory ����X�ШӨM�w�I�s���Ӹ��J�禡
    extern int IsInMemory;
    void* pNewData = IsInMemory ?
        LoadResourceInPack(id, a3, a4) :
        LoadResource(id, a3, a4);

    if (!pNewData) {
        return 0; // ���J����
    }

    // �N�s�귽�[�J�}�C
    ResourceInfo& newInfo = m_pResourceArray[m_uResourceCount];
    newInfo.id = id;
    newInfo.pData = pNewData;
    newInfo.refCount = 0; // ��l�ޥά�0�AGet�禡�|�ߧY�N��+1
    newInfo.timestamp = 0;

    m_uResourceCount++;

    return 3; // �s�W���\
}

// �����ϽsĶ�X: 0x00544580
int cltBaseResource::Delete(unsigned int id) {
    if (!id || m_uResourceCount == 0) {
        return 0;
    }

    for (unsigned int i = 0; i < m_uResourceCount; ++i) {
        if (m_pResourceArray[i].id == id) {
            // �I�s�µ����禡����귽���
            FreeResource(m_pResourceArray[i].pData);

            // �N�}�C�����򪺤������e���ʡA�л\���Q�R��������
            unsigned int numToMove = m_uResourceCount - (i + 1);
            if (numToMove > 0) {
                memmove(
                    &m_pResourceArray[i],
                    &m_pResourceArray[i + 1],
                    sizeof(ResourceInfo) * numToMove
                );
            }

            m_uResourceCount--;
            // �M�z���ʫ�h�X�Ӫ��̫�@�Ӥ�����m
            memset(&m_pResourceArray[m_uResourceCount], 0, sizeof(ResourceInfo));

            return 1; // �R�����\
        }
    }

    return 0; // �䤣��귽
}