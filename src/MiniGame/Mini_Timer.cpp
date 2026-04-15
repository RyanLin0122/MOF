#include "MiniGame/Mini_Timer.h"

#include "global.h"  // for timeGetTime

Mini_Timer::Mini_Timer()
    : m_dwStartTick(0)
    , m_dwPrevTick(0)
    , m_dwCurTick(0)
    , m_fFrameSecond(0.0f)
    , m_secondStep(0)
    , m_lastSecond(0)
{
}

Mini_Timer::~Mini_Timer() = default;

void Mini_Timer::InitTimer(int secondStep)
{
    unsigned int now = timeGetTime();
    m_dwStartTick  = now;
    m_dwPrevTick   = now;
    m_dwCurTick    = 0;
    m_fFrameSecond = 0.0f;
    m_secondStep   = secondStep;
    m_lastSecond   = 0;
}

long long Mini_Timer::GetCurrentSecond()
{
    // mofclient.c：((timeGetTime() - start) * 0.001) 的截斷值。
    return static_cast<long long>(
        static_cast<double>(timeGetTime() - m_dwStartTick) * 0.001);
}

int Mini_Timer::GetCurrentSecondDelta()
{
    long long now = static_cast<long long>(
        static_cast<double>(timeGetTime() - m_dwStartTick) * 0.001);
    if (static_cast<int>(now) - m_lastSecond < m_secondStep)
        return 0;
    m_lastSecond = static_cast<int>(now);
    return 1;
}

double Mini_Timer::GetCurrentFrameTime()
{
    // mofclient.c：函式簽章回傳 double，內部 m_fFrameSecond 為 float。
    // 以 float 儲存後再以 double 傳回（將 float 擴展為 double）。
    unsigned int now  = timeGetTime();
    unsigned int prev = m_dwPrevTick;
    m_dwCurTick  = now;
    m_dwPrevTick = now;
    m_fFrameSecond = static_cast<float>(
        static_cast<double>(now - prev) * 0.001);
    return m_fFrameSecond;
}
