#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"

// 每筆固定 20 bytes，對齊必須與反編譯碼一致
#pragma pack(push, 1)
struct strLessonKindInfo
{
    uint8_t  bClassType;         // Offset 0  : 課程種類
    uint8_t  bRankingClassType;  // Offset 1  : 排名用課程種類
    uint8_t  bLessonType;        // Offset 2  : 課程類型
    uint8_t  padding_0;          // Offset 3

    uint16_t wNameCode;          // Offset 4
    uint16_t wDescriptionCode;   // Offset 6
    uint16_t wPlaceCode;         // Offset 8

    uint8_t  padding_1[2];       // Offset 10~11

    uint32_t IconResourceId;     // Offset 12~15

    uint16_t wBlockId;           // Offset 16~17
    uint8_t  padding_2[2];       // Offset 18~19
};
#pragma pack(pop)

static_assert(sizeof(strLessonKindInfo) == 20, "strLessonKindInfo size must be 20 bytes");


class cltLessonKindInfo
{
public:
    cltLessonKindInfo();

    int Initialize(char* fileName);
    void Free();

    strLessonKindInfo* GetLessonKindInfo(uint8_t classType);
    strLessonKindInfo* GetLessonKindInfoByIndex(int index);

    int GetLessonKind(char* text);
    int GetLessonKindOfRanking(char* text);
    uint8_t GetLessonType(char* text);
    int IsValidLessonKind(uint8_t classType);

private:
    strLessonKindInfo* m_pInfo;
    int m_nCount;
};
