#include "Image/cltGIResource.h"
#include "Image/CDeviceResetManager.h"
#include "CMOFPacking.h"
#include <cstdio>
#include <string.h>
#include <windows.h> // For MessageBoxA

// 控制是從獨立檔案(0)還是封裝檔(1)載入
extern int IsInMemory;

// 假設的字型和區域處理類別，用於模擬原始碼中的行為
class MoFFont {
public:
    // 模擬 GetNationCode，原始碼根據此回傳值決定區域化路徑
    unsigned int GetNationCode() { return 2; /* 假設 2 代表繁體中文 */ }
};
extern MoFFont g_MoFFont;
MoFFont g_MoFFont;


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
    // 基底類別的解構函式會自動被呼叫
}

void cltGIResource::Initialize(const char* pPath, unsigned int timeout)
{
    // 複製基礎路徑
    strcpy_s(m_szBasePath, sizeof(m_szBasePath), pPath);

    // 呼叫基底類別的 Initialize，並傳入固定的容量 2000 (0x7D0)
    cltBaseResource::Initialize(2000, timeout);

    // 設定旗標，表示尚未搜尋過區域化路徑，與 C 版本 `*((_DWORD *)this + 136) = 1;` 一致
    m_bNationPathSearched = true;
}

ImageResourceListData* cltGIResource::LoadResourceInPack(unsigned int id, int a3, unsigned char a4)
{
    // 備註：原始 C 程式碼根據 'a3' 參數切換不同的封裝檔 (g_clMofPacking/g_clCharPacking)。
    // 由於提供的 CMofPacking C++ API 為 Singleton 模式，這裡我們使用單一實例。
    // 'a3' 參數依然會被傳遞給 CreateImageResource 函式。
    CMofPacking* pPacker = CMofPacking::GetInstance();
    if (!pPacker || !pPacker->m_pNfsHandle) {
        return nullptr; // 如果封裝檔未開啟，直接返回
    }

    char szSearchPattern[1024];
    NfsGlobResults* pResults = nullptr;

    // 1. 第一次嘗試：在基礎路徑中搜尋
    wsprintfA(szSearchPattern, "%s/%08X*.*", m_szBasePath, id);
    pPacker->ChangeString(szSearchPattern); // 將搜尋字串轉為小寫
    pResults = pPacker->SearchString(szSearchPattern);

    // 2. 第二次嘗試 (備用路徑)：如果找不到，則在區域化路徑中搜尋
    if (!pResults) {
        if (m_bNationPathSearched) { // 等同於 C 版本檢查 `*((_DWORD *)this + 136)`
            m_bNationPathSearched = false; // 設定為 false，避免重複搜尋

            // 此邏輯模擬 C 版本中根據 NationCode 設定備用路徑的行為
            // 此處簡化為固定的 "TChinese" 路徑
            const char* nationString = "TChinese";
            wsprintfA(m_szNationPath, "MofData/%s", nationString);
        }

        // 使用區域化路徑進行搜尋
        wsprintfA(szSearchPattern, "MofData/%s/%08X*.*", m_szNationPath, id);
        pPacker->ChangeString(szSearchPattern);
        pResults = pPacker->SearchString(szSearchPattern);
    }

    // 3. 處理搜尋結果
    if (!pResults) {
        char szErrorMsg[256];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "Cannot find GI file in pack. (ID: %08X)", id);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        return nullptr;
    }

    // 如果找到多個檔案，視為錯誤
    if (pResults->gl_pathc > 1) {
        char szErrorMsg[256];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "Found multiple GI files in pack. (ID: %08X)", id);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        pPacker->DeleteSearchData(); // 清理搜尋結果
        return nullptr;
    }

    // 取得唯一的檔案路徑
    char szFinalFileName[1024];
    strcpy_s(szFinalFileName, sizeof(szFinalFileName), pResults->gl_pathv[0]);

    // 釋放搜尋資料結構
    pPacker->DeleteSearchData();

    // 4. 建立圖片資源
    ImageResourceListData* pImageData = CDeviceResetManager::GetInstance()->CreateImageResource(szFinalFileName, 0, a4, a3);

    if (!pImageData) {
        char szErrorMsg[1024 + 100];
        sprintf_s(szErrorMsg, sizeof(szErrorMsg), "%s GI  failure file loading .", szFinalFileName);
        MessageBoxA(NULL, szErrorMsg, "Error", MB_OK);
        return nullptr;
    }

    return pImageData;
}


// --- 其他函式 (維持原樣，僅為完整性) ---

ImageResourceListData* cltGIResource::LoadResource(unsigned int id, int a3, unsigned char a4)
{
    // 此函式用於從獨立檔案載入，其邏輯與 C 版本相符，無需修改。
    char szFullPath[1024];
    char szSearchPath[1024];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // 1. 嘗試在基礎路徑下尋找檔案
    wsprintfA(szSearchPath, "%s/%08X*.*", m_szBasePath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "%s/%s", m_szBasePath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 2. 如果基礎路徑找不到，且是第一次，則嘗試尋找區域化路徑
    if (m_bNationPathSearched)
    {
        m_bNationPathSearched = false;
        wsprintfA(m_szNationPath, "MofData/%s", "TChinese");
    }

    // 3. 嘗試在區域化路徑下尋找
    wsprintfA(szSearchPath, "MofData/%s/%08X*.*", m_szNationPath, id);
    hFind = FindFirstFileA(szSearchPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        wsprintfA(szFullPath, "MofData/%s/%s", m_szNationPath, findFileData.cFileName);
        FindClose(hFind);
        return CDeviceResetManager::GetInstance()->CreateImageResource(szFullPath, 0, a4, a3);
    }

    // 都找不到，載入失敗
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