#pragma once
#include <cstdint>
#include <cstdio>
#include "Character/CATypes.h"

// =============================================================================
// CAManager (restored from mofclient.c)
//
// Stores parsed .ca binary animation data for both dot-art (in-world sprite)
// characters and illustration (UI portrait) characters, plus the lookup
// tables that map item-ID → ITEMCAINFO / sex+hair index → layer index.
//
// CAManager is an application-level singleton: mofclient.c holds a single
// global `g_CAManager` of exactly this type; we mirror that and expose a
// matching `g_CAManager` extern in global.h.
// =============================================================================
class CAManager
{
public:
    CAManager();
    ~CAManager();

    // ----- loaders (text manifest + binary timeline files) ---------------
    FILE* LoadCADataDot(char* filename);
    int   LoadCADataIllust(char* filename);
    int   ParsingFashionItemType(const char* name);
    int   LoadTimelineInPack(const char* filename, TIMELINEINFO* out, CANIMATIONINFO* anim);
    int   LoadTimeline(const char* filename, TIMELINEINFO* out, CANIMATIONINFO* anim);
    void  LoadDefinImage();

    // ----- parsing helpers -----------------------------------------------
    uint16_t ItemtoWORD(const char* name);

    // ----- item-table accessors ------------------------------------------
    uint16_t GetItemID(uint16_t itemIndex);
    uint8_t  GetItemKind(uint16_t itemIndex);
    uint16_t GetItemSpecial(uint16_t itemIndex);

    // ----- layer / frame accessors ---------------------------------------
    LAYERINFO*         GetDotLayer(int kind, int index);
    ITEMCAINFO_DOT*    GetItemCAInfoDot(uint16_t itemIndex);
    int                GetHairLayerIndexDot(int hairIdx, uint8_t sex);
    int                GetFaceLayerIndexDot(int faceIdx, uint8_t sex);

    FRAMEINFO*         GetIllustFrame(int kind, int layerIdx, int frameIdx);
    ITEMCAINFO_ILLUST* GetItemCAInfoIllust(uint16_t itemIndex);
    int                GetHairFrameIndexIllust(int hairIdx, uint8_t sex);
    int                GetFaceFrameIndexIllust(int faceIdx, uint8_t sex);

    TIMELINEINFO*      GetTimeLineInfoBaseDot();
    TIMELINEINFO*      GetTimeLineInfoBaseIllust();
    CANIMATIONINFO*    GetAnimationInfoBaseDot();

public:
    // --- definition tables for characters ---
    // Constructor creates 30 "hair definition" entries and 30 "face definition"
    // entries.  The hair block carries filename "Dot_Character_HAIR.ca" and
    // increments m_nLayerIndex by 2 every 2 slots; the face block mirrors the
    // face-kind layout.  These are accessed via byte-index arithmetic in the
    // original (`this + 30*v3 + 27` etc.), but because every member is
    // contiguous here, index k simply maps to m_DefineInfoDot[k].
    ITEMCAINFO_DOT     m_DefineInfoDot[60];   // [0..29]=hair, [30..59]=face

    // ItemID → ITEMCAINFO_DOT* (one pointer per possible 16-bit item ID).
    // Populated by LoadCADataDot; looked up via GetItemCAInfoDot / GetItemKind.
    ITEMCAINFO_DOT*    m_pItemCAInfoDot[65535];

    // 16 per-kind timeline infos (HAIR, FACE, COAT, ... , DUALWEAPON).
    // Original: 35 slots; we keep 35 to match the binary's vector allocation.
    TIMELINEINFO       m_TimelineInfoDot[35];

    // Illustration definition tables (same layout as dot versions).
    ITEMCAINFO_ILLUST  m_DefineInfoIllust[60]; // [0..29]=hair, [30..59]=face

    // ItemID → ITEMCAINFO_ILLUST*.
    ITEMCAINFO_ILLUST* m_pItemCAInfoIllust[65535];

    // 35 illustration timelines (up to 9 in use per sex/age + 1 base).
    TIMELINEINFO       m_TimelineInfoIllust[35];

    // 9 filename buffers for illustration sprites (child/teen/adult × M/F).
    ILLUST_CADATA_INFO m_IllustPaths[9];

    // Singleton per-character-style timelines shared between sexes.
    TIMELINEINFO       m_TimelineIllustBase;  // GetTimeLineInfoBaseIllust()
    TIMELINEINFO       m_TimelineDotBase;     // GetTimeLineInfoBaseDot()
    CANIMATIONINFO     m_AnimationInfoDot;    // GetAnimationInfoBaseDot()
};
