#pragma once

#include <cstdint>

#include "MiniGame/InfoMgr.h"
#include "MiniGame/PowerBar.h"
#include "MiniGame/AccuracyBar.h"
#include "MiniGame/BoardMgr.h"
#include "MiniGame/BackGroundMgr.h"
#include "Image/BackGroundImage.h"
#include "MiniGame/Mini_Timer.h"

class GameImage;

// mofclient.c 還原：Sword2 — 獨立的「擊劍小遊戲回合控制器」。
// 被 cltMini_Sword_2 直接持有，也是 cltMini_Sword 參考的基礎邏輯。
// 狀態機：2 = PowerBar 階段、3 = AccuracyBar 階段、4 = 結算動畫。
class Sword2 {
public:
    Sword2();
    ~Sword2();

    void InitSword2(int degree);
    void InitRound();
    int  Process();   // 回傳 BOOL
    void Render();

    // 供 cltMini_Sword_2 讀取本局累積得分（原始為 *((_DWORD *)&sword2 + 5)）
    int  GetTotalScore() const { return m_totalScore; }

private:
    int             m_state;          // 狀態 0/2/3/4
    int             m_round;          // 目前第幾回合
    int             m_remainSec;      // 剩餘秒數
    int             m_power;          // 上一輪 PowerBar 結算值
    int             m_pad4;           // 對應原始 +16：未使用（InitSword2 中設 0）
    int             m_totalScore;     // 累積總分
    int             m_degree;         // 難度

    InfoMgr         m_info;           // +28
    PowerBar        m_powerBar;       // +608
    AccuracyBar     m_accBar;         // +688
    BoardMgr        m_board;          // +780
    BackGroundMgr   m_bgMgr;          // +1276
    BackGroundImage m_blackTop;       // +1328
    BackGroundImage m_blackBottom;    // +1632
    Mini_Timer      m_timer;          // +1936

    // 結算動畫相關欄位（+1960 起）
    uint8_t         m_gameOver;       // +1960
    uint8_t         m_roundFinished;  // +1961
    int             m_screenX;        // +1988  (497 dword)
    int             m_screenY;        // +1992  (498 dword)

    // 結算 FrameSkip（+1976~+1984 範圍）
    float           m_endAccum;       // +1980
    float           m_endThreshold;   // +1984 (1/60)

    // 收官動畫階段計時
    int             m_endPhase;       // +1304
    float           m_endY;           // +1308  = *((float*)this + 327) in mofclient
    int             m_endSubState;    // +1312 (+328)

    GameImage*      m_pBgImage;       // +1312 actually — Sword2 top hat
    GameImage*      m_pShake1;        // +1320 ("330")
    GameImage*      m_pShake2;        // +1324 ("331")

    float           m_shakeY;         // +1316 ("329") initial 5.0

    // 結算控制
    int             m_endGraphStep;   // +1972 in mofclient: *((_DWORD *)this + 493)
    float           m_endGraphTimer;  // +1968 in mofclient: *((float *)this + 492)
    float           m_endGameOverTimer; // +1964 in mofclient: *((float *)this + 491)
};
