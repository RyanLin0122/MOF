#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h"
#include "CMOFPacking.h"
#include <cstdio>
#include <string.h>
#include <windows.h> // For MessageBoxA

// ����O�q�W���ɮ�(0)�٬O�ʸ���(1)���J
extern int IsInMemory;

// ���]���r���M�ϰ�B�z���O�A�Ω������l�X�����欰
class MoFFont {
public:
    // ���� GetNationCode�A��l�X�ھڦ��^�ǭȨM�w�ϰ�Ƹ��|
    unsigned int GetNationCode() { return 2; /* ���] 2 �N���c�餤�� */ }
};
extern MoFFont g_MoFFont;
MoFFont g_MoFFont;


// --- cltGIResource ���O����{ ---

cltGIResource::cltGIResource()
    : cltBaseResource() // �I�s�����O���غc�禡
{
    // �b C++ ���Avftable �|�ѽsĶ���۰ʳ]�w
    memset(m_szBasePath, 0, sizeof(m_szBasePath));
    memset(m_szNationPath, 0, sizeof(m_szNationPath));
    m_bNationPathSearched = false;
}

cltGIResource::~cltGIResource()
{
    // �����O���Ѻc�禡�|�۰ʳQ�I�s
}

void cltGIResource::Initialize(const char* pPath, unsigned int timeout)
{
    // �ƻs��¦���|
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), pPath);

    // �I�s�����O�� Initialize�A�öǤJ�T�w���e�q 2000 (0x7D0)
    cltBaseResource::Initialize(2000, timeout);

    // �]�w�X�СA��ܩ|���j�M�L�ϰ�Ƹ��|�A�P C ���� `*((_DWORD *)this + 136) = 1;` �@�P
    m_bNationPathSearched = true;
}

ImageResourceListData* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4)
{
    // �Ƶ��G��l C �{���X�ھ� 'a3' �ѼƤ������P���ʸ��� (g_clMofPacking/g_clCharPacking)�C
    // �ѩ󴣨Ѫ� CMofPacking C++ API �� Singleton �Ҧ��A�o�̧ڭ̨ϥγ�@��ҡC
    // 'a3' �Ѽƨ̵M�|�Q�ǻ��� CreateImageResource �禡�C
    CMofPacking* pPacker = CMofPacking::GetInstance();
    if (!pPacker || !pPacker->m_pNfsHandle) {
        return nullptr; // �p�G�ʸ��ɥ��}�ҡA������^
    }

    char szSearchPattern[1024];
    NfsGlobResults* pResults = nullptr;

    // 1. �Ĥ@�����աG�b��¦���|���j�M
    wsprintfA(szSearchPattern, "%s/%08X*.*", m_szBasePath, id);
    pPacker->ChangeString(szSearchPattern); // �N�j�M�r���ର�p�g
    pResults = pPacker->SearchString(szSearchPattern);

    // 2. �ĤG������ (�ƥθ��|)�G�p�G�䤣��A�h�b�ϰ�Ƹ��|���j�M
    if (!pResults) {
        if (m_bNationPathSearched) { // ���P�� C �����ˬd `*((_DWORD *)this + 136)`
            m_bNationPathSearched = false; // �]�w�� false�A�קK���Ʒj�M

            // ���޿���� C �������ھ� NationCode �]�w�ƥθ��|���欰
            // ���B²�Ƭ��T�w�� "TChinese" ���|
            const char* nationString = "TChinese";
            wsprintfA(m_szNationPath, "MofData/%s", nationString);
        }

        // �ϥΰϰ�Ƹ��|�i��j�M
        wsprintfA(szSearchPattern, "MofData/%s/%08X*.*", m_szNationPath, id);
        pPacker->ChangeString(szSearchPattern);
        pResults = pPacker->SearchString(szSearchPattern);
    }

    // 3. �B�z�j�M���G
    if (!pResults) {
        char szErrorMsg[256];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "Cannot find GI file in pack. (ID: %08X)", id);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        return nullptr;
    }

    // �p�G���h���ɮסA�������~
    if (pResults->gl_pathc > 1) {
        char szErrorMsg[256];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "Found multiple GI files in pack. (ID: %08X)", id);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        pPacker->DeleteSearchData(); // �M�z�j�M���G
        return nullptr;
    }

    // ���o�ߤ@���ɮ׸��|
    char szFinalFileName[1024];
    strcpy_s(szFinalFileName, sizeof(szFinalFileName), pResults->gl_pathv[0]);

    // ����j�M��Ƶ��c
    pPacker->DeleteSearchData();

    // 4. �إ߹Ϥ��귽
    ImageResourceListData* pImageData = CDeviceResetManager::GetInstance()->CreateImageResource(szFinalFileName, 0, a4, a3);

    if (!pImageData) {
        char szErrorMsg[1024 + 100];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "%s GI  failure file loading .", szFinalFileName);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        return nullptr;
    }

    return pImageData;
}


// --- ��L�禡 (������ˡA�Ȭ������) ---

ImageResourceListData* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4)
{
    // ���禡�Ω�q�W���ɮ׸��J�A���޿�P C �����۲šA�L�ݭק�C
    char szFullPath[1024];
    char szSearchPath[1024];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // 1. ���զb��¦���|�U�M���ɮ�
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szBasePath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 2. �p�G��¦���|�䤣��A�B�O�Ĥ@���A�h���մM��ϰ�Ƹ��|
    if (m_bNationPathSearched)
    {
        m_bNationPathSearched = false;
        wsprintfA(m_szNationPath, "MofData/%s", "TChinese");
    }

    // 3. ���զb�ϰ�Ƹ��|�U�M��
    wsprintfA(szSearchPath, "MofData/%s/%08X*.*", m_szNationPath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "MofData/%s/%s", m_szNationPath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // ���䤣��A���J����
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "GI file %08X not found.", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResource Error", MB_OK);

    return nullptr;
}


void cltGIResource::FreeResource(void* pResourceData)
{
    if (pResourceData)
    {
        CDeviceResetManager::GetInstance()->DeleteImageResource(static_cast<ImageResourceListData*>(pResourceData));
    }
}