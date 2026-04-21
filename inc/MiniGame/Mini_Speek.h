#pragma once

#include <cstdint>

#include "Effect/CCAEffect.h"  // for FrameSkip (embedded in Mini_Speek_Thanks DWORD[7..9])

class GameImage;

// mofclient.c 還原：Mini_Speek_Mgr — 顯示「需要的藥水」對話框
// 病患被回收 (RecallPatient) 後使用，blockID = patientType（0..3）。
class Mini_Speek_Mgr {
public:
    Mini_Speek_Mgr();
    ~Mini_Speek_Mgr();

    void SetPosition(float x, float y, int patientType);
    void SetUsed(bool used);
    void Process();
    void Render();

public:
    GameImage*   m_pImage;       // DWORD[0]
    float        m_fX;           // DWORD[1]
    float        m_fY;           // DWORD[2]
    std::int32_t m_patientType;  // DWORD[3]  -> blockID
    std::uint8_t m_bUsed;        // BYTE[16]
    std::uint8_t m_pad[3];
};

// mofclient.c 還原：Mini_Speek_Thanks — 治癒成功時短暫飄出的「感謝」對話框
// 透過 FrameSkip 漸隱、上飄；前 30 frame 飄到目標 alpha，之後線性收尾。
class Mini_Speek_Thanks {
public:
    Mini_Speek_Thanks();
    ~Mini_Speek_Thanks();

    void InitPosition(float x, float y);
    void SetUsed(bool used);
    void Process(float dt);
    void Render();

public:
    GameImage*   m_pImage;        // DWORD[0]
    float        m_fX;            // DWORD[1]
    float        m_fY;            // DWORD[2]
    std::uint8_t m_state;         // BYTE[12]   0=fading-in, 1=floating-up
    std::uint8_t m_pad0[3];
    float        m_alphaScale;    // DWORD[4]   bits 1132396544 = 255.f  -> m_dwAlpha
    float        m_scale;         // DWORD[5]   bits 1120403456 = 100.f  -> m_nScale (100 = 100%)
    std::uint8_t m_bUsed;         // BYTE[24]
    std::uint8_t m_pad1[3];
    FrameSkip    m_FrameSkip;     // DWORD[7..9] (vftable + accum + threshold=1/60)
};
