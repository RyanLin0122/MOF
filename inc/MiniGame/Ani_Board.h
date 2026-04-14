#pragma once

class GameImage;

// mofclient.c 還原：Ani_Board — 小遊戲中「木板」的動畫元件。
// 來自 BoardMgr 的子物件，負責呼叫 GameImage 動畫、在 Play() 時噴發碎木特效。
// 內嵌一個輕量 FrameSkip 計時器（原始碼中以偏移 +24 的子物件實作），
// 這裡以三個欄位平坦化。邏輯與原始實作等價。
class Ani_Board {
public:
    Ani_Board();
    ~Ani_Board();

    void InitAni_Board(float x, float y);
    void Play(int targetFrame);
    void SetPosition(float x, float y);
    char Process(float dt);
    void Render();

private:
    GameImage* m_pImage;        // +0:  GameImage*
    int        m_curFrame;       // +4:  現在播放到第幾張
    int        m_targetFrame;    // +8:  目標影格（Play 指定）
    bool       m_bPlaying;       // +12: 是否正在播放中
    float      m_x;              // +16: 繪製 X
    float      m_y;              // +20: 繪製 Y
    // FrameSkip 內嵌：原始 +24 為 vtable，+28 為 accumulator，+32 為 threshold。
    float      m_frameAccum;     // 累計時間
    float      m_frameThreshold; // 每張動畫的播放秒數（預設 0.05）
};
