#include "Character/CCAillust.h"
#include "Character/CCA.h"
#include "Character/CAManager.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceResetManager.h"
#include "Image/GameImage.h"
#include "global.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <cstring>
#include <cstdint>
#include <cstddef>

// ============================================================================
// CCAillust — restored from mofclient.c (0x00528BA0 .. 0x005299E0)
// ============================================================================

// -----------------------------------------------------------------------------
// Helpers for the internal vector<GameImage*> (triple {begin, end, endCap}).
// Mirrors the identical helpers in CCA.cpp / CCAClone.cpp.
// -----------------------------------------------------------------------------
static void CCAillust_VectorPushBack(CCAillust* self, GameImage* pImage)
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

static size_t CCAillust_VectorSize(const CCAillust* self)
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
// Constructor / Destructor (0x00528BA0 / 0x00528C70)
// =============================================================================
CCAillust::CCAillust()
    : m_pSprite(nullptr)
    , m_nVecPad(0)
    , m_pVecBegin(nullptr)
    , m_pVecEnd(nullptr)
    , m_pVecEndCap(nullptr)
    , m_fPosX(0.0f)
    , m_fPosY(0.0f)
{
    std::memset(m_pFrameSlots, 0, sizeof(m_pFrameSlots));
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

CCAillust::~CCAillust()
{
    if (m_pVecBegin)
    {
        ::operator delete(m_pVecBegin);
        m_pVecBegin  = nullptr;
        m_pVecEnd    = nullptr;
        m_pVecEndCap = nullptr;
    }
}

// =============================================================================
// Reset (0x00528CA0)
// =============================================================================
void CCAillust::Reset()
{
    std::memset(m_pFrameSlots, 0, sizeof(m_pFrameSlots));
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
}

// =============================================================================
// LoadCA (0x00528CC0)
//
// Seed m_pFrameSlots[0..15] from CAManager::GetTimeLineInfoBaseIllust().  The
// original walks the base timeline's LAYERINFO array with a 64-byte stride
// (sizeof LAYERINFO in the 32-bit binary): for each of 16 layers, if the layer
// has at least one frame (m_nFrameCount != 0) store layer.m_pFrames into the
// slot, otherwise store nullptr.  The sprite object is lazily fetched once.
// =============================================================================
void CCAillust::LoadCA(const char* /*path*/)
{
    TIMELINEINFO* pTL = g_CAManager.GetTimeLineInfoBaseIllust();
    if (pTL)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (pTL->m_pLayers && i < pTL->m_nLayerCount &&
                pTL->m_pLayers[i].m_nFrameCount != 0)
            {
                m_pFrameSlots[i] = pTL->m_pLayers[i].m_pFrames;
            }
            else
            {
                m_pFrameSlots[i] = nullptr;
            }
        }

        if (!m_pSprite)
        {
            CDeviceResetManager* pRM = CDeviceResetManager::GetInstance();
            if (pRM) m_pSprite = pRM->GetSpriteObject();
        }
    }
}

// =============================================================================
// Process (0x00528D20)
//
// Rebuild the draw list: clear the vector, then walk all 16 layer slots.  For
// each slot that carries a FRAMEINFO with m_nCount1 > 0, iterate every
// CA_DRAWENTRY, resolve a GameImage via cltImageManager::GetGameImage, insert
// it into the vector, and write back per-image state (block, position, and —
// when rendering the hair slots 1 and 11 — the normalized hair color).
// =============================================================================
void CCAillust::Process()
{
    // Equivalent of std::vector<GameImage*>::erase(begin, end) — recycle the
    // storage for this frame without releasing the buffer.
    m_pVecEnd = m_pVecBegin;

    for (unsigned int slot = 0; slot < 16; ++slot)
    {
        FRAMEINFO* pFrame = m_pFrameSlots[slot];
        if (!pFrame) continue;
        int nEntry = pFrame->m_nCount1;
        if (nEntry <= 0) continue;

        CA_DRAWENTRY* pEntries = static_cast<CA_DRAWENTRY*>(pFrame->m_pEntries1);
        if (!pEntries) continue;

        for (int e = 0; e < nEntry; ++e)
        {
            CA_DRAWENTRY* pEntry = &pEntries[e];

            cltImageManager* pIM = cltImageManager::GetInstance();
            GameImage* pGI = pIM ? pIM->GetGameImage(0, pEntry->m_dwImageID, 0, 0) : nullptr;

            // std::vector<GameImage*>::insert(..., 1u, &v18)
            CCAillust_VectorPushBack(this, pGI);

            size_t nVec = CCAillust_VectorSize(this);
            if (nVec == 0) continue;
            GameImage* pBack = m_pVecBegin[nVec - 1];
            if (!pBack) continue;
            if (!pBack->m_pGIData) continue;

            pBack->SetDefaultTextureColor();

            // Hair slots 1 (front) and 11 (back) get the normalized hair-color
            // overwrite applied to their texture tint.
            if (slot == 1 || slot == 11)
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
            pBack->m_fPosX     = m_fPosX + pEntry->m_fOffsetX;
            pBack->m_bFlag_447 = true;
            pBack->m_fPosY     = m_fPosY + pEntry->m_fOffsetY;
        }
    }
}

