#include "Character/CAManager.h"
#include "Character/CATypes.h"
#include "Image/cltImageManager.h"
#include "Text/cltTextFileManager.h"
#include "FileSystem/CMOFPacking.h"
#include "global.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <new>
#include <windows.h>

// ============================================================================
// Helpers for type constructors (match original 0-initialization semantics).
// ============================================================================

FRAMEINFO::FRAMEINFO()
    : m_nCount1(0), m_pEntries1(nullptr), m_nEnd(0), m_nCount2(0), m_pEntries2(nullptr) {}

LAYERINFO::LAYERINFO()
    : m_nFrameCount(0), m_pFrames(nullptr), m_nShow(1)
{
    std::memset(m_header, 0, sizeof(m_header));
}

LAYERINFO::~LAYERINFO() {}

CA_KEYINFO::CA_KEYINFO()
    : m_nStartFrame(0), m_nEndFrame(0)
{
    std::memset(m_szName, 0, sizeof(m_szName));
}

TIMELINEINFO::TIMELINEINFO()
    : m_nVersion(12)
    , m_nUnk260(0)
    , m_nLayerCount(0)
    , m_pLayers(nullptr)
    , m_nUnk272(0)
    , m_nUnk276(0)
    , m_nUnk280(0)
{
    std::memset(m_header, 0, sizeof(m_header));
}

TIMELINEINFO::~TIMELINEINFO()
{
    if (m_pLayers)
    {
        delete[] m_pLayers;
        m_pLayers = nullptr;
    }
}

CANIMATIONINFO::~CANIMATIONINFO()
{
    if (m_pKeys)
    {
        delete[] m_pKeys;
        m_pKeys = nullptr;
    }
}

ITEMCAINFO_DOT::ITEMCAINFO_DOT()
    : m_wItemID(0), m_wPad2(0), m_nType(0), m_nLayerIndex(0), m_ucItemKind(0), m_nSpecial(0)
{
    std::memset(m_szFileName, 0, sizeof(m_szFileName));
    std::memset(m_pad113, 0, sizeof(m_pad113));
}

ITEMCAINFO_ILLUST::ITEMCAINFO_ILLUST()
    : m_wItemID(0), m_uc2(0), m_pad3(0), m_nValue(0), m_ucKind(0)
{
    std::memset(m_pad9, 0, sizeof(m_pad9));
}

