#pragma once

#include <cstdint>

class cltLevelSystem;

class cltRestBonusSystem {
public:
    cltRestBonusSystem();

    int Initialize(cltLevelSystem* levelSystem, int logoutTime, int* ioRestEndTime, int* ioExpAdvantage);
    void Initialize(cltLevelSystem* levelSystem, int remainedSeconds, int expAdvantage);
    void Free();
    void Poll(int now);
    int GetExpAdvantage();
    void Update(int logoutTime);
    int GetleftTime();

private:
    cltLevelSystem* m_pLevelSystem = nullptr;
    int m_iRestEndTime = 0;
    int m_iExpAdvantage = 0;
};
