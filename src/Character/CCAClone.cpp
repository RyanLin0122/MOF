#include "Character/CCAClone.h"
#include "Character/CAManager.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceResetManager.h"
#include "Image/CDeviceManager.h"
#include "Image/GameImage.h"
#include "global.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <windows.h>
#include <cstring>
#include <cstdint>
#include <cstddef>

// ============================================================================
// CCAClone — restored from mofclient.c (0x00527700 .. 0x00528560)
// ============================================================================

// The per-slot fallback case map (byte_5280FC) used by Process()'s recovery
// branch now lives in global.cpp so tooling can repurpose it without touching
// CCAClone internals.  See global.h for the case-value legend.

// -----------------------------------------------------------------------------
// Fallback helper: re-resolve a frame entry from an alternate (kind, idx)
// timeline layer.  Mirrors mofclient.c 243031-243223 — identical to the
// CCA helper, duplicated here to keep each class's fallback logic self
// contained (the function is tiny and the runtime cost is identical).
// -----------------------------------------------------------------------------
static bool CCAClone_TryFallbackResolve(int kind, int idx, uint16_t frame, int e,
                                        cltImageManager* pIM,
                                        GameImage** outPGI, CA_DRAWENTRY** outPEntry)
{
    LAYERINFO* pFL = g_CAManager.GetDotLayer(kind, idx);
    if (!pFL || !pFL->m_pFrames) return false;
    if (static_cast<int>(frame) > pFL->m_nFrameCount - 1) return false;

    FRAMEINFO* pFR = &pFL->m_pFrames[frame];
    if (e > pFR->m_nCount1 - 1) return false;
    if (!pFR->m_pEntries1) return false;

    CA_DRAWENTRY* pEnt = static_cast<CA_DRAWENTRY*>(pFR->m_pEntries1) + e;
    GameImage* pRec = pIM ? pIM->GetGameImage(0, pEnt->m_dwImageID, 0, 0) : nullptr;
    if (!pRec || !pRec->m_pGIData) return false;

    *outPGI    = pRec;
    *outPEntry = pEnt;
    return true;
}

// -----------------------------------------------------------------------------
// Helpers for the internal vector<GameImage*> (triple {begin, end, endCap}).
// -----------------------------------------------------------------------------
static void CCAClone_VectorPushBack(CCAClone* self, GameImage* pImage)
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