// ============================================================================
// CAManager constructor — initialize hair/face definition tables with the
// identical filenames / layer-indices strings mofclient.c sets up.
// ============================================================================
CAManager::CAManager()
{
    // Seed the per-slot "definition" entries.  The original binary populates
    // 9 "hair definition" slots and 12 "face definition" slots with hardcoded
    // base indices and filenames; we replicate that here.
    struct HairInit { int slot; int layerIdx; };
    static const HairInit kHairInit[] = {
        {0,  0}, {1,  2}, {2,  4}, {3,  6}, {4,  8}, {5, 10},
        {6, 28}, {7, 30}
    };
    for (const auto& e : kHairInit)
    {
        ITEMCAINFO_DOT& rec = m_DefineInfoDot[e.slot];
        rec.m_wItemID     = 1;
        rec.m_nType       = 0;
        std::strncpy(rec.m_szFileName, "Dot_Character_HAIR.ca", sizeof(rec.m_szFileName) - 1);
        rec.m_nLayerIndex = e.layerIdx;
        rec.m_ucItemKind  = 0;
    }

    struct FaceInit { int slot; int layerIdx; };
    static const FaceInit kFaceInit[] = {
        {0,  0}, {1,  1}, {2,  2}, {3,  3}, {4,  4}, {5,  5},
        {6,  8}, {7,  9}, {8,  6}, {9,  7}, {10, 10}, {11, 11}, {12, 12}, {13, 13}
    };
    for (const auto& e : kFaceInit)
    {
        ITEMCAINFO_DOT& rec = m_DefineInfoDot[30 + e.slot];
        rec.m_wItemID     = 1;
        rec.m_nType       = 1;
        std::strncpy(rec.m_szFileName, "Dot_Character_FACE.ca", sizeof(rec.m_szFileName) - 1);
        rec.m_nLayerIndex = e.layerIdx;
        rec.m_ucItemKind  = 1;
    }

    // Zero the item-id → pointer lookup tables.
    std::memset(m_pItemCAInfoDot,    0, sizeof(m_pItemCAInfoDot));
    std::memset(m_pItemCAInfoIllust, 0, sizeof(m_pItemCAInfoIllust));

    // Illust CA path defaults.
    static const char* const kIllustPaths[] = {
        "MoFData/Character/Illust_Character_Child_Female.ca",
        "MoFData/Character/Illust_Character_Teen_Female.ca",
        "MoFData/Character/Illust_Character_Adult_Female.ca",
        "MoFData/Character/Illust_Character_Child_Male.ca",
        "MoFData/Character/Illust_Character_Teen_Male.ca",
        "MoFData/Character/Illust_Character_Adult_Male.ca"
    };
    for (int i = 0; i < 6; ++i)
        std::strncpy(m_IllustPaths[i].m_szFileName, kIllustPaths[i], sizeof(m_IllustPaths[i].m_szFileName) - 1);

    // Seed the illust hair/face definition tables.
    struct IllustHair { int slot; int val; uint8_t kind; };
    static const IllustHair kHairIllustInit[] = {
        {0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {3, 7, 0}
    };
    for (const auto& e : kHairIllustInit)
    {
        ITEMCAINFO_ILLUST& rec = m_DefineInfoIllust[e.slot];
        rec.m_wItemID = 1;
        rec.m_nValue  = e.val;
        rec.m_ucKind  = e.kind;
    }
    struct IllustFace { int slot; int val; uint8_t kind; };
    static const IllustFace kFaceIllustInit[] = {
        {0, 0, 1}, {1, 1, 1}, {2, 2, 1}, {3, 4, 1}, {4, 3, 1}, {5, 5, 1}, {6, 6, 1}
    };
    for (const auto& e : kFaceIllustInit)
    {
        ITEMCAINFO_ILLUST& rec = m_DefineInfoIllust[30 + e.slot];
        rec.m_wItemID = 1;
        rec.m_nValue  = e.val;
        rec.m_ucKind  = e.kind;
    }
}

// ============================================================================
// Destructor — release heap-allocated item-info entries.
// ============================================================================
CAManager::~CAManager()
{
    for (int i = 0; i < 65535; ++i)
    {
        if (m_pItemCAInfoDot[i])
        {
            delete m_pItemCAInfoDot[i];
            m_pItemCAInfoDot[i] = nullptr;
        }
    }
    for (int i = 0; i < 65535; ++i)
    {
        if (m_pItemCAInfoIllust[i])
        {
            delete m_pItemCAInfoIllust[i];
            m_pItemCAInfoIllust[i] = nullptr;
        }
    }
}

// ============================================================================
// LoadCADataDot (0x0052A950)
// ============================================================================
FILE* CAManager::LoadCADataDot(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return nullptr;

    char buffer[356];
    char nameBuf[8];
    char typeA[32];
    char typeB[32];
    char imgFile[256];
    int  fieldA = 0;
    int  fieldB = 0;

    std::fgets(buffer, 256, fp);   // header line
    while (std::fgets(buffer, 256, fp))
    {
        std::sscanf(buffer, "%s%s%s%d%s%d", nameBuf, typeA, imgFile, &fieldA, typeB, &fieldB);
        uint16_t itemID = ItemtoWORD(nameBuf);
        if (!m_pItemCAInfoDot[itemID])
        {
            ITEMCAINFO_DOT* pRec = new ITEMCAINFO_DOT();
            m_pItemCAInfoDot[itemID] = pRec;
            pRec->m_wItemID = itemID;

            int typeAResult = ParsingFashionItemType(typeA);
            if (typeAResult == -1) { g_clTextFileManager.fclose(fp); return nullptr; }
            pRec->m_nType        = typeAResult;
            pRec->m_nLayerIndex  = fieldA;

            int typeBResult = ParsingFashionItemType(typeB);
            if (typeBResult == -1) { g_clTextFileManager.fclose(fp); return nullptr; }
            pRec->m_ucItemKind = static_cast<uint8_t>(typeBResult);
            pRec->m_nSpecial   = fieldB;
        }
    }
    g_clTextFileManager.fclose(fp);

    // Load per-kind timelines.
    static const char* const kDotTlFiles[16] = {
        "MoFData/Character/Dot_Character_HAIR.ca",
        "MoFData/Character/Dot_Character_FACE.ca",
        "MoFData/Character/Dot_Character_COAT.ca",
        "MoFData/Character/Dot_Character_TRIUSERS.ca",
        "MoFData/Character/Dot_Character_SHOES.ca",
        "MoFData/Character/Dot_Character_HAND.ca",
        "MoFData/Character/Dot_Character_CLOCK.ca",
        "MoFData/Character/Dot_Character_RIGHTHANDWEAPON.ca",
        "MoFData/Character/Dot_Character_LEFTHANDWEAPON.ca",
        "MoFData/Character/Dot_Character_ACCESSORY1.ca",
        "MoFData/Character/Dot_Character_ACCESSORY2.ca",
        "MoFData/Character/Dot_Character_ACCESSORY3.ca",
        "MoFData/Character/Dot_Character_MASKHAIR.ca",
        "MoFData/Character/Dot_Character_SUIT.ca",
        "MoFData/Character/Dot_Character_EMOTICON.ca",
        "MoFData/Character/Dot_Character_DUALWEAPON.ca"
    };
    // Mofclient uses dword_829254 as the packed-vs-direct flag; default to packed.
    const bool usePacked = true;
    if (usePacked)
    {
        for (int k = 0; k < 16; ++k)
            LoadTimelineInPack(kDotTlFiles[k], &m_TimelineInfoDot[k], nullptr);
        LoadTimelineInPack("MoFData/Character/Dot_Character.ca", &m_TimelineDotBase, &m_AnimationInfoDot);
    }
    else
    {
        for (int k = 0; k < 16; ++k)
            LoadTimeline(kDotTlFiles[k], &m_TimelineInfoDot[k], nullptr);
        LoadTimeline("MoFData/Character/Dot_Character.ca", &m_TimelineDotBase, &m_AnimationInfoDot);
    }

    LoadDefinImage();
    return reinterpret_cast<FILE*>(1);
}

// ============================================================================
// LoadCADataIllust (0x0052ADB0)
// ============================================================================
int CAManager::LoadCADataIllust(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp)
    {
        ::MessageBoxA(nullptr, "Illust Character Animation infomation file is not found.", "", 0);
        return 0;
    }

    char buffer[256];
    char nameBuf[8];
    char kindStr[32];
    char byteField = 0;
    int  valField  = 0;

    std::fgets(buffer, 256, fp);   // header
    while (std::fgets(buffer, 256, fp))
    {
        std::sscanf(buffer, "%s%d%d%s", nameBuf, &byteField, &valField, kindStr);
        uint16_t itemID = ItemtoWORD(nameBuf);

        ITEMCAINFO_ILLUST* pRec = new ITEMCAINFO_ILLUST();
        m_pItemCAInfoIllust[itemID] = pRec;
        pRec->m_wItemID = itemID;
        pRec->m_uc2     = static_cast<uint8_t>(byteField);
        pRec->m_nValue  = valField;

        int kindResult = ParsingFashionItemType(kindStr);
        if (kindResult == -1) { g_clTextFileManager.fclose(fp); return 0; }
        pRec->m_ucKind = static_cast<uint8_t>(kindResult);
    }
    g_clTextFileManager.fclose(fp);

    const bool usePacked = true;
    if (usePacked)
    {
        for (int i = 0; i < 6; ++i)
            LoadTimelineInPack(m_IllustPaths[i].m_szFileName, &m_TimelineInfoIllust[i], nullptr);
        LoadTimelineInPack("MoFData/Character/Illust_Character.ca", &m_TimelineIllustBase, nullptr);
    }
    else
    {
        for (int i = 0; i < 6; ++i)
            LoadTimeline(m_IllustPaths[i].m_szFileName, &m_TimelineInfoIllust[i], nullptr);
        LoadTimeline("MoFData/Character/Illust_Character.ca", &m_TimelineIllustBase, nullptr);
    }
    return 1;
}

