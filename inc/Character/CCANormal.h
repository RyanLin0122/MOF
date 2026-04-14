#pragma once
#include <cstdint>
#include "Character/CATypes.h"

class GameImage;

// =============================================================================
// CCANormal — restored from mofclient.c (0x0052C240 .. 0x0052D2A0)
//
// A self-contained per-instance CA player that OWNS its own timeline and
// image-pointer table.  Used by ClientCharacter to play non-character
// animations (*.ca files) such as portraits and effects.
//
// Layout follows the 32-bit binary (see constructor at 0x0052C240):
//   +0    vtable
//   +4    m_nKeyCount     (dword 1)
//   +8    m_pKeys         (dword 2)
//   +12   m_nTotalFrames  (dword 3)
//   +16   m_nVersion      (dword 4, init 12)
//   +20   m_header[256]   (file-loaded header block)
//   +276  m_nUnk276       (dword 69)
//   +280  m_nLayerCount   (dword 70)
//   +284  m_pLayers       (dword 71, owned LAYERINFO[])
//   +288  m_nUnk288       (dword 72)
//   +292  m_nUnk292       (dword 73)
//   +296  m_nUnk296       (dword 74)
//   +300  m_fPosX         (dword 75)
//   +304  m_fPosY         (dword 76)
//   +308  m_ucAlpha       (byte)
//   +312  m_ppImages      (dword 78, owned GameImage*[])
//   +316  m_nImageCount   (dword 79)
//   +320  m_nUnk320       (dword 80)
//   +324  m_bLoop         (byte)
//   +325  m_bPlaying      (byte)
//   +326  m_bVisible      (byte, init 1)
//   +328  m_nMotionIndex  (dword 82)
//   +332  m_nUnk332       (dword 83)
//   +336  m_nStartFrame   (dword 84)
//   +340  m_nEndFrame     (dword 85)
//   +344  m_bResetShader  (byte, init 1)
//   +345..+349  file-loaded shader blend params
// =============================================================================
class CCANormal
{
public:
    CCANormal();
    virtual ~CCANormal();

    void Reset();
    void LoadCAInPack(char* filename);
    void LoadCA(const char* filename);
    bool Process(uint16_t frameBias);
    bool Draw();
    void Play(int motion, bool loop);
    void Pause();

public:
    // ---- binary layout ----------------------------------------------------
    int             m_nKeyCount;         // +4
    CA_KEYINFO*     m_pKeys;             // +8
    int             m_nTotalFrames;      // +12
    int             m_nVersion;          // +16  (init 12)

    char            m_header[256];       // +20..+275  file-loaded header

    int             m_nUnk276;           // +276
    int             m_nLayerCount;       // +280
    LAYERINFO*      m_pLayers;           // +284  owned LAYERINFO[]
    int             m_nUnk288;           // +288
    int             m_nUnk292;           // +292
    int             m_nUnk296;           // +296

    float           m_fPosX;             // +300
    float           m_fPosY;             // +304

    uint8_t         m_ucAlpha;           // +308
    uint8_t         m_pad309[3];

    GameImage**     m_ppImages;          // +312  owned GameImage*[]
    int             m_nImageCount;       // +316
    int             m_nUnk320;           // +320

    uint8_t         m_bLoop;             // +324
    uint8_t         m_bPlaying;          // +325
    uint8_t         m_bVisible;          // +326  init 1
    uint8_t         m_pad327;

    int             m_nMotionIndex;      // +328
    int             m_nUnk332;           // +332
    int             m_nStartFrame;       // +336
    int             m_nEndFrame;         // +340

    uint8_t         m_bResetShader;      // +344  init 1
    uint8_t         m_ucShaderParam1;    // +345  (file-loaded)
    uint8_t         m_ucShaderParam2;    // +346  (file-loaded)
    uint8_t         m_ucShaderParam3;    // +347  (optional, if ucShaderParam1==8)
    uint8_t         m_ucShaderParam4;    // +348
    uint8_t         m_ucShaderParam5;    // +349
    uint8_t         m_pad350;            // +350
};
