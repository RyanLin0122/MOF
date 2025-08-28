#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h"
#include "CMOFPacking.h"
#include <windows.h>
#include <cstdio> // 使用 sprintf, wsprintfA

extern unsigned char NationCode;


cltGIResource::cltGIResource() : cltBaseResource() {
    // 基底類別建構函式已被呼叫
    // 清理衍生類別的成員
    memset(m_szBasePath, 0, sizeof(m_szBasePath));
    memset(m_szNationPath, 0, sizeof(m_szNationPath));
    m_bFirstLoad = 1; // 初始化旗標
}

cltGIResource::~cltGIResource() {
}

void cltGIResource::Initialize(const char* basePath, unsigned int timeout) {
    // 呼叫基底類別的Initialize來設定容量和超時
    cltBaseResource::Initialize(2000, timeout); // 容量2000是硬編碼在原始碼中的
    // 複製基礎路徑
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), basePath);
    m_bFirstLoad = 1; // 重設旗標
}

void* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4) {
    char searchString[1024];
    char finalPath[1024];
    NfsGlobResults* pResults = nullptr;

    CMofPacking* pPacker = CMofPacking::GetInstance();
    if (!pPacker || !pPacker->m_pNfsHandle) {
        return nullptr; // 如果封裝檔未開啟，直接返回
    }

    // 1. 在基礎路徑中尋找
    wsprintfA(searchString, "%s/%08X*.*", m_szBasePath, id);
    
    char* changedStr = pPacker->ChangeString(searchString);
    pResults = pPacker->SearchString(changedStr);

    if (pResults) {
        // 找到了，從搜尋結果中取得最終路徑
        // 原始碼的 **((const char ***)searchResult + 1) 結構較複雜，此處簡化其意圖
        strcpy_s(finalPath, sizeof(finalPath), pResults->gl_pathv[0]);
        pPacker->DeleteSearchData();
        return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
    }

    // 2. 如果找不到，嘗試在國家/地區特定路徑中尋找
    if (m_bFirstLoad) {
        m_bFirstLoad = 0; // 確保只設定一次
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
        wsprintfA(searchString, "%s/%08X*.*", m_szNationPath, id);
        changedStr = pPacker->ChangeString(searchString);
        pResults = pPacker->SearchString(changedStr);
        if (pResults) {
            strcpy_s(finalPath, sizeof(finalPath), pResults->gl_pathv[0]);
            pPacker->DeleteSearchData();
            return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
        }
    }

    // 原始碼中有對搜尋結果數量>1和找不到的錯誤彈窗，此處省略以保持簡潔
    char errorMsg[1024];
    sprintf_s(errorMsg, "Cannot find Resource %08X in Packfile.", id);
    MessageBoxA(nullptr, errorMsg, "ERROR", MB_OK);

    return nullptr;
}

void* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4) {
    char searchPath[1024];
    char finalPath[1024];
    WIN32_FIND_DATAA findFileData;

    // 1. 在基礎路徑中尋找
    wsprintfA(searchPath, "%s/%08X*.*", m_szBasePath, id);
    HANDLE hFind = FindFirstFileA(searchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        // 找到了，組合最終路徑
        wsprintfA(finalPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
    }

    // 2. 如果找不到，嘗試在國家/地區特定路徑中尋找
    if (m_bFirstLoad) {
        m_bFirstLoad = 0; // 確保只設定一次
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
        wsprintfA(searchPath, "%s/%08X_*.*", m_szNationPath, id);
        hFind = FindFirstFileA(searchPath, &findFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            wsprintfA(finalPath, "%s/%s", m_szNationPath, findFileData.cFileName);
            FindClose(hFind);
            return CDeviceResetManager::GetInstance()->CreateImageResource(finalPath, 0, a4, a3);
        }
    }

    // 如果都找不到，返回失敗
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "GI file %08X not found.", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResource Error", MB_OK);
    return nullptr;
}

void cltGIResource::FreeResource(void* pResourceData) {
    if (pResourceData) {
        // 將資源指標轉交給Device_Reset_Manager進行刪除
        CDeviceResetManager::GetInstance()->DeleteImageResource(static_cast<ImageResourceListData*>(pResourceData));
    }
}