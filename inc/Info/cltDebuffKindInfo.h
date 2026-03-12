#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "Text/cltTextFileManager.h"
#include "global.h"

// 與反編譯對齊：資料大小必須是 72 bytes
#pragma pack(push, 1)
struct strDebuffKindInfo
{
    uint16_t wId;                 // Offset: 0
    uint8_t  padding_0[2];        // Offset: 2
    uint32_t dwDuration;          // Offset: 4
    uint32_t dwBossDuration;      // Offset: 8
    uint32_t dwRepeatCycle;       // Offset: 12
    uint32_t dwBossRepeatCycle;   // Offset: 16
    uint32_t dwDamageType;        // Offset: 20
    uint32_t dwDamageParam1;      // Offset: 24
    uint32_t dwDamageParam2;      // Offset: 28
    uint32_t dwDamageParam3;      // Offset: 32
    uint32_t dwDamageParam4;      // Offset: 36
    uint8_t  padding_1[24];       // Offset: 40
    uint16_t wMonsterTop1;        // Offset: 64
    uint16_t wMonsterTop2;        // Offset: 66
    uint16_t wMonsterBot1;        // Offset: 68
    uint16_t wMonsterBot2;        // Offset: 70
};
#pragma pack(pop)
static_assert(sizeof(strDebuffKindInfo) == 72, "strDebuffKindInfo must be 72 bytes");


class cltDebuffKindInfo
{
public:
    cltDebuffKindInfo();
    ~cltDebuffKindInfo();

    // 依照反編譯：成功完整讀取回傳 1，否則 0
    int Initialize(char* fileName);
    void Free();

    strDebuffKindInfo* GetDebuffKindInfo(uint16_t id);
    static uint16_t TranslateKindCode(char* text);

private:
    strDebuffKindInfo* m_pInfo;
    int              m_nCount;
};