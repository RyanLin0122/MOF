#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：cltMagic_Target — cltMini_Magic_2 專用的飛行目標物件。
// 可設定方向（左→右 / 右→左）、外型種類（小妖/中妖/骷髏）、速度，
// 碰到畫面邊界時自動 Release。
class cltMagic_Target {
public:
    cltMagic_Target();
    virtual ~cltMagic_Target();

    void Initialize(std::uint8_t type, std::uint8_t kind,
                    std::uint8_t direction, int active);
    void SetActive(int active);
    int  GetActive();
    int  GetLive();
    int  GetPoint();
    void SetDie();
    void SetPos(float x, float y);
    void GetPos(float* outX, float* outY);
    float GetPosY();
    void GetHSize(float* outHX, float* outHY);
    std::uint8_t GetDirection();
    void Release();
    void Poll();
    void PrepareDrawing();
    void Draw();

public:
    // +4
    std::uint8_t  m_type;           // BYTE[4]: 0/1/2/3 → speed
    std::uint8_t  m_kind;           // BYTE[5]: 0=小妖, 1=中妖, 2=骷髏
    std::uint8_t  m_pad6[2];
    // +8
    int           m_active;         // DWORD[2]
    // +12
    int           m_live;           // DWORD[3]
    // +16
    float         m_speed;          // float[4]
    // +20
    std::uint8_t  m_direction;      // BYTE[20]: 0=右→左, 1=左→右
    std::uint8_t  m_pad21[3];
    // +24
    float         m_posX;           // float[6]
    float         m_posY;           // float[7]
    // +32
    int           m_flip;           // DWORD[8]: 0 or 1
    // +36
    std::uint32_t m_resID;          // DWORD[9]
    // +40
    std::int16_t  m_currentFrame;   // WORD[20]
    std::int16_t  m_startFrame;     // WORD[21]
    std::int16_t  m_frameCount;     // WORD[22]
    std::int16_t  m_pad46;
    // +48
    float         m_animCounter;    // float[12]
    // +52
    float         m_halfSizeX;      // float[13]
    float         m_halfSizeY;      // float[14]
    // +60
    GameImage*    m_pImage;         // DWORD[15]
    // +64
    int           m_screenBaseX;    // DWORD[16]
    int           m_screenBaseY;    // DWORD[17]
};
