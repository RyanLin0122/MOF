#pragma once

#include <cstdint>

#include "Effect/CCAEffect.h"       // for FrameSkip (embedded at DWORD[11..13])
#include "MiniGame/Mini_AniCtrl.h"  // for Mini_AniInfo

class GameImage;

// mofclient.c 還原：MedicalKit_Button — 治療包選單上的單一按鈕
// 4 個方向（上 / 左 / 下 / 右），開啟時從中央向外彈出，關閉時收回中央。
class MedicalKit_Button {
public:
    MedicalKit_Button();
    ~MedicalKit_Button();

    void CreateMedicalKit_Button(Mini_AniInfo* aniInfo, int buttonIdx);
    void OpenMedicalKit(float x, float y);
    void CloseMedicalKit();
    void SetSelect(bool selected);
    int  Process(float dt);
    void Render();

public:
    // --- mofclient.c 欄位 ---
    Mini_AniInfo  m_ani[2];         // DWORD[0..3] 兩張：一般 / 選中
    std::uint8_t  m_bSelected;      // BYTE[16]
    std::uint8_t  m_pad[3];
    std::int32_t  m_buttonIdx;      // DWORD[5] 0..3
    GameImage*    m_pImage;         // DWORD[6]
    float         m_fX;             // DWORD[7]
    float         m_fY;             // DWORD[8]
    float         m_fAnchor;        // DWORD[9]   目標位置（開/關交界）
    std::int32_t  m_state;          // DWORD[10]  0=opening,1=open,2=closing,3=closed
    FrameSkip     m_FrameSkip;      // DWORD[11..13] (vftable + accum + threshold)
};
