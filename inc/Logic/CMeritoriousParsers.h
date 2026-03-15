#pragma once

#include <array>
#include <cstdint>
#include <map>

#include "global.h"

struct stExpRewardInfo {
    unsigned int circleQuizRewardExp{};
    unsigned int meritoriousRewardExp{};
};

class CExpRewardParser {
public:
    CExpRewardParser();
    ~CExpRewardParser();

    int Initialize(char* fileName);
    void Free();
    unsigned int GetCircleQuizRewardExp(int level);
    unsigned int GetMeritoriousRewardExp(int level);

private:
    std::array<stExpRewardInfo, 101> infos_{}; // 1-based
    int count_{};
};

struct CSupplyMeritoriousInfo {
    std::uint16_t itemKind{};
    std::uint16_t itemCount{};
    std::uint16_t rewardPoint{};
    std::uint32_t rewardLibi{};

    CSupplyMeritoriousInfo() = default;
    CSupplyMeritoriousInfo(std::uint16_t kind, std::uint16_t count, std::uint16_t point, std::uint32_t libi)
        : itemKind(kind), itemCount(count), rewardPoint(point), rewardLibi(libi) {}
};

class CSupplyMeritoriousParser {
public:
    CSupplyMeritoriousParser();
    ~CSupplyMeritoriousParser();

    int Initialize(char* fileName);
    std::uint16_t GetSupplyMeritoriousID();
    void AddSupplyMeritorious(std::uint16_t id, std::uint16_t itemKind, std::uint16_t itemCount,
                              std::uint16_t rewardPoint, std::uint32_t rewardLibi);
    std::uint16_t TranslateKindCode(char* kindCode);

    CSupplyMeritoriousInfo& operator[](std::uint16_t id) { return infos_[id]; }
    const std::map<std::uint16_t, CSupplyMeritoriousInfo>& GetMap() const { return infos_; }

private:
    std::map<std::uint16_t, CSupplyMeritoriousInfo> infos_;
};

struct stMeritoriousRewardItemInfo {
    std::uint16_t rewardID{};
    std::uint16_t itemKind{};
    std::uint32_t requirePoint{};
    std::uint16_t quantity{};
};

class CMeritoriousRewardParser {
public:
    CMeritoriousRewardParser();
    ~CMeritoriousRewardParser();

    int Initialize(char* fileName);
    stMeritoriousRewardItemInfo** GetMeritoriousRewardItemList();
    stMeritoriousRewardItemInfo* GetMeritoriousRewardItemList(std::uint16_t itemKind);
    unsigned int GetMeritoriousRewardItemRequirePoint(std::uint16_t itemKind);
    std::uint16_t GetMeritoriousRewardItemKind(std::uint16_t index);
    std::uint16_t TranslateKindCode(char* kindCode);

private:
    std::array<stMeritoriousRewardItemInfo*, 100> list_{};
    std::uint16_t count_{};
};

struct stMeritoriousGradeInfo {
    std::uint16_t grade{};
    std::uint32_t needPoint{};
    std::uint16_t nextGrade{};
    std::uint16_t rank{};
    std::uint16_t emblem{};
};

class CMeritoriousGradeParser {
public:
    CMeritoriousGradeParser();
    ~CMeritoriousGradeParser();

    int Initialize(char* fileName);
    stMeritoriousGradeInfo** GetMeritoriousGradeInfoList();
    stMeritoriousGradeInfo* GetMeritoriousGradeInfoList(std::uint16_t index);
    int CalcMeritoriousGrade(std::uint16_t point, std::uint16_t grade, std::uint16_t* outGrade, std::uint16_t* outGradePoint);

private:
    std::array<stMeritoriousGradeInfo*, 100> list_{};
    std::uint16_t count_{};
};
