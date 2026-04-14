#pragma once
#include <cstdint>
#include <cstddef>

// =============================================================================
// Support types for CCA / CAManager (restored from mofclient.c decompiled layout)
// =============================================================================

// ---------------------------------------------------------------------------
// CA_DRAWENTRY (32 bytes): the per-frame draw record stored inside each
// FRAMEINFO.m_pEntries1 array.  Loaded from file and initialized with defaults
// {imageID:0, block:0, offset(0,0), alpha:255, rot:0, scale:100, flip:0, 0}.
// ---------------------------------------------------------------------------
struct CA_DRAWENTRY
{
    uint32_t m_dwImageID;  // +0
    uint16_t m_wBlockID;   // +4
    uint16_t m_wPad6;      // +6
    float    m_fOffsetX;   // +8
    float    m_fOffsetY;   // +12
    int      m_nAlpha;     // +16  (init 255)
    int      m_nRotation;  // +20
    int      m_nScale;     // +24  (init 100)
    uint8_t  m_ucFlip;     // +28
    uint8_t  m_ucFlag29;   // +29
    uint8_t  m_pad30[2];   // +30..+31
};
static_assert(sizeof(CA_DRAWENTRY) == 32, "CA_DRAWENTRY must be 32 bytes");

// NOTE: the original binary is 32-bit x86; this port builds as x64.  All byte
// sizes / offsets in the comments below describe the on-disk binary layout,
// not the in-memory layout — pointers on x64 are 8 bytes, so every struct
// containing pointers is a few bytes larger here.  We do NOT enforce the
// original sizes with static_assert; no external code indexes these types
// by raw byte arithmetic in our port.

// ---------------------------------------------------------------------------
// FRAMEINFO (original: 20 bytes / 0x14)
// ---------------------------------------------------------------------------
struct FRAMEINFO
{
    int    m_nCount1;     // +0
    void*  m_pEntries1;   // CA_DRAWENTRY[m_nCount1]
    int    m_nEnd;
    int    m_nCount2;
    void*  m_pEntries2;   // 102-byte entries (secondary)

    FRAMEINFO();
    ~FRAMEINFO() {}
};

// ---------------------------------------------------------------------------
// LAYERINFO (original: 64 bytes / 0x40)
// ---------------------------------------------------------------------------
struct LAYERINFO
{
    char       m_header[52];   // file-loaded parameters
    int        m_nFrameCount;
    FRAMEINFO* m_pFrames;
    int        m_nShow;        // init 1

    LAYERINFO();
    ~LAYERINFO();

    static int IsShow(LAYERINFO* self) { return self->m_nShow; }
};

// ---------------------------------------------------------------------------
// CA_KEYINFO (original: 28 bytes / 0x1C)
// Named with a CA_ prefix to avoid collision with CCAEffect.h's KEYINFO.
// ---------------------------------------------------------------------------
struct CA_KEYINFO
{
    char m_szName[20];
    int  m_nStartFrame;    // original offset +20
    int  m_nEndFrame;      // original offset +24

    CA_KEYINFO();
    ~CA_KEYINFO() {}
};

// ---------------------------------------------------------------------------
// TIMELINEINFO (original: 284 bytes / 0x11C)
// ---------------------------------------------------------------------------
struct TIMELINEINFO
{
    int        m_nVersion;        // init 12
    char       m_header[256];
    int        m_nUnk260;
    int        m_nLayerCount;
    LAYERINFO* m_pLayers;
    int        m_nUnk272;
    int        m_nUnk276;
    int        m_nUnk280;

    TIMELINEINFO();
    ~TIMELINEINFO();
};

// ---------------------------------------------------------------------------
// CANIMATIONINFO (original: ~12 bytes)
// ---------------------------------------------------------------------------
struct CANIMATIONINFO
{
    int         m_nKeyCount;
    CA_KEYINFO* m_pKeys;
    int         m_nTotalFrames;   // used by CCA::LoadCA to derive 1/fps

    CANIMATIONINFO() : m_nKeyCount(0), m_pKeys(nullptr), m_nTotalFrames(0) {}
    ~CANIMATIONINFO();
};

// ---------------------------------------------------------------------------
// ITEMCAINFO_DOT (original: 120 bytes / 0x78)
// ---------------------------------------------------------------------------
struct ITEMCAINFO_DOT
{
    uint16_t m_wItemID;
    uint16_t m_wPad2;
    int      m_nType;           // fashion type from ParsingFashionItemType
    char     m_szFileName[100];
    int      m_nLayerIndex;     // base layer index
    uint8_t  m_ucItemKind;      // LayerPutOn switch key
    uint8_t  m_pad113[3];
    int      m_nSpecial;        // low word used by GetItemSpecial

    ITEMCAINFO_DOT();
    ~ITEMCAINFO_DOT() {}
};

// ---------------------------------------------------------------------------
// ITEMCAINFO_ILLUST (original: 12 bytes / 0xC)
// ---------------------------------------------------------------------------
struct ITEMCAINFO_ILLUST
{
    uint16_t m_wItemID;
    uint8_t  m_uc2;
    uint8_t  m_pad3;
    int      m_nValue;
    uint8_t  m_ucKind;
    uint8_t  m_pad9[3];

    ITEMCAINFO_ILLUST();
    ~ITEMCAINFO_ILLUST() {}
};

// ---------------------------------------------------------------------------
// ILLUST_CADATA_INFO (100 bytes / 0x64) — holds a file name string
// ---------------------------------------------------------------------------
struct ILLUST_CADATA_INFO
{
    char m_szFileName[100];

    ILLUST_CADATA_INFO() { m_szFileName[0] = 0; }
    ~ILLUST_CADATA_INFO() {}
};

// ---------------------------------------------------------------------------
// CCAFrameSkip — private FrameSkip subobject embedded into CCA (vtable + two
// floats, with m_fTimePerFrame defaulting to 1/60 s).
// ---------------------------------------------------------------------------
class CCAFrameSkip
{
public:
    CCAFrameSkip() : m_fAccumulatedTime(0.0f), m_fTimePerFrame(1.0f / 60.0f) {}
    virtual ~CCAFrameSkip() {}

    float m_fAccumulatedTime;   // +4
    float m_fTimePerFrame;      // +8
};
