#include "Effect/CEAManager.h"
#include "Effect/CCAEffect.h"
#include "CMOFPacking.h"
#include <new>

extern int IsInMemory;
CEAManager* CEAManager::s_pInstance = nullptr;

// --- GetInstance, Constructor, Destructor, Reset 維持不變 ---
CEAManager* CEAManager::GetInstance() {
    if (!s_pInstance) s_pInstance = new (std::nothrow) CEAManager();
    return s_pInstance;
}
CEAManager::CEAManager() {
    memset(m_pEaData, 0, sizeof(m_pEaData));
}
CEAManager::~CEAManager() {
    Reset();
}
void CEAManager::Reset() {
    for (int i = 0; i < 65535; ++i) {
        if (m_pEaData[i]) {
            delete m_pEaData[i]; // 假設 EADATALISTINFO 的解構函式會處理內部指標
            m_pEaData[i] = nullptr;
        }
    }
}


// --- GetEAData 維持不變，其懶漢式載入邏輯依然適用 ---
void CEAManager::GetEAData(int effectID, const char* szFileName, CCAEffect* pEffect)
{
    if (effectID < 0 || effectID >= 65535 || !pEffect) return;

    if (m_pEaData[effectID] == nullptr) {
        m_pEaData[effectID] = new (std::nothrow) EADATALISTINFO();
        if (!m_pEaData[effectID]) return;

        // 根據旗標決定從獨立檔案或封裝檔載入
        if (IsInMemory) {
            char tempFileName[256];
            strcpy_s(tempFileName, sizeof(tempFileName), szFileName);
            LoadEAInPack(effectID, tempFileName);
        }
        else {
            LoadEA(effectID, szFileName);
        }
    }

    // 將快取資料綁定到 CCAEffect 物件
    EADATALISTINFO* pData = m_pEaData[effectID];
    pEffect->SetData(pData);

    if (pData) {
        if (pData->m_ucBlendOp > 7) {
            pEffect->m_ucRenderStateSelector = 1;
        }
        else {
            pEffect->m_ucRenderStateSelector = 0;
        }
    }
}


/**
 * @brief 從獨立的 .ea 檔案載入特效動畫數據。
 *
 * MODIFICATION: 此函式的實作已被完全重寫，以符合 `ea_parser.py` 所揭示的
 * 簡單檔案格式：Header(12 bytes) -> Frame Array -> Trailer(6+ bytes)。
 * 它將檔案內容解析為一個單一圖層的動畫。
 */
void CEAManager::LoadEA(int effectID, const char* szFileName)
{
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) return;

    FILE* pFile = nullptr;
    if (fopen_s(&pFile, szFileName, "rb") != 0 || pFile == nullptr) {
        // 錯誤處理...
        return;
    }

    // 1. 讀取 Header (num_keys, num_layers, num_frames)
    // Python: num_keys, num_layers, num_frames = struct.unpack_from('<III', data, 0)
    int num_keys, num_layers, num_frames;
    fread(&num_keys, sizeof(int), 1, pFile);
    fread(&num_layers, sizeof(int), 1, pFile);
    fread(&num_frames, sizeof(int), 1, pFile);

    // 2. 填充 EADATALISTINFO 結構
    // 我們將此格式視為具有一個圖層和一個預設動畫片段
    pData->m_nTotalFrames = num_frames;
    pData->m_nLayerCount = 1; // 簡化為單一圖層
    pData->m_pLayers = nullptr;
    pData->m_nAnimationCount = 1; // 創建一個涵蓋所有影格的預設動畫片段
    pData->m_pKeyFrames = nullptr;


    // 3. 分配並設定圖層
    if (pData->m_nLayerCount > 0 && num_frames > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) { fclose(pFile); return; }

        VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[0];
        pLayer->m_nFrameCount = num_frames;
        pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[num_frames];
        if (!pLayer->m_pFrames) {
            delete[] pData->m_pLayers;
            pData->m_pLayers = nullptr;
            fclose(pFile);
            return;
        }

        // 4. 一次性讀取所有影格資料
        // Python: ... loop ... struct.unpack_from ...
        // C++: A single fread is more efficient here.
        // Requires VERTEXANIMATIONFRAMEINFO to be exactly 120 bytes.
        fread(pLayer->m_pFrames, sizeof(VERTEXANIMATIONFRAMEINFO), num_frames, pFile);
    }

    // 5. 建立預設的 KeyFrame
    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (pData->m_pKeyFrames) {
            strcpy_s(pData->m_pKeyFrames[0].m_szName, sizeof(pData->m_pKeyFrames[0].m_szName), "default");
            pData->m_pKeyFrames[0].m_nStartFrame = 0;
            pData->m_pKeyFrames[0].m_nEndFrame = num_frames > 0 ? num_frames - 1 : 0;
        }
    }

    // 6. 讀取檔案尾部的渲染狀態 (Trailer)
    // Python: trailer = data[offset:]
    fread(&pData->m_ucBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucDestBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcDestBlend, sizeof(unsigned char), 1, pFile);

    fclose(pFile);
}

/**
 * @brief 從封裝檔 (mof.pak) 的記憶體緩衝區載入 .ea 數據。
 *
 * MODIFICATION: 此函式也被重寫，以匹配新的解析邏輯。
 * 它從記憶體指標而不是檔案指標讀取資料。
 */
void CEAManager::LoadEAInPack(int effectID, char* szFileName)
{
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) return;

    // 1. 從封裝檔管理器獲取檔案的記憶體緩衝區
    CMofPacking* packer = CMofPacking::GetInstance();
    char* fileBuffer = packer->FileRead(packer->ChangeString(szFileName));
    if (!fileBuffer) {
        // 錯誤處理...
        return;
    }
    char* current_ptr = fileBuffer;

    // 2. 解析 Header
    int num_keys = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    int num_layers = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    int num_frames = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);

    // 3. 填充 EADATALISTINFO (邏輯同 LoadEA)
    pData->m_nTotalFrames = num_frames;
    pData->m_nLayerCount = 1;
    pData->m_pLayers = nullptr;
    pData->m_nAnimationCount = 1;
    pData->m_pKeyFrames = nullptr;

    // 4. 分配、設定並複製圖層與影格資料
    if (pData->m_nLayerCount > 0 && num_frames > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) return;

        VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[0];
        pLayer->m_nFrameCount = num_frames;
        pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[num_frames];
        if (!pLayer->m_pFrames) {
            delete[] pData->m_pLayers;
            pData->m_pLayers = nullptr;
            return;
        }
        size_t frameDataSize = sizeof(VERTEXANIMATIONFRAMEINFO) * num_frames;
        memcpy(pLayer->m_pFrames, current_ptr, frameDataSize);
        current_ptr += frameDataSize;
    }

    // 5. 建立預設 KeyFrame (邏輯同 LoadEA)
    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (pData->m_pKeyFrames) {
            strcpy_s(pData->m_pKeyFrames[0].m_szName, sizeof(pData->m_pKeyFrames[0].m_szName), "default");
            pData->m_pKeyFrames[0].m_nStartFrame = 0;
            pData->m_pKeyFrames[0].m_nEndFrame = num_frames > 0 ? num_frames - 1 : 0;
        }
    }

    // 6. 解析 Trailer 的渲染狀態
    pData->m_ucBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;

    // 記憶體緩衝區由 CMofPacking 管理，此處不釋放
}