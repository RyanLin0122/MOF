#include "System/cltLevelSystem.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

std::int64_t* cltLevelSystem::m_pi64ExpData = nullptr;
int cltLevelSystem::m_iMaxLevel = 0;

void cltLevelSystem::Release() {
    delete[] m_pi64ExpData;
    m_pi64ExpData = nullptr;
}

int cltLevelSystem::InitializeStaticVariable(char* String2) {
    char buffer[1024]{};

    int idx = 1;
    m_iMaxLevel = 0;

    FILE* f = g_clTextFileManager.fopen(String2);
    if (!f) return 0;

    fpos_t pos;
    std::fgetpos(f, &pos);

    while (std::fgets(buffer, sizeof(buffer), f)) {
        ++m_iMaxLevel;
    }

    delete[] m_pi64ExpData;
    m_pi64ExpData = nullptr;

    if (m_iMaxLevel <= 0) {
        g_clTextFileManager.fclose(f);
        return 0;
    }

    m_pi64ExpData = new std::int64_t[m_iMaxLevel + 1]{};

    std::fsetpos(f, &pos);

    while (std::fgets(buffer, sizeof(buffer), f)) {
        if (idx > m_iMaxLevel) {
            g_clTextFileManager.fclose(f);
            return 0;
        }

        m_pi64ExpData[idx] = std::atoll(buffer);

        if (idx == 1) {
            if (m_pi64ExpData[1] != 0) {
                g_clTextFileManager.fclose(f);
                return 0;
            }
        }
        else {
            if (m_pi64ExpData[idx] <= m_pi64ExpData[idx - 1]) {
                g_clTextFileManager.fclose(f);
                return 0;
            }
        }

        ++idx;
    }

    g_clTextFileManager.fclose(f);
    return 1;
}

int cltLevelSystem::GetMaxLevel() { return m_iMaxLevel; }

cltLevelSystem::cltLevelSystem() = default;

cltLevelSystem::~cltLevelSystem() = default;

void cltLevelSystem::Initilaize(cltPlayerAbility* a2, cltEmblemSystem* a3, char a4, std::int64_t a5) {
    m_pPlayerAbility = a2;
    m_pEmblemSystem = a3;
    m_byLevel = static_cast<std::uint8_t>(a4);
    m_i64Exp = a5;
}

std::uint8_t cltLevelSystem::GetLevel() const { return m_byLevel; }

std::int64_t cltLevelSystem::GetExp() const { return m_i64Exp; }

std::int64_t cltLevelSystem::GetTotalExpOfLevel() const {
    int v1 = m_byLevel;
    if (v1 == GetMaxLevel()) {
        return 0;
    }
    return m_pi64ExpData[v1 + 1] - m_pi64ExpData[v1];
}

std::uint64_t cltLevelSystem::GetCurrentExpOfLevel() const {
    const int v1 = m_byLevel;
    const std::uint64_t base = static_cast<std::uint64_t>(m_pi64ExpData[v1]);
    const std::uint64_t exp = static_cast<std::uint64_t>(m_i64Exp);
    if (base <= exp) {
        return exp - base;
    }
    return 0;
}

int cltLevelSystem::GetExpPercent() const {
    const std::int64_t v2 = GetTotalExpOfLevel();
    const std::uint64_t v3 = GetCurrentExpOfLevel();
    if (v2) {
        return static_cast<int>((100 * v3) / static_cast<std::uint64_t>(v2));
    }
    return 0;
}

int cltLevelSystem::IncreaseExp(std::int64_t a2) {
    m_i64Exp += a2;
    const std::uint8_t v5 = static_cast<std::uint8_t>(GetLevelByExp(m_i64Exp));
    if (GetLevel() >= v5) {
        return 0;
    }

    const std::uint8_t v6 = GetLevel();
    m_byLevel = v5;
    //cltPlayerAbility::IncreaseBonusPoint(m_pPlayerAbility, 5 * v5 - 5 * v6);
    //cltEmblemSystem::UpdateValidity(m_pEmblemSystem);
    return 1;
}

void cltLevelSystem::DecreaseExp(std::int64_t a2) {
    const int v2 = m_byLevel;
    const std::int64_t minExp = m_pi64ExpData[v2];
    if (((m_i64Exp - minExp) >> 32) > (a2 >> 32)) {
        m_i64Exp -= a2;
    } else {
        m_i64Exp = minExp;
    }
}

int cltLevelSystem::GetLevelByExp(std::int64_t a1) {
    int v1 = 2;
    if (GetMaxLevel() < 2) {
        return GetMaxLevel();
    }

    while (m_pi64ExpData[v1] <= a1) {
        ++v1;
        if (v1 > GetMaxLevel()) {
            return GetMaxLevel();
        }
    }
    return v1 - 1;
}

std::int64_t cltLevelSystem::GetExpByLevel(std::uint8_t a1) {
    if (a1 <= 1) {
        return 0;
    }
    if (a1 <= m_iMaxLevel) {
        return m_pi64ExpData[a1] - m_pi64ExpData[a1 - 1];
    }
    return 0;
}

std::int64_t cltLevelSystem::GetCurrentExpOfLevel(std::int64_t a2) {
    return a2 - m_pi64ExpData[static_cast<std::uint8_t>(GetLevelByExp(a2))];
}

void cltLevelSystem::ResetLevel() {
    m_byLevel = 1;
    m_i64Exp = 0;
    //cltPlayerAbility::ResetAbility(m_pPlayerAbility);
}

void cltLevelSystem::SetExp(std::int64_t a2) { m_i64Exp = a2; }

std::int64_t cltLevelSystem::GetExpDestination() const { return m_pi64ExpData[m_byLevel]; }
