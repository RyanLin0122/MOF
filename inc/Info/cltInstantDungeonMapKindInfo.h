#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"
#include "Info/cltMapInfo.h"
#include "Info/cltCharKindInfo.h"

// ==================================================================
// strInstantDungeonMapKindInfo
//   來源檔：indunmapkindinfo.txt（리스트: 인스턴스 던전 맵 정보）
//   反編譯位置：mofclient.c:303942 cltInstantDungeonMapKindInfo::Initialize
//   每筆 sizeof = 236 bytes（operator new(236 * count)）
//   配置時整片以 memset 清零；padding 與未填欄位全為 0
// ==================================================================
#pragma pack(push, 1)
struct strInstantDungeonMapKindInfo
{
    // ---- 入口資訊 (0x00 .. 0x0B) ----
    uint16_t i_map_id;          // [0x00] 韓: i_map_id (인스턴트 던전 맵 ID, Dxxxx) / 中: 副本入口地圖 ID
    uint16_t map_id;            // [0x02] 韓: map_id (실제 맵 ID, Ixxxx) / 中: 對應的實際地圖 ID
    int32_t  x;                 // [0x04] 韓: x (입장 X) / 中: 進入點 X 座標
    int32_t  y;                 // [0x08] 韓: y (입장 Y) / 中: 進入點 Y 座標

    // ---- 傳送門 #1 (0x0C .. 0x1B, 16 bytes) ----
    int32_t  p1_portaltype;     // [0x0C] 韓: p1_portaltype (포탈1 타입; FIX=1, RANDOM=2, EXIT=3, 그외=0) / 中: 傳送門 1 型別
    uint32_t p1_dest;           // [0x10] 韓: p1_dest (목적지 Dxxxx) / 中: 傳送門 1 目的地 (TranslateKindCode 16-bit 結果零擴展為 32-bit 寫入)
    int32_t  p1_x;              // [0x14] 韓: p1_x (포탈1 X) / 中: 傳送門 1 X
    int32_t  p1_y;              // [0x18] 韓: p1_y (포탈1 Y) / 中: 傳送門 1 Y

    // ---- 傳送門 #2 (0x1C .. 0x2B, 16 bytes) ----
    int32_t  p2_portaltype;     // [0x1C] 韓: p2_portaltype / 中: 傳送門 2 型別
    uint32_t p2_dest;           // [0x20] 韓: p2_dest / 中: 傳送門 2 目的地
    int32_t  p2_x;              // [0x24] 韓: p2_x / 中: 傳送門 2 X
    int32_t  p2_y;              // [0x28] 韓: p2_y / 中: 傳送門 2 Y

    // ---- 一般怪物群 #1 (0x2C .. 0x4B, 32 bytes) ----
    uint16_t m1_mob_id;         // [0x2C] 韓: m1_mob_id (몬스터1 ID, Jxxxx) / 中: 怪物 1 種類 ID (cltCharKindInfo::TranslateKindCode)
    uint16_t _pad2E{ 0 };       // [0x2E] 對齊用 padding，反編譯中由 memset 清為 0
    int32_t  m1_initnum;        // [0x30] 韓: m1_initnum (초기 리젠 수) / 中: 怪物 1 初始數量
    int32_t  m1_maxnum;         // [0x34] 韓: m1_maxnum (최대 리젠 수) / 中: 怪物 1 最大數量
    int32_t  m1_x;              // [0x38] 韓: m1_x (리젠 영역 X) / 中: 怪物 1 重生區左上 X
    int32_t  m1_y;              // [0x3C] 韓: m1_y (리젠 영역 Y) / 中: 怪物 1 重生區左上 Y
    int32_t  m1_width;          // [0x40] 韓: m1_width (= m1_x + 入力된 너비) / 中: 怪物 1 重生區右下 X (= x + 寬度增量)
    int32_t  m1_height;         // [0x44] 韓: m1_height (= m1_y + 入力된 높이) / 中: 怪物 1 重生區右下 Y (= y + 高度增量)
    int32_t  m1_regentime;      // [0x48] 韓: m1_regentime (리젠 간격, ms) / 中: 怪物 1 重生間隔 (毫秒)

