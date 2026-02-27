#include "Effect/CEAManager.h"
#include "Effect/CCAEffect.h"
#include "FileSystem/CMOFPacking.h"
#include <new>

extern int IsInMemory;
CEAManager* CEAManager::s_pInstance = nullptr;

// --- GetInstance, Constructor, Destructor, Reset 維持不變 ---
CEAManager* CEAManager::GetInstance() {
    if (!s_pInstance) s_pInstance = new (std::nothrow) CEAManager();
    return s_pInstance;
}

CEAManager::CEAManager()
{
    size_t totalSize = sizeof(EADATALISTINFO*) * 0xFFFF;
    m_pEAData = (EADATALISTINFO**)std::malloc(totalSize);
    if (m_pEAData) {
        std::memset(m_pEAData, 0, totalSize);
    }
    m_dwTailFlag = 0;
}

CEAManager::~CEAManager()
{
    if (m_pEAData)
    {
        for (int i = 0; i < 0xFFFF; ++i)
        {
            EADATALISTINFO* p = m_pEAData[i];
            if (p)
            {
                p->~EADATALISTINFO();
                ::operator delete(p);
                m_pEAData[i] = nullptr;
            }
        }
        std::free(m_pEAData);
        m_pEAData = nullptr;
    }
}

void CEAManager::Reset()
{
    if (!m_pEAData) return;

    for (int i = 0; i < 0xFFFF; ++i)
    {
        EADATALISTINFO* p = m_pEAData[i];
        if (p)
        {
            p->~EADATALISTINFO();
            ::operator delete(p);
            m_pEAData[i] = nullptr;
        }
    }
    m_dwTailFlag = 0;
}

static inline uint32_t read_u32(const uint8_t*& p)
{
    uint32_t v;
    std::memcpy(&v, p, sizeof(v));
    p += 4;
    return v;
}

static inline void read_bytes(const uint8_t*& p, void* dst, size_t n)
{
    std::memcpy(dst, p, n);
    p += n;
}

void CEAManager::GetEAData(int effectId, const char* fileName, CCAEffect* outEffect)
{
    // IDA signature: (this, a2=effectId, a3=fileName, a4=effect)
    EADATALISTINFO* p = m_pEAData[effectId];

    if (p)
    {
        // effect->(dword+1) = p
        outEffect->SetEffectData(p);
    }
    else
    {
        void* mem = ::operator new(48u);
        EADATALISTINFO* np = nullptr;
        if (mem)
        {
            np = new (mem) EADATALISTINFO();
            // 依 CCAEffect.h 的 EADATALISTINFO layout 初始化
            np->animationCount = 0;
            np->keys = nullptr;
            np->totalFrames = 0;
            np->unknown = 0;
            np->layerCount = 0;
            np->layers = nullptr;
            np->m_ucBlendOp = 0;
            np->m_ucSrcBlend = 0;
            np->m_ucEtcBlendOp = 0;
            np->m_ucEtcSrcBlend = 0;
            np->m_ucEtcDestBlend = 0;
        }

        m_pEAData[effectId] = np;

        if (IsInMemory) {
            char tempFileName[256];
            strcpy_s(tempFileName, sizeof(tempFileName), fileName);
            LoadEAInPack(effectId, tempFileName);
        }
        else
            LoadEA(effectId, fileName);

        outEffect->SetEffectData(m_pEAData[effectId]);
        p = m_pEAData[effectId];
    }

    auto* ea = m_pEAData[effectId];              // EADATALISTINFO*
    outEffect->SetLayerList(reinterpret_cast<VERTEXANIMATIONLAYERLISTINFO*>(&ea->unknown));

    outEffect->m_ucBlendIndex = p->m_ucBlendOp;
    const uint8_t blendOp = p->m_ucBlendOp;

    outEffect->m_ucUnk81 = p->m_ucSrcBlend;
    outEffect->m_ucEtcBlendOp = p->m_ucEtcBlendOp;
    outEffect->m_ucEtcSrcBlend = p->m_ucEtcSrcBlend;
    outEffect->m_ucEtcDestBlend = p->m_ucEtcDestBlend;
    outEffect->m_ucRenderMode = static_cast<uint8_t>(blendOp > 7u);
}

