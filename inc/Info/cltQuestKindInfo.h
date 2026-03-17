#pragma once

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <unordered_set>
#include "System/cltLevelSystem.h"


#pragma pack(push, 1)
struct stQuestKindInfo {
    std::uint16_t IsFig(std::uint16_t classCode) const;
    std::uint16_t IsARC(std::uint16_t classCode) const;
    std::uint16_t IsMAG(std::uint16_t classCode) const;
    std::uint16_t IsCLA(std::uint16_t classCode) const;
    std::uint16_t GetRewardItem(std::uint16_t classCode, char groupIndex) const;
    bool CanAcceptQuestByClass(std::uint16_t classCode) const;

    std::uint16_t wQuestId;                    // 0
    std::uint16_t wQuestNameCode;              // 2
    std::uint8_t  bQuestApplicable;            // 4
    std::uint8_t  padding_1[1];                // 5
    std::uint16_t wDescriptionScript;          // 6
    std::uint16_t wStartDialogue;              // 8
    std::uint16_t wMidDialogue1;               // 10
    std::uint16_t wMidDialogue2;               // 12
    std::uint16_t wMidDialogue3;               // 14
    std::uint16_t wEndDialogue;                // 16
    std::uint8_t  bPlayType;                   // 18
    std::uint8_t  bDifficulty;                 // 19
    std::uint8_t  bConditionLevel;             // 20
    std::uint8_t  bConditionReputation;        // 21
    std::uint16_t wAvailableClasses;           // 22
    std::uint16_t wAvailableClassesTextId;     // 24
    std::uint8_t  padding_2[2];                // 26~27
    std::uint32_t dwTimeLimit;                 // 28
    std::uint32_t dwRewardLib;                 // 32
    std::uint16_t wReward1ItemId_Group1;       // 36
    std::uint16_t wReward2ItemId_Group1;       // 38
    std::uint16_t wReward3ItemId_Group1;       // 40
    std::uint16_t wReward4ItemId_Group1;       // 42
    std::uint8_t  bGroup1ItemCount;            // 44
    std::uint8_t  padding_3[1];                // 45
    std::uint16_t wReward1ItemId_Group2;       // 46
    std::uint16_t wReward2ItemId_Group2;       // 48
    std::uint16_t wReward3ItemId_Group2;       // 50
    std::uint16_t wReward4ItemId_Group2;       // 52
    std::uint8_t  bGroup2ItemCount;            // 54
    std::uint8_t  padding_4[1];                // 55
    std::uint32_t dwRewardExperience;          // 56
    std::uint16_t wRewardReputation;           // 60
    std::uint8_t  padding_5[2];                // 62~63
    std::uint32_t dwRewardClassPoints_Sword;   // 64
    std::uint32_t dwRewardClassPoints_Bow;     // 68
    std::uint32_t dwRewardClassPoints_Magic;   // 72
    std::uint32_t dwRewardClassPoints_Theology;// 76
    std::uint16_t wRewardMajorPoints;          // 80
    std::uint16_t wRewardMeritPoints;          // 82
    std::uint32_t dwRewardRankPoints;          // 84
    std::uint16_t wPrerequisiteQuest;          // 88
    std::uint16_t wPrerequisiteQuestNpc;       // 90
    std::uint32_t dwPermanentAbandon;          // 92

    union {
        struct {
            std::uint16_t wQuestItem1;         // 96
            std::uint16_t wQuestItemCount1;    // 98
            std::uint16_t wQuestItem1MonsterId;// 100
            std::uint16_t wNpcId1;             // 102
            std::uint16_t wQuestItem2;         // 104
            std::uint16_t wQuestItemCount2;    // 106
            std::uint16_t wQuestItem2MonsterId;// 108
            std::uint16_t wNpcId2;             // 110
            std::uint16_t wMainHabitat;        // 112
            std::uint16_t reserved;            // 114
        } collection;

        struct {
            std::uint16_t wItemId;             // 96
            std::uint16_t wReceiveNpcId;       // 98
            std::uint16_t wReceiveNpcDialogue; // 100
            std::uint8_t  reserved[14];
        } delivery;

        struct {
            std::uint16_t wTargetId1;          // 96
            std::uint16_t wCount;              // 98
            std::uint16_t wMainHabitat;        // 100
            std::uint8_t  reserved[14];
        } hunt;

        struct {
            std::uint16_t wItemId;             // 96
            std::uint16_t wAssignedNpcId;      // 98
            std::uint16_t wReceiverNpcId;      // 100
            std::uint16_t wReceiverNpcDialogue;// 102
            std::uint8_t  reserved[12];
        } oneWayDelivery;

        struct {
            std::uint16_t wDungeonId;          // 96
            std::uint16_t wDungeonItemCode;    // 98
            std::uint8_t  reserved[16];
        } playIndun;

        struct {
            std::uint16_t wEmblemItemId;       // 96
            std::uint16_t wEmblemNpcId;        // 98
            std::uint8_t  reserved[16];
        } buyEmblem;

        std::uint8_t tail[20];
    } extra;
};
#pragma pack(pop)

static_assert(sizeof(stQuestKindInfo) == 0x74, "stQuestKindInfo size must be 0x74");
static_assert(offsetof(stQuestKindInfo, extra) == 96, "tail union must start at offset 96");

class cltQuestKindInfo {
public:
    static constexpr std::size_t kQuestSlotCount = 0xFFFF;

    cltQuestKindInfo();
    virtual ~cltQuestKindInfo();

    int LoadQuestInfo(char* filePath);
    int LoadCollectionQuestInfo(char* filePath);
    int LoadDeliveryQuestInfo(char* filePath);
    int LoadHuntQuestInfo(char* filePath);
    int LoadOneWayDeliveryQuestInfo(char* filePath);
    int LoadBuyEmblem();
    int LoadPlayIndun(char* filePath);

    static std::uint16_t TranslateKindCode(const char* code);
    static std::uint16_t TranslateClassCode(const char* code);

    stQuestKindInfo* GetQuestKindInfo(std::uint16_t questCode);
    const stQuestKindInfo* GetQuestKindInfo(std::uint16_t questCode) const;

    std::uint8_t GetQuestPlayType(std::uint16_t questCode) const;
    std::uint8_t GetDemandLevel(std::uint16_t questCode) const;
    std::uint8_t GetDemandFame(std::uint16_t questCode) const;


    bool IsValidRewardExpByLevel(std::uint8_t level, std::int64_t exp) const;
    int IsValidRewardLibiByLevel(std::uint8_t level, int libi) const;

    // 給外部載入正式等級表用。
    //cltLevelSystem& LevelSystem();
    //const cltLevelSystem& LevelSystem() const;

private:
    static bool IsDigitString(const char* s);
    static bool IsAlphabetString(const char* s);
    static std::uint8_t ParsePlayType(const char* token);
    static std::uint8_t ParseDifficulty(const char* token);
    static bool ValidateRewardGroup(const stQuestKindInfo& info, bool firstGroup);

    std::array<stQuestKindInfo*, kQuestSlotCount> m_infos{};
    cltLevelSystem m_levelSystem;
};