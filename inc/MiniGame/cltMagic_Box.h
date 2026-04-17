#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：cltMagic_Box — cltMini_Magic_2 用的閃爍方塊。
// 有淡入/淡出循環，用 alpha 值呈現。
class cltMagic_Box {
public:
    cltMagic_Box();
    virtual ~cltMagic_Box();

    void Initailize(float x, float y);  // 原名拼寫 typo，保持一致
    float GetPosX();
    float GetPosY();
    void  SetPosY(float y);
    void  SetActive(int active);
    int   GetActive();
    void  Poll();
    void  PrepareDrawing();
    void  Draw();

public:
    // +4
    int           m_active;       // DWORD[1]
    // +8
    float         m_posX;         // float[2]
    float         m_posY;         // float[3]
    // +16
    std::uint32_t m_resID;        // DWORD[4]
    // +20
    std::int16_t  m_frame;        // WORD[10]
    std::uint8_t  m_alpha;        // BYTE[22]
    std::uint8_t  m_pad23;
    // +24
    int           m_fadeDir;      // DWORD[6]: 1=fade-in, 0=fade-out
    // +28
    GameImage*    m_pImage;       // DWORD[7]
};
