#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMapInfo.h"
#include "Info/cltExtraRegenMonsterKindInfo.h"

// 對應 mofclient.c:343045 等處 strWeddingHallKindInfo
// 結構大小 = 56 bytes（與反編譯 stride `v7 += 56` 一致）
// 各欄位 offset 由反編譯 *(_WORD *)(v7 + N) / *(_DWORD *)(v7 + N) 直接得到。
//
// 注意：txt 欄位順序為 ..., 부케id, 부케수량, 청첩장ID, 청첩장수량，
// 但記憶體順序為 ..., 청첩장(+48,+50), 부케(+52,+54)。
// 原始程式以 v24 = (_WORD*)(v7+50) 對 *(v24-1) 與 *v24 兩次回寫達成此倒序。
struct strWeddingHallKindInfo {
    uint16_t weddingKind;        // +0  웨딩id            / 婚禮 ID（W####；TranslateKindCode）
    uint16_t ticketItemKind;     // +2  결혼식이용권id    / 結婚式利用券 ID（I####；cltItemKindInfo::TranslateKindCode）
    uint16_t mapWeddingHall;     // +4  맵id(웨딩홀)      / 結婚禮堂 地圖 ID（A####；cltMapInfo::TranslateKindCode）
    uint16_t _pad0;              // +6  對齊填充（memset 為 0；GT 從未寫入此處）
    int32_t  hallX;              // +8  웨딩홀 x좌표      / 婚禮堂 X 座標（atoi）
    int32_t  hallY;              // +12 웨딩홀 y좌표      / 婚禮堂 Y 座標（atoi）
    uint16_t mapStudio;          // +16 맵id(스튜디오)    / 工作室 地圖 ID（cltMapInfo::TranslateKindCode）
    uint16_t _pad1;              // +18 對齊填充
    int32_t  studioX;            // +20 스튜디오 x좌표    / 工作室 X 座標
    int32_t  studioY;            // +24 스튜디오 y좌표    / 工作室 Y 座標
    uint16_t mapHoneymoon;       // +28 맵id(허니문)      / 蜜月 地圖 ID（cltMapInfo::TranslateKindCode）
    uint16_t _pad2;              // +30 對齊填充
    int32_t  honeymoonX;         // +32 허니문 x좌표      / 蜜月 X 座標
    int32_t  honeymoonY;         // +36 허니문 y좌표      / 蜜月 Y 座標
    uint16_t extraRegenMonster;  // +40 extraregenmonsterkind / 額外重生怪物 ID（F####；cltExtraRegenMonsterKindInfo::TranslateKindCode）
    uint16_t _pad3;              // +42 對齊填充
    int32_t  durationSec;        // +44 진행시간(秒)      / 進行時間（秒，atoi）
    uint16_t invitationItemKind; // +48 청첩장 ID         / 喜帖 物品 ID（I####；cltItemKindInfo::TranslateKindCode）
    uint16_t invitationCount;    // +50 청첩장 수량       / 喜帖 數量（atoi，新郎新婦各自分發）
    uint16_t bouquetItemKind;    // +52 부케 id           / 花束 物品 ID（I####；cltItemKindInfo::TranslateKindCode）
    uint16_t bouquetCount;       // +54 부케 수량         / 花束 數量（atoi）
};

class cltWeddingHallKindInfo {
public:
    cltWeddingHallKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltWeddingHallKindInfo() { Free(); }

    // 對應：int __thiscall Initialize(char* filename)  mofclient.c:343045
    int Initialize(char* filename);

    // 對應：void __thiscall Free()  mofclient.c:343227
    void Free();

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfo(uint16_t code)  mofclient.c:343238
    strWeddingHallKindInfo* GetWeddingHallKindInfo(uint16_t code);

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfoByItemKind(uint16_t itemKind)  mofclient.c:343259
    strWeddingHallKindInfo* GetWeddingHallKindInfoByItemKind(uint16_t itemKind);

    // 對應：strWeddingHallKindInfo* GetWeddingHallKindInfoByMapKind(uint16_t mapKind)  mofclient.c:343280
    strWeddingHallKindInfo* GetWeddingHallKindInfoByMapKind(uint16_t mapKind);

    // 對應：static uint16_t __cdecl TranslateKindCode(char* s)  mofclient.c:343301
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline const strWeddingHallKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strWeddingHallKindInfo* m_items;  // +0 (在 GT 為 *(_DWORD *)this)
    int                     m_count;  // +4 (在 GT 為 *((_DWORD *)this + 1))
};
