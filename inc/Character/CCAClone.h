#pragma once
#include <cstdint>
#include "Character/CATypes.h"

class GameImage;

// =============================================================================
// CCAClone — restored from mofclient.c (0x00527700 .. 0x00528560)
//
// A lightweight "clone" of CCA that reuses an external LAYERINFO* table
// (typically the one owned by a CCA instance, fetched via CCA::GetLayerInfo).
// The clone maintains its own animation state (motion, frame, accumulator)
// and rebuilds a vector<GameImage*> for rendering each tick.
//
// The original binary lays the class out in exactly 84 bytes; our x64 port
// uses named fields and tolerates the extra padding from 8-byte pointers.
// =============================================================================
class CCAClone
{
public:
    CCAClone();
    virtual ~CCAClone();

    void Reset();
    void SetLayerInfo(LAYERINFO** layers, char sex, int hairIdx, int faceIdx);
    void Process();
    void Draw();
    void Play(int motion, bool pause);
    void Pause();
    void SetHairColor(unsigned int color);
    void SetDefaultHairColor();
    void GetHairColor(float* out);
    bool findGameImageInList(GameImage* target);

public:
    // ---- binary layout (see constructor in mofclient.c at 0x00527700) -----
    // +0   vptr (implicit via virtual dtor)
    void*           m_pSprite;          // +4   ID3DXSprite* (lazy-init in Process)

    uint32_t        m_nVecPad;          // +8   (uninitialized garbage in orig)
    GameImage**     m_pVecBegin;        // +12
    GameImage**     m_pVecEnd;          // +16
    GameImage**     m_pVecEndCap;       // +20

    LAYERINFO**     m_ppLayers;         // +24  external 23-slot layer table
    CANIMATIONINFO* m_pAnimationInfo;   // +28  from CAManager::GetAnimationInfoBaseDot

    float           m_fPosX;            // +32
    float           m_fPosY;            // +36

    uint8_t         m_bMirrored;        // +40
    uint8_t         m_bPlaying;         // +41
    uint8_t         m_bVisible;         // +42  init 1
    uint8_t         m_pad43;

    int             m_nMotionIndex;     // +44
    int             m_nCurFrame;        // +48  (low word used as frame base)
    float           m_fAccumFrame;      // +52

    uint8_t         m_ucSex;            // +56  low byte of dword 14
    uint8_t         m_pad57[3];

    int             m_nHairIndex;       // +60
    int             m_nFaceIndex;       // +64

    // Normalized hair color RGBA (all init to 1.0f)
    float           m_fHairColor[4];    // +68..+80
};