    // ---- 一般怪物群 #2 (0x4C .. 0x6B, 32 bytes) ----
    uint16_t m2_mob_id;         // [0x4C] 韓: m2_mob_id / 中: 怪物 2 種類 ID
    uint16_t _pad4E{ 0 };       // [0x4E] padding
    int32_t  m2_initnum;        // [0x50] 韓: m2_initnum / 中: 怪物 2 初始數量
    int32_t  m2_maxnum;         // [0x54] 韓: m2_maxnum / 中: 怪物 2 最大數量
    int32_t  m2_x;              // [0x58] 韓: m2_x / 中: 怪物 2 重生區左上 X
    int32_t  m2_y;              // [0x5C] 韓: m2_y / 中: 怪物 2 重生區左上 Y
    int32_t  m2_width;          // [0x60] 韓: m2_width (= m2_x + 너비) / 中: 怪物 2 重生區右下 X
    int32_t  m2_height;         // [0x64] 韓: m2_height (= m2_y + 높이) / 中: 怪物 2 重生區右下 Y
    int32_t  m2_regentime;      // [0x68] 韓: m2_regentime / 中: 怪物 2 重生間隔

    // ---- 一般怪物群 #3 (0x6C .. 0x8B, 32 bytes) ----
    uint16_t m3_mob_id;         // [0x6C] 韓: m3_mob_id / 中: 怪物 3 種類 ID
    uint16_t _pad6E{ 0 };       // [0x6E] padding
    int32_t  m3_initnum;        // [0x70] 韓: m3_initnum / 中: 怪物 3 初始數量
    int32_t  m3_maxnum;         // [0x74] 韓: m3_maxnum / 中: 怪物 3 最大數量
    int32_t  m3_x;              // [0x78] 韓: m3_x / 中: 怪物 3 重生區左上 X
    int32_t  m3_y;              // [0x7C] 韓: m3_y / 中: 怪物 3 重生區左上 Y
    int32_t  m3_width;          // [0x80] 韓: m3_width (= m3_x + 너비) / 中: 怪物 3 重生區右下 X
    int32_t  m3_height;         // [0x84] 韓: m3_height (= m3_y + 높이) / 中: 怪物 3 重生區右下 Y
    int32_t  m3_regentime;      // [0x88] 韓: m3_regentime / 中: 怪物 3 重生間隔

    // ---- 一般怪物群 #4 (0x8C .. 0xAB, 32 bytes) ----
    uint16_t m4_mob_id;         // [0x8C] 韓: m4_mob_id / 中: 怪物 4 種類 ID
    uint16_t _pad8E{ 0 };       // [0x8E] padding
    int32_t  m4_initnum;        // [0x90] 韓: m4_initnum / 中: 怪物 4 初始數量
    int32_t  m4_maxnum;         // [0x94] 韓: m4_maxnum / 中: 怪物 4 最大數量
    int32_t  m4_x;              // [0x98] 韓: m4_x / 中: 怪物 4 重生區左上 X
    int32_t  m4_y;              // [0x9C] 韓: m4_y / 中: 怪物 4 重生區左上 Y
    int32_t  m4_width;          // [0xA0] 韓: m4_width (= m4_x + 너비) / 中: 怪物 4 重生區右下 X
    int32_t  m4_height;         // [0xA4] 韓: m4_height (= m4_y + 높이) / 中: 怪物 4 重生區右下 Y
    int32_t  m4_regentime;      // [0xA8] 韓: m4_regentime / 中: 怪物 4 重生間隔

