#include "Character/CCANormal.h"
#include "Character/CAManager.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "FileSystem/CMOFPacking.h"
#include "global.h"

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <new>

// ============================================================================
// CCANormal — restored from mofclient.c (0x0052C240 .. 0x0052D2A0)
// ============================================================================

// -----------------------------------------------------------------------------
// The original Process() keeps a function-static layer index that it uses as
// both the loop counter AND the destination slot for its freshly-fetched
// GameImage pointer.  The static is zeroed at the start of every call, so the
// only thing that matters is that it walks 0..m_nLayerCount-1.  We mirror the
// symbol to preserve fidelity with the decompilation.
// -----------------------------------------------------------------------------
static int s_LayerIndex_Process_CCANormal = 0;

// ----------------------------------------------------------------------------
// Source cursors used by LoadCA / LoadCAInPack.
// ----------------------------------------------------------------------------
namespace
{
    struct FileSrc
    {
        FILE* fp;
        void Read(void* dst, size_t n) { std::fread(dst, n, 1, fp); }
    };

    struct MemSrc
    {
        const uint8_t* base;
        size_t         pos;
        void Read(void* dst, size_t n)
        {
            std::memcpy(dst, base + pos, n);
            pos += n;
        }
    };

    // Allocate and zero-initialize a CA_DRAWENTRY[count] using the same
    // defaults the original binary sets inline (alpha=255, scale=100).
    CA_DRAWENTRY* AllocDrawEntries(int count)
    {
        if (count <= 0) return nullptr;
        CA_DRAWENTRY* p = new CA_DRAWENTRY[count];
        for (int i = 0; i < count; ++i)
        {
            p[i].m_dwImageID = 0;
            p[i].m_wBlockID  = 0;
            p[i].m_wPad6     = 0;
            p[i].m_fOffsetX  = 0.0f;
            p[i].m_fOffsetY  = 0.0f;
            p[i].m_nAlpha    = 255;
            p[i].m_nRotation = 0;
            p[i].m_nScale    = 100;
            p[i].m_ucFlip    = 0;
            p[i].m_ucFlag29  = 0;
            p[i].m_pad30[0]  = 0;
            p[i].m_pad30[1]  = 0;
        }
        return p;
    }

