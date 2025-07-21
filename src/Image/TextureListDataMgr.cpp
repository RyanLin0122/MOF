#include "Image/TextureListDataMgr.h"
#include <cstring> // �Ω� memset
#include <new>

// ���]�������ܼơA�Ӧ۩��l�X�W�U��
extern IDirect3DDevice9* Device; // ����D3D�˸m

template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// TextureListDataMgr ���غc�禡
// �����ϽsĶ�X: 0x00544710
TextureListDataMgr::TextureListDataMgr() {
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// TextureListDataMgr ���Ѻc�禡
// �����ϽsĶ�X: �b Device_Reset_Manager::~Device_Reset_Manager ���Q���t�I�s
TextureListDataMgr::~TextureListDataMgr() {
    DeleteAll();
}

// �s�W�@�Ӹ`�I���쵲��C������
// �����ϽsĶ�X: 0x00544720
TextureListData* TextureListDataMgr::Add() {
    // ���t�@�ӷs���`�I
    TextureListData* pNewNode = new (std::nothrow) TextureListData();
    if (!pNewNode) {
        return nullptr; // �O������t����
    }

    // ��l�Ʒs�`�I�������ܼ�
    pNewNode->pPrev = nullptr;
    pNewNode->pNext = nullptr;
    pNewNode->pTexture = nullptr;
    std::memset(pNewNode->szFileName, 0, sizeof(pNewNode->szFileName));
    pNewNode->flag = 0;

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
// �����ϽsĶ�X: 0x00544780
void TextureListDataMgr::Delete(TextureListData* pNode) {
    if (!pNode || m_nCount == 0) {
        return; // �L�ľާ@
    }

    // �ھڸ`�I�b�쵲��C������m��s����
    if (pNode == m_pHead) {
        m_pHead = pNode->pNext;
        if (m_pHead) {
            m_pHead->pPrev = nullptr;
        }
        else {
            m_pTail = nullptr;
        }
    }
    else if (pNode == m_pTail) {
        m_pTail = pNode->pPrev;
        m_pTail->pNext = nullptr;
    }
    else {
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }

    // �P���`�I���� (�|�۰ʩI�s��Ѻc�禡�H����Texture)
    delete pNode;
    m_nCount--;

    // �p�G�p�ƾ���0�A�T�O�Y�����г�����
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

// �R���Ҧ��`�I
// �����ϽsĶ�X: 0x00544850
void TextureListDataMgr::DeleteAll() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        TextureListData* pNext = pCurrent->pNext;
        delete pCurrent; // �I�s�Ѻc�禡������O����
        pCurrent = pNext;
    }
    // ���]�޲z�����A
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// �˸m�򥢮ɡA����Ҧ�D3D�귽
// �����ϽsĶ�X: 0x00544890
void TextureListDataMgr::DeviceLostToRelease() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // ����Texture�A���O�d�`�I�����Ψ��ɮצW
        SafeRelease(pCurrent->pTexture);
        pCurrent = pCurrent->pNext;
    }
}

// �˸m���]��A���s���JD3D�귽
// �����ϽsĶ�X: 0x005448C0
void TextureListDataMgr::DeviceLostToReLoad() {
    TextureListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        // �p�G�ɮצW�s�b�A�h���խ��s���J���z
        if (pCurrent->szFileName[0] != '\0') {
            D3DXCreateTextureFromFileExA(
                Device,
                pCurrent->szFileName,
                D3DX_DEFAULT_NONPOW2, // Width
                D3DX_DEFAULT_NONPOW2, // Height
                D3DX_DEFAULT,         // MipLevels
                0,                    // Usage
                D3DFMT_UNKNOWN,       // Format
                D3DPOOL_MANAGED,      // Pool
                D3DX_FILTER_LINEAR,   // Filter
                D3DX_FILTER_LINEAR,   // MipFilter
                0xFFFF00FF,           // ColorKey
                nullptr,              // SrcInfo
                nullptr,              // Palette
                &pCurrent->pTexture);
        }
        pCurrent = pCurrent->pNext;
    }
}