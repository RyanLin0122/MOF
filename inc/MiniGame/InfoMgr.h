#pragma once

#include <cstdint>

#include "MiniGame/Mini_IMG_Number.h"

class GameImage;

// mofclient.c 還原：InfoMgr — Sword2 上方的得分/時間 HUD。
// 含 5 個回合分數 + 1 個總分 + 1 個時間顯示。
class InfoMgr {
public:
    InfoMgr();
    ~InfoMgr();

    void InitInfoMgr();
    void SetScore(int slot, int value);
    void SetTime(int seconds);
    void Process(float dt);
    void Render();

private:
    GameImage*      m_pBgLeft;
    GameImage*      m_pBgRight;
    uint8_t         m_bActive;

    // slot 0..4 = 每回合分數，slot 5 = 本局總分
    Mini_IMG_Number m_scoreNum[6];
    // 時間顯示
    Mini_IMG_Number m_timeNum;

    int   m_screenX;
    int   m_screenY;

    // 6 個分數顯示的 x / y
    float m_scoreX[6];
    float m_scoreY[6];

    // 每個分數 slot 是否已設定（對應 mofclient.c 的 +492..+497 旗標陣列）
    uint8_t m_scoreActive[6];
};
