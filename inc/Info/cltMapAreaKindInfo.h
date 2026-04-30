#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "Info/cltMapInfo.h"

// ────────────────────────────────────────────────────────────────────────
// strMapAreaKindInfo  (=== 20 bytes ===)
//
// text_dump/mapareakindinfo.txt 之每筆紀錄結構。
// 標頭：「필드아이템박스 맵 정보」(field-item-box map info, 田野道具箱地圖資訊)
// 欄位 (file column header)：
//   ID    mapkind    x    y    width    heigth   (檔案內欄名拼為 "heigth")
//
// 反編譯寫入點 (mofclient.c:310226-310249)：
//   v9 +  0  WORD  areaKind   = cltMapAreaKindInfo::TranslateKindCode(col0)
//   v9 +  2  WORD  mapKind    = cltMapInfo::TranslateKindCode(col1)
//   v9 +  4  DWORD x          = atoi(col2)
//   v9 +  8  DWORD y          = atoi(col3)
//   v9 + 12  DWORD width      = atoi(col4)
//   v9 + 16  DWORD height     = atoi(col5)
// 步距：v9 += 20 / 紀錄；總長 20 bytes。
// ────────────────────────────────────────────────────────────────────────
#pragma pack(push, 1)
struct strMapAreaKindInfo {
    uint16_t areaKind;  // +0   區역ID / 區域ID    (필드아이템박스 ID, 如 B0001)
                        //      用 cltMapAreaKindInfo::TranslateKindCode 翻譯
    uint16_t mapKind;   // +2   맵 ID / 地圖ID     (如 M0027 / D0020 / C0016 / A0006)
                        //      用 cltMapInfo::TranslateKindCode 翻譯；0 視為 parse 失敗
    int32_t  x;         // +4   영역 좌상단 X / 區域左上角 X 座標
    int32_t  y;         // +8   영역 좌상단 Y / 區域左上角 Y 座標
    int32_t  width;     // +12  영역 폭 / 區域寬
    int32_t  height;    // +16  영역 높이 / 區域高 (檔案欄名拼為 "heigth")
};
#pragma pack(pop)

static_assert(sizeof(strMapAreaKindInfo) == 20, "strMapAreaKindInfo must be 20 bytes");

// ────────────────────────────────────────────────────────────────────────
// cltMapAreaKindInfo
//   필드아이템박스 맵 영역 정보 / 田野道具箱地圖區域資訊管理器。
//   啟動流程於 cltMapInfo::Initialize 結尾載入 (mofclient.c:310172)。
//   m_items 為連續 20-byte 陣列；m_count 為預掃所得行數。
// 物件 layout (反編譯依 *((_DWORD *)this + N) 推得)：
//   +0  m_items  (strMapAreaKindInfo*)
//   +4  m_count  (int)
// ────────────────────────────────────────────────────────────────────────
class cltMapAreaKindInfo {
public:
    cltMapAreaKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltMapAreaKindInfo() { Free(); }

    // 反編譯：int __thiscall Initialize(this, char* filename)  @ 0x57973C
    int  Initialize(char* filename);

    // 反編譯：void __thiscall Free(this)                       @ 0x5798F0
    void Free();

    // 반편역：strMapAreaKindInfo* GetMapAreaKindInfo(this, u16) @ 0x579970
    strMapAreaKindInfo* GetMapAreaKindInfo(uint16_t code);

    // 反編譯：static u16 __cdecl TranslateKindCode(char* s)     @ 0x579920
    static uint16_t TranslateKindCode(char* s);

    // 便利 accessor (非反編譯成員)
    inline const strMapAreaKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strMapAreaKindInfo* m_items;  // +0  20-byte 連續陣列基底
    int                 m_count;  // +4  預掃所得資料列數
};