    template <typename Src>
    void ReadCADataImpl(CCANormal* self, Src& src)
    {
        // ---- header block (version + 255-byte header + 4 unknown dwords +
        //      layer count). Order matches mofclient.c LoadCA exactly.
        src.Read(&self->m_nVersion, 4);
        src.Read(self->m_header, 255);
        src.Read(&self->m_nUnk276, 4);
        src.Read(&self->m_nUnk288, 4);
        src.Read(&self->m_nUnk292, 4);
        src.Read(&self->m_nUnk296, 4);
        src.Read(&self->m_nLayerCount, 4);

        // ---- image pointer array (one entry per layer, zero-initialized).
        //      The original allocates `new int[layerCount]` then zeros it.
        self->m_nImageCount = self->m_nLayerCount;
        if (self->m_nLayerCount > 0)
        {
            self->m_ppImages = new GameImage*[self->m_nLayerCount];
            for (int i = 0; i < self->m_nLayerCount; ++i)
                self->m_ppImages[i] = nullptr;
        }
        else
        {
            self->m_ppImages = nullptr;
        }

        // ---- layer table.
        if (self->m_nLayerCount > 0)
        {
            self->m_pLayers = new LAYERINFO[self->m_nLayerCount];
        }

        for (int li = 0; li < self->m_nLayerCount; ++li)
        {
            LAYERINFO& layer = self->m_pLayers[li];
            src.Read(layer.m_header, 50);
            src.Read(&layer.m_nFrameCount, 4);

            const int fc = layer.m_nFrameCount;
            layer.m_pFrames = (fc > 0) ? new FRAMEINFO[fc]() : nullptr;

            for (int fi = 0; fi < fc; ++fi)
            {
                FRAMEINFO& frame = layer.m_pFrames[fi];

                // Primary draw entries.
                src.Read(&frame.m_nCount1, 4);
                if (frame.m_nCount1 > 0)
                {
                    CA_DRAWENTRY* entries = AllocDrawEntries(frame.m_nCount1);
                    frame.m_pEntries1 = entries;
                    for (int j = 0; j < frame.m_nCount1; ++j)
                        src.Read(entries + j, 32);
                }

                // Secondary 102-byte entries (effect/shader metadata).
                src.Read(&frame.m_nCount2, 4);
                if (frame.m_nCount2 > 0)
                {
                    const size_t nBytes = static_cast<size_t>(frame.m_nCount2) * 102;
                    void* pSub = ::operator new(nBytes);
                    std::memset(pSub, 0, nBytes);
                    frame.m_pEntries2 = pSub;
                    for (int j = 0; j < frame.m_nCount2; ++j)
                        src.Read(static_cast<char*>(pSub) + 102 * j, 102);
                }

                src.Read(&frame.m_nEnd, 4);
            }
        }

        // ---- total frames + key animation table.
        src.Read(&self->m_nTotalFrames, 4);
        src.Read(&self->m_nKeyCount, 4);
        if (self->m_nKeyCount > 0)
        {
            self->m_pKeys = new CA_KEYINFO[self->m_nKeyCount];
            for (int k = 0; k < self->m_nKeyCount; ++k)
                src.Read(self->m_pKeys + k, 28);
        }

        // ---- optional shader blend params (1 or 5 trailing bytes).
        src.Read(&self->m_ucShaderParam1, 1);
        src.Read(&self->m_ucShaderParam2, 1);
        if (self->m_ucShaderParam1 == 8)
        {
            src.Read(&self->m_ucShaderParam3, 1);
            src.Read(&self->m_ucShaderParam4, 1);
            src.Read(&self->m_ucShaderParam5, 1);
        }
    }

    // Destroy a single LAYERINFO's owned frame table.
    void DestroyLayer(LAYERINFO& layer)
    {
        if (!layer.m_pFrames) return;
        for (int i = 0; i < layer.m_nFrameCount; ++i)
        {
            FRAMEINFO& f = layer.m_pFrames[i];
            if (f.m_pEntries1) { delete[] static_cast<CA_DRAWENTRY*>(f.m_pEntries1); f.m_pEntries1 = nullptr; }
            if (f.m_pEntries2) { ::operator delete(f.m_pEntries2); f.m_pEntries2 = nullptr; }
        }
        delete[] layer.m_pFrames;
        layer.m_pFrames = nullptr;
        layer.m_nFrameCount = 0;
    }
}

// =============================================================================
// Constructor / Destructor (0x0052C240 / 0x0052C290)
// =============================================================================
CCANormal::CCANormal()
    : m_nKeyCount(0)
    , m_pKeys(nullptr)
    , m_nTotalFrames(0)
    , m_nVersion(12)
    , m_nUnk276(0)
    , m_nLayerCount(0)
    , m_pLayers(nullptr)
    , m_nUnk288(0)
    , m_nUnk292(0)
    , m_nUnk296(0)
    , m_fPosX(0.0f)
    , m_fPosY(0.0f)
    , m_ucAlpha(0)
    , m_ppImages(nullptr)
    , m_nImageCount(0)
    , m_nUnk320(0)
    , m_bLoop(0)
    , m_bPlaying(0)
    , m_bVisible(1)
    , m_pad327(0)
    , m_nMotionIndex(0)
    , m_nUnk332(0)
    , m_nStartFrame(0)
    , m_nEndFrame(0)
    , m_bResetShader(1)
    , m_ucShaderParam1(0)
    , m_ucShaderParam2(0)
    , m_ucShaderParam3(0)
    , m_ucShaderParam4(0)
    , m_ucShaderParam5(0)
    , m_pad350(0)
{
    std::memset(m_header,  0, sizeof(m_header));
    std::memset(m_pad309,  0, sizeof(m_pad309));
}

