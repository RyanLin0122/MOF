#include "Image/ImageResourceListDataMgr.h"
#include <cstring> // �Ω� strcpy

// ���]�o�O�@�ӥ����ܼơA�Ω�M�w�귽�����J�覡�A�p�P�b�ϽsĶ�{���X���Ҩ�
// 0: �q�W���ɮ׸��J, �D0: �q�ʸ��ɸ��J
extern int IsInMemory;

//--------------------------------------------------------------------------------
// ImageResourceListData (�`�I) ����{
//--------------------------------------------------------------------------------

ImageResourceListData::ImageResourceListData() {
    pPrev = nullptr;
    pNext = nullptr;

    // m_Resource ���غc�禡�|�Q�۰ʩI�s

    // �ھڭ�l�X�����欰�A�b Add() �禡���o�ǭȷ|�Q��l��
    memset(m_szFileName, 0, sizeof(m_szFileName));
    m_cFlag = 0;
    m_ucPackerType = 0;
}

ImageResourceListData::~ImageResourceListData() {
    // m_Resource ���Ѻc�禡�|�b���B�Q�۰ʩI�s�A����䤺���귽
}


//--------------------------------------------------------------------------------
// ImageResourceListDataMgr ����{
//--------------------------------------------------------------------------------

ImageResourceListDataMgr::ImageResourceListDataMgr() {
    // ��l���쵲��C���Y�B�����ЩM�p�ƾ�
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

ImageResourceListDataMgr::~ImageResourceListDataMgr() {
    // �b�޲z���Q�P���ɡA�T�O�Ҧ��w���t���`�I���Q����
    DeleteAll();
}

ImageResourceListData* ImageResourceListDataMgr::Add() {
    // �إߤ@�ӷs���`�I����
    ImageResourceListData* pNewNode = new ImageResourceListData();

    // �p�G�쵲��C�O�Ū��A�h�s�`�I�J�O�Y�]�O��
    if (m_pHead == nullptr) {
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    }
    else {
        // �_�h�A�N�s�`�I���[���쵲��C������
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;
        m_pTail = pNewNode;
    }

    // �W�[�`�I�p��
    m_nCount++;

    return pNewNode;
}

void ImageResourceListDataMgr::Delete(ImageResourceListData* pNodeToDelete) {
    // �p�G�p�Ƭ�0�Ϋ��Ь��šA�h���������ާ@
    if (m_nCount == 0 || !pNodeToDelete) {
        return;
    }

    // ��s�۾F�`�I������
    if (pNodeToDelete->pPrev) {
        pNodeToDelete->pPrev->pNext = pNodeToDelete->pNext;
    }
    if (pNodeToDelete->pNext) {
        pNodeToDelete->pNext->pPrev = pNodeToDelete->pPrev;
    }

    // ��s�Y/�����С]�p�G�Q�R�����`�I�O�Y�Χ��^
    if (m_pHead == pNodeToDelete) {
        m_pHead = pNodeToDelete->pNext;
    }
    if (m_pTail == pNodeToDelete) {
        m_pTail = pNodeToDelete->pPrev;
    }

    // ��ڧR���`�I������O����
    delete pNodeToDelete;

    // ��ָ`�I�p��
    m_nCount--;

    // �p�G�쵲��C�ܪšA���]�Y������
    if (m_nCount == 0) {
        m_pHead = nullptr;
        m_pTail = nullptr;
    }
}

void ImageResourceListDataMgr::DeleteAll() {
    ImageResourceListData* pCurrent = m_pHead;
    while (pCurrent != nullptr) {
        ImageResourceListData* pNext = pCurrent->pNext;
        delete pCurrent;
        pCurrent = pNext;
    }

    // ���]�޲z�����A
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

void ImageResourceListDataMgr::DeviceLostToRelease() {
    // �M���Ҧ��`�I�A�I�s��귽�� ResetGIData ��k
    for (ImageResourceListData* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        pNode->m_Resource.ResetGIData();
    }
}

void ImageResourceListDataMgr::DeviceLostToReLoad() {
    // �M���Ҧ��`�I�A�ھ��ɮרӷ����s���J�귽
    for (ImageResourceListData* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext) {
        if (IsInMemory) {
            // �q�ʸ��ɸ��J (packerType �x�s�b m_ucPackerType ��)
            // �`�N�G��l�X�� LoadGIInPack ���ĤT�ӰѼ� a5 ���b ImageResourceListData ���x�s
            // �o�̰��]�䬰 0 �Ψ�L�w�]�ȡC
            pNode->m_Resource.LoadGIInPack(pNode->m_szFileName, pNode->m_ucPackerType, 0);
        }
        else {
            // �q�W���ɮ׸��J
            pNode->m_Resource.LoadGI(pNode->m_szFileName, pNode->m_ucPackerType);
        }
    }
}