// ============================================================================
// ParsingFashionItemType (0x0052B070) — string → item kind
// ============================================================================
int CAManager::ParsingFashionItemType(const char* s)
{
    struct Entry { const char* name; int kind; };
    static const Entry kTbl[] = {
        {"HAIR",       0}, {"FACE",        1}, {"COAT",        2},
        {"TRIUSERS",   3}, {"SHOES",       4}, {"HAND",        5},
        {"CLOCK",      6}, {"RWEAPON",     7}, {"LWEAPON",     8},
        {"ACC1",       9}, {"ACC2",       10}, {"ACC3",       11},
        {"MASKHAIR",  12}, {"SUIT",       13}, {"EMOTICON",   14},
        {"DUALWEAPON",15}
    };
    for (const auto& e : kTbl)
        if (_stricmp(s, e.name) == 0) return e.kind;
    return -1;
}

// ============================================================================
// LoadTimelineInPack (0x0052B210)
// Reads a .ca file from the background-load buffer that was populated by
// CMofPacking::FileReadBackGroundLoading.  The buffer layout is identical to
// the on-disk format, so we reuse a shared in-memory cursor.
// ============================================================================
namespace
{
    struct MemCursor
    {
        const uint8_t* base;
        size_t         pos;

        template <typename T> T Read()
        {
            T out;
            std::memcpy(&out, base + pos, sizeof(T));
            pos += sizeof(T);
            return out;
        }
        void Read(void* dst, size_t n)
        {
            std::memcpy(dst, base + pos, n);
            pos += n;
        }
    };

