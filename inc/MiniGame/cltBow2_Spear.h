#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：cltBow2_Spear — Bow2 小遊戲中的飛矛投射物。
// 每支矛有位置、方向、速度，Poll() 驅動移動，超出距離後自動 Release。
class cltBow2_Spear {
public:
    cltBow2_Spear();
    virtual ~cltBow2_Spear();

    // 建立飛矛。posIndex=0..16 查表取位移，direction=1..4 四方向，
    // moveType=100/101/102/103 決定軌跡類型，speed 為移動速度倍率。
    void Create(std::uint8_t posIndex, std::uint8_t direction, float moveType, float speed);
    void Release();
    int  GetActive();
    void Poll();
    void PrepareDrawing();
    void Draw();

public:
    float     m_posX;           // 目前位置 X
    float     m_posY;           // 目前位置 Y
    int       m_frame;          // 動畫 frame（低 16 位亦為 blockID）
    float     m_animCounter;    // 浮點累計動畫計數
    float     m_radius;         // 活動半徑（初始 300.0f）
    float     m_maxDist;        // 距離上限平方（50000.0f）
    int       m_active;         // 是否存活
    float     m_centerX;        // 中心點 X（螢幕中央偏移）
    float     m_centerY;        // 中心點 Y
    GameImage* m_pImage;        // 目前 frame 的 GameImage 指標
    float     m_speedX;         // X 軸速度分量
    float     m_speedY;         // Y 軸速度分量
    float     m_moveType;       // 100.0=垂直, 101.0=水平, 其他=斜線
    float     m_intercept;      // 斜線軌跡的 y 截距
    float     m_dirX;           // X 方向 (-1.0 或 1.0)
    float     m_dirY;           // Y 方向 (-1.0 或 1.0)
};

// mofclient.c 還原：stBowSpear — cltMini_Bow_2 使用的飛矛容器。
// 每個元素包含一個 active 旗標與 cltBow2_Spear 實體，大小 84 bytes (0x54)。
struct stBowSpear {
    int           m_active;     // +0: 是否被使用中
    cltBow2_Spear m_spear;     // +4: 飛矛實體

    stBowSpear();
    ~stBowSpear();
};