static size_t CCAClone_VectorSize(const CCAClone* self)
{
    return self->m_pVecBegin ? static_cast<size_t>(self->m_pVecEnd - self->m_pVecBegin) : 0;
}

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
// Constructor / Destructor (0x00527700 / 0x00527790)
// =============================================================================
CCAClone::CCAClone()
    : m_pSprite(nullptr)
    , m_nVecPad(0)
    , m_pVecBegin(nullptr)
    , m_pVecEnd(nullptr)
    , m_pVecEndCap(nullptr)
    , m_ppLayers(nullptr)
    , m_pAnimationInfo(nullptr)
    , m_fPosX(0.0f)
    , m_fPosY(0.0f)
    , m_bMirrored(0)
    , m_bPlaying(0)
    , m_bVisible(1)
    , m_pad43(0)
    , m_nMotionIndex(0)
    , m_nCurFrame(0)
    , m_fAccumFrame(0.0f)
    , m_ucSex(0)
    , m_nHairIndex(0)
    , m_nFaceIndex(0)
{
    std::memset(m_pad57, 0, sizeof(m_pad57));
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

CCAClone::~CCAClone()
{
    m_pAnimationInfo = nullptr;
    if (m_pVecBegin)
    {
        ::operator delete(m_pVecBegin);
        m_pVecBegin  = nullptr;
        m_pVecEnd    = nullptr;
        m_pVecEndCap = nullptr;
    }
}

// =============================================================================
// Reset (0x005277C0)
// =============================================================================
void CCAClone::Reset()
{
    m_bVisible       = 1;
    m_pAnimationInfo = nullptr;
    m_ppLayers       = nullptr;
    m_fPosX          = 0.0f;
    m_fPosY          = 0.0f;
    m_bMirrored      = 0;
    m_bPlaying       = 0;
    m_nMotionIndex   = 0;
    m_nCurFrame      = 0;
    m_fAccumFrame    = 0.0f;
    m_pSprite        = nullptr;
    m_nFaceIndex     = 0;
    m_nHairIndex     = 0;
}

// =============================================================================
// SetLayerInfo (0x005277F0)
// =============================================================================
void CCAClone::SetLayerInfo(LAYERINFO** layers, char sex, int hairIdx, int faceIdx)
{
    m_pAnimationInfo = g_CAManager.GetAnimationInfoBaseDot();
    m_ppLayers       = layers;
    m_ucSex          = static_cast<uint8_t>(sex);
    m_nHairIndex     = hairIdx;
    m_nFaceIndex     = faceIdx;
}

// =============================================================================
// Process (0x00527820)
// =============================================================================
void CCAClone::Process()
{
    if (!m_pSprite)
    {
        CDeviceResetManager* pRM = CDeviceResetManager::GetInstance();
        if (pRM) m_pSprite = pRM->GetSpriteObject();
    }

    if (!m_pAnimationInfo || !m_ppLayers || !m_bVisible)
        return;

    // Advance time accumulator by 0.3 "ticks" per Process call.
    m_fAccumFrame += 0.30000001f;

    CA_KEYINFO* pKeys = m_pAnimationInfo->m_pKeys;
    if (!pKeys) return;
    int motionIdx = m_nMotionIndex;
    uint16_t frame = static_cast<uint16_t>(
        static_cast<uint16_t>(m_nCurFrame) + static_cast<int>(m_fAccumFrame));
    if (pKeys[motionIdx].m_nEndFrame < static_cast<int>(frame))
    {
        frame = static_cast<uint16_t>(m_nCurFrame);
        m_fAccumFrame = 0.0f;
    }

    // Reset vector end pointer (recycle storage).
    m_pVecEnd = m_pVecBegin;

    // Iterate the 23 layer slots.  For every visible layer with a frame
    // record, resolve the GameImage, apply overrides, and push it.
    for (int slot = 0; slot < 23; ++slot)
    {
        LAYERINFO* pLayer = m_ppLayers[slot];
        if (!pLayer) continue;
        if (!LAYERINFO::IsShow(pLayer)) continue;
        if (static_cast<int>(frame) >= pLayer->m_nFrameCount) continue;

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
            bool bNeedFallback = false;
            if (slot == 11)
            {
                // Slot 11 (dualweapon back): validate only that the GIData
                // node exists; the texture may legitimately be lazy-loaded.
                pGI = pIM ? pIM->GetGameImage(0, pEntry->m_dwImageID, 0, 0) : nullptr;
                bNeedFallback = (!pGI || !pGI->m_pGIData);
            }
            else
            {
                pGI = pIM ? pIM->GetGameImage(0, pEntry->m_dwImageID, 0, 1) : nullptr;
                // GT (mofclient.c 243016-243019) additionally checks
                // m_pGIData->m_Resource.m_pTexture here: if the texture has
                // not been uploaded to D3D yet, treat the fetch as failed
                // and drop into the fallback recovery path.
                bNeedFallback = (!pGI || !pGI->m_pGIData ||
                                 !pGI->m_pGIData->m_Resource.m_pTexture);
            }

            // Fallback recovery (byte_5280FC case map).  byte_5280FC is
            // weak/zero in the shipped binary, so every slot lands in case 0
            // by default — but all 9 cases (0..8) are wired up here so the
            // table can be re-purposed at runtime via global.cpp.  Cases
            // mirror mofclient.c 243031-243223.
            if (bNeedFallback)
            {
                // GT resets a2/v71 = 0 at LABEL_21 before attempting recovery,
                // so a failed fallback drops this entry entirely instead of
                // keeping the (potentially texture-less) original GameImage.
                pGI = nullptr;
                const int hairIdx = m_nHairIndex;
                const int faceIdx = m_nFaceIndex;
                if (hairIdx != -1 && faceIdx != -1 &&
                    m_ucSex <= 1 && slot >= 1 && slot <= 19)
                {
                    GameImage* pAlt = nullptr;
                    CA_DRAWENTRY* pAltEntry = nullptr;

                    switch (byte_5280FC[slot - 1])
                    {
                    case 0:  // re-resolve via front hair layer
                    {
                        int idx = g_CAManager.GetHairLayerIndexDot(hairIdx, m_ucSex);
                        if (idx != -1)
                            CCAClone_TryFallbackResolve(0, idx, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    }
                    case 1:  // hand layer low (kind 5, 2*sex)
                        CCAClone_TryFallbackResolve(5, 2 * m_ucSex, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    case 2:  // re-resolve via face layer
                    {
                        int idx = g_CAManager.GetFaceLayerIndexDot(faceIdx, m_ucSex);
                        if (idx != -1)
                            CCAClone_TryFallbackResolve(1, idx, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    }
                    case 3:  // shoes layer (kind 4, sex)
                        CCAClone_TryFallbackResolve(4, m_ucSex, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    case 4:  // triusers layer (kind 3, sex)
                        CCAClone_TryFallbackResolve(3, m_ucSex, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    case 5:  // coat layer (kind 2, sex)
                        CCAClone_TryFallbackResolve(2, m_ucSex, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    case 6:  // back hair layer (front hair + 1)
                    {
                        int idx = g_CAManager.GetHairLayerIndexDot(hairIdx, m_ucSex);
                        if (idx != -1)
                            CCAClone_TryFallbackResolve(0, idx + 1, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    }
                    case 7:  // hand layer high (kind 5, 2*sex + 1)
                        CCAClone_TryFallbackResolve(5, 2 * m_ucSex + 1, frame, e, pIM, &pAlt, &pAltEntry);
                        break;
                    case 8:  // explicit skip — leave pGI null
                    default:
                        break;
                    }

                    if (pAlt)
                    {
                        pGI    = pAlt;
                        pEntry = pAltEntry;
                    }
                }
            }

            if (!pGI || !pGI->m_pGIData) continue;

            if (!findGameImageInList(pGI))
                CCAClone_VectorPushBack(this, pGI);

            size_t nVec = CCAClone_VectorSize(this);
            if (nVec == 0) continue;
            GameImage* pBack = m_pVecBegin[nVec - 1];
            if (!pBack || !pBack->m_pGIData)
            {
                if (m_pVecEnd > m_pVecBegin) --m_pVecEnd;
                continue;
            }

            pBack->SetDefaultTextureColor();

            // Apply hair-color overwrite when rendering hair layers (slot 1 or 13).
            if (slot == 1 || slot == 13)
            {
                int r = ClampFloatToByte(m_fHairColor[0]);
                int g = ClampFloatToByte(m_fHairColor[1]);
                int b = ClampFloatToByte(m_fHairColor[2]);
                int a = ClampFloatToByte(m_fHairColor[3]);
                pBack->SetOverWriteTextureColor(PackRGBA(a, r, g, b));
                pBack->SetOverWriteTextureColorDraw(true);
            }

            pBack->m_wBlockID  = pEntry->m_wBlockID;
            pBack->m_bFlag_446 = true;

            // Ground truth walks the per-block hot-spot table on the live
            // GameImage (m_pGIData->m_Resource.m_pAnimationFrames[blockID])
            // and adds offsetX/offsetY into the CA_DRAWENTRY + CCA origin
            // formula.  Mirror the exact arithmetic from mofclient.c
            // lines 243302-243318 so per-sprite translations match.
            float blockHotX = 0.0f;
            float blockHotY = 0.0f;
            if (pBack->m_pGIData && pBack->m_pGIData->m_Resource.m_pAnimationFrames)
            {
                AnimationFrameData* pFrames = pBack->m_pGIData->m_Resource.m_pAnimationFrames;
                const int blockID = pEntry->m_wBlockID;
                blockHotX = static_cast<float>(pFrames[blockID].offsetX);
                blockHotY = static_cast<float>(pFrames[blockID].offsetY);
            }
            const float v58 = blockHotX + pEntry->m_fOffsetX;
            const float px  = m_bMirrored ? (m_fPosX - v58) : (v58 + m_fPosX);
            const float py  = blockHotY + pEntry->m_fOffsetY + m_fPosY;
            pBack->m_bFlag_447 = true;
            pBack->m_fPosX = px;
            pBack->m_fPosY = py;
        }
    }
}

// =============================================================================
// Draw (0x00528110)
// =============================================================================
void CCAClone::Draw()
{
    if (!m_bVisible)   return;
    if (byte_21CB35D)  return;
    if (!m_pSprite)    return;

    ID3DXSprite* pSprite = reinterpret_cast<ID3DXSprite*>(m_pSprite);
    pSprite->Begin(0);

    // Ground truth (D3DX8) issues the sprite's Draw vtable entry with
    // (tex, srcRect, scale, center, rot, pos, color) and then calls
    // GameImage::SetDefaultTextureColor after each image.  In our D3DX9 port
    // the per-image transform / tint / mirror flip is already baked into the
    // GameImage vertex buffer by Process(), so we just invoke GameImage::Draw
    // to issue the DrawPrimitive and restore the default tint afterwards.
    size_t count = CCAClone_VectorSize(this);
    for (size_t i = 0; i < count; ++i)
    {
        GameImage* pGI = m_pVecBegin[i];
        if (!pGI || !pGI->m_pGIData) continue;
        pGI->Draw();
        pGI->SetDefaultTextureColor();
    }

    pSprite->End();
}

// =============================================================================
// Play (0x00528380)
// =============================================================================
void CCAClone::Play(int motion, bool /*pause*/)
{
    CANIMATIONINFO* pAnim = m_pAnimationInfo;
    if (!pAnim || !pAnim->m_pKeys) return;
    if (pAnim->m_nKeyCount > motion)
    {
        m_bPlaying     = 1;
        m_nMotionIndex = motion;
        m_nCurFrame    = pAnim->m_pKeys[motion].m_nStartFrame;
    }
}

// =============================================================================
// Pause (0x005283B0)
// =============================================================================
void CCAClone::Pause()
{
    m_bPlaying = (m_bPlaying == 0) ? 1 : 0;
}

// =============================================================================
// SetHairColor / SetDefaultHairColor / GetHairColor (0x005283C0 ..)
// =============================================================================
void CCAClone::SetHairColor(unsigned int color)
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

void CCAClone::SetDefaultHairColor()
{
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

void CCAClone::GetHairColor(float* out)
{
    out[0] = m_fHairColor[0];
    out[1] = m_fHairColor[1];
    out[2] = m_fHairColor[2];
    out[3] = m_fHairColor[3];
}

// =============================================================================
// findGameImageInList (0x00528520)
// =============================================================================
bool CCAClone::findGameImageInList(GameImage* target)
{
    if (!target) return false;
    size_t n = CCAClone_VectorSize(this);
    for (size_t i = 0; i < n; ++i)
        if (m_pVecBegin[i] == target) return true;
    return false;
}
