#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"
#include "Info/cltInstantDungeonMapKindInfo.h"

#pragma pack(push, 1)
struct strInstantDungeonKindInfo
{
    std::uint16_t wIndunType;                    // 0
    std::uint16_t wBaseIndunId;                 // 2
    std::uint16_t wIndunNameTextCode;           // 4
    std::uint8_t  bLevelLimit;                  // 6
    std::uint8_t  padding_0[1];                 // 7
    std::uint16_t wItemIdForGeneration;         // 8
    std::uint16_t wNpcId;                       // 10
    std::uint16_t wBossMonsterId;               // 12
    std::uint16_t wRandomMonsterSpawn;          // 14
    std::uint16_t wSlowMonsterId;               // 16
    std::uint16_t wSlowMonsterProbability;      // 18
    std::uint16_t wPowerManMonsterId;           // 20
    std::uint16_t wPowerManMonsterProbability;  // 22
    std::uint16_t wIndunGuideTextCode;          // 24
    std::uint8_t  padding_1[2];                 // 26
    std::uint32_t dwTimeLimit;                  // 28
    std::uint32_t LoadingResourceId;            // 32
    std::uint16_t wMap1Id;                      // 36
    std::uint16_t wMap2Id;                      // 38
    std::uint16_t wMap3Id;                      // 40
    std::uint16_t wMap4Id;                      // 42
};
#pragma pack(pop)

static_assert(sizeof(strInstantDungeonKindInfo) == 44, "strInstantDungeonKindInfo size must be 44 bytes");

class cltInstantDungeonKindInfo
{
public:
    cltInstantDungeonKindInfo();
    ~cltInstantDungeonKindInfo();

    int Initialize(char* fileName);
    void Free();

    strInstantDungeonKindInfo* GetInstantDungeonKindInfo(std::uint16_t kind);
    strInstantDungeonKindInfo* GetInstantDungeonKindInfoByItem(std::uint16_t itemKind);
    std::uint16_t GetInstantDungeonKindByItem(std::uint16_t itemKind);
    static std::uint16_t TranslateKindCode(char* text);
    strInstantDungeonKindInfo* GetInstantDungeonKindInfoByIndex(int index);

    int GetCount() const { return m_count; }
    const strInstantDungeonKindInfo* GetData() const { return m_data; }

private:
    strInstantDungeonKindInfo* m_data;
    int m_count;
};

bool IsAlphaNumeric(const char* text);
