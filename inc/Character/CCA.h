#pragma once
#include <cstdint>
#include <vector>
#include <d3dx9.h>
#include "Character/CATypes.h"

struct CEffectBase;
class  GameImage;

// =============================================================================
// CCA  (292 bytes in the original binary)
//
// The offsets below MUST match mofclient.c because external modules (pet /
// transport objects) peek at CCA::m_fPosX at +128 and CCA::m_fPosY at +132.
// Offsets are asserted at the bottom of CCA.cpp.
// =============================================================================

class CCA
{
public:
    CCA();
    virtual ~CCA();

    // Loaders / animation control
    void LoadCA(const char* path, CEffectBase** pEffectBefore = nullptr, CEffectBase** pEffectAfter = nullptr);
    void Play(int motion, bool loop);
    bool Process();                 // top-level per-frame tick
    bool Process(GameImage* a2);    // per-layer draw-list rebuild
    bool Draw(int viewport = 0);

    // Equipment helpers
    void InitItem(uint8_t sex, uint16_t hairIdx, uint16_t faceIdx, uint32_t hairColor);
    void ResetItem(uint8_t sex, uint16_t hairIdx, uint16_t faceIdx, uint8_t slotIndex);
    void SetHairDefine(uint16_t hairIdx, uint8_t sex);
    void SetFaceDefine(uint16_t faceIdx, uint8_t sex);
    void BegineEmoticon(int index);
    void EndEmoticon(uint16_t faceIdx, uint8_t sex);
    void SetLayerInfo(LAYERINFO** src);
    LAYERINFO** GetLayerInfo();
    void SetItemID(uint16_t itemID, uint8_t sex, int mode, int hairIdx, int faceIdx, uint8_t slotIndex);
    void LayerPutOn(uint16_t itemID, uint16_t extraItemID);
    void LayerPutOff(uint16_t kind, uint8_t sex, int hairIdx, int faceIdx);
    void SetLayerByItemKind(uint16_t itemID, int putOn, uint8_t sex, int hairIdx, int faceIdx, uint16_t extraItemID);
    void SetHairColor(uint32_t color);
    void SetDefaultHairColor();
    void GetHairColor(float* out);
    bool findGameImageInList(GameImage* target);
    void SetTransportActive(int active);

public:
    // ---- binary layout (see constructor in mofclient.c at 0x00524C8C) ------
    // +0   vptr (implicit via virtual dtor)
    void*         m_pSprite;          // +4  ID3DXSprite* from Device_Reset_Manager

    // vector<GameImage*> layout (std::vector<GameImage*> triple):
    uint32_t      m_nVecPad;          // +8  (uninitialized garbage in orig)
    GameImage**   m_pVecBegin;        // +12 (index 3)
    GameImage**   m_pVecEnd;          // +16 (index 4)
    GameImage**   m_pVecEndCap;       // +20 (index 5)

    // 23 layer slots (indices 6..28 in dword units → bytes 24..115)
    LAYERINFO*    m_pLayers[23];      // +24..+115

    CANIMATIONINFO* m_pAnimationInfo; // +116 (index 29)

    // +120..+139 (dword indices 30..34)
    int           m_unk30;            // +120
    int           m_nAlpha;           // +124 (init 255)
    float         m_fPosX;            // +128 (accessed externally)
    float         m_fPosY;            // +132 (accessed externally)
    int           m_nJumpOffset;      // +136

    // +140 (byte 140) mirrored / transport-flag
    uint8_t       m_bMirrored;        // +140
    uint8_t       m_pad141[3];

    // +144..+159
    int           m_nMotionIndex;     // +144
    int           m_unk37;            // +148
    int           m_nCurFrame;        // +152 (start frame — low word = v3 in Process)
    int           m_nEndFrame;        // +156

    // +160..+223: 16 × (itemID_male, itemID_female) pairs
    // Indexed as m_ItemIDs[kind][slot] where slot ∈ {0,1}.
    uint16_t      m_ItemIDs[16][2];   // +160..+223

    // Embedded FrameSkip (vptr + accumulator + timePerFrame)
    CCAFrameSkip  m_FrameSkip;        // +224..+235

    // +236 byte
    uint8_t       m_cProcessedIdx;    // +236 (written as byte, read as int)
    uint8_t       m_pad237[3];

    // +240..+247
    CEffectBase** m_pEffectBefore;    // +240 (index 60)
    CEffectBase** m_pEffectAfter;     // +244 (index 61)

    // +248 low byte = sex of current character
    uint32_t      m_uCurSex;          // +248 (index 62; only low byte is used)

    // +252..+256
    int           m_nHairIndex;       // +252 (index 63)
    int           m_nFaceIndex;       // +256 (index 64)

    // +260..+275: normalized hair color (RGBA)
    float         m_fHairColor[4];    // +260..+275 (indices 65..68; all init 1.0)

    // +276 alpha-override float (used in the color-override branch of Process)
    float         m_fOverwriteAlpha;  // +276 (index 69, init 0.6f)

    int           m_nOverwriteFlag;   // +280 (index 70, init 0)
    int           m_nTransportActive; // +284 (index 71, init 0)
    int           m_nTransportMask;   // +288 (index 72, set to -1 or 0 in Process)
};

// Free function restored from mofclient.c (0042DE80)
unsigned char ExGetIllustCharSexCode(char charKind);