CCANormal::~CCANormal()
{
    if (m_ppImages)
    {
        ::operator delete(m_ppImages);
        m_ppImages = nullptr;
    }
    if (m_pLayers)
    {
        for (int i = 0; i < m_nLayerCount; ++i)
            DestroyLayer(m_pLayers[i]);
        delete[] m_pLayers;
        m_pLayers = nullptr;
    }
    m_nKeyCount = 0;
    if (m_pKeys)
    {
        delete[] m_pKeys;
        m_pKeys = nullptr;
    }
}

// =============================================================================
// Reset (0x0052C390)
// =============================================================================
void CCANormal::Reset()
{
    // Drop the key table.
    m_nKeyCount = 0;
    if (m_pKeys) { delete[] m_pKeys; m_pKeys = nullptr; }

    // Drop the layer table.
    m_nVersion  = 12;
    m_nUnk276   = 0;
    const int oldLayerCount = m_nLayerCount;
    m_nLayerCount = 0;
    if (m_pLayers)
    {
        for (int i = 0; i < oldLayerCount; ++i)
            DestroyLayer(m_pLayers[i]);
        delete[] m_pLayers;
        m_pLayers = nullptr;
    }

    m_nUnk288 = 0;
    m_nUnk292 = 0;
    m_nUnk296 = 0;
    m_fPosX   = 0.0f;
    m_fPosY   = 0.0f;
    m_ucAlpha = 0;
    m_bLoop    = 0;
    m_bPlaying = 0;
    m_bVisible = 1;

    m_nMotionIndex = 0;
    m_nUnk332      = 0;
    m_nStartFrame  = 0;
    m_nEndFrame    = 0;

    m_bResetShader   = 1;
    m_ucShaderParam1 = 0;
    m_ucShaderParam2 = 0;
    m_ucShaderParam3 = 0;
    m_ucShaderParam4 = 0;
    m_ucShaderParam5 = 0;
    m_pad350         = 0;

    // Original just nulls the pointer (does not delete).
    m_ppImages    = nullptr;
    m_nImageCount = 0;
    m_nUnk320     = 0;
}

// =============================================================================
// LoadCAInPack (0x0052C4A0)
// =============================================================================
void CCANormal::LoadCAInPack(char* filename)
{
    char localPath[256];
    std::strncpy(localPath, filename, sizeof(localPath) - 1);
    localPath[sizeof(localPath) - 1] = 0;

    CMofPacking* pack = CMofPacking::GetInstance();
    if (!pack)
    {
        char msg[256];
        std::snprintf(msg, sizeof(msg), "%s didn't find Character Animation File.", filename);
        ::MessageBoxA(nullptr, msg, nullptr, 0);
        return;
    }

    char* changed = pack->ChangeString(localPath);
    pack->FileReadBackGroundLoading(changed);

    const uint8_t* buf = reinterpret_cast<const uint8_t*>(pack->m_backgroundLoadBufferField);
    if (!buf || pack->m_nReadBytes <= 0)
    {
        char msg[256];
        std::snprintf(msg, sizeof(msg), "%s didn't find Character Animation File.", filename);
        ::MessageBoxA(nullptr, msg, nullptr, 0);
        return;
    }

    MemSrc cursor{ buf, 0 };
    ReadCADataImpl(this, cursor);
}

// =============================================================================
// LoadCA (0x0052C9E0)
// =============================================================================
void CCANormal::LoadCA(const char* filename)
{
    FILE* fp = std::fopen(filename, "rb");
    if (!fp)
    {
        char msg[256];
        std::snprintf(msg, sizeof(msg), "%s didn't find Character Animation File.", filename);
        ::MessageBoxA(nullptr, msg, nullptr, 0);
        return;
    }
    FileSrc cursor{ fp };
    ReadCADataImpl(this, cursor);
    std::fclose(fp);
}

