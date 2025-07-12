#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h" // ���]���쪺 g_Device_Reset_Manager �b��
#include <cstdio>  // for sprintf, wsprintfA
#include <string.h> // for strcpy, memset


// ����O�q�W���ɮ�(0)�٬O�ʸ���(1)���J
extern int IsInMemory;

// ���]���ʸ��ɳB�z���O�M����
class CMofPacking { /* ... */ };
extern CMofPacking g_clMofPacking;
extern CMofPacking g_clCharPacking;

// ���]���r���M�ϰ�B�z���O
class MoFFont { /* ... */ };
extern MoFFont g_MoFFont;


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
    // �����O���Ѻc�禡�|�۰ʳQ�I�s�A�H����귽
}

void cltGIResource::Initialize(const char* pPath, unsigned int timeout)
{
    // �ƻs��¦���|
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), pPath);

    // �I�s�����O�� Initialize�A�öǤJ�T�w���e�q 2000 (0x7D0)
    cltBaseResource::Initialize(2000, timeout);

    // �]�w�X�СA��ܩ|���j�M�L�ϰ�Ƹ��|
    m_bNationPathSearched = true;
}

ImageResourceListData* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4)
{
    char szFullPath[1024];
    char szSearchPath[1024];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // 1. ���զb��¦���|�U�M���ɮ�
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szBasePath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        // ���F�A�զX������|�ø��J
        wsprintfA(szFullPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 2. �p�G��¦���|�䤣��A�B�O�Ĥ@���A�h���մM��ϰ�Ƹ��|
    if (m_bNationPathSearched)
    {
        m_bNationPathSearched = false; // �аO���w�j�M�L
        // �o�����޿�O�ھڤϽsĶ�{���X���_���A�Ω�]�w m_szNationPath
        // ���B�ٲ��F MoFFont::GetNationCode ������ switch�A�ȥܷN
        wsprintfA(m_szNationPath, "MofData/%s", "TChinese"); // ���]���c�餤����|
    }

    // 3. ���զb�ϰ�Ƹ��|�U�M��
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szNationPath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "%s/%s", m_szNationPath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // ���䤣��A���J����
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "GI �ɮ� %08X �����C", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResource Error", MB_OK);

    return nullptr;
}

ImageResourceListData* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4)
{
    // �o�Ө禡���ר̿�~�����ʸ��ɨt�� (CMofPacking)�A�o�̥u��������޿�y�{�C
    // �b�z����ڱM�פ��A�ݭn�� CMofPacking �������{�C

    char szSearchPattern[1024];
    char szFinalFileName[1024];

    // 1. ���ͦb�ʸ��ɤ��j�M���Ҧ� (��¦���|)
    wsprintfA(szSearchPattern, "%s/%08X*.*", m_szBasePath, id);

    // ���]���j�M�禡
    // const char* foundFile = g_clMofPacking.SearchString(szSearchPattern, a3);
    const char* foundFile = nullptr; // ���]�����

    if (foundFile)
    {
        // �p�G���A�������J
        return CDeviceResetManager::GetInstance()->CreateImageResource(foundFile, 0, a4, a3);
    }

    // 2. �p�G�䤣��A���ճƥΪ��ϰ�Ƹ��|
    if (m_bNationPathSearched)
    {
        m_bNationPathSearched = false;
        wsprintfA(m_szNationPath, "MofData/%s", "TChinese");
    }

    wsprintfA(szSearchPattern, "%s/%08X*.*", m_szNationPath, id);
    // foundFile = g_clMofPacking.SearchString(szSearchPattern, a3);

    if (foundFile)
    {
        return CDeviceResetManager::GetInstance()->CreateImageResource(foundFile, 0, a4, a3);
    }

    // ���䤣��A���J����
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "�ʸ��ɤ������ GI �귽 %08X�C", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResourceInPack Error", MB_OK);

    return nullptr;
}

void cltGIResource::FreeResource(void* pResourceData)
{
    if (pResourceData)
    {
        // �N void* ������^ ImageResourceListData* �å浹�޲z���R��
        CDeviceResetManager::GetInstance()->DeleteImageResource(static_cast<ImageResourceListData*>(pResourceData));
    }
}