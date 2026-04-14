#include "Character/CCA.h"
#include "Character/CAManager.h"
#include "Effect/CEffectBase.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceResetManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <windows.h>
#include <mmsystem.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstddef>

// ============================================================================
// CCA �X restored from mofclient.c (0x00524C8C .. 0x00526E00)
// ============================================================================

// -----------------------------------------------------------------------------
// NOTE: the original binary is 32-bit x86 and places m_fPosX/m_fPosY at byte
// offsets 128/132.  We build as x64, where the class has a larger vtable and
// 8-byte pointers, so those byte offsets no longer apply.  All call sites in
// this port use named access (m_pCCA->m_fPosX) �X search the repo to confirm.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Statics used by CCA::Process() to maintain frame accumulation (original
// names: __S209__1__Process_CCA, _lastTime__1__Process_CCA, _FrameIndex__1).
// -----------------------------------------------------------------------------
static bool  g_bProcessInit  = false;
static float g_fLastTime     = 0.0f;
static int   g_nFrameIndex   = 0;

// Layer-slot �� case-number LUT used by the fallback image-lookup branch in
// CCA::Process().  The binary's byte_525EB4 is a weak symbol (all zeros), so
// we preserve that default here �X every slot falls through to case 0 at load,
// and data-driven tooling can override it later if needed.
static unsigned char byte_525EB4[20] = { 0 };

// -----------------------------------------------------------------------------
// Helpers for CCA's internal vector<GameImage*> (triple {begin, end, endCap}).
// -----------------------------------------------------------------------------
static void CCA_VectorPushBack(CCA* self, GameImage* pImage)
{
    if (self->m_pVecEnd < self->m_pVecEndCap)
    {
        *self->m_pVecEnd++ = pImage;
        return;
    }
    size_t oldSize = static_cast<size_t>(self->m_pVecEnd - self->m_pVecBegin);
    size_t newCap  = oldSize == 0 ? 4 : oldSize * 2;
    GameImage** pNew = static_cast<GameImage**>(::operator new(newCap * sizeof(GameImage*)));
    for (size_t i = 0; i < oldSize; ++i) pNew[i] = self->m_pVecBegin[i];
    pNew[oldSize] = pImage;
    if (self->m_pVecBegin) ::operator delete(self->m_pVecBegin);
    self->m_pVecBegin  = pNew;
    self->m_pVecEnd    = pNew + oldSize + 1;
    self->m_pVecEndCap = pNew + newCap;
}

static size_t CCA_VectorSize(const CCA* self)
{
    return self->m_pVecBegin ? static_cast<size_t>(self->m_pVecEnd - self->m_pVecBegin) : 0;
}

// Clamp a float to a byte [0..255] using the same (rounded, saturating) pattern
// as the original decompilation (cast via int64 to emulate cvttsd2si).
static int ClampFloatToByte(float v)
{
    if (v >= 1.0f) return 255;
    if (v <= 0.0f) return 0;
    return static_cast<int>(v * 255.0f + 0.5f);
}

static inline unsigned int PackRGBA(int a, int r, int g, int b)
{
    return static_cast<unsigned int>(b) |
           ((static_cast<unsigned int>(g) |
             ((static_cast<unsigned int>(r) |
               (static_cast<unsigned int>(a) << 8)) << 8)) << 8);
}

