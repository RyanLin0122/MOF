#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#if defined(_MSC_VER)
#define STRCASECMP _stricmp
#else
#include <strings.h>
#define STRCASECMP strcasecmp
#endif

// 單筆資料，嚴格對齊 IDA offset / stride = 36
struct strMyItemKindInfo
{
    uint16_t wId;                         // 0
    uint16_t wNameTextCode;               // 2
    uint32_t dwExpAdv100;                 // 4
    uint32_t dwDropMoneyAmountAdv100;     // 8
    uint32_t dwIsPremiumQuickSlot;        // 12
    uint32_t dwSpouseDailySummonCharge;   // 16
    uint32_t dwDuplicate;                 // 20
    uint32_t ResourceId;                  // 24, 由 "%x" 讀入
    uint16_t wBlockNumber;                // 28
    uint16_t wEffectDescription;          // 30
    uint8_t  bType;                       // 32
    uint8_t  reserved[3];                 // 33~35，補足單筆 36 bytes
};

static_assert(offsetof(strMyItemKindInfo, wId) == 0, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, wNameTextCode) == 2, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, dwExpAdv100) == 4, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, dwDropMoneyAmountAdv100) == 8, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, dwIsPremiumQuickSlot) == 12, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, dwSpouseDailySummonCharge) == 16, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, dwDuplicate) == 20, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, ResourceId) == 24, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, wBlockNumber) == 28, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, wEffectDescription) == 30, "offset mismatch");
static_assert(offsetof(strMyItemKindInfo, bType) == 32, "offset mismatch");
static_assert(sizeof(strMyItemKindInfo) == 36, "size mismatch");


class cltMyItemKindInfo
{
public:
    cltMyItemKindInfo();
    ~cltMyItemKindInfo();

    int Initialize(char* filePath);
    void Free();

    strMyItemKindInfo* GetMyItemKindInfo(uint16_t kindCode);
    uint8_t GetMyItemType(uint16_t kindCode);

    static uint16_t TranslateKindCode(char* text);

private:
    strMyItemKindInfo* m_pInfo;
    int m_nCount;
};