#pragma once
#include <cstdint>
#include <d3d9.h>
#include <d3dx9.h>
#include "Character/CATypes.h"

class GameImage;
class CCA;

// =============================================================================
// CCAillust — restored from mofclient.c (0x00528BA0 .. 0x00529A10)
//
// Illustration-portrait variant of CCA.  Instead of per-frame animation it
// holds exactly one FRAMEINFO* per layer slot (16 slots), rebuilt from
// CAManager::GetTimeLineInfoBaseIllust() in LoadCA().  Process() walks those
// slots, resolves each CA_DRAWENTRY via cltImageManager::GetGameImage, and
// publishes them into an internal vector<GameImage*>.  Draw() iterates the
// list and renders every GameImage with an ID3DXSprite batch.
//
// The 32-bit binary places the class in exactly 112 bytes:
//   +0    vptr
//   +4    ID3DXSprite*
//   +8    std::vector<GameImage*> triple (_first/_last/_end)
//   +24   FRAMEINFO*   m_pFrameSlots[16]
//   +88   float        m_fPosX
//   +92   float        m_fPosY
//   +96   float        m_fHairColor[4]  (RGBA, init 1.0)
// Our x64 build uses named members and tolerates the extra padding from
// 8-byte pointers.
// =============================================================================
class CCAillust
{
public:
    CCAillust();
    virtual ~CCAillust();

    void Reset();
    void LoadCA(const char* path);
    void Process();
    void Draw(int viewport);

    // Equipment helpers
    void SetItemID(uint16_t itemID, uint8_t sex, uint8_t age, bool putOn,
                   int hairExtra, int faceExtra);
    void SetHairDefine(uint16_t hairIdx, uint8_t sex, uint8_t age);
    void SetFaceDefine(uint16_t faceIdx, uint8_t sex, uint8_t age);
    void SetItemtoDot(CCA* pSource, uint8_t sex, uint8_t age,
                      int hairExtra, int faceExtra);
    void InitItem(uint8_t sex, uint8_t age, uint16_t hairIdx, uint16_t faceIdx,
                  uint32_t hairColor);

    // Low-level layer manipulation
    void LayerPutOn(uint16_t itemID, uint8_t sex, uint8_t age);
    void LayerPutOff(uint16_t kind, uint8_t sex, uint8_t age,
                     int hairExtra, int faceExtra);
    void SetLayerByItemKind(uint16_t itemID, int putOn, uint8_t sex, uint8_t age,
                            int hairExtra, int faceExtra);

    // Hair color
    void SetHairColor(uint32_t color);
    void SetDefaultHairColor();
    void GetHairColor(float* out);

public:
    // +0   vptr (implicit via virtual dtor)
    void*        m_pSprite;          // +4   ID3DXSprite* from CDeviceResetManager

    // vector<GameImage*> triple {begin, end, endCap}
    uint32_t     m_nVecPad;          // +8   (uninitialized garbage in orig)
    GameImage**  m_pVecBegin;        // +12
    GameImage**  m_pVecEnd;          // +16
    GameImage**  m_pVecEndCap;       // +20

    // 16 layer slots.  Each entry points at the first (and only) FRAMEINFO of
    // a LAYERINFO owned by CAManager's illust timelines.  LoadCA seeds them
    // from GetTimeLineInfoBaseIllust(); LayerPutOn/LayerPutOff rebind them to
    // per-item FRAMEINFOs via CAManager::GetIllustFrame.
    FRAMEINFO*   m_pFrameSlots[16];  // +24..+87

    float        m_fPosX;            // +88
    float        m_fPosY;            // +92
    float        m_fHairColor[4];    // +96..+111
};