// =============================================================================
// Constructor / Destructor (0x00524C8C / 0x00524DC0)
// =============================================================================
CCA::CCA()
    : m_pSprite(nullptr)
    , m_nVecPad(0)
    , m_pVecBegin(nullptr)
    , m_pVecEnd(nullptr)
    , m_pVecEndCap(nullptr)
    , m_pAnimationInfo(nullptr)
    , m_unk30(0)
    , m_nAlpha(255)
    , m_fPosX(0.0f)
    , m_fPosY(0.0f)
    , m_nJumpOffset(0)
    , m_bMirrored(0)
    , m_nMotionIndex(0)
    , m_unk37(0)
    , m_nCurFrame(0)
    , m_nEndFrame(0)
    , m_cProcessedIdx(0)
    , m_pEffectBefore(nullptr)
    , m_pEffectAfter(nullptr)
    , m_uCurSex(0)
    , m_nHairIndex(0)
    , m_nFaceIndex(0)
    , m_fOverwriteAlpha(0.6f)
    , m_nOverwriteFlag(0)
    , m_nTransportActive(0)
    , m_nTransportMask(0)
{
    std::memset(m_pLayers, 0, sizeof(m_pLayers));
    std::memset(m_ItemIDs, 0, sizeof(m_ItemIDs));
    std::memset(m_pad141, 0, sizeof(m_pad141));
    std::memset(m_pad237, 0, sizeof(m_pad237));
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

CCA::~CCA()
{
    if (m_pVecBegin)
    {
        ::operator delete(m_pVecBegin);
        m_pVecBegin = nullptr;
        m_pVecEnd = nullptr;
        m_pVecEndCap = nullptr;
    }
}

// =============================================================================
// LoadCA (0x00524E00)
// =============================================================================
void CCA::LoadCA(const char* /*path*/, CEffectBase** pEffectBefore, CEffectBase** pEffectAfter)
{
    TIMELINEINFO* base = g_CAManager.GetTimeLineInfoBaseDot();

    // Seed m_pLayers[0..22] by copying pointers from the base timeline's layer
    // array at fixed kind offsets (the original uses 64-byte stride iteration
    // over LAYERINFO*).  The base timeline has its own layer pointer table at
    // index 67; we copy the first 23 pointers.
    if (base->m_pLayers)
    {
        for (int i = 0; i < 23; ++i)
        {
            // Original indexes dword-step: this+24+4*i = base->pLayers[0] + stride*i
            // Each stride is 64 (LAYERINFO size).  The original was really
            // assigning per-index progression using v5[67] + offset, so we
            // preserve that behaviour: slot i points at LAYERINFO index i.
            m_pLayers[i] = (i < base->m_nLayerCount) ? &base->m_pLayers[i] : nullptr;
        }
    }

    m_pAnimationInfo = g_CAManager.GetAnimationInfoBaseDot();
    if (m_pAnimationInfo && m_pAnimationInfo->m_nTotalFrames > 0)
    {
        m_FrameSkip.m_fTimePerFrame = 1.0f / static_cast<float>(m_pAnimationInfo->m_nTotalFrames);
    }

    if (!m_pSprite)
    {
        CDeviceResetManager* pRM = CDeviceResetManager::GetInstance();
        if (pRM) m_pSprite = pRM->GetSpriteObject();
    }

    m_pEffectBefore    = pEffectBefore;
    m_pEffectAfter     = pEffectAfter;
    m_nTransportActive = 0;
}

// =============================================================================
// Play (0x00524E80)
// =============================================================================
void CCA::Play(int motion, bool /*loop*/)
{
    CANIMATIONINFO* pAnim = m_pAnimationInfo;
    if (!pAnim || !pAnim->m_pKeys) return;
    if (pAnim->m_nKeyCount > motion)
    {
        m_nMotionIndex = motion;
        m_nCurFrame    = pAnim->m_pKeys[motion].m_nStartFrame;
        m_nEndFrame    = pAnim->m_pKeys[motion].m_nEndFrame;
    }
}

// =============================================================================
// InitItem / ResetItem / SetHairDefine / SetFaceDefine (0x00524EC0 .. 0x00525120)
// =============================================================================
void CCA::InitItem(uint8_t sex, uint16_t hairIdx, uint16_t faceIdx, uint32_t hairColor)
{
    for (int i = 0; i < 16; ++i)
    {
        LayerPutOff(static_cast<uint16_t>(i), sex, hairIdx, faceIdx);
    }
    SetHairColor(hairColor);
}

void CCA::ResetItem(uint8_t sex, uint16_t hairIdx, uint16_t faceIdx, uint8_t slotIndex)
{
    // Iterate 16 kinds; re-register each currently-held item on the given slot.
    for (int k = 0; k < 16; ++k)
    {
        uint16_t id = m_ItemIDs[k][slotIndex];
        if (id) SetItemID(id, sex, 0, hairIdx, faceIdx, slotIndex);
    }

    if (m_ItemIDs[0][1] == 0)
    {
        m_ItemIDs[0][0] = hairIdx;
        SetHairDefine(hairIdx, sex);
    }
    SetFaceDefine(faceIdx, sex);

    // Reapply hair color using the already-normalized hair-color floats.
    int r = ClampFloatToByte(m_fHairColor[0]);
    int g = ClampFloatToByte(m_fHairColor[1]);
    int b = ClampFloatToByte(m_fHairColor[2]);
    int a = ClampFloatToByte(m_fHairColor[3]);
    SetHairColor(PackRGBA(a, r, g, b));
}

void CCA::SetHairDefine(uint16_t hairIdx, uint8_t sex)
{
    LayerPutOff(0, sex, hairIdx, 0);
    m_ItemIDs[0][0] = hairIdx;
}

void CCA::SetFaceDefine(uint16_t faceIdx, uint8_t sex)
{
    LayerPutOff(1u, sex, 0, faceIdx);
}

// =============================================================================
// Emoticons (0x00525140 / 0x00525170)
// =============================================================================
void CCA::BegineEmoticon(int index)
{
    m_pLayers[6]  = g_CAManager.GetDotLayer(14, index);     // face slot (dword+12)
    m_pLayers[22] = g_CAManager.GetDotLayer(14, index + 1); // emoticon slot (dword+28)
}

void CCA::EndEmoticon(uint16_t faceIdx, uint8_t sex)
{
    LayerPutOff(1u, sex, 0, faceIdx);
    m_pLayers[22] = nullptr;
}

// =============================================================================
// Process() �X top-level per-frame tick (0x005251A0)
// =============================================================================
bool CCA::Process()
{
    if (!g_bProcessInit)
    {
        g_bProcessInit = true;
        g_fLastTime    = static_cast<float>(::timeGetTime());
    }

    float curTime = static_cast<float>(::timeGetTime());
    float acc     = (curTime - g_fLastTime) * 0.001f + m_FrameSkip.m_fAccumulatedTime;
    bool  notReady = acc < m_FrameSkip.m_fTimePerFrame;
    m_FrameSkip.m_fAccumulatedTime = acc;

    int step = 0;
    if (!notReady)
    {
        int whole = static_cast<int>(acc / m_FrameSkip.m_fTimePerFrame);
        if (whole)
            m_FrameSkip.m_fAccumulatedTime = acc - static_cast<float>(whole) * m_FrameSkip.m_fTimePerFrame;
        step = whole;
    }

    g_nFrameIndex += step;
    // The original sums the frame index as (step + frameIndex) and uses it as
    // a pointer-sized int, which is meaningless here �X we simulate the same
    // behaviour by advancing the same counter and clamping.
    int frameIdxLocal = g_nFrameIndex;
    uint16_t frame = static_cast<uint16_t>(frameIdxLocal + static_cast<uint16_t>(m_nCurFrame));
    if (m_nEndFrame < static_cast<int>(frame))
    {
        frameIdxLocal  = 0;
        g_nFrameIndex  = 0;
    }

    g_fLastTime = curTime;
    Process(reinterpret_cast<GameImage*>(static_cast<intptr_t>(frameIdxLocal)));
    return true;
}

// =============================================================================
// Process(GameImage*) �X per-layer draw-list rebuild (0x00525290)
//
// Rebuilds m_pVec* with the GameImage records for every visible layer at the
// current animation frame, applying position/offset/alpha overrides and
// optional hair-color overlay.  This is the heaviest function in CCA.
// =============================================================================
bool CCA::Process(GameImage* pFrameAsPtr)
{
    const uint16_t baseFrameHalfOffset = static_cast<uint16_t>(m_nCurFrame);
    m_cProcessedIdx = 0;
    uint16_t frame = static_cast<uint16_t>(reinterpret_cast<intptr_t>(pFrameAsPtr)) + baseFrameHalfOffset;

    // Reset end pointer to begin (recycle the vector for this frame).
    m_pVecEnd = m_pVecBegin;

    // Transport-masked slot: mark highest-z entry or -1.
    m_nTransportMask = (m_nTransportActive == 1) ? -1 : 0;

    // Pre-insert a "ground shadow" sprite at the beginning of the draw list.
    if (!m_nTransportActive)
    {
        cltImageManager* pIM = cltImageManager::GetInstance();
        GameImage* pShadow = pIM ? pIM->GetGameImage(0, 0x0D000027u, 0, 1) : nullptr;
        if (pShadow)
        {
            CCA_VectorPushBack(this, pShadow);
            GameImage* pBack = m_pVecBegin[0];
            pBack->m_wBlockID = (m_bMirrored != 0) ? 1 : 0;
            pBack->m_bFlag_446 = true;
            float dx = m_fPosX - (m_bMirrored != 0 ? 19.0f : 16.0f);
            pBack->m_bFlag_447 = true;
            pBack->m_fPosX = dx;
            float dy = m_fPosY - (m_bMirrored != 0 ? 26.0f : 28.0f)
                     - static_cast<float>(m_nJumpOffset) * 0.3f;
            pBack->m_fPosY = dy;
        }
    }

    // Iterate all 23 layer slots.  For each visible layer with a frame record,
    // resolve the GameImage for this frame, optionally apply color overrides,
    // and update its transform.
    for (int slot = 0; slot < 23; ++slot)
    {
        LAYERINFO* pLayer = m_pLayers[slot];
        if (!pLayer || !LAYERINFO::IsShow(pLayer))
            continue;
        if (frame >= static_cast<uint16_t>(pLayer->m_nFrameCount))
            continue;

        // Transport filter: when transport is active, only a few slots render.
        if (m_nTransportActive)
        {
            // slot values are 0-based here (0..22), the original was 1..22 via v13+6.
            // The original checks v13 ? {1,6,7,13,14,15,16,22} �� our slots {1,6,7,13,14,15,16,22}.
            switch (slot)
            {
            case 1: case 6: case 7: case 13:
            case 14: case 15: case 16: case 22:
                break;
            default:
                continue;
            }
        }

        FRAMEINFO* pFrameArr = pLayer->m_pFrames;
        if (!pFrameArr) continue;
        FRAMEINFO* pFrame = &pFrameArr[frame];
        int nEntry = pFrame->m_nCount1;
        if (nEntry <= 0) continue;

        for (int e = 0; e < nEntry; ++e)
        {
            CA_DRAWENTRY* pEntry = static_cast<CA_DRAWENTRY*>(pFrame->m_pEntries1) + e;
            if (!pEntry) continue;
            cltImageManager* pIM = cltImageManager::GetInstance();
            GameImage* pGI = nullptr;
            if (slot == 11)  // dualweapon back
            {
                pGI = pIM ? pIM->GetGameImage(0, pEntry->m_dwImageID, 0, 0) : nullptr;
            }
            else
            {
                pGI = pIM ? pIM->GetGameImage(0, pEntry->m_dwImageID, 0, 1) : nullptr;
            }
            if (!pGI || !pGI->m_pGIData)
            {
                // Fallback recovery path (byte_525EB4 case map).  Because the
                // weak table is all-zero, every slot falls through to case 0
                // (re-resolve via hair layer index), matching the original.
                int hairIdx = m_nHairIndex;
                int faceIdx = m_nFaceIndex;
                if (hairIdx == -1 || faceIdx == -1) { pGI = nullptr; }
                else
                {
                    uint8_t curSex = static_cast<uint8_t>(m_uCurSex);
                    if (curSex > 1 || slot < 1 || slot > 19)
                    {
                        pGI = nullptr;
                    }
                    else
                    {
                        int fallbackCase = byte_525EB4[slot - 1];
                        if (fallbackCase == 0)
                        {
                            int idx = g_CAManager.GetHairLayerIndexDot(hairIdx, curSex);
                            LAYERINFO* pFL = g_CAManager.GetDotLayer(0, idx);
                            if (idx != -1 && pFL && static_cast<int>(frame) <= pFL->m_nFrameCount - 1)
                            {
                                FRAMEINFO* pFR = &pFL->m_pFrames[frame];
                                CA_DRAWENTRY* pEnt = static_cast<CA_DRAWENTRY*>(pFR->m_pEntries1) + e;
                                if (pEnt)
                                {
                                    pGI = pIM ? pIM->GetGameImage(0, pEnt->m_dwImageID, 0, 0) : nullptr;
                                    pEntry = pEnt;
                                    if (!pGI || !pGI->m_pGIData) pGI = nullptr;
                                }
                            }
                        }
                        // (other fallback cases unused �X byte_525EB4 is weak/zero)
                    }
                }
            }

            if (!pGI || !pGI->m_pGIData) { continue; }

            if (!findGameImageInList(pGI))
                CCA_VectorPushBack(this, pGI);

            size_t nVec = CCA_VectorSize(this);
            if (nVec == 0) continue;
            GameImage* pBack = m_pVecBegin[nVec - 1];
            if (!pBack || !pBack->m_pGIData)
            {
                if (m_pVecEnd > m_pVecBegin) --m_pVecEnd;
                continue;
            }

            pBack->m_bFlag_446 = true;
            pBack->m_wBlockID  = pEntry->m_wBlockID;

            // Apply hair-color overwrite when rendering hair layers (slot 1 or 13).
            if (slot == 1 || slot == 13)
            {
                pBack->SetDefaultTextureColor();
                int r = ClampFloatToByte(m_fHairColor[0]);
                int g = ClampFloatToByte(m_fHairColor[1]);
                int b = ClampFloatToByte(m_fHairColor[2]);
                int a = ClampFloatToByte(m_fHairColor[3]);
                pBack->SetOverWriteTextureColor(PackRGBA(a, r, g, b));
                pBack->SetOverWriteTextureColorDraw(true);
            }

            // Cache index for later z-priority slot tracking.
            if (slot == 18 || slot == 19 || slot == 20 || slot <= 21)
                m_cProcessedIdx = static_cast<uint8_t>(nVec - 1);

            // Brightness / alpha override pass.
            if (m_nOverwriteFlag == 1)
            {
                float oc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                pBack->GetOverWriteTextureColor(oc);
                oc[3] = m_fOverwriteAlpha;
                int r = ClampFloatToByte(oc[0]);
                int g = ClampFloatToByte(oc[1]);
                int b = ClampFloatToByte(oc[2]);
                int a = ClampFloatToByte(oc[3]);
                pBack->SetOverWriteTextureColor(PackRGBA(a, r, g, b));
            }

            // Position: walk the GameImage's per-block hot-spot table via
            //   v45 = pGIData->m_Resource.m_pAnimationFrames[pEntry->m_wBlockID]
            //   blockHotX = v45->offsetX  (int, then cast to float)
            //   blockHotY = v45->offsetY
            // and combine those with the CA_DRAWENTRY offsets and the CCA
            // origin.  Mirrors mofclient.c lines 241540-241561 exactly.
            float blockHotX = 0.0f;
            float blockHotY = 0.0f;
            if (pBack->m_pGIData && pBack->m_pGIData->m_Resource.m_pAnimationFrames)
            {
                AnimationFrameData* pFrames = pBack->m_pGIData->m_Resource.m_pAnimationFrames;
                const int blockID = pEntry->m_wBlockID;
                blockHotX = static_cast<float>(pFrames[blockID].offsetX);
                blockHotY = static_cast<float>(pFrames[blockID].offsetY);
            }
            const float v46 = blockHotX + pEntry->m_fOffsetX;
            const float px  = m_bMirrored ? (m_fPosX - v46) : (v46 + m_fPosX);
            const float py  = blockHotY + pEntry->m_fOffsetY + m_fPosY;
            pBack->m_bFlag_447 = true;
            pBack->m_fPosX = px;
            if (m_nTransportActive)
                pBack->m_fPosY = py - static_cast<float>(m_nJumpOffset) + 5.0f;
            else
                pBack->m_fPosY = py - static_cast<float>(m_nJumpOffset);
        }
    }

    return true;
}

// =============================================================================
// Draw (0x00525ED0)
// =============================================================================
bool CCA::Draw(int viewport)
{
    if (byte_21CB35D) return false;
    if (!m_pSprite)   return false;

    bool renderStateTouched = false;

    // Effects rendered before the character body.  Ground truth dispatches
    // through vtable slot +12 on the first CEffectBase* held at m_pEffectBefore,
    // which corresponds to CEffectBase::Draw() in our class layout
    // (dtor/FrameProcess/Process/Draw => slots 0/4/8/12 in 32-bit).  Call the
    // virtual directly so derived effects render their pre-body pass.
    if (m_pEffectBefore && !m_nTransportActive)
    {
        CEffectBase* pEff = *m_pEffectBefore;
        if (pEff)
        {
            pEff->Draw();
            renderStateTouched = true;
        }
        if (renderStateTouched)
        {
            g_clDeviceManager.ResetRenderState();
        }
    }

    // Begin sprite batch (original: sprite->Begin(viewport))
    ID3DXSprite* pSprite = reinterpret_cast<ID3DXSprite*>(m_pSprite);
    if (pSprite) pSprite->Begin(static_cast<DWORD>(viewport));

    // Ground truth (D3DX8) path: for every image in the draw list, compute
    // scale+tint+position then dispatch the sprite's Draw vtable entry with
    // (tex, srcRect, scale, center, rot, pos, color).  Our port is D3DX9 and
    // GameImage owns its own vertex buffer pipeline, so the per-image vertex
    // state (position, tint, mirror flip) is already set up in Process(); we
    // only need to invoke GameImage::Draw() here to issue the DrawPrimitive.
    size_t count = CCA_VectorSize(this);
    for (size_t i = 0; i < count; ++i)
    {
        GameImage* pGI = m_pVecBegin[i];
        if (!pGI || !pGI->m_pGIData) continue;
        pGI->Draw();
    }

    if (pSprite) pSprite->End();

    // Effects rendered after the character body.  Ground truth iterates
    // m_pEffectAfter over 9 slots (for i = 0; i < 36; i += 4) and dispatches
    // vtable slot +12 on each non-null entry; that is CEffectBase::Draw().
    if (m_pEffectAfter)
    {
        for (int i = 0; i < 9; ++i)
        {
            CEffectBase* pEff = m_pEffectAfter[i];
            if (pEff)
            {
                pEff->Draw();
                renderStateTouched = true;
            }
        }
    }
    if (renderStateTouched) g_clDeviceManager.ResetRenderState();
    return true;
}

// =============================================================================
// SetLayerInfo / GetLayerInfo (0x00526190 / 0x005261B0)
// =============================================================================
void CCA::SetLayerInfo(LAYERINFO** src)
{
    std::memcpy(m_pLayers, src, sizeof(m_pLayers));
}

LAYERINFO** CCA::GetLayerInfo()
{
    return m_pLayers;
}

// =============================================================================
// SetItemID (0x005261C0) �X dispatches to LayerPutOn / LayerPutOff / SetLayerByItemKind
// =============================================================================
void CCA::SetItemID(uint16_t itemID, uint8_t sex, int mode, int hairIdx, int faceIdx, uint8_t slotIndex)
{
    uint8_t itemKind = g_CAManager.GetItemKind(itemID);
    if (itemKind == 0xFF || itemKind >= 0x10) return;

    m_uCurSex    = sex;
    m_nFaceIndex = faceIdx;
    m_nHairIndex = hairIdx;

    if (slotIndex == 0)
    {
        // slot 0 �X equipment
        if (!mode)
        {
            if (m_ItemIDs[itemKind][0] == 0)
            {
                SetLayerByItemKind(itemID, mode, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
            }
            m_ItemIDs[itemKind][0] = 0;
            if (itemKind == 0) m_ItemIDs[0][0] = static_cast<uint16_t>(hairIdx);
            return;
        }

        if (m_ItemIDs[itemKind][1])
        {
            if (itemKind == 7)
            {
                if (g_CAManager.GetItemSpecial(m_ItemIDs[8][0]) == 1) m_ItemIDs[8][0] = 0;
                m_ItemIDs[7][0] = itemID;
            }
            else if (itemKind == 8)
            {
                if (g_CAManager.GetItemSpecial(8u) == 1) m_ItemIDs[7][0] = 0;
                m_ItemIDs[8][0] = itemID;
            }
            else
            {
                m_ItemIDs[itemKind][0] = itemID;
            }
        }
        else
        {
            SetLayerByItemKind(itemID, mode, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
            if (itemKind == 7)
            {
                if (g_CAManager.GetItemSpecial(m_ItemIDs[8][0]) == 1) m_ItemIDs[8][0] = 0;
                m_ItemIDs[7][0] = itemID;
            }
            else if (itemKind == 8)
            {
                if (g_CAManager.GetItemSpecial(8u) == 1) m_ItemIDs[7][0] = 0;
                m_ItemIDs[8][0] = itemID;
            }
            else
            {
                m_ItemIDs[itemKind][0] = itemID;
            }
        }
        if (itemKind == 0) m_ItemIDs[0][0] = static_cast<uint16_t>(hairIdx);
        return;
    }

    // slotIndex == 1 �X fashion override
    if (mode)
    {
        SetLayerByItemKind(itemID, mode, sex, hairIdx, faceIdx, m_ItemIDs[8][1]);
        switch (itemKind)
        {
        case 7:
            if (g_CAManager.GetItemSpecial(m_ItemIDs[8][1]) == 1) m_ItemIDs[8][1] = 0;
            m_ItemIDs[7][1] = itemID;
            return;
        case 8:
            if (g_CAManager.GetItemSpecial(8u) == 1) m_ItemIDs[7][1] = 0;
            m_ItemIDs[8][1] = itemID;
            return;
        case 13:
            m_ItemIDs[2][1] = itemID;
            m_ItemIDs[3][1] = itemID;
            m_ItemIDs[4][1] = itemID;
            m_ItemIDs[5][1] = itemID;
            m_ItemIDs[13][1] = itemID;
            return;
        case 0:
        {
            bool oldEmpty = (m_ItemIDs[12][0] == 0);
            m_ItemIDs[0][1] = itemID;
            if (!oldEmpty) LayerPutOff(0xCu, sex, hairIdx, faceIdx);
            m_ItemIDs[12][1] = itemID;
            if (mode) m_ItemIDs[0][0] = static_cast<uint16_t>(hairIdx);
            return;
        }
        }
        m_ItemIDs[itemKind][1] = itemID;
        return;
    }

    // mode == 0 branch
    if (itemKind == 13)
    {
        // SUIT �X spread across coat/pants/shoes/hand/suit slots.
        uint16_t cur = m_ItemIDs[13][0];
        if (cur)
        {
            SetLayerByItemKind(cur, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
            m_ItemIDs[13][1] = m_ItemIDs[13][0];
        }
        else
        {
            LayerPutOff(0xDu, sex, hairIdx, faceIdx);
            m_ItemIDs[13][1] = 0;
        }
        uint16_t cur2 = m_ItemIDs[2][0];
        if (cur2) { SetLayerByItemKind(cur2, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]); m_ItemIDs[2][1] = m_ItemIDs[13][0]; }
        else      { LayerPutOff(2u, sex, hairIdx, faceIdx); m_ItemIDs[2][1] = 0; }
        uint16_t cur3 = m_ItemIDs[3][0];
        if (cur3) { SetLayerByItemKind(cur3, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]); m_ItemIDs[3][1] = m_ItemIDs[13][0]; }
        else      { LayerPutOff(3u, sex, hairIdx, faceIdx); m_ItemIDs[3][1] = 0; }
        uint16_t cur4 = m_ItemIDs[4][0];
        if (cur4) { SetLayerByItemKind(cur4, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]); m_ItemIDs[4][1] = m_ItemIDs[13][0]; }
        else      { LayerPutOff(4u, sex, hairIdx, faceIdx); m_ItemIDs[4][1] = 0; }
        uint16_t cur5 = m_ItemIDs[5][0];
        if (cur5) { SetLayerByItemKind(cur5, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]); m_ItemIDs[5][1] = m_ItemIDs[13][0]; }
        else      { LayerPutOff(5u, sex, hairIdx, faceIdx); m_ItemIDs[5][1] = 0; }
        return;
    }

    if (itemKind == 0)
    {
        m_nHairIndex = static_cast<uint16_t>(hairIdx);
        m_ItemIDs[12][1] = 0;
        SetHairDefine(static_cast<uint16_t>(hairIdx), sex);
        uint16_t cur = m_ItemIDs[12][0];
        if (cur) SetLayerByItemKind(cur, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
        m_ItemIDs[0][0] = static_cast<uint16_t>(hairIdx);
        return;
    }

    uint16_t holder = m_ItemIDs[itemKind][0];
    if (holder)
    {
        SetLayerByItemKind(holder, 1, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
    }
    else
    {
        SetLayerByItemKind(itemID, 0, sex, hairIdx, faceIdx, m_ItemIDs[8][0]);
    }
    m_ItemIDs[itemKind][1] = 0;
}

// =============================================================================
// LayerPutOn (0x00526700)
// =============================================================================
void CCA::LayerPutOn(uint16_t itemID, uint16_t extraItemID)
{
    ITEMCAINFO_DOT* pInfo = g_CAManager.GetItemCAInfoDot(itemID);
    if (!pInfo) return;
    int base = pInfo->m_nLayerIndex;
    switch (pInfo->m_ucItemKind)
    {
    case 0:  // HAIR
        m_pLayers[1]  = g_CAManager.GetDotLayer(0, base);
        m_pLayers[13] = g_CAManager.GetDotLayer(0, base + 1);
        break;
    case 1:  // FACE
        m_pLayers[6] = g_CAManager.GetDotLayer(1, base);
        break;
    case 2:  // COAT
        m_pLayers[10] = g_CAManager.GetDotLayer(2, base);
        break;
    case 3:  // TRIUSERS
        m_pLayers[9] = g_CAManager.GetDotLayer(3, base);
        break;
    case 4:  // SHOES
        m_pLayers[8] = g_CAManager.GetDotLayer(4, base);
        break;
    case 5:  // HAND
        m_pLayers[4]  = g_CAManager.GetDotLayer(5, base);
        m_pLayers[19] = g_CAManager.GetDotLayer(5, base + 1);
        break;
    case 6:  // CLOCK
        m_pLayers[0]  = g_CAManager.GetDotLayer(6, base);
        m_pLayers[2]  = g_CAManager.GetDotLayer(6, base + 1);
        m_pLayers[12] = g_CAManager.GetDotLayer(6, base + 2);
        m_pLayers[17] = g_CAManager.GetDotLayer(6, base + 3);
        break;
    case 7:  // RWEAPON
    {
        if (extraItemID)
        {
            ITEMCAINFO_DOT* pExtra = g_CAManager.GetItemCAInfoDot(extraItemID);
            if (pExtra && pExtra->m_nSpecial == 1)
            {
                m_pLayers[3]  = nullptr;
                m_pLayers[20] = nullptr;
            }
        }
        m_pLayers[5]  = nullptr;
        m_pLayers[18] = g_CAManager.GetDotLayer(7, base);
        m_pLayers[21] = g_CAManager.GetDotLayer(7, base + 1);
        break;
    }
    case 8:  // LWEAPON
        if (pInfo->m_nSpecial == 1)
        {
            m_pLayers[18] = nullptr;
            m_pLayers[21] = nullptr;
        }
        m_pLayers[3]  = g_CAManager.GetDotLayer(8, base);
        m_pLayers[20] = g_CAManager.GetDotLayer(8, base + 1);
        break;
    case 9:  // ACC1
        m_pLayers[7] = g_CAManager.GetDotLayer(9, base);
        break;
    case 10: // ACC2
        m_pLayers[15] = g_CAManager.GetDotLayer(10, base);
        break;
    case 11: // ACC3
        m_pLayers[16] = g_CAManager.GetDotLayer(11, base);
        break;
    case 12: // MASKHAIR
        m_pLayers[14] = g_CAManager.GetDotLayer(12, base);
        break;
    case 13: // SUIT
        m_pLayers[10] = nullptr;
        m_pLayers[9]  = nullptr;
        m_pLayers[8]  = nullptr;
        m_pLayers[11] = g_CAManager.GetDotLayer(13, base);
        m_pLayers[4]  = g_CAManager.GetDotLayer(13, base + 1);
        m_pLayers[19] = g_CAManager.GetDotLayer(13, base + 2);
        break;
    case 15: // DUALWEAPON
        m_pLayers[3]  = nullptr;
        m_pLayers[20] = nullptr;
        m_pLayers[5]  = g_CAManager.GetDotLayer(15, base);
        m_pLayers[18] = g_CAManager.GetDotLayer(15, base + 1);
        m_pLayers[21] = g_CAManager.GetDotLayer(15, base + 2);
        break;
    default:
        return;
    }
}

// =============================================================================
// LayerPutOff (0x005269D0) �X clears layer slots for a given kind
// =============================================================================
void CCA::LayerPutOff(uint16_t kind, uint8_t sex, int hairIdx, int faceIdx)
{
    for (;;)
    {
        switch (kind)
        {
        case 0u:  // HAIR
        {
            int idx = g_CAManager.GetHairLayerIndexDot(hairIdx, sex);
            if (idx != -1)
            {
                m_pLayers[1]  = g_CAManager.GetDotLayer(0, idx);
                m_pLayers[13] = g_CAManager.GetDotLayer(0, idx + 1);
            }
            break;
        }
        case 1u:  // FACE
        {
            int idx = g_CAManager.GetFaceLayerIndexDot(faceIdx, sex);
            m_pLayers[6] = g_CAManager.GetDotLayer(1, idx);
            break;
        }
        case 2u:  m_pLayers[10] = g_CAManager.GetDotLayer(2, sex); break;
        case 3u:  m_pLayers[9]  = g_CAManager.GetDotLayer(3, sex); break;
        case 4u:  m_pLayers[8]  = g_CAManager.GetDotLayer(4, sex); break;
        case 5u:
            m_pLayers[4]  = g_CAManager.GetDotLayer(5, 2 * sex);
            m_pLayers[19] = g_CAManager.GetDotLayer(5, 2 * sex + 1);
            break;
        case 6u:
            m_pLayers[2]  = nullptr;
            m_pLayers[12] = nullptr;
            m_pLayers[17] = nullptr;
            m_pLayers[0]  = nullptr;
            break;
        case 7u:
            m_pLayers[18] = nullptr;
            m_pLayers[21] = nullptr;
            break;
        case 8u:
            m_pLayers[3]  = nullptr;
            m_pLayers[20] = nullptr;
            break;
        case 9u:  m_pLayers[7]  = nullptr; break;
        case 0xAu: m_pLayers[15] = nullptr; break;
        case 0xBu: m_pLayers[16] = nullptr; break;
        case 0xCu: m_pLayers[14] = nullptr; break;
        case 0xDu:
            LayerPutOff(2u, sex, hairIdx, faceIdx);
            LayerPutOff(3u, sex, hairIdx, faceIdx);
            LayerPutOff(4u, sex, hairIdx, faceIdx);
            LayerPutOff(5u, sex, hairIdx, faceIdx);
            m_pLayers[11] = nullptr;
            break;
        case 0xFu:
            m_pLayers[5] = nullptr;
            kind = 7;
            continue;
        default:
            return;
        }
        return;
    }
}

// =============================================================================
// SetLayerByItemKind (0x00526BF0)
// =============================================================================
void CCA::SetLayerByItemKind(uint16_t itemID, int putOn, uint8_t sex, int hairIdx, int faceIdx, uint16_t extraItemID)
{
    ITEMCAINFO_DOT* pInfo = g_CAManager.GetItemCAInfoDot(itemID);
    if (!pInfo) return;
    if (putOn)
        LayerPutOn(itemID, extraItemID);
    else
        LayerPutOff(static_cast<uint16_t>(pInfo->m_ucItemKind), sex, hairIdx, faceIdx);
}

// =============================================================================
// SetHairColor / SetDefaultHairColor / GetHairColor (0x00526C50)
// =============================================================================
void CCA::SetHairColor(uint32_t color)
{
    if (color)
    {
        m_fHairColor[0] = static_cast<float>((color >> 16) & 0xFFu) * (1.0f / 255.0f);
        m_fHairColor[1] = static_cast<float>((color >>  8) & 0xFFu) * (1.0f / 255.0f);
        m_fHairColor[2] = static_cast<float>( color        & 0xFFu) * (1.0f / 255.0f);
        m_fHairColor[3] = static_cast<float>((color >> 24) & 0xFFu) * (1.0f / 255.0f);
    }
    else
    {
        m_fHairColor[0] = 1.0f;
        m_fHairColor[1] = 1.0f;
        m_fHairColor[2] = 1.0f;
        m_fHairColor[3] = 1.0f;
    }
}

void CCA::SetDefaultHairColor()
{
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

void CCA::GetHairColor(float* out)
{
    out[0] = m_fHairColor[0];
    out[1] = m_fHairColor[1];
    out[2] = m_fHairColor[2];
    out[3] = m_fHairColor[3];
}

// =============================================================================
// findGameImageInList (0x00526DC0)
// =============================================================================
bool CCA::findGameImageInList(GameImage* target)
{
    if (!target) return true;
    size_t n = CCA_VectorSize(this);
    for (size_t i = 0; i < n; ++i)
        if (m_pVecBegin[i] == target) return true;
    return false;
}

// =============================================================================
// SetTransportActive (0x00526E00)
// =============================================================================
void CCA::SetTransportActive(int active)
{
    m_nTransportActive = active;
}

// =============================================================================
// ExGetIllustCharSexCode (0x0042DE80)
// =============================================================================
unsigned char ExGetIllustCharSexCode(char charKind)
{
    if (charKind == 70) return 0;
    if (charKind == 77) return 1;
    return static_cast<unsigned char>(charKind);
}
