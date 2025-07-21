#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h"
#include "CMOFPacking.h"
#include <windows.h>
#include <cstdio> // �ϥ� sprintf, wsprintfA

extern unsigned char NationCode;


// �����ϽsĶ�X: 0x00545310
cltGIResource::cltGIResource() : cltBaseResource() {
    // �����O�غc�禡�w�Q�I�s
    // �M�z�l�����O������
    memset(m_szBasePath, 0, sizeof(m_szBasePath));
    memset(m_szNationPath, 0, sizeof(m_szNationPath));
    m_bFirstLoad = 1; // ��l�ƺX��
}

// �����ϽsĶ�X: ~cltGIResource() �O�ѽsĶ���۰ʲ��ͪ��A�S���B�~�޿�
cltGIResource::~cltGIResource() {
    // �Ѻc�ɷ|�۰ʩI�s�����O���Ѻc�禡
}

// �����ϽsĶ�X: 0x005453A0
void cltGIResource::Initialize(const char* basePath, unsigned int timeout) {
    // �I�s�����O��Initialize�ӳ]�w�e�q�M�W��
    cltBaseResource::Initialize(2000, timeout); // �e�q2000�O�w�s�X�b��l�X����
    // �ƻs��¦���|
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), basePath);
    m_bFirstLoad = 1; // ���]�X��
}



// �����ϽsĶ�X: 0x00545400
void* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4) {
    char searchString[1024];
    char finalPath[1024];
    NfsGlobResults* pResults = nullptr;

    CMofPacking* pPacker = CMofPacking::GetInstance();
    if (!pPacker || !pPacker->m_pNfsHandle) {
        return nullptr; // �p�G�ʸ��ɥ��}�ҡA������^
    }

    // 1. �b��¦���|���M��
    wsprintfA(searchString, "%s/%08X*.*", m_szBasePath, id);
    char* changedStr = pPacker->ChangeString(searchString);
    pResults = pPacker->SearchString(changedStr);

    if (pResults) {
        // ���F�A�q�j�M���G�����o�̲׸��|
        // ��l�X�� **((const char ***)searchResult + 1) ���c�������A���B²�ƨ�N��
        strcpy_s(finalPath, sizeof(finalPath), pResults->gl_pathv[0]);
        pPacker->DeleteSearchData();
        return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
    }

    // ����j�M��Ƶ��c
    

    // 2. �p�G�䤣��A���զb��a/�a�ϯS�w���|���M��
    if (m_bFirstLoad) {
        m_bFirstLoad = 0; // �T�O�u�]�w�@��
        switch (NationCode) {
        case 1: strcpy_s(m_szNationPath, "Nation_kor"); break;
        case 2: strcpy_s(m_szNationPath, "Nation_jp"); break;
        case 3: strcpy_s(m_szNationPath, "Nation_tai"); break;
        case 4: strcpy_s(m_szNationPath, "Nation_In"); break;
        case 5: strcpy_s(m_szNationPath, "Nation_Hk"); break;
        default: m_szNationPath[0] = '\0'; break;
        }
    }

    if (m_szNationPath[0] != '\0') {
        wsprintfA(searchString, "MofData/%s/%08X*.*", m_szNationPath, id);
        changedStr = pPacker->ChangeString(searchString);
        pResults = pPacker->SearchString(changedStr);
        if (pResults) {
            strcpy_s(finalPath, sizeof(finalPath), pResults->gl_pathv[0]);
            pPacker->DeleteSearchData();
            return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
        }
    }

    // ��l�X������j�M���G�ƶq>1�M�䤣�쪺���~�u���A���B�ٲ��H�O��²��
    char errorMsg[1024];
    sprintf_s(errorMsg, "Cannot find Resource %08X in Packfile.", id);
    MessageBoxA(nullptr, errorMsg, "ERROR", MB_OK);

    return nullptr;
}

// �����ϽsĶ�X: 0x00545760
void* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4) {
    char searchPath[1024];
    char finalPath[1024];
    WIN32_FIND_DATAA findFileData;

    // 1. �b��¦���|���M��
    wsprintfA(searchPath, "%s/%08X*.*", m_szBasePath, id);
    HANDLE hFind = FindFirstFileA(searchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        // ���F�A�զX�̲׸��|
        wsprintfA(finalPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
    }

    // 2. �p�G�䤣��A���զb��a/�a�ϯS�w���|���M��
    if (m_bFirstLoad) {
        m_bFirstLoad = 0; // �T�O�u�]�w�@��
        switch (NationCode) {
        case 1: strcpy_s(m_szNationPath, "Nation_kor"); break;
        case 2: strcpy_s(m_szNationPath, "Nation_jp"); break;
        case 3: strcpy_s(m_szNationPath, "Nation_tai"); break;
        case 4: strcpy_s(m_szNationPath, "Nation_In"); break;
        case 5: strcpy_s(m_szNationPath, "Nation_Hk"); break;
        default: m_szNationPath[0] = '\0'; break;
        }
    }

    if (m_szNationPath[0] != '\0') {
        wsprintfA(searchPath, "MofData/%s/%08X_*.*", m_szNationPath, id);
        hFind = FindFirstFileA(searchPath, &findFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            wsprintfA(finalPath, "MofData/%s/%s", m_szNationPath, findFileData.cFileName);
            FindClose(hFind);
            return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
        }
    }

    // �p�G���䤣��A��^����
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "GI file %08X not found.", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResource Error", MB_OK);
    return nullptr;
}

// �����ϽsĶ�X: 0x005459D0
void cltGIResource::FreeResource(void* pResourceData) {
    if (pResourceData) {
        // �N�귽������浹Device_Reset_Manager�i��R��
        CDeviceResetManager::GetInstance()->DeleteImageResource(static_cast<ImageResourceListData*>(pResourceData));
    }
}