void CEAManager::LoadEA(int effectID, const char* szFileName)
{
    EADATALISTINFO* pData = m_pEAData[effectID];
    if (!pData) return;

    // 若同一個 effectID 重複載入，先清掉舊資料避免 leak
    if (pData->layers) {
        for (uint32_t i = 0; i < pData->layerCount; ++i) {
            delete[] pData->layers[i].m_pFrames;
            pData->layers[i].m_pFrames = nullptr;
            pData->layers[i].m_nFrameCount = 0;
        }
        delete[] pData->layers;
        pData->layers = nullptr;
    }
    delete[] pData->keys;
    pData->keys = nullptr;

    pData->layerCount = 0;
    pData->animationCount = 0;
    pData->totalFrames = 0;
    pData->m_ucBlendOp = 0;
    pData->m_ucSrcBlend = 0;
    pData->m_ucEtcBlendOp = 0;
    pData->m_ucEtcSrcBlend = 0;
    pData->m_ucEtcDestBlend = 0;

    FILE* pFile = nullptr;
    if (fopen_s(&pFile, szFileName, "rb") != 0 || pFile == nullptr) {
        return;
    }

    uint32_t unknown_tag = 0;
    uint32_t layerCount = 0;
    if (fread(&unknown_tag, sizeof(uint32_t), 1, pFile) != 1 ||
        fread(&layerCount, sizeof(uint32_t), 1, pFile) != 1) {
        fclose(pFile);
        return;
    }

    pData->layerCount = layerCount;
    pData->layers = (layerCount > 0) ? new (std::nothrow) VERTEXANIMATIONLAYERINFO[layerCount] : nullptr;
    if (layerCount > 0 && !pData->layers) { fclose(pFile); return; }

    // Layers + Frames
    for (uint32_t li = 0; li < layerCount; ++li) {
        uint32_t frameCount = 0;
        if (fread(&frameCount, sizeof(uint32_t), 1, pFile) != 1) { fclose(pFile); return; }

        VERTEXANIMATIONLAYERINFO& layer = pData->layers[li];
        layer.m_nFrameCount = static_cast<int>(frameCount);
        layer.m_pFrames = (frameCount > 0) ? new (std::nothrow) VERTEXANIMATIONFRAMEINFO[frameCount] : nullptr;
        if (frameCount > 0 && !layer.m_pFrames) { fclose(pFile); return; }

        for (uint32_t fi = 0; fi < frameCount; ++fi) {
            VERTEXANIMATIONFRAMEINFO& fr = layer.m_pFrames[fi];

            // 固定 0x74 bytes：imageID + 4 vertices
            if (fread(&fr.m_dwImageID, sizeof(uint32_t), 1, pFile) != 1) { fclose(pFile); return; }
            if (fread(&fr.m_Vertices[0], sizeof(GIVertex), 4, pFile) != 4) { fclose(pFile); return; }

            // extraCount（在檔案中緊接於 0x74 bytes 後面）
            uint32_t extraCount = 0;
            if (fread(&extraCount, sizeof(uint32_t), 1, pFile) != 1) { fclose(pFile); return; }

            // 注意：目前的 VERTEXANIMATIONFRAMEINFO（CCAEffect.h）沒有 extraCount 欄位，
            // 先把 count 暫存在 m_ptrExtra，並把真正的 extra records 直接略過。
            fr.m_ptrExtra = extraCount;

            if (extraCount) {
                const size_t skip = static_cast<size_t>(extraCount) * 0x66u;
                if (fseek(pFile, static_cast<long>(skip), SEEK_CUR) != 0) { fclose(pFile); return; }
            }
        }
    }

    // totalFrames / animationCount
    uint32_t totalFrames = 0;
    uint32_t animationCount = 0;
    if (fread(&totalFrames, sizeof(uint32_t), 1, pFile) != 1 ||
        fread(&animationCount, sizeof(uint32_t), 1, pFile) != 1) {
        fclose(pFile);
        return;
    }
    pData->totalFrames = totalFrames;
    pData->animationCount = animationCount;

    // keys（每個 24 bytes）
    if (animationCount) {
        pData->keys = new (std::nothrow) KEYINFO[animationCount];
        if (!pData->keys) { fclose(pFile); return; }

        for (uint32_t i = 0; i < animationCount; ++i) {
            uint8_t raw[24];
            if (fread(raw, 1, sizeof(raw), pFile) != sizeof(raw)) { fclose(pFile); return; }

            std::memcpy(pData->keys[i].m_szName, raw, 16);
            pData->keys[i].m_szName[15] = '\0';
            std::memcpy(&pData->keys[i].m_nStartFrame, raw + 16, 4);
            std::memcpy(&pData->keys[i].m_nEndFrame, raw + 20, 4);
        }
    }

    // blend params
    uint8_t blendOp = 0, srcBlend = 0;
    if (fread(&blendOp, 1, 1, pFile) != 1 || fread(&srcBlend, 1, 1, pFile) != 1) {
        fclose(pFile);
        return;
    }
    pData->m_ucBlendOp = blendOp;
    pData->m_ucSrcBlend = srcBlend;

    if (blendOp == 8u) {
        (void)fread(&pData->m_ucEtcBlendOp, 1, 1, pFile);
        (void)fread(&pData->m_ucEtcSrcBlend, 1, 1, pFile);
        (void)fread(&pData->m_ucEtcDestBlend, 1, 1, pFile);
    }

    fclose(pFile);
}

