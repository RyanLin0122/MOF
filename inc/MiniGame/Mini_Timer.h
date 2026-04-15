#pragma once

// mofclient.c 還原：Mini_Timer — 小遊戲內統一用的時間計算輔助類。
// 以 timeGetTime() 為基準，提供「自開始以來經過幾秒」「每秒觸發 a2 次」「每 Frame 的秒差」。
class Mini_Timer {
public:
    Mini_Timer();
    ~Mini_Timer();

    void         InitTimer(int secondStep);
    long long    GetCurrentSecond();
    int          GetCurrentSecondDelta();
    double       GetCurrentFrameTime();

private:
    unsigned int m_dwStartTick;   // +0  : 起始 tick
    unsigned int m_dwPrevTick;    // +4  : 上一次 GetCurrentFrameTime 時的 tick
    unsigned int m_dwCurTick;     // +8  : 最近一次 GetCurrentFrameTime 取得的 tick
    float        m_fFrameSecond;  // +12 : 上一次 frame delta（秒）
    int          m_secondStep;    // +16 : GetCurrentSecondDelta 的秒間隔
    int          m_lastSecond;    // +20 : 上次觸發時記錄的秒數
};
