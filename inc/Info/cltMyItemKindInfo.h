#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"

#pragma pack(push, 1)
// 與反編譯位移一致：每筆 36 bytes
struct strMyItemKindInfo
{
    // +0
    uint16_t kind;               // ID（5碼→16bit）

    // +2
    uint16_t name_text_code;     // NAMETEXTCODE

    // +4
    int32_t  exp_adv_100;        // EXPADV(100)

    // +8
    int32_t  dropmoney_adv_100;  // DROPMONEYAMOUNTADV(100)

    // +12
    int32_t  premium_quickslot;  // 프리미엄퀵슬롯여부(0/1)

    // +16
    int32_t  spouse_daily_recall;// 배우자 일일 소환 충전

    // +20
    int32_t  duplicate_flag;     // DUPLICATE：T→1, F→0

    // +24
    uint32_t resource_id_hex;    // 리소스아이디（十六進位字串）

    // +28
    uint16_t block_no;           // 블럭넘버

    // +30
    uint16_t effect_text_code;   // 효과설명 텍스트코드

    // +32
    uint8_t  type;               // EXP=1, LIBI=2, PSLOT=3, ITEMDROP=4, STATUS=5,
    // FASTMOVE=6, EMBLEM=7, WAYPOINT=8, RECALL=9

// +33~35：保留，僅為對齊（反編譯未使用）
    uint8_t  _pad33{ 0 };
    uint16_t _pad34{ 0 };
};
#pragma pack(pop)

static_assert(sizeof(strMyItemKindInfo) == 36, "strMyItemKindInfo must be 36 bytes");

class cltMyItemKindInfo
{
public:
    cltMyItemKindInfo() : m_list(nullptr), m_count(0) {}
    ~cltMyItemKindInfo() { Free(); }

    // 成功完整讀畢回傳 1；否則 0（無資料行時亦回傳 1，與反編譯一致）
    int Initialize(char* filename);
    void Free();

    // 查詢
    strMyItemKindInfo* GetMyItemKindInfo(uint16_t kind);
    uint8_t            GetMyItemType(uint16_t kind); // 若找不到回傳 0

    // 工具
    static uint16_t TranslateKindCode(char* s); // 5碼→16bit

private:
    static uint8_t ParseType(const char* s) {
        if (!s) return 0;
        if (_stricmp(s, "EXP") == 0) return 1;
        if (_stricmp(s, "LIBI") == 0) return 2;
        if (_stricmp(s, "PSLOT") == 0) return 3;
        if (_stricmp(s, "ITEMDROP") == 0) return 4;
        if (_stricmp(s, "STATUS") == 0) return 5;
        if (_stricmp(s, "FASTMOVE") == 0) return 6;
        if (_stricmp(s, "EMBLEM") == 0) return 7;
        if (_stricmp(s, "WAYPOINT") == 0) return 8;
        if (_stricmp(s, "RECALL") == 0) return 9;
        return 0;
    }

private:
    strMyItemKindInfo* m_list;
    int                m_count;
};