    // Populate a single TIMELINEINFO from an in-memory or file cursor.
    // kind == 0 → memory cursor, kind == 1 → FILE*.
    template <typename Src>
    void ReadLayerData(TIMELINEINFO* out, Src& src, CANIMATIONINFO* anim)
    {
        // +0..+3: m_nVersion, +4..+259: header, +260..+280: six ints.
        // Original reads are:
        //   read 4 bytes into this+0
        //   read 0xFF into this+4
        //   read 4 bytes into this+260, +272, +276, +280, +264
        src.Read(&out->m_nVersion, 4);
        src.Read(out->m_header, 255);
        src.Read(&out->m_nUnk260, 4);
        src.Read(&out->m_nUnk272, 4);
        src.Read(&out->m_nUnk276, 4);
        src.Read(&out->m_nUnk280, 4);
        src.Read(&out->m_nLayerCount, 4);

        const int layerCount = out->m_nLayerCount;
        out->m_pLayers = (layerCount > 0) ? new LAYERINFO[layerCount] : nullptr;

        for (int li = 0; li < layerCount; ++li)
        {
            LAYERINFO& layer = out->m_pLayers[li];
            src.Read(layer.m_header, 50);
            src.Read(&layer.m_nFrameCount, 4);

            const int fc = layer.m_nFrameCount;
            layer.m_pFrames = (fc > 0) ? new FRAMEINFO[fc]() : nullptr;

            for (int fi = 0; fi < fc; ++fi)
            {
                FRAMEINFO& frame = layer.m_pFrames[fi];
                src.Read(&frame.m_nCount1, 4);

                if (frame.m_nCount1 > 0)
                {
                    CA_DRAWENTRY* pEntries = new CA_DRAWENTRY[frame.m_nCount1]();
                    for (int j = 0; j < frame.m_nCount1; ++j)
                    {
                        pEntries[j].m_dwImageID = 0;
                        pEntries[j].m_wBlockID  = 0;
                        pEntries[j].m_fOffsetX  = 0.0f;
                        pEntries[j].m_fOffsetY  = 0.0f;
                        pEntries[j].m_nAlpha    = 255;
                        pEntries[j].m_nRotation = 0;
                        pEntries[j].m_nScale    = 100;
                        pEntries[j].m_ucFlip    = 0;
                        pEntries[j].m_ucFlag29  = 0;
                    }
                    frame.m_pEntries1 = pEntries;
                    for (int j = 0; j < frame.m_nCount1; ++j)
                        src.Read(pEntries + j, 32);
                }

                src.Read(&frame.m_nCount2, 4);
                if (frame.m_nCount2 > 0)
                {
                    void* pSub = ::operator new(static_cast<size_t>(frame.m_nCount2) * 102);
                    std::memset(pSub, 0, static_cast<size_t>(frame.m_nCount2) * 102);
                    frame.m_pEntries2 = pSub;
                    for (int j = 0; j < frame.m_nCount2; ++j)
                        src.Read(static_cast<char*>(pSub) + 102 * j, 102);
                }
                src.Read(&frame.m_nEnd, 4);
            }
        }

        if (anim)
        {
            src.Read(&anim->m_nTotalFrames, 4);
            src.Read(&anim->m_nKeyCount,    4);
            if (anim->m_nKeyCount > 0)
            {
                anim->m_pKeys = new CA_KEYINFO[anim->m_nKeyCount]();
                for (int k = 0; k < anim->m_nKeyCount; ++k)
                    src.Read(anim->m_pKeys + k, 28);
            }
        }
    }
}

