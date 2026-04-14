#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：PowerBar — Sword2 力量條。
// 值由 0 逐漸往負方向衰減，玩家按鍵時以絕對值回傳 1..10。
class PowerBar {
public:
    PowerBar();
    ~PowerBar();

    void InitPowerBar(int degree, float x, float y);
    int  GetPower();
    void Process(float dt);
    void Render();

    bool IsRunning() const { return m_running != 0; }

private:
    int        m_degree;
    float      m_maxRange;          // 固定 200.0f (1127743488)
    float      m_decaySpeed;        // 每 tick 減少量
    float      m_curOffset;         // 目前偏移（≤ 0）

    uint8_t    m_running;           // GetPower 後變 0

    GameImage* m_pBar;              // bar
    GameImage* m_pCursor;           // cursor
    uint8_t    m_highlightUp;
    GameImage* m_pHighlight;
    int        m_highlightAlpha;

    // FrameSkip：highlight alpha (1/120)
    float      m_highlightAccum;
    float      m_highlightThreshold;

    float      m_baseX;
    float      m_baseY;

    // FrameSkip：pointer 移動 (1/30)
    float      m_pointerAccum;
    float      m_pointerThreshold;
};