// =============================================================================
// Draw (0x00528FD0)
//
// Render the draw list built by Process().  Matches the decompile: bail out
// when the global "don't draw" flag is set or there is no sprite, then
// sprite->Begin(flags), iterate, sprite->End().  For each image we compose a
// translation matrix from its per-image position, pull the tint from the
// GameImage's overwrite-color state, and dispatch through ID3DXSprite::Draw.
// =============================================================================
void CCAillust::Draw(int viewport)
{
    if (byte_21CB35D) return;
    ID3DXSprite* pSprite = reinterpret_cast<ID3DXSprite*>(m_pSprite);
    if (!pSprite) return;

    pSprite->Begin(static_cast<DWORD>(viewport));

    size_t count = CCAillust_VectorSize(this);
    for (size_t i = 0; i < count; ++i)
    {
        GameImage* pGI = m_pVecBegin[i];
        if (!pGI) continue;
        ImageResourceListData* pData = pGI->m_pGIData;
        if (!pData) continue;
        IDirect3DTexture9* pTexture = pData->m_Resource.m_pTexture;
        if (!pTexture) continue;

        D3DXMATRIX trans;
        D3DXMatrixTranslation(&trans, pGI->m_fPosX, pGI->m_fPosY, 0.0f);

        float oc[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        pGI->GetOverWriteTextureColor(oc);
        int r = ClampFloatToByte(oc[0]);
        int g = ClampFloatToByte(oc[1]);
        int b = ClampFloatToByte(oc[2]);
        int a = ClampFloatToByte(oc[3]);
        D3DCOLOR color = static_cast<D3DCOLOR>(PackRGBA(a, r, g, b));

        RECT rc;
        pGI->GetBlockRect(&rc);

        // The decompile dispatches through a wrapper that takes (texture, rect,
        // matrix, color).  ID3DXSprite::Draw's signature is (tex, rect, center,
        // pos, color) — feed the matrix translation as the position vector to
        // preserve behaviour.
        D3DXVECTOR3 pos(pGI->m_fPosX, pGI->m_fPosY, 0.0f);
        pSprite->SetTransform(&trans);
        pSprite->Draw(pTexture, &rc, nullptr, &pos, color);
    }

    pSprite->End();
}

// =============================================================================
// SetItemID (0x005291D0)
// =============================================================================
void CCAillust::SetItemID(uint16_t itemID, uint8_t sex, uint8_t age, bool putOn,
                          int hairExtra, int faceExtra)
{
    SetLayerByItemKind(itemID, putOn ? 1 : 0, sex, age, hairExtra, faceExtra);
}

// =============================================================================
// SetHairDefine / SetFaceDefine (0x00529200 / 0x00529220)
// =============================================================================
void CCAillust::SetHairDefine(uint16_t hairIdx, uint8_t sex, uint8_t age)
{
    LayerPutOff(0, sex, age, hairIdx, 0);
}

void CCAillust::SetFaceDefine(uint16_t faceIdx, uint8_t sex, uint8_t age)
{
    LayerPutOff(1u, sex, age, 0, faceIdx);
}

// =============================================================================
// SetItemtoDot (0x00529240)
//
// Pack the current normalized hair color into a 32-bit value, initialize every
// layer slot for the given sex/age/hair/face, then re-apply every fashion item
// stored in the source CCA's second (fashion-override) item-ID column.
// =============================================================================
void CCAillust::SetItemtoDot(CCA* pSource, uint8_t sex, uint8_t age,
                             int hairExtra, int faceExtra)
{
    float color[4];
    GetHairColor(color);
    int r = ClampFloatToByte(color[0]);
    int g = ClampFloatToByte(color[1]);
    int b = ClampFloatToByte(color[2]);
    int a = ClampFloatToByte(color[3]);
    uint32_t packed = PackRGBA(a, r, g, b);

    InitItem(sex, age, static_cast<uint16_t>(hairExtra),
             static_cast<uint16_t>(faceExtra), packed);

    // Walk pSource->m_ItemIDs[0..15][1] (the fashion-override column).  The
    // original uses raw byte arithmetic starting at (char*)a2 + 162 with a
    // 4-byte stride; our named layout is equivalent.  No nullptr guard on
    // pSource — the ground truth assumes a live CCA is always passed in.
    for (int k = 0; k < 16; ++k)
    {
        uint16_t id = pSource->m_ItemIDs[k][1];
        if (id)
            SetLayerByItemKind(id, 1, sex, age, hairExtra, faceExtra);
    }
}

// =============================================================================
// InitItem (0x005293E0)
// =============================================================================
void CCAillust::InitItem(uint8_t sex, uint8_t age, uint16_t hairIdx, uint16_t faceIdx,
                         uint32_t hairColor)
{
    for (int i = 0; i < 16; ++i)
    {
        LayerPutOff(static_cast<uint16_t>(i), sex, age, hairIdx, faceIdx);
    }
    SetHairColor(hairColor);
}

// =============================================================================
// LayerPutOn (0x00529430)
//
// Dispatch by the item's ITEMCAINFO_ILLUST::m_ucKind (the field at byte offset
// +8 in the original struct).  Each kind binds one or more of the 16 slot
// pointers to a FRAMEINFO obtained from CAManager::GetIllustFrame.  The
// "kind" parameter fed to GetIllustFrame is `age + 3*sex`, mapping into the
// 9-entry m_TimelineInfoIllust[] table.
// =============================================================================
void CCAillust::LayerPutOn(uint16_t itemID, uint8_t sex, uint8_t age)
{
    ITEMCAINFO_ILLUST* pInfo = g_CAManager.GetItemCAInfoIllust(itemID);
    if (!pInfo) return;

    int frameIdx = pInfo->m_nValue;
    int kind     = static_cast<int>(age) + 3 * static_cast<int>(sex);

    switch (pInfo->m_ucKind)
    {
    case 0:  // HAIR — front + back
        m_pFrameSlots[1]  = g_CAManager.GetIllustFrame(kind, 1,  frameIdx);
        m_pFrameSlots[11] = g_CAManager.GetIllustFrame(kind, 11, frameIdx);
        break;
    case 1:  // FACE
        m_pFrameSlots[3]  = g_CAManager.GetIllustFrame(kind, 3,  frameIdx);
        break;
    case 2:  // COAT
        m_pFrameSlots[8]  = g_CAManager.GetIllustFrame(kind, 8,  frameIdx);
        break;
    case 3:  // TRIUSERS
        m_pFrameSlots[7]  = g_CAManager.GetIllustFrame(kind, 7,  frameIdx);
        break;
    case 4:  // SHOES
        m_pFrameSlots[5]  = g_CAManager.GetIllustFrame(kind, 5,  frameIdx);
        break;
    case 5:  // HAND
        m_pFrameSlots[6]  = g_CAManager.GetIllustFrame(kind, 6,  frameIdx);
        m_pFrameSlots[15] = g_CAManager.GetIllustFrame(kind, 15, frameIdx);
        break;
    case 6:  // CLOCK (four-layer accessory)
        m_pFrameSlots[0]  = g_CAManager.GetIllustFrame(kind, 0,  frameIdx);
        m_pFrameSlots[2]  = g_CAManager.GetIllustFrame(kind, 2,  frameIdx);
        m_pFrameSlots[10] = g_CAManager.GetIllustFrame(kind, 10, frameIdx);
        m_pFrameSlots[14] = g_CAManager.GetIllustFrame(kind, 14, frameIdx);
        break;
    case 9:  // ACC1
        m_pFrameSlots[4]  = g_CAManager.GetIllustFrame(kind, 4,  frameIdx);
        break;
    case 0xA:  // ACC2
        m_pFrameSlots[12] = g_CAManager.GetIllustFrame(kind, 12, frameIdx);
        break;
    case 0xB:  // ACC3
        m_pFrameSlots[13] = g_CAManager.GetIllustFrame(kind, 13, frameIdx);
        break;
    case 0xD:  // SUIT — clears coat/pants/shoes/hand then binds the suit layer
        m_pFrameSlots[8]  = nullptr;
        m_pFrameSlots[7]  = nullptr;
        m_pFrameSlots[5]  = nullptr;
        m_pFrameSlots[6]  = nullptr;
        m_pFrameSlots[15] = nullptr;
        m_pFrameSlots[9]  = g_CAManager.GetIllustFrame(kind, 9, frameIdx);
        break;
    default:
        return;
    }
}

// =============================================================================
// LayerPutOff (0x00529630)
//
// Rebind slots back to their "base" FRAMEINFO — derived from the current hair
// or face definition for kinds 0/1, or frame 0 of the corresponding layer for
// the remaining kinds.  Kind 0xD (SUIT) rebuilds coat/pants/shoes/hand/suit
// and falls through to the common hand-restore tail at LABEL_12 (shared with
// case 5).  The original switch has no 0xF case — DUALWEAPON is handled
// elsewhere.
// =============================================================================
void CCAillust::LayerPutOff(uint16_t kind, uint8_t sex, uint8_t age,
                            int hairExtra, int faceExtra)
{
    int kindIdx = static_cast<int>(age) + 3 * static_cast<int>(sex);

    switch (kind)
    {
    case 0u:  // HAIR
    {
        int hairFrame = g_CAManager.GetHairFrameIndexIllust(hairExtra, sex);
        m_pFrameSlots[1]  = g_CAManager.GetIllustFrame(kindIdx, 1,  hairFrame);
        m_pFrameSlots[11] = g_CAManager.GetIllustFrame(kindIdx, 11, hairFrame);
        return;
    }
    case 1u:  // FACE
    {
        int faceFrame = g_CAManager.GetFaceFrameIndexIllust(faceExtra, sex);
        m_pFrameSlots[3]  = g_CAManager.GetIllustFrame(kindIdx, 3, faceFrame);
        return;
    }
    case 2u:
        m_pFrameSlots[8]  = g_CAManager.GetIllustFrame(kindIdx, 8, 0);
        return;
    case 3u:
        m_pFrameSlots[7]  = g_CAManager.GetIllustFrame(kindIdx, 7, 0);
        return;
    case 4u:
        m_pFrameSlots[5]  = g_CAManager.GetIllustFrame(kindIdx, 5, 0);
        return;
    case 5u:
        // Ground truth's case 5 is `goto LABEL_12;` which runs only the two
        // slot-6/slot-15 writes (the shared tail).
        m_pFrameSlots[6]  = g_CAManager.GetIllustFrame(kindIdx, 6,  0);
        m_pFrameSlots[15] = g_CAManager.GetIllustFrame(kindIdx, 15, 0);
        return;
    case 6u:
        m_pFrameSlots[0]  = g_CAManager.GetIllustFrame(kindIdx, 0,  0);
        m_pFrameSlots[2]  = g_CAManager.GetIllustFrame(kindIdx, 2,  0);
        m_pFrameSlots[10] = g_CAManager.GetIllustFrame(kindIdx, 10, 0);
        m_pFrameSlots[14] = g_CAManager.GetIllustFrame(kindIdx, 14, 0);
        return;
    case 9u:
        m_pFrameSlots[4]  = g_CAManager.GetIllustFrame(kindIdx, 4, 0);
        return;
    case 0xAu:
        m_pFrameSlots[12] = g_CAManager.GetIllustFrame(kindIdx, 12, 0);
        return;
    case 0xBu:
        m_pFrameSlots[13] = g_CAManager.GetIllustFrame(kindIdx, 13, 0);
        return;
    case 0xDu:
        // SUIT: rebuild body slots then fall through the LABEL_12 tail
        // (shared with case 5) for the hand layers.
        m_pFrameSlots[9]  = g_CAManager.GetIllustFrame(kindIdx, 9, 0);
        m_pFrameSlots[8]  = g_CAManager.GetIllustFrame(kindIdx, 8, 0);
        m_pFrameSlots[7]  = g_CAManager.GetIllustFrame(kindIdx, 7, 0);
        m_pFrameSlots[5]  = g_CAManager.GetIllustFrame(kindIdx, 5, 0);
        m_pFrameSlots[6]  = g_CAManager.GetIllustFrame(kindIdx, 6, 0);
        m_pFrameSlots[15] = g_CAManager.GetIllustFrame(kindIdx, 15, 0);
        return;
    default:
        return;
    }
}

// =============================================================================
// SetLayerByItemKind (0x00529850)
// =============================================================================
void CCAillust::SetLayerByItemKind(uint16_t itemID, int putOn, uint8_t sex, uint8_t age,
                                   int hairExtra, int faceExtra)
{
    ITEMCAINFO_ILLUST* pInfo = g_CAManager.GetItemCAInfoIllust(itemID);
    if (!pInfo) return;

    if (putOn)
        LayerPutOn(itemID, sex, age);
    else
        LayerPutOff(static_cast<uint16_t>(pInfo->m_ucKind), sex, age, hairExtra, faceExtra);
}

// =============================================================================
// SetHairColor / SetDefaultHairColor / GetHairColor (0x005298B0 / 0x00529990 / 0x005299E0)
// =============================================================================
void CCAillust::SetHairColor(uint32_t color)
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

void CCAillust::SetDefaultHairColor()
{
    m_fHairColor[0] = 1.0f;
    m_fHairColor[1] = 1.0f;
    m_fHairColor[2] = 1.0f;
    m_fHairColor[3] = 1.0f;
}

void CCAillust::GetHairColor(float* out)
{
    out[0] = m_fHairColor[0];
    out[1] = m_fHairColor[1];
    out[2] = m_fHairColor[2];
    out[3] = m_fHairColor[3];
}
