#include "Image/cltImageManager.h"
#include <windows.h> // ���F MessageBoxA
#include <cstdio>    // ���F wsprintfA

// ��l���R�A��������
cltImageManager* cltImageManager::s_pInstance = nullptr;

// �R�A GetInstance ��k����{
cltImageManager* cltImageManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) cltImageManager();
    }
    return s_pInstance;
}

cltImageManager::cltImageManager() {
    // C++ ���}�C���� m_Images �|�b���B�۰ʩI�s 5000 �� GameImage ���w�]�غc�禡�C
    // �o�����F `eh vector constructor iterator` ���欰�C
    // ���ݭn�B�~���{���X�C
}

cltImageManager::~cltImageManager() {
    // C++ �b�P�� cltImageManager ����ɡA�|�۰ʩI�s m_Images �}�C��
    // �C�� GameImage ���󪺸Ѻc�禡�C
    // �o�����F `eh vector destructor iterator` ���欰�C
    // ���ݭn�B�~���{���X�C
}

void cltImageManager::Initialize() {
    // ���������C�@�� GameImage ����w���إ߳��I�w�İϡC
    for (int i = 0; i < MAX_IMAGES; ++i) {
        m_Images[i].CreateVertexBuffer();
    }
}

void cltImageManager::Free() {
    // ���]�����Ҧ��� GameImage ����A���񥦭̦��Ϊ��귽�C
    for (int i = 0; i < MAX_IMAGES; ++i) {
        m_Images[i].ResetGI();
    }
}

GameImage* cltImageManager::GetGameImage(unsigned int dwGroupID, unsigned int dwResourceID, int a4, int a5) {
    // �Ѽ� a2 �b�ϽsĶ�{���X���Q�R�W�� dwGroupID
    // �Ѽ� a3 �b�ϽsĶ�{���X���Q�R�W�� dwResourceID

    // �ϽsĶ�{���X�����@���ˬd�A�p�G groupID �� 0�A�h�ˬd�귽�O�_�w�b���J�C���C
    if (dwGroupID == 0 && ResourceMgr::GetInstance()->FindInResLoadingList(dwResourceID)) {
        return nullptr;
    }

    // �M��������A�M��Ĥ@�ӥ��Q�ϥΪ� GameImage�C
    // �P�_�̾ڬO��귽���� m_GIData �O�_���šC
    // �b GameImage.c ���Am_GIData ��󰾲��q+8����m (*((_DWORD *)this + 2))�C
    for (int i = 0; i < MAX_IMAGES; ++i) {
        // �ڭ̰��] GameImage ���@�� IsInUse() ��k���ˬd��귽���ЬO�_���šC
        if (!m_Images[i].IsInUse()) {
            // ���F���m����A�����h����Ϥ��귽�C
            GameImage* pImage = &m_Images[i];
            pImage->GetGIData(dwGroupID, dwResourceID, a4, a5);
            return pImage;
        }
    }

    // �p�G�M���������S��춢�m����A��ܪ�����w���C
    CHAR Text[256];
    wsprintfA(Text, "Put image over=>%0x:%0x", dwGroupID, dwResourceID);
    MessageBoxA(NULL, Text, "Error", 0);

    return nullptr;
}

void cltImageManager::ReleaseGameImage(GameImage* pImage) {
    if (pImage) {
        // �k�٪�����������k�N�O�I�s�� ReleaseGIData�A
        // �o�|�����Ϥ��귽�A�ñN��аO�����ϥΪ��A�C
        // ��l�X���|�ˬd�귽���ЬO�_�s�b�A�ڭ̤]��`�o���޿�C
        if (pImage->IsInUse()) {
            pImage->ReleaseGIData();
        }
    }
}

void cltImageManager::ReleaseAllGameImage() {
    for (int i = 0; i < MAX_IMAGES; ++i) {
        if (m_Images[i].IsInUse()) {
            m_Images[i].ReleaseGIData();
        }
    }
}

void cltImageManager::ProcessAllGameImage() {
    for (int i = 0; i < MAX_IMAGES; ++i) {
        // �u�勵�b�ϥΤ��� GameImage �i�泻�I�B��C
        if (m_Images[i].IsInUse()) {
            m_Images[i].Process();
        }
    }
}