int CAManager::LoadTimelineInPack(const char* filename, TIMELINEINFO* out, CANIMATIONINFO* anim)
{
    if (!filename || !out) return 0;
    CMofPacking* pack = CMofPacking::GetInstance();
    if (!pack) return 0;

    char localPath[256];
    std::strncpy(localPath, filename, sizeof(localPath) - 1);
    localPath[sizeof(localPath) - 1] = 0;

    char* changed = pack->ChangeString(localPath);
    pack->FileReadBackGroundLoading(changed);

    const uint8_t* src = reinterpret_cast<const uint8_t*>(pack->m_backgroundLoadBufferField);
    if (!src)
    {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s CA didn't find.", filename);
        ::MessageBoxA(nullptr, buf, nullptr, 0);
        return 0;
    }

    MemCursor cursor{ src, 0 };
    ReadLayerData(out, cursor, anim);
    return 1;
}

// ============================================================================
// LoadTimeline (0x0052B760) — direct file read
// ============================================================================
namespace
{
    struct FileCursor
    {
        FILE* fp;
        template <typename T> T Read()
        {
            T out;
            std::fread(&out, sizeof(T), 1, fp);
            return out;
        }
        void Read(void* dst, size_t n)
        {
            std::fread(dst, n, 1, fp);
        }
    };
}

int CAManager::LoadTimeline(const char* filename, TIMELINEINFO* out, CANIMATIONINFO* anim)
{
    FILE* fp = std::fopen(filename, "rb");
    if (!fp)
    {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s CA didn't find.", filename);
        ::MessageBoxA(nullptr, buf, nullptr, 0);
        return 0;
    }
    FileCursor cursor{ fp };
    ReadLayerData(out, cursor, anim);
    std::fclose(fp);
    return 1;
}

