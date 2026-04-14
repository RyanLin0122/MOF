#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：AccuracyBar — Sword2 精準度條。
// 指針左右擺動，玩家按鍵時依偏離中央的距離回傳 0..9（0=最準）。
class AccuracyBar {
public:
    AccuracyBar();
    ~AccuracyBar();

    void InitAccuracyBar(int degree, float x, float y);
    void Run();
    int  GetAccuracy();
    bool Process(float dt);
    void Render();

    bool IsRunning() const { return m_running != 0; }

private:
    int        m_degree;
    float      m_speed;
    float      m_center;           // 固定 432.0f
    float      m_half;              // Init 時為 0；後續未被改動
    float      m_curPos;            // 指針目前位置
    uint8_t    m_dir;               // 0/1
    uint8_t    m_running;
    GameImage* m_pBar;
    GameImage* m_pCursor;
    GameImage* m_pHighlight;
    uint8_t    m_highlightUp;
    int        m_highlightAlpha;

    // FrameSkip：highlight alpha 的步長 (1/120 ≒ 0.008333)
    float      m_highlightAccum;
    float      m_highlightThreshold;

    float      m_baseX;             // Init 傳入
    float      m_baseY;             // Init 傳入

    // FrameSkip：pointer 的步長 (1/30 ≒ 0.033333)
    float      m_pointerAccum;
    float      m_pointerThreshold;

    uint8_t    m_hitLocked;         // GetAccuracy 後被鎖住
    float      m_endTimer;          // GetAccuracy 後累積的延遲
};
