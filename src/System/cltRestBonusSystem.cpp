#include "System/cltRestBonusSystem.h"

cltRestBonusSystem::cltRestBonusSystem() {
    m_pLevelSystem = nullptr;
    m_iRestEndTime = 0;
    m_iExpAdvantage = 0;
}

int cltRestBonusSystem::Initialize(cltLevelSystem* levelSystem, int logoutTime, int* ioRestEndTime, int* ioExpAdvantage) {
    std::time_t now{};
    std::time(&now);

    m_pLevelSystem = levelSystem;
    if (*ioRestEndTime && *ioRestEndTime > now) {
        m_iRestEndTime = *ioRestEndTime;
        m_iExpAdvantage = *ioExpAdvantage;
        return 0;
    }

    Update(logoutTime);
    *ioRestEndTime = m_iRestEndTime;
    *ioExpAdvantage = m_iExpAdvantage;
    return 1;
}

void cltRestBonusSystem::Initialize(cltLevelSystem* levelSystem, int remainedSeconds, int expAdvantage) {
    std::time_t now{};
    std::time(&now);

    m_pLevelSystem = levelSystem;
    m_iExpAdvantage = expAdvantage;
    m_iRestEndTime = static_cast<int>(now) + remainedSeconds;
}

void cltRestBonusSystem::Free() {
    m_pLevelSystem = nullptr;
    m_iRestEndTime = 0;
    m_iExpAdvantage = 0;
}

void cltRestBonusSystem::Poll(int now) {
    if (m_iRestEndTime && m_iRestEndTime <= now) {
        m_iRestEndTime = 0;
        m_iExpAdvantage = 0;
    }
}

int cltRestBonusSystem::GetExpAdvantage() { return m_iExpAdvantage; }

void cltRestBonusSystem::Update(int logoutTime) {
    std::time_t now{};
    std::time(&now);

    const double diff = logoutTime ? std::difftime(now, logoutTime) : 0.0;
    const std::int64_t restHours = static_cast<std::int64_t>(diff / 3600.0);

    if (restHours < 12 || !m_pLevelSystem || m_pLevelSystem->GetLevel() < 20) {
        m_iExpAdvantage = 0;
        m_iRestEndTime = 0;
        return;
    }

    if (restHours < 36) {
        m_iExpAdvantage = 5;
    } else if (restHours < 60) {
        m_iExpAdvantage = 10;
    } else {
        m_iExpAdvantage = 15;
    }
    m_iRestEndTime = static_cast<int>(now) + 3600;
}

int cltRestBonusSystem::GetleftTime() {
    std::time_t now{};
    std::time(&now);

    if (m_iRestEndTime >= now) return m_iRestEndTime - static_cast<int>(now);
    return 0;
}
