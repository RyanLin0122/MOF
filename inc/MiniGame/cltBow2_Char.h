#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：cltBow2_Char — Bow2 小遊戲中的角色。
// 角色有位置、方向、動畫，可在半徑內移動。
class cltBow2_Char {
public:
    cltBow2_Char();
    virtual ~cltBow2_Char();

    void Initalize(float radius, std::uint8_t degree, int screenX, int screenY);
    void SetActive(int active);
    void SetDirection(std::uint8_t direction);
    void SetPos(float x, float y);
    double GetPosX();
    double GetPosY();
    void Poll();
    void PrepareDrawing();
    void Draw();

public:
    float          m_initPosX;      // float[1]  初始位置 X
    float          m_initPosY;      // float[2]  初始位置 Y
    float          m_curPosX;       // float[3]  目前位置 X
    float          m_curPosY;       // float[4]  目前位置 Y
    std::uint32_t  m_resID;         // DWORD[5]  圖像資源 ID
    int            m_frame;         // DWORD[6]  目前動畫 frame
    std::uint8_t   m_direction;     // BYTE[28]  目前方向 (0..8)
    std::uint8_t   m_frameCount;    // BYTE[29]  動畫 frame 數量
    float          m_animCounter;   // float[8]  浮點累計動畫計數
    float          m_radius;        // float[9]  活動半徑
    int            m_blockIDBase;   // DWORD[10] 目前方向對應的起始 blockID
    GameImage*     m_pImage;        // DWORD[11] 目前 frame 的 GameImage 指標
    int            m_active;        // DWORD[12] 是否啟用
};
