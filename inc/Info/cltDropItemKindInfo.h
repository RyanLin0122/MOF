#ifndef CLT_DROP_ITEM_KIND_INFO_H
#define CLT_DROP_ITEM_KIND_INFO_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include "Info/cltItemKindInfo.h"
#include "Info/cltMapInfo.h"
#include "Info/cltCharKindInfo.h"
#include "System/cltCountrySystem.h"


#pragma pack(push, 1)
struct strDropItemRareGroup {
    std::int32_t cumulativeProbability; // +0
    std::uint16_t itemKind[3];          // +4
    std::uint16_t padding0A;            // +10
    std::int32_t itemCount;             // +12
};
static_assert(sizeof(strDropItemRareGroup) == 16, "strDropItemRareGroup size mismatch");

struct strDropItemGenericGroup {
    std::int32_t probability;           // +0
    std::uint16_t itemKind;             // +4
    std::uint16_t padding06;            // +6
    std::int32_t minCount;              // +8
    std::int32_t maxCount;              // +12
};
static_assert(sizeof(strDropItemGenericGroup) == 16, "strDropItemGenericGroup size mismatch");

struct strDropItemKindInfo {
    std::uint16_t kindCode;                 // +0x00
    std::uint16_t padding02;                // +0x02
    std::int32_t rareDropProbability;       // +0x04
    strDropItemRareGroup rareGroups[5];     // +0x08 .. +0x57
    std::int32_t padding58;                 // +0x58
    strDropItemGenericGroup genericGroups[5]; // +0x5C .. +0xAB
    std::int32_t moneyDropProbability;      // +0xAC
    std::int32_t moneyDropAmount;           // +0xB0
    std::int32_t paddingB4;                 // +0xB4
};
static_assert(sizeof(strDropItemKindInfo) == 184, "strDropItemKindInfo size mismatch");

#pragma pack(pop)

class cltDropItemKindInfo {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo,
                                         cltCharKindInfo* charKindInfo,
                                         cltCountrySystem* countrySystem);

    cltDropItemKindInfo();

    int Initialize(char* fileName);
    void Free();

    int GetDropItemKindNum() const;
    strDropItemKindInfo* GetDropItemKindInfo(std::uint16_t kindCode) const;
    strDropItemKindInfo* GetDropItemKindInfoByIndex(int index) const;

    static std::uint16_t TranslateKindCode(char* text);

    int GenearteDropMoney(int monsterRank,
                          int probabilityScale,
                          double rareMultiplier,
                          int premiumBonusPerMille,
                          int partyBonusPercent,
                          int moneyBonusPercent,
                          int eventMoneyBonusFlag,
                          std::uint16_t dropKindCode) const;

    int GenerateRareDropItem(int playerLevel,
                             int probabilityScale,
                             double rareMultiplier,
                             int premiumBonusPerMille,
                             int eventBonusFlag,
                             std::uint16_t dropKindCode,
                             std::uint16_t* outItemKind,
                             std::uint16_t* outItemCount) const;

    int GenerateFieldDropItem(stMapInfo* mapInfo,
                              int playerLevel,
                              int probabilityScale,
                              double rareMultiplier,
                              int premiumBonusPerMille,
                              int eventBonusFlag,
                              std::uint16_t dropKindCode,
                              std::uint16_t* outItemKind,
                              std::uint16_t* outItemCount) const;

    int GenerateGenericDropItem(int playerLevel,
                                int probabilityScale,
                                double rareMultiplier,
                                int premiumBonusPerMille,
                                int eventBonusFlag,
                                std::uint16_t dropKindCode,
                                std::uint16_t* outItemKind,
                                std::uint16_t* outItemCount) const;

    int GenerateEventDropItem(int playerLevel,
                              int probabilityScale,
                              double rareMultiplier,
                              int premiumBonusPerMille,
                              int eventBonusFlag,
                              int eventDropKindCount,
                              std::uint16_t* eventDropKindList,
                              std::uint16_t* outItemKind,
                              std::uint16_t* outItemCount) const;

    int GenerateDropItem(stMapInfo* mapInfo,
                         int playerLevel,
                         int probabilityScale,
                         double rareMultiplier,
                         int premiumBonusPerMille,
                         int moneyBonusPercent,
                         int fieldMoneyBonusPercent,
                         int eventMoneyBonusFlag,
                         int eventDropKindCount,
                         std::uint16_t* eventDropKindList,
                         std::uint16_t dropKindCode,
                         std::uint16_t* outItemKind,
                         std::uint16_t* outItemCount,
                         int* outMoney) const;

    int GenerateDropItem(std::uint16_t dropKindCode,
                         std::uint16_t* outItemKind,
                         std::uint16_t* outItemCount,
                         int* outMoney) const;

private:
    static cltItemKindInfo* m_pclItemKindInfo;
    static cltCharKindInfo* m_pclCharKindInfo;
    static cltCountrySystem* m_pclCountrySystem;

    strDropItemKindInfo* m_pRecords;
    std::uint16_t m_recordCount;
    std::uint16_t padding06;
};

#if INTPTR_MAX == INT32_MAX
static_assert(sizeof(cltDropItemKindInfo) == 8, "cltDropItemKindInfo size mismatch on 32-bit build");
#endif

#endif // CLT_DROP_ITEM_KIND_INFO_H