    // ---- 稀有怪物群 #1 (0xAC .. 0xCB, 32 bytes) ----
    // 重要：r1 的 width / height 由 m1_x / m1_y 為基準計算（disasm: *(v31-30) 指向 m1_x、*(v31-29) 指向 m1_y）
    uint16_t r1_mob_id;         // [0xAC] 韓: r1_mob_id (희귀 몬스터1 ID) / 中: 稀有怪 1 種類 ID
    uint16_t _padAE{ 0 };       // [0xAE] padding
    int32_t  r1_initnum;        // [0xB0] 韓: r1_initnum / 中: 稀有怪 1 初始數量
    int32_t  r1_maxnum;         // [0xB4] 韓: r1_maxnum / 中: 稀有怪 1 最大數量
    int32_t  r1_x;              // [0xB8] 韓: r1_x / 中: 稀有怪 1 重生區左上 X
    int32_t  r1_y;              // [0xBC] 韓: r1_y / 中: 稀有怪 1 重生區左上 Y
    int32_t  r1_width;          // [0xC0] 韓: r1_width (= m1_x + 너비) / 中: 稀有怪 1 重生區右下 X (參考 m1_x)
    int32_t  r1_height;         // [0xC4] 韓: r1_height (= m1_y + 높이) / 中: 稀有怪 1 重生區右下 Y (參考 m1_y)
    int32_t  r1_regentime;      // [0xC8] 韓: r1_regentime / 中: 稀有怪 1 重生間隔

    // ---- 稀有怪物群 #2 (0xCC .. 0xEB, 32 bytes) ----
    // 同理：r2 的 width / height 由 m2_x / m2_y 為基準計算
    uint16_t r2_mob_id;         // [0xCC] 韓: r2_mob_id / 中: 稀有怪 2 種類 ID
    uint16_t _padCE{ 0 };       // [0xCE] padding
    int32_t  r2_initnum;        // [0xD0] 韓: r2_initnum / 中: 稀有怪 2 初始數量
    int32_t  r2_maxnum;         // [0xD4] 韓: r2_maxnum / 中: 稀有怪 2 最大數量
    int32_t  r2_x;              // [0xD8] 韓: r2_x / 中: 稀有怪 2 重生區左上 X
    int32_t  r2_y;              // [0xDC] 韓: r2_y / 中: 稀有怪 2 重生區左上 Y
    int32_t  r2_width;          // [0xE0] 韓: r2_width (= m2_x + 너비) / 中: 稀有怪 2 重生區右下 X (參考 m2_x)
    int32_t  r2_height;         // [0xE4] 韓: r2_height (= m2_y + 높이) / 中: 稀有怪 2 重生區右下 Y (參考 m2_y)
    int32_t  r2_regentime;      // [0xE8] 韓: r2_regentime / 中: 稀有怪 2 重生間隔
};
#pragma pack(pop)

static_assert(sizeof(strInstantDungeonMapKindInfo) == 236, "strInstantDungeonMapKindInfo must be 236 bytes");

class cltInstantDungeonMapKindInfo
{
public:
    cltInstantDungeonMapKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltInstantDungeonMapKindInfo() { Free(); }

    // 解析 indunmapkindinfo.txt
    //   讀畢全部資料行 → 1
    //   m_count == 0（檔案無資料）→ 1（與 disasm 一致）
    //   解析中途任一 strtok 失敗 → 0（disasm 不釋放，等待 dtor）
    int Initialize(char* filename);
    void Free();

    // 線性掃描 m_list，比對 i_map_id 與 a2 是否相等
    strInstantDungeonMapKindInfo* GetInstantDungeonMapKindInfo(uint16_t kind);

    // 5 字元 KindCode → 16-bit ID（與 cltCharKindInfo / cltMapInfo 同型）
    static uint16_t TranslateKindCode(char* s);

    // "FIX"=1, "RANDOM"=2, "EXIT"=3, 其他=0
    static int GetIndunPortalType(char* s);

    inline int Count() const { return m_count; }
    inline const strInstantDungeonMapKindInfo* Data() const { return m_list; }

private:
    strInstantDungeonMapKindInfo* m_list;   // [+0x00] 反編譯：*(_DWORD *)this
    int                           m_count;  // [+0x04] 反編譯：*((_DWORD *)this + 1)
};