// =============================================================================
// Process (0x0052CFB0)
// =============================================================================
bool CCANormal::Process(uint16_t a2)
{
    const uint16_t frame = static_cast<uint16_t>(a2 + static_cast<uint16_t>(m_nStartFrame));
    if (!m_bPlaying) return true;

    s_LayerIndex_Process_CCANormal = 0;
    if (m_nLayerCount <= 0) return true;

    cltImageManager* pIM = cltImageManager::GetInstance();

    for (int li = 0; li < m_nLayerCount; ++li)
    {
        s_LayerIndex_Process_CCANormal = li;

        LAYERINFO* pLayer = &m_pLayers[li];
        if (frame >= static_cast<uint16_t>(pLayer->m_nFrameCount)) continue;
        FRAMEINFO* pFrameArr = pLayer->m_pFrames;
        if (!pFrameArr) continue;
        FRAMEINFO* pFrame = &pFrameArr[frame];
        if (pFrame->m_nCount1 <= 0) continue;

        CA_DRAWENTRY* pEntry = static_cast<CA_DRAWENTRY*>(pFrame->m_pEntries1);
        if (!pEntry) continue;

        uint32_t imageID = pEntry[0].m_dwImageID;
        if (!imageID) continue;

        GameImage* pGI = pIM ? pIM->GetGameImage(1, imageID, 0, 1) : nullptr;
        m_ppImages[li] = pGI;
        if (!pGI) continue;

        pGI->m_bFlag_446 = true;
        pGI->m_wBlockID  = pEntry[0].m_wBlockID;

        pGI->m_bFlag_447 = true;
        pGI->m_fPosX     = pEntry[0].m_fOffsetX + m_fPosX;
        pGI->m_fPosY     = pEntry[0].m_fOffsetY + m_fPosY;

        // Per-entry alpha override sourced from CCANormal::m_ucAlpha.  The
        // original writes integer pos/alpha into three adjacent GameImage
        // fields at +404/+408/+412; those fields serve a different purpose
        // in our port, so we route the alpha through the dedicated channel.
        pGI->m_dwAlpha = m_ucAlpha;

        // Shader/overlay reset — the original writes default hot-spot /
        // angle floats and toggles the "draw-part-2" flag.  Our GameImage
        // owns its own shader-state pipeline, so we simply flip the flag
        // and let Draw fall back to defaults.
        pGI->m_bDrawPart2 = (m_bResetShader != 0);
    }
    return true;
}

// =============================================================================
// Draw (0x0052D1D0)
// =============================================================================
bool CCANormal::Draw()
{
    if (!m_bVisible) return false;
    for (int i = 0; i < m_nLayerCount; ++i)
    {
        GameImage* pGI = m_ppImages[i];
        if (!pGI) continue;
        if (!pGI->m_pGIData) continue;
        // Original also guards on (m_pGIData->m_internal != 0); our
        // GameImage pipeline treats a live m_pGIData as sufficient.
        pGI->Draw();
        m_ppImages[i] = nullptr;
    }
    return true;
}

// =============================================================================
// Play (0x0052D230)
// =============================================================================
void CCANormal::Play(int motion, bool loop)
{
    if (m_nKeyCount <= motion) return;
    m_nMotionIndex = motion;
    m_bPlaying     = 1;
    m_bLoop        = 0;
    if (m_pKeys)
    {
        m_nStartFrame = m_pKeys[motion].m_nStartFrame;
        m_nEndFrame   = m_pKeys[motion].m_nEndFrame;
    }
    m_bLoop = static_cast<uint8_t>(loop ? 1 : 0);
}

// =============================================================================
// Pause (0x0052D280)
// =============================================================================
void CCANormal::Pause()
{
    m_bPlaying = (m_bPlaying == 0) ? 1 : 0;
}