// ============================================================================
// ItemtoWORD (0x0052BC60)
// ============================================================================
uint16_t CAManager::ItemtoWORD(const char* a2)
{
    if (!a2 || std::strlen(a2) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a2[0])) + 31) << 11;
    uint16_t num = static_cast<uint16_t>(std::atoi(a2 + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

// ============================================================================
// Accessors (0x0052BCC0 ..)
// ============================================================================
uint16_t CAManager::GetItemID(uint16_t itemIndex)
{
    ITEMCAINFO_DOT* p = m_pItemCAInfoDot[itemIndex];
    return p ? p->m_wItemID : 0;
}

uint8_t CAManager::GetItemKind(uint16_t itemIndex)
{
    ITEMCAINFO_DOT* p = m_pItemCAInfoDot[itemIndex];
    return p ? p->m_ucItemKind : static_cast<uint8_t>(0xFFu);
}

uint16_t CAManager::GetItemSpecial(uint16_t itemIndex)
{
    ITEMCAINFO_DOT* p = m_pItemCAInfoDot[itemIndex];
    return p ? static_cast<uint16_t>(p->m_nSpecial) : 0;
}

LAYERINFO* CAManager::GetDotLayer(int kind, int index)
{
    if (index < 0 || kind < 0 || kind >= 16) return nullptr;
    TIMELINEINFO& tl = m_TimelineInfoDot[kind];
    if (index >= tl.m_nLayerCount) return nullptr;
    return tl.m_pLayers ? &tl.m_pLayers[index] : nullptr;
}

ITEMCAINFO_DOT* CAManager::GetItemCAInfoDot(uint16_t itemIndex)
{
    if (itemIndex == 0xFFFF) return nullptr;
    return m_pItemCAInfoDot[itemIndex];
}

int CAManager::GetHairLayerIndexDot(int hairIdx, uint8_t sex)
{
    int slot = sex + 2 * hairIdx;
    if (slot <= 29) return m_DefineInfoDot[slot].m_nLayerIndex;
    return -1;
}

int CAManager::GetFaceLayerIndexDot(int faceIdx, uint8_t sex)
{
    int slot = sex + 2 * faceIdx;
    if (slot <= 29) return m_DefineInfoDot[30 + slot].m_nLayerIndex;
    return -1;
}

FRAMEINFO* CAManager::GetIllustFrame(int kind, int layerIdx, int frameIdx)
{
    if (kind < 0 || kind >= 9) return nullptr;
    TIMELINEINFO& tl = m_TimelineInfoIllust[kind];
    if (layerIdx >= tl.m_nLayerCount) return nullptr;
    LAYERINFO* pLayer = &tl.m_pLayers[layerIdx];
    if (frameIdx < pLayer->m_nFrameCount) return &pLayer->m_pFrames[frameIdx];
    return nullptr;
}

ITEMCAINFO_ILLUST* CAManager::GetItemCAInfoIllust(uint16_t itemIndex)
{
    return m_pItemCAInfoIllust[itemIndex];
}

int CAManager::GetHairFrameIndexIllust(int hairIdx, uint8_t /*sex*/)
{
    if (hairIdx < 0 || hairIdx >= 30) return 0;
    return m_DefineInfoIllust[hairIdx].m_nValue;
}

int CAManager::GetFaceFrameIndexIllust(int faceIdx, uint8_t /*sex*/)
{
    if (faceIdx < 0 || faceIdx >= 30) return 0;
    return m_DefineInfoIllust[30 + faceIdx].m_nValue;
}

TIMELINEINFO* CAManager::GetTimeLineInfoBaseDot()      { return &m_TimelineDotBase; }
TIMELINEINFO* CAManager::GetTimeLineInfoBaseIllust()   { return &m_TimelineIllustBase; }
CANIMATIONINFO* CAManager::GetAnimationInfoBaseDot()   { return &m_AnimationInfoDot; }

// ============================================================================
// LoadDefinImage (0x0052BF00) — pre-warm GameImage cache for each hair/face
// definition so that the initial frame has no loading spike.
// ============================================================================
void CAManager::LoadDefinImage()
{
    cltImageManager* pIM = cltImageManager::GetInstance();
    if (!pIM) return;

    for (int i = 0; i < 30; ++i)
    {
        const int hairIdx = m_DefineInfoDot[i].m_nLayerIndex;
        LAYERINFO* pLayer0 = GetDotLayer(0, hairIdx);
        if (!pLayer0 || !pLayer0->m_pFrames) continue;

        auto* pDraw = static_cast<CA_DRAWENTRY*>(pLayer0->m_pFrames[0].m_pEntries1);
        if (pDraw) pIM->GetGameImage(0, pDraw->m_dwImageID, 0, 0);

        LAYERINFO* pLayer1 = GetDotLayer(0, hairIdx + 1);
        if (pLayer1 && pLayer1->m_pFrames)
        {
            pDraw = static_cast<CA_DRAWENTRY*>(pLayer1->m_pFrames[0].m_pEntries1);
            if (pDraw) pIM->GetGameImage(0, pDraw->m_dwImageID, 0, 0);
        }

        const int faceIdx = m_DefineInfoDot[30 + i].m_nLayerIndex;
        LAYERINFO* pLayerFace = GetDotLayer(1, faceIdx);
        if (pLayerFace && pLayerFace->m_pFrames)
        {
            pDraw = static_cast<CA_DRAWENTRY*>(pLayerFace->m_pFrames[0].m_pEntries1);
            if (pDraw) pIM->GetGameImage(0, pDraw->m_dwImageID, 0, 0);
        }
    }

    // Pre-warm illustration base-timeline frames (6 kinds × pair of layers)
    for (int kind = 0; kind < 6; ++kind)
    {
        TIMELINEINFO& tl = m_TimelineInfoIllust[kind];
        if (tl.m_pLayers && tl.m_nLayerCount > 2)
        {
            for (int li : { 0, 2 })
            {
                LAYERINFO& layer = tl.m_pLayers[li];
                if (layer.m_pFrames && layer.m_nFrameCount > 0)
                {
                    auto* pDraw = static_cast<CA_DRAWENTRY*>(layer.m_pFrames[0].m_pEntries1);
                    if (pDraw) pIM->GetGameImage(0, pDraw->m_dwImageID, 0, 0);
                }
            }
        }
    }
}
