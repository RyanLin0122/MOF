#include "Image/cltImageManager.h"
#include <windows.h> // 為了 MessageBoxA
#include <cstdio>    // 為了 wsprintfA

// 初始化靜態成員指標
cltImageManager* cltImageManager::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
cltImageManager* cltImageManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) cltImageManager();
    }
    return s_pInstance;
}

cltImageManager::cltImageManager() {
    // C++ 的陣列成員 m_Images 會在此處自動呼叫 5000 次 GameImage 的預設建構函式。
    // 這對應了 `eh vector constructor iterator` 的行為。
    // 不需要額外的程式碼。
}

cltImageManager::~cltImageManager() {
    // C++ 在銷毀 cltImageManager 物件時，會自動呼叫 m_Images 陣列中
    // 每個 GameImage 物件的解構函式。
    // 這對應了 `eh vector destructor iterator` 的行為。
    // 不需要額外的程式碼。
}

void cltImageManager::Initialize() {
    // 為池中的每一個 GameImage 物件預先建立頂點緩衝區。
    for (int i = 0; i < MAX_IMAGES; ++i) {
        m_Images[i].CreateVertexBuffer();
    }
}

void cltImageManager::Free() {
    // 重設池中所有的 GameImage 物件，釋放它們佔用的資源。
    for (int i = 0; i < MAX_IMAGES; ++i) {
        m_Images[i].ResetGI();
    }
}

GameImage* cltImageManager::GetGameImage(unsigned int dwGroupID, unsigned int dwResourceID, int a4, int a5) {
    // 參數 a2 在反編譯程式碼中被命名為 dwGroupID
    // 參數 a3 在反編譯程式碼中被命名為 dwResourceID

    // 反編譯程式碼中的一個檢查，如果 groupID 為 0，則檢查資源是否已在載入列表中。
    if (dwGroupID == 0 && ResourceMgr::GetInstance()->FindInResLoadingList(dwResourceID)) {
        return nullptr;
    }

    // 遍歷物件池，尋找第一個未被使用的 GameImage。
    for (int i = 0; i < MAX_IMAGES; ++i) {
        if (!m_Images[i].IsInUse()) {
            // 找到了閒置物件，讓它去獲取圖片資源。
            GameImage* pImage = &m_Images[i];
            pImage->GetGIData(dwGroupID, dwResourceID, a4, a5);
            // 在將 GameImage 物件交出去之前，確保其 D3D 紋理已經被建立。
            if (pImage->IsInUse()) {
                // 使用我們剛剛新增的 getter 取得資源資料指標
                ImageResourceListData* pGIData = pImage->GetGIDataPtr();
                if (pGIData) {
                    // 觸發從記憶體到 VRAM 的紋理載入
                    pGIData->m_Resource.LoadTexture();
                }
            }
            return pImage;
        }
    }

    if (dwGroupID == ResourceMgr::RES_EFFECT) {
        static int s_effectRecycleCursor = 0;
        for (int step = 0; step < MAX_IMAGES; ++step) {
            int idx = (s_effectRecycleCursor + step) % MAX_IMAGES;
            GameImage* pImage = &m_Images[idx];
            if (!pImage->IsInUse() || pImage->m_dwGroupID != dwGroupID) {
                continue;
            }

            pImage->ReleaseGIData();
            pImage->GetGIData(dwGroupID, dwResourceID, a4, a5);
            if (pImage->IsInUse()) {
                ImageResourceListData* pGIData = pImage->GetGIDataPtr();
                if (pGIData) {
                    pGIData->m_Resource.LoadTexture();
                }
            }
            s_effectRecycleCursor = (idx + 1) % MAX_IMAGES;
            return pImage;
        }
    }

    // 如果遍歷完畢都沒找到閒置物件，表示物件池已滿。
    CHAR Text[256];
    wsprintfA(Text, "Put image over=>%0x:%0x", dwGroupID, dwResourceID);
    MessageBoxA(NULL, Text, "Error", 0);

    return nullptr;
}

void cltImageManager::ReleaseGameImage(GameImage* pImage) {
    if (pImage) {
        // 歸還物件到池中的方法就是呼叫其 ReleaseGIData，
        // 這會釋放其圖片資源，並將其標記為未使用狀態。
        // 原始碼中會檢查資源指標是否存在，我們也遵循這個邏輯。
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
        // 只對正在使用中的 GameImage 進行頂點運算。
        if (m_Images[i].IsInUse()) {
            m_Images[i].Process();
        }
    }
}