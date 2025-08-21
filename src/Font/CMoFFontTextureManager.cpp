#include "Font/CMoFFontTextureManager.h"
#include <windows.h> // for MessageBoxA
#include <vector>

// 初始化靜態成員指標
CMoFFontTextureManager* CMoFFontTextureManager::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
CMoFFontTextureManager* CMoFFontTextureManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CMoFFontTextureManager();
    }
    return s_pInstance;
}

// 對應反編譯碼: 0x0051E670
CMoFFontTextureManager::CMoFFontTextureManager() {
    // C++ 會自動呼叫 m_FTInfoPool 陣列中每個元素的建構函式
    m_pDevice = nullptr;
    m_nNextAvailableIndex = FONT_TEXTURE_POOL_SIZE - 1;
    
    // 雖然原始碼中進行了 memset，但 Init 函式會重新賦值，所以這裡可以省略
}

// 對應反編譯碼: 0x0051E710
CMoFFontTextureManager::~CMoFFontTextureManager() {
    // C++ 會自動呼叫 m_FTInfoPool 陣列中每個元素的解構函式，
    // FTInfo 的解構函式會負責釋放 D3D 資源。
}

// 對應反編譯碼: 0x0051E780
void CMoFFontTextureManager::InitCMoFFontTextureManager(IDirect3DDevice9* pDevice) {
    m_pDevice = pDevice;
    if (!m_pDevice) {
        return;
    }

    for (int i = 0; i < FONT_TEXTURE_POOL_SIZE; ++i) {
        FTInfo* pCurrentInfo = &m_FTInfoPool[i];

        // 創建紋理
        HRESULT hr = m_pDevice->CreateTexture(
            FONT_TEXTURE_WIDTH,
            FONT_TEXTURE_HEIGHT,
            1,                      // MipLevels
            0,                      // Usage
            D3DFMT_A8R8G8B8,        // Format (原始碼中的 23)
            D3DPOOL_MANAGED,        // Pool (原始碼中的 1)
            &pCurrentInfo->m_pTexture,
            nullptr
        );

        if (FAILED(hr)) {
            // 原始碼中會呼叫 PrintDXError，這裡簡化為 MessageBox
            MessageBoxA(nullptr, "Font texture initialize Fail.", "Error", MB_OK);
            return;
        }

        // 獲取紋理表面，並填充 FTInfo 結構
        if (pCurrentInfo->m_pTexture) {
            pCurrentInfo->m_pTexture->GetSurfaceLevel(0, &pCurrentInfo->m_pSurface);
            pCurrentInfo->m_dwIndex = i;
            // 將指向物件的指標放入可用堆疊中
            m_pAvailableTextureStack[i] = pCurrentInfo;
        }
    }
}

// 對應反編譯碼: 0x0051E730
void CMoFFontTextureManager::Reset() {
    m_pDevice = nullptr;
    m_nNextAvailableIndex = FONT_TEXTURE_POOL_SIZE - 1;

    // 遍歷物件池，釋放所有資源並重設狀態
    for (int i = 0; i < FONT_TEXTURE_POOL_SIZE; ++i) {
        m_FTInfoPool[i].Release();
        m_FTInfoPool[i].m_wRefCnt = 0;
        m_FTInfoPool[i].m_bUsed = FALSE;
        // 重新填充可用堆疊
        m_pAvailableTextureStack[i] = &m_FTInfoPool[i];
    }
}

// 對應反編譯碼: 0x0051E810
FTInfo* CMoFFontTextureManager::GetFontTexture() {
    if (m_nNextAvailableIndex < 0) {
        MessageBoxA(nullptr, "Not enough font to print out", "warning", MB_ICONWARNING);
        return nullptr;
    }

    // 從堆疊頂部取出一個可用的 FTInfo
    FTInfo* pAvailableInfo = m_pAvailableTextureStack[m_nNextAvailableIndex];
    if (pAvailableInfo) {
        pAvailableInfo->m_bUsed = TRUE;
        m_nNextAvailableIndex--;
        return pAvailableInfo;
    }

    return nullptr;
}

// 對應反編譯碼: 0x0051E850
void CMoFFontTextureManager::DeleteFontTexture(FTInfo* pFTInfo) {
    if (!pFTInfo) {
        return;
    }

    // 減少引用計數
    if (--pFTInfo->m_wRefCnt == 0) {
        // 當引用計數為0時，將其歸還至可用堆疊
        m_nNextAvailableIndex++;
        pFTInfo->m_bUsed = FALSE;

        if (m_nNextAvailableIndex < FONT_TEXTURE_POOL_SIZE) {
            m_pAvailableTextureStack[m_nNextAvailableIndex] = pFTInfo;
        } else {
            // 堆疊溢位，這是一個邏輯錯誤
            m_nNextAvailableIndex = FONT_TEXTURE_POOL_SIZE - 1; // 防止索引越界
            // 原始碼中會呼叫 DCTTextManager 和 CMessageBoxManager
            MessageBoxA(nullptr, "Font texture stack overflow!", "Error", MB_OK);
        }
    }
}
