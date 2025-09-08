#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"
#include "Info/cltMapInfo.h"
#include "Info/cltCharKindInfo.h"

// 與反編譯記憶體佈局一致：每筆 236 bytes
#pragma pack(push, 1)
struct strInstantDungeonMapKindInfo
{
    // 0x00
    uint16_t i_map_id;      // Dxxxx（本行地圖編號）
    uint16_t map_id;        // Ixxxx（實際地圖代碼）
    int32_t  x;             // 進入點 X
    int32_t  y;             // 進入點 Y

    // 0x0C ~ 0x1B：p1
    int32_t  p1_portaltype; // FIX=1, RANDOM=2, EXIT=3, 其他=0
    uint32_t p1_dest;       // Dxxxx
    int32_t  p1_x;
    int32_t  p1_y;

    // 0x1C ~ 0x2B：p2
    int32_t  p2_portaltype;
    uint32_t p2_dest;       // Dxxxx 或 0
    int32_t  p2_x;
    int32_t  p2_y;

    // 0x2C ~ 0x53：m1
    uint16_t m1_mob_id;     // Jxxxx
    uint16_t _pad2C{ 0 };
    int32_t  m1_initnum;
    int32_t  m1_maxnum;
    int32_t  m1_x;
    int32_t  m1_y;
    int32_t  m1_width;      // = m1_x + (檔案給的寬度增量)
    int32_t  m1_height;     // = m1_y + (檔案給的高度增量)
    int32_t  m1_regentime;  // (ms)

    // 0x4C ~ 0x73：m2
    uint16_t m2_mob_id;     // Jxxxx
    uint16_t _pad4C{ 0 };
    int32_t  m2_initnum;
    int32_t  m2_maxnum;
    int32_t  m2_x;
    int32_t  m2_y;
    int32_t  m2_width;      // = m2_x + delta
    int32_t  m2_height;     // = m2_y + delta
    int32_t  m2_regentime;

    // 0x6C ~ 0x93：m3
    uint16_t m3_mob_id;     // Jxxxx
    uint16_t _pad6C{ 0 };
    int32_t  m3_initnum;
    int32_t  m3_maxnum;
    int32_t  m3_x;
    int32_t  m3_y;
    int32_t  m3_width;      // = m3_x + delta
    int32_t  m3_height;     // = m3_y + delta
    int32_t  m3_regentime;

    // 0x8C ~ 0xB3：m4
    uint16_t m4_mob_id;     // Jxxxx
    uint16_t _pad8C{ 0 };
    int32_t  m4_initnum;
    int32_t  m4_maxnum;
    int32_t  m4_x;
    int32_t  m4_y;
    int32_t  m4_width;      // = m4_x + delta
    int32_t  m4_height;     // = m4_y + delta
    int32_t  m4_regentime;

    // 0xAC ~ 0xC3：r1（稀有怪組1）
    uint16_t r1_mob_id;     // Jxxxx
    uint16_t _padAC{ 0 };
    int32_t  r1_initnum;
    int32_t  r1_maxnum;
    int32_t  r1_x;
    int32_t  r1_y;
    int32_t  r1_width;      // = m1_x + delta（照反編譯：參照 m1_x/m1_y）
    int32_t  r1_height;     // = m1_y + delta
    int32_t  r1_regentime;

    // 0xC4 ~ 0xDB：r2（稀有怪組2）
    uint16_t r2_mob_id;     // Jxxxx
    uint16_t _padC4{ 0 };
    int32_t  r2_initnum;
    int32_t  r2_maxnum;
    int32_t  r2_x;
    int32_t  r2_y;
    int32_t  r2_width;      // = m2_x + delta（照反編譯：參照 m2_x/m2_y）
    int32_t  r2_height;     // = m2_y + delta
    int32_t  r2_regentime;
};
#pragma pack(pop)

static_assert(sizeof(strInstantDungeonMapKindInfo) == 236, "strInstantDungeonMapKindInfo must be 236 bytes");

class cltInstantDungeonMapKindInfo
{
public:
    cltInstantDungeonMapKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltInstantDungeonMapKindInfo() { Free(); }

    // 成功完整讀畢→1；中途失敗→0；若資料行數=0 也回傳 1（與反編譯一致）
    int Initialize(char* filename);
    void Free();

    // 查找
    strInstantDungeonMapKindInfo* GetInstantDungeonMapKindInfo(uint16_t kind);

    // 工具
    static uint16_t TranslateKindCode(char* s); // 5 碼 → 16-bit
    static int GetIndunPortalType(char* s);     // "FIX"/"RANDOM"/"EXIT" → 1/2/3

    // 便利
    inline int Count() const { return m_count; }
    inline const strInstantDungeonMapKindInfo* Data() const { return m_list; }

private:
    strInstantDungeonMapKindInfo* m_list;
    int                           m_count;
};
