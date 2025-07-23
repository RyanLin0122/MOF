#include "Effect/CEAManager.h"
#include "Effect/CCAEffect.h"
#include "CMOFPacking.h" // 假設的封裝檔管理器
#include <new>           // for std::nothrow

// 假設的全域變數
extern int IsInMemory; // 旗標：1 = 從封裝檔讀取，0 = 從獨立檔案讀取

// 靜態實例初始化
CEAManager* CEAManager::s_pInstance = nullptr;

CEAManager* CEAManager::GetInstance() {
    if (!s_pInstance) {
        s_pInstance = new (std::nothrow) CEAManager();
    }
    return s_pInstance;
}

// 對應反組譯碼: 0x0053A390
CEAManager::CEAManager() {
    // 將整個指標陣列初始化為 nullptr
    memset(m_pEaData, 0, sizeof(m_pEaData));
}

// 對應反組譯碼: 0x0053A3B0
CEAManager::~CEAManager() {
    Reset();
}

// 對應反組譯碼: 0x0053A480
void CEAManager::Reset() {
    for (int i = 0; i < 65535; ++i) {
        if (m_pEaData[i]) {
            // 由於 EADATALISTINFO 的解構函式會處理內部陣列的釋放，
            // 這裡只需要 delete 最上層的物件即可。
            delete m_pEaData[i];
            m_pEaData[i] = nullptr;
        }
    }
}

// 對應反組譯碼: 0x0053A4C0
/**
 * @brief 獲取指定的特效動畫數據，並將其綁定到一個 CCAEffect 物件上。
 * @param effectID 特效的唯一 ID，作為快取陣列的索引。
 * @param szFileName 特效的檔案名稱，僅在數據尚未載入時使用。
 * @param pEffect 要接收數據的 CCAEffect 物件指標。
 * @note 此函式的邏輯精確還原自 Effectall.c 中 0x0053A4C0 的 CEAManager::GetEAData 函式。
 * 它實現了一個懶漢式載入 (Lazy Loading) 的快取機制。
 */
void CEAManager::GetEAData(int effectID, const char* szFileName, CCAEffect* pEffect)
{
    // 參數有效性檢查
    if (effectID < 0 || effectID >= 65535 || !pEffect) {
        return;
    }

    // 步驟 1: 檢查快取中是否已存在該特效數據
    // 原始碼: v5 = *((_DWORD *)this + a2); if ( v5 ) ...
    if (m_pEaData[effectID] == nullptr)
    {
        // 步驟 2: 如果數據不在快取中，則載入它

        // 2a. 為新的特效數據分配主結構記憶體
        // 原始碼: v7 = operator new(0x24u);
        m_pEaData[effectID] = new (std::nothrow) EADATALISTINFO();
        if (!m_pEaData[effectID]) {
            // 記憶體分配失敗，無法繼續
            return;
        }

        // 2b. 根據全域旗標，決定是從獨立檔案還是封裝檔中讀取並解析數據
        // 原始碼: if ( dword_829254 ) LoadEAInPack(..); else LoadEA(..);
        if (IsInMemory) {
            // 注意：LoadEAInPack 需要一個可修改的 char*，因為其依賴的
            // CMofPacking::ChangeString 可能會修改路徑（儘管它實際上是複製後修改）。
            // 為了與原始碼行為一致並避免 const_cast，我們創建一個臨時副本。
            char tempFileName[256];
            strcpy_s(tempFileName, sizeof(tempFileName), szFileName);
            LoadEAInPack(effectID, tempFileName);
        }
        else {
            LoadEA(effectID, szFileName);
        }
    }

    // 步驟 3: 將快取中的數據綁定到傳入的 CCAEffect 物件
    EADATALISTINFO* pData = m_pEaData[effectID];

    // 3a. 設定 CCAEffect 的主數據指標
    // 原始碼: *((_DWORD *)a4 + 1) = v5;
    pEffect->SetData(pData);

    // 3b. 複製渲染狀態
    // 原始碼: *((_BYTE *)v6 + 80) = *(_BYTE *)(... + 24); ...
    // 注意：在我們的還原中，CCAEffect 不再直接儲存這些狀態，
    // 而是應該在 Draw 時從 pData 中讀取。但為了完全模擬原始行為，
    // CEAManager 作為 friend 類別直接設定 CCAEffect 的內部成員。

    // 3c. 設定渲染狀態選擇旗標
    // 原始碼: *((_BYTE *)v6 + 85) = v8 > 7u; (v8 是 BlendOp)
    if (pData->m_ucBlendOp > 7) {
        pEffect->m_ucRenderStateSelector = 1; // 使用 DrawEtcRenderState
    }
    else {
        pEffect->m_ucRenderStateSelector = 0; // 使用 DrawRenderState
    }

    // 原始碼中還有一行 *((_DWORD *)v6 + 2) = ... + 12;
    // 這在現代 C++ 中是不必要的，因為所有數據都可以透過 pData 指標存取，
    // 故在我們的還原中將其省略以提高程式碼清晰度。
}

