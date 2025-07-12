#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h" // 假設全域的 g_Device_Reset_Manager 在此
#include <cstdio>  // for sprintf, wsprintfA
#include <string.h> // for strcpy, memset


// 控制是從獨立檔案(0)還是封裝檔(1)載入
extern int IsInMemory;

// 假設的封裝檔處理類別和物件
class CMofPacking { /* ... */ };
extern CMofPacking g_clMofPacking;
extern CMofPacking g_clCharPacking;

// 假設的字型和區域處理類別
class MoFFont { /* ... */ };
extern MoFFont g_MoFFont;


// --- cltGIResource 類別的實現 ---

cltGIResource::cltGIResource()
    : cltBaseResource() // 呼叫基底類別的建構函式
{
    // 在 C++ 中，vftable 會由編譯器自動設定
    memset(m_szBasePath, 0, sizeof(m_szBasePath));
    memset(m_szNationPath, 0, sizeof(m_szNationPath));
    m_bNationPathSearched = false;
}

cltGIResource::~cltGIResource()
{
    // 基底類別的解構函式會自動被呼叫，以釋放資源
}

void cltGIResource::Initialize(const char* pPath, unsigned int timeout)
{
    // 複製基礎路徑
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), pPath);

    // 呼叫基底類別的 Initialize，並傳入固定的容量 2000 (0x7D0)
    cltBaseResource::Initialize(2000, timeout);

    // 設定旗標，表示尚未搜尋過區域化路徑
    m_bNationPathSearched = true;
}

ImageResourceListData* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4)
{
    char szFullPath[1024];
    char szSearchPath[1024];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // 1. 嘗試在基礎路徑下尋找檔案
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szBasePath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        // 找到了，組合完整路徑並載入
        wsprintfA(szFullPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 2. 如果基礎路徑找不到，且是第一次，則嘗試尋找區域化路徑
    if (m_bNationPathSearched)
    {
        m_bNationPathSearched = false; // 標記為已搜尋過
        // 這部分邏輯是根據反編譯程式碼推斷的，用於設定 m_szNationPath
        // 此處省略了 MoFFont::GetNationCode 的複雜 switch，僅示意
        wsprintfA(m_szNationPath, "MofData/%s", "TChinese"); // 假設為繁體中文路徑
    }

    // 3. 嘗試在區域化路徑下尋找
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szNationPath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "%s/%s", m_szNationPath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 都找不到，載入失敗
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "GI 檔案 %08X 未找到。", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResource Error", MB_OK);

    return nullptr;
}

ImageResourceListData* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4)
{
    // 這個函式高度依賴外部的封裝檔系統 (CMofPacking)，這裡只能模擬其邏輯流程。
    // 在您的實際專案中，需要有 CMofPacking 的完整實現。

    char szSearchPattern[1024];
    char szFinalFileName[1024];

    // 1. 產生在封裝檔中搜尋的模式 (基礎路徑)
    wsprintfA(szSearchPattern, "%s/%08X*.*", m_szBasePath, id);

    // 假設的搜尋函式
    // const char* foundFile = g_clMofPacking.SearchString(szSearchPattern, a3);
    const char* foundFile = nullptr; // 假設未找到

    if (foundFile)
    {
        // 如果找到，直接載入
        return CDeviceResetManager::GetInstance()->CreateImageResource(foundFile, 0, a4, a3);
    }

    // 2. 如果找不到，嘗試備用的區域化路徑
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

    // 都找不到，載入失敗
    char szErrorMsg[256];
    sprintf_s(szErrorMsg, sizeof(szErrorMsg), "封裝檔中未找到 GI 資源 %08X。", id);
    MessageBoxA(NULL, szErrorMsg, "LoadResourceInPack Error", MB_OK);

    return nullptr;
}

void cltGIResource::FreeResource(void* pResourceData)
{
    if (pResourceData)
    {
        // 將 void* 指標轉回 ImageResourceListData* 並交給管理器刪除
        CDeviceResetManager::GetInstance()->DeleteImageResource(static_cast<ImageResourceListData*>(pResourceData));
    }
}