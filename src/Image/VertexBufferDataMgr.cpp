#include "Image/VertexBufferDataMgr.h"
#include <new>
#include "Image/GIVertex.h"

// ���]�������ܼƩM�w�q�A�Ӧ۩��l�X�W�U��
extern IDirect3DDevice9* Device; // ����D3D�˸m

// VertexBufferDataMgr ���غc�禡
// �����ϽsĶ�X: 0x00544910
VertexBufferDataMgr::VertexBufferDataMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// VertexBufferDataMgr ���Ѻc�禡
// �����ϽsĶ�X: �b Device_Reset_Manager::~Device_Reset_Manager ���Q���t�I�s
VertexBufferDataMgr::~VertexBufferDataMgr() {
    DeleteAll();
}

// �s�W�@�Ӹ`�I���쵲��C������
// �����ϽsĶ�X: 0x00544920
VertexBufferData* VertexBufferDataMgr::Add() {
    // ���t�@�ӷs���`�I
    VertexBufferData* pNewNode = new (std::nothrow) VertexBufferData();
    if (!pNewNode) {
        return nullptr; // �O������t����
    }

    // ��l�Ʒs�`�I�������ܼ�
    pNewNode->pPrev = nullptr;
    pNewNode->pNext = nullptr;
    pNewNode->pVertexBuffer = nullptr;
    pNewNode->capacity = 0;
    pNewNode->type = 0;

    // �N�s�`�I�[�J���쵲��C������
    if (m_pTail == nullptr) {
        // �p�G�쵲��C�O�Ū�
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    }
    else {
        // �p�G�쵲��C�D��
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }

    m_nCount++;
    return pNewNode;
}

// �q�쵲��C���R�����w���`�I
// �����ϽsĶ�X: 0x00544980
void VertexBufferDataMgr::Delete(VertexBufferData* pNode) {
    if (!pNode || m_nCount == 0) {
        return; // �L�ľާ@
    }

    // �ھڸ`�I�b�쵲��C������m��s����
    if (pNode == m_pHead) {
        // �`�I�O�Y�`�I
        m_pHead = pNode->pNext;
        if (m_pHead) {
            m_pHead->pPrev = nullptr;
        }
        else {
            // �p�G�R�����쵲��C���šA�����Ф]�ݳ]����
            m_pTail = nullptr;
        }
    }
    else if (pNode == m_pTail) {
        // �`�I�O���`�I
        m_pTail = pNode->pPrev;
        m_pTail->pNext = nullptr;
    }
    else {
        // �`�I�b����
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }

    // �P���`�I���� (�|�۰ʩI�s��Ѻc�禡�H����VB)
    delete pNode;
    m_nCount--;

    // �p�G�p�ƾ���0�A�T�O�Y�����г�����
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

// �R���Ҧ��`�I
// �����ϽsĶ�X: 0x00544A50
void VertexBufferDataMgr::DeleteAll() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        VertexBufferData* pNext = pCurrent->pNext;
        delete pCurrent; // �I�s�Ѻc�禡������O����
        pCurrent = pNext;
    }
    // ���]�޲z�����A
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// �˸m�򥢮ɡA����Ҧ�D3D�귽
// �����ϽsĶ�X: 0x00544A90
void VertexBufferDataMgr::DeviceLostToRelease() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // ����VertexBuffer�A���O�d�`�I�����Ψ���
        SafeRelease(pCurrent->pVertexBuffer);
        pCurrent = pCurrent->pNext;
    }
}

// �˸m���]��A���s���JD3D�귽
// �����ϽsĶ�X: 0x00544AC0
void VertexBufferDataMgr::DeviceLostToReLoad() {
    VertexBufferData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // �p�G�o�Ӹ`�I���e�����I�w�İ� (�Y�ϲ{�b�O�Ū�) �B���e�q
        if (pCurrent->capacity > 0) {
            // �ھ��x�s�������M�e�q���s�إ�VertexBuffer
            // �o�̥u�٭�F�ϽsĶ�X���X�{�� GIVertex �����޿�
            switch (pCurrent->type) {
                // �ھڤϽsĶ�X�A���� 0, 1, 2, 3 ���|�i�J�o�Ӥ���
            case 0:
            case 1:
            case 2:
            case 3:
                Device->CreateVertexBuffer(
                    28 * pCurrent->capacity, // �j�p
                    520,                     // �Ϊk (D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC)
                    GIVertex::FVF,           // FVF
                    D3DPOOL_MANAGED,         // �O�����
                    &pCurrent->pVertexBuffer,
                    NULL
                );
                break;
            }
        }
        pCurrent = pCurrent->pNext;
    }
}