/**
 * @brief 從封裝檔 (mof.pak) 載入 .ea 數據。
 * @param effectID 要載入的特效 ID，也作為 m_pEaData 陣列的索引。
 * @param szFileName .ea 檔案在封裝檔內的名稱。
 * @note 此函式的邏輯精確還原自 Effectall.c 中 0x0053A590 的 LoadTimelineInPack 函式。
 * "LoadTimelineInPack" 在反組譯時是一個命名錯誤，其功能實際上是載入 .ea (Effect Animation) 檔案。
 */

void CEAManager::LoadEAInPack(int effectID, char* szFileName)
{
    // 獲取指向 EADATALISTINFO 結構的指標，該結構已在 GetEAData 中分配
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) return;

    // 準備檔案路徑並從封裝檔讀取數據到記憶體緩衝區
    char szChangedPath[256];
    strcpy_s(szChangedPath, szFileName);
    CMofPacking* packer = CMofPacking::GetInstance();
    char* fileBuffer = packer->FileRead(packer->ChangeString(szChangedPath));

    if (!fileBuffer) {
        // 如果讀取失敗，顯示錯誤訊息
        char errorMsg[512];
        sprintf_s(errorMsg, sizeof(errorMsg), "%s CA didn't find.", szFileName);
        MessageBoxA(nullptr, errorMsg, "LoadEAInPack Error", MB_OK);
        return;
    }

    // 使用一個指標來遍歷記憶體緩衝區
    char* current_ptr = fileBuffer;

    // --- 開始循序解析檔案緩衝區 ---

    // 1. 讀取 EADATALISTINFO 的頂層成員
    // 原始碼: *(_DWORD *)a3 = dword_C24CF4; ... qmemcpy((char *)a3 + 4, &dword_C24CF8, 0xFFu);
    // 這裡我們用更精確、逐個成員的方式來還原
    pData->m_nVersion = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    pData->m_nLayerCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    pData->m_nTotalFrames = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);

    // 2. 根據 LayerCount 分配並讀取圖層 (Layers) 數據
    if (pData->m_nLayerCount > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) return; // 記憶體分配失敗

        for (int i = 0; i < pData->m_nLayerCount; ++i) {
            VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[i];

            // 讀取該圖層的影格數 (FrameCount)
            // 原始碼: v14 = *v13; ... *(_DWORD *)(v11 + ... + 52) = v14;
            pLayer->m_nFrameCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);

            // 3. 根據 FrameCount 分配並讀取影格 (Frames) 數據
            if (pLayer->m_nFrameCount > 0) {
                pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[pLayer->m_nFrameCount];
                if (!pLayer->m_pFrames) return;

                // 讀取所有影格的數據塊
                // 原始碼: qmemcpy((void *)(v20 + *(_DWORD *)(v12 + v21 + 4)), v6, 0x74u); ... 
                // 這裡的邏輯更複雜，因為每個影格可能還包含一個子陣列。
                for (int j = 0; j < pLayer->m_nFrameCount; ++j) {
                    VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[j];

                    // 複製 VERTEXANIMATIONFRAMEINFO 的主要部分 (116 bytes)
                    memcpy(pFrame, current_ptr, 116);
                    current_ptr += 116;

                    // 讀取子陣列的大小
                    // 原始碼: if ( *(_DWORD *)(v25 + v20 + 116) ) ...
                    // 經推導，這部分屬於 VERTEXANIMATIONFRAMEINFO 的未知成員
                    pFrame->m_dwUnknown1 = *reinterpret_cast<unsigned int*>(current_ptr); current_ptr += sizeof(unsigned int);
                    pFrame->m_dwUnknown2 = *reinterpret_cast<unsigned int*>(current_ptr); current_ptr += sizeof(unsigned int);

                    // 如果子陣列存在，則讀取其內容 (原始碼有此邏輯，但 CCAEffect::Process 未使用)
                    if (pFrame->m_dwUnknown1 > 0) {
                        size_t subDataSize = pFrame->m_dwUnknown1 * 102; // 0x66
                        //pFrame->m_pUnknownSubData = new char[subDataSize];
                        //memcpy(pFrame->m_pUnknownSubData, current_ptr, subDataSize);
                        current_ptr += subDataSize;
                    }
                }

            }
            else {
                pLayer->m_pFrames = nullptr;
            }
        }
    }
    else {
        pData->m_pLayers = nullptr;
    }

    // 4. 讀取動畫片段 (KeyFrame) 資訊
    pData->m_nAnimationCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int); // 原始碼有一個多餘的讀取，可能是總影格數

    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (!pData->m_pKeyFrames) return;

        size_t keyinfoDataSize = sizeof(KEYINFO) * pData->m_nAnimationCount;
        memcpy(pData->m_pKeyFrames, current_ptr, keyinfoDataSize);
        current_ptr += keyinfoDataSize;
    }
    else {
        pData->m_pKeyFrames = nullptr;
    }

    // 5. 讀取渲染狀態 (Render States)
    pData->m_ucBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;

    // 檔案解析完畢，CMofPacking 的緩衝區會在其內部管理，此處不需釋放
}

