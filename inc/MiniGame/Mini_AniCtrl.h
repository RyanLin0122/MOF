#pragma once

#include <cstdint>

#include "Effect/CCAEffect.h"  // for FrameSkip (embedded at DWORD[10..12])

class GameImage;

// mofclient.c 還原：Mini_AniInfo / Mini_KeyInfo
//   Mini_AniInfo  (8 bytes)：對應「一張 sprite 圖」= (resourceID, blockID)
//   Mini_KeyInfo  (8 bytes)：對應「一段動畫的 frame 區段」= (startFrame, lastFrame)
struct Mini_AniInfo {
    std::uint32_t resourceID;   // +0
    std::uint16_t blockID;      // +4
    std::uint16_t pad;          // +6
};

struct Mini_KeyInfo {
    std::int32_t startFrame;    // +0
    std::int32_t lastFrame;     // +4
};

// mofclient.c 還原：Mini_AniCtrl — 小遊戲動畫播放器（4-byte vftable 起源於 FrameSkip）
// 將一組 Mini_AniInfo 視為「frame 大表」，再以 Mini_KeyInfo 把 frame 大表切成多段
// 動畫；Play(idx) 切換到第 idx 段、Process(dt) 透過 FrameSkip 累計遞進到下一格。
class Mini_AniCtrl {
public:
    Mini_AniCtrl();
    ~Mini_AniCtrl();

    void Create_Mini_AniCtrl(Mini_AniInfo* aniArr, int aniCount,
                             Mini_KeyInfo* keyArr, int keyCount, int frameRate);
    void Play(int keyIdx);
    void SetFrameRate(int frameRate);
    void SetPosition(float x, float y);
    void SetAlpha(int alpha);
    void Process(float dt);
    void Render();

public:
    // --- mofclient.c 欄位（按 DWORD 序排列） ---
    GameImage*    m_pImage;         // DWORD[0]
    Mini_AniInfo* m_pAniArr;        // DWORD[1]
    std::int32_t  m_aniCount;       // DWORD[2]
    Mini_KeyInfo* m_pKeyArr;        // DWORD[3]
    std::int32_t  m_keyCount;       // DWORD[4]
    std::int32_t  m_curKey;         // DWORD[5]
    std::int32_t  m_curFrame;       // DWORD[6]
    float         m_fX;             // DWORD[7]
    float         m_fY;             // DWORD[8]
    std::int32_t  m_alpha;          // DWORD[9]
    FrameSkip     m_FrameSkip;      // DWORD[10..12] (vftable + accum + threshold)
    std::int32_t  m_dword13;        // DWORD[13]
};
