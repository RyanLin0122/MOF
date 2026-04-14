#pragma once

class GameImage;

// mofclient.c 還原：BackGroundMgr — 小遊戲的滾動背景。
// 原始 Image_UP/DOWN 使用 DWORD 寫入以實作特定 float 位元模式，
// 這裡直接使用浮點常數對應其邏輯。
class BackGroundMgr {
public:
    BackGroundMgr();

    void InitBackGroundMgr(int type, float x, float y);
    char Image_UP(float dy);
    char Image_DOWN(float dy);
    void Process(float dt);
    void Render();

private:
    float        m_x;
    float        m_y;
    int          m_type;
    unsigned int m_dwResID;
    GameImage*   m_pImage;
};
