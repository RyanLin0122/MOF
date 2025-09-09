#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMapInfo.h"
#include "Info/cltExtraRegenMonsterKindInfo.h"

// 結構總長 56 bytes；欄位排列對應反編譯中的固定偏移
struct strWeddingHallKindInfo {
    uint16_t weddingKind;        // +0  웨딩id -> TranslateKindCode
    uint16_t ticketItemKind;     // +2  결혼식이용권id -> cltItemKindInfo::TranslateKindCode
    uint16_t mapWeddingHall;     // +4  맵id(웨딩홀) -> cltMapInfo::TranslateKindCode
    int32_t  hallX;              // +8  (웨딩홀) X
    int32_t  hallY;              // +12 (웨딩홀) Y
    uint16_t mapStudio;          // +16 맵id(스튜디오) -> cltMapInfo::TranslateKindCode
    int32_t  studioX;            // +20 (스튜디오) X
    int32_t  studioY;            // +24 (스튜디오) Y
    uint16_t mapHoneymoon;       // +28 맵id(허니문) -> cltMapInfo::TranslateKindCode
    int32_t  honeymoonX;         // +32 (허니문) X
    int32_t  honeymoonY;         // +36 (허니문) Y
    uint16_t extraRegenMonster;  // +40 extraregenmonsterkind -> cltExtraRegenMonsterKindInfo::TranslateKindCode
    int32_t  durationSec;        // +44 진행시간(秒)
    uint16_t invitationItemKind; // +48 청첩장 ID -> cltItemKindInfo::TranslateKindCode
    uint16_t invitationCount;    // +50 청첩장 數量
    uint16_t bouquetItemKind;    // +52 부케 id -> cltItemKindInfo::TranslateKindCode
    uint16_t bouquetCount;       // +54 부케 數量
};

class cltWeddingHallKindInfo {
public:
    cltWeddingHallKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltWeddingHallKindInfo() { Free(); }

    // 對應：int __thiscall Initialize(char* filename)
    int Initialize(char* filename);

    // 對應：void __thiscall Free()
    void Free();

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfo(uint16_t code)
    strWeddingHallKindInfo* GetWeddingHallKindInfo(uint16_t code);

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfoByItemKind(uint16_t itemKind)
    strWeddingHallKindInfo* GetWeddingHallKindInfoByItemKind(uint16_t itemKind);

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfoByMapKind(uint16_t mapKind)
    strWeddingHallKindInfo* GetWeddingHallKindInfoByMapKind(uint16_t mapKind);

    // 對應：static uint16_t __cdecl TranslateKindCode(char* s)
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline const strWeddingHallKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strWeddingHallKindInfo* m_items;
    int                     m_count;
};