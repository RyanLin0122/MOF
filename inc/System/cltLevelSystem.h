#pragma once

#include <cstdint>
#include <cstdio>
#include "global.h"

class cltPlayerAbility {
public:
    static void IncreaseBonusPoint(cltPlayerAbility* self, int value);
    static void ResetAbility(cltPlayerAbility* self);
};

class cltEmblemSystem {
public:
    static void UpdateValidity(cltEmblemSystem* self);
};


class cltLevelSystem {
public:
    static void Release();
    static int InitializeStaticVariable(char* String2);
    static int GetMaxLevel();

    cltLevelSystem();
    ~cltLevelSystem();

    void Initilaize(cltPlayerAbility* a2, cltEmblemSystem* a3, char a4, std::int64_t a5);
    std::uint8_t GetLevel() const;
    std::int64_t GetExp() const;
    std::int64_t GetTotalExpOfLevel() const;
    std::uint64_t GetCurrentExpOfLevel() const;
    int GetExpPercent() const;
    int IncreaseExp(std::int64_t a2);
    void DecreaseExp(std::int64_t a2);

    static int GetLevelByExp(std::int64_t a1);
    static std::int64_t GetExpByLevel(std::uint8_t a1);

    std::int64_t GetCurrentExpOfLevel(std::int64_t a2);
    void ResetLevel();
    void SetExp(std::int64_t a2);
    std::int64_t GetExpDestination() const;

    static std::int64_t* m_pi64ExpData;
    static int m_iMaxLevel;

private:
    cltPlayerAbility* m_pPlayerAbility = nullptr;
    cltEmblemSystem* m_pEmblemSystem = nullptr;
    std::uint8_t m_byLevel = 0;
    std::uint8_t m_pad9 = 0;
    std::uint8_t m_padA = 0;
    std::uint8_t m_padB = 0;
    std::int64_t m_i64Exp = 0;
};