void CEAManager::LoadEAInPack(int effectID, char* szFileName)
{
    EADATALISTINFO* pData = m_pEAData[effectID];
    if (!pData) return;

    // 若同一個 effectID 重複載入，先清掉舊資料避免 leak
    if (pData->layers) {
        for (uint32_t i = 0; i < pData->layerCount; ++i) {
            delete[] pData->layers[i].m_pFrames;
            pData->layers[i].m_pFrames = nullptr;
            pData->layers[i].m_nFrameCount = 0;
        }
        delete[] pData->layers;
        pData->layers = nullptr;
    }
    delete[] pData->keys;
    pData->keys = nullptr;

    pData->layerCount = 0;
    pData->animationCount = 0;
    pData->totalFrames = 0;
    pData->m_ucBlendOp = 0;
    pData->m_ucSrcBlend = 0;
    pData->m_ucEtcBlendOp = 0;
    pData->m_ucEtcSrcBlend = 0;
    pData->m_ucEtcDestBlend = 0;

    // 1. 從封裝檔管理器獲取檔案的記憶體緩衝區
    CMofPacking* packer = CMofPacking::GetInstance();
    char* changed = packer->ChangeString(szFileName);

    const char prefix[] = "mofdata/";
    if (changed && std::strncmp(changed, prefix, sizeof(prefix) - 1) == 0) {
        std::memmove(changed, changed + (sizeof(prefix) - 1),
            std::strlen(changed + (sizeof(prefix) - 1)) + 1);
    }

    char* fileBuffer = packer->FileRead(changed);
    if (!fileBuffer) {
        return;
    }

    const uint8_t* p = reinterpret_cast<const uint8_t*>(fileBuffer);

    // 2. Header：unknown_tag + layerCount
    (void)read_u32(p);
    const uint32_t layerCount = read_u32(p);
    pData->layerCount = layerCount;

    pData->layers = (layerCount > 0) ? new (std::nothrow) VERTEXANIMATIONLAYERINFO[layerCount] : nullptr;
    if (layerCount > 0 && !pData->layers) return;

    // 3. Layers + Frames
    for (uint32_t li = 0; li < layerCount; ++li) {
        const uint32_t frameCount = read_u32(p);

        VERTEXANIMATIONLAYERINFO& layer = pData->layers[li];
        layer.m_nFrameCount = static_cast<int>(frameCount);
        layer.m_pFrames = (frameCount > 0) ? new (std::nothrow) VERTEXANIMATIONFRAMEINFO[frameCount] : nullptr;
        if (frameCount > 0 && !layer.m_pFrames) return;

        for (uint32_t fi = 0; fi < frameCount; ++fi) {
            VERTEXANIMATIONFRAMEINFO& fr = layer.m_pFrames[fi];
            fr.m_dwImageID = read_u32(p);
            read_bytes(p, &fr.m_Vertices[0], sizeof(GIVertex) * 4);

            const uint32_t extraCount = read_u32(p);
            fr.m_ptrExtra = extraCount; // 暫存 count
            if (extraCount) {
                p += static_cast<size_t>(extraCount) * 0x66u;
            }
        }
    }

    // 4. totalFrames / animationCount
    pData->totalFrames = read_u32(p);
    pData->animationCount = read_u32(p);

    // 5. keys（每個 28 bytes）
    if (pData->animationCount) {
        pData->keys = new (std::nothrow) KEYINFO[pData->animationCount];
        if (!pData->keys) return;

        for (uint32_t i = 0; i < pData->animationCount; ++i) {
            uint8_t raw[28];
            read_bytes(p, raw, sizeof(raw));

            std::memcpy(pData->keys[i].m_szName, raw, 20);
            pData->keys[i].m_szName[15] = '\0';
            std::memcpy(&pData->keys[i].m_nStartFrame, raw + 20, 4);
            std::memcpy(&pData->keys[i].m_nEndFrame, raw + 24, 4);
        }
    }

    // 6. blend params
    pData->m_ucBlendOp = *p++;
    pData->m_ucSrcBlend = *p++;
    if (pData->m_ucBlendOp == 8u) {
        pData->m_ucEtcBlendOp = *p++;
        pData->m_ucEtcSrcBlend = *p++;
        pData->m_ucEtcDestBlend = *p++;
    }

    // 記憶體緩衝區由 CMofPacking 管理，此處不釋放
}