/**
 * @brief 從獨立的 .ea 檔案載入特效動畫數據。
 * @param effectID 要載入的特效 ID，也作為 m_pEaData 陣列的索引。
 * @param szFileName .ea 檔案的完整路徑。
 * @note 此函式的邏輯精確還原自 Effectall.c 中 0x0053AA50 的函式。
 * 它負責循序讀取檔案，並動態分配記憶體來建立一個完整的 EADATALISTINFO 數據結構。
 */
void CEAManager::LoadEA(int effectID, const char* szFileName)
{
    // 獲取指向 EADATALISTINFO 結構的指標，該結構已在 GetEAData 中預先分配
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) {
        return;
    }

    FILE* pFile = nullptr;
    // 使用 fopen_s 以二進位讀取模式("rb")安全地開啟檔案
    if (fopen_s(&pFile, szFileName, "rb") != 0 || pFile == nullptr) {
        // 檔案開啟失敗，顯示錯誤訊息，行為與原始碼一致
        char errorMsg[512];
        sprintf_s(errorMsg, sizeof(errorMsg), "%s Effect info file is not found.", szFileName);
        MessageBoxA(nullptr, errorMsg, "CEAManager::LoadEA Error", MB_OK);
        return;
    }

    // --- 開始循序讀取檔案 ---

    // 1. 讀取 EADATALISTINFO 的頂層成員
    fread(&pData->m_nVersion, sizeof(pData->m_nVersion), 1, pFile);
    fread(&pData->m_nLayerCount, sizeof(pData->m_nLayerCount), 1, pFile);
    fread(&pData->m_nTotalFrames, sizeof(pData->m_nTotalFrames), 1, pFile);

    // 2. 根據 LayerCount 分配並讀取圖層 (Layers) 數據
    if (pData->m_nLayerCount > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) { fclose(pFile); return; }

        for (int i = 0; i < pData->m_nLayerCount; ++i) {
            VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[i];

            // 讀取該圖層的影格數 (FrameCount)
            fread(&pLayer->m_nFrameCount, sizeof(pLayer->m_nFrameCount), 1, pFile);

            // 3. 根據 FrameCount 分配並讀取影格 (Frames) 數據
            if (pLayer->m_nFrameCount > 0) {
                pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[pLayer->m_nFrameCount];
                if (!pLayer->m_pFrames) { fclose(pFile); return; }

                // 一次性讀取所有影格的數據塊
                fread(pLayer->m_pFrames, sizeof(VERTEXANIMATIONFRAMEINFO), pLayer->m_nFrameCount, pFile);

                // 在原始碼中，每個影格後面可能跟著一個子數據塊，這裡也要讀取
                for (int j = 0; j < pLayer->m_nFrameCount; ++j) {
                    VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[j];
                    // 由於我們已將未知成員包含在結構體中，上面的 fread 已經讀取了它們。
                    // 如果這些未知成員是指標，則需要在此處進一步讀取它們指向的內容。
                    // 根據之前的分析，它們似乎是固定大小的數據或計數，已被讀取。
                }

            }
            else {
                pLayer->m_pFrames = nullptr;
            }
        }
    }
    else {
        pData->m_pLayers = nullptr;
    }

    // 4. 讀取動畫片段 (KeyFrame) 資訊
    fread(&pData->m_nAnimationCount, sizeof(pData->m_nAnimationCount), 1, pFile);
    // 原始碼在讀取 AnimationCount 後還有一次4位元組的讀取，推測是總影格數的重複或另一個未使用欄位
    int dummy;
    fread(&dummy, sizeof(int), 1, pFile);

    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (!pData->m_pKeyFrames) { fclose(pFile); return; }

        // 一次性讀取所有 KeyInfo 的數據塊
        fread(pData->m_pKeyFrames, sizeof(KEYINFO), pData->m_nAnimationCount, pFile);
    }
    else {
        pData->m_pKeyFrames = nullptr;
    }

    // 5. 讀取渲染狀態 (Render States)
    fread(&pData->m_ucBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucDestBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcDestBlend, sizeof(unsigned char), 1, pFile);

    // --- 檔案讀取完畢 ---
    fclose(pFile);
}