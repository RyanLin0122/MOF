#pragma once

#include <cstdint>

#include "MiniGame/MedicalKit_Button.h"
#include "MiniGame/Mini_AniCtrl.h"
#include "MiniGame/Mini_Speek.h"
#include "MiniGame/Mini_Timer.h"

// mofclient.c 還原：CMedical — 4 個 MedicalKit_Button 的容器
//   m_select  = 4 表「未選擇」；按方向鍵選 0/1/2/3，按空白送出 → 收回。
class CMedical {
public:
    CMedical();
    ~CMedical();

    void OpenMedical(float x, float y);
    int  GetSelectMedical();   // 回傳 m_select（0..3 或 4）
    int  Process(float dt, bool inputEnabled);
    void Render();

public:
    std::int32_t       m_select;          // DWORD[0]
    MedicalKit_Button  m_buttons[4];      // 各 56 bytes，總 224 bytes
    std::int32_t       m_btnState[4];     // DWORD[57..60]  最近一次 Process 回傳
};

// mofclient.c 還原：CPatient — 病床上的單一病人
class CPatient {
public:
    CPatient();
    ~CPatient();

    void InitPatient(float x, float y);
    int  UseMedical(int medicalKind);   // 回傳 1 = 治癒成功 / -1 = 給錯藥
    void RecallPatient();               // 從 None 進到「等待救援」狀態
    int  GetPatientState();
    void Process(float dt);
    void Render();

public:
    std::int32_t  m_state;          // DWORD[0]
    std::int32_t  m_medicalKind;    // DWORD[1]
    Mini_Timer    m_timer;          // +8  (24 bytes)
    std::int32_t  m_pad32;          // DWORD[8] = +32  剩餘秒數
    Mini_AniCtrl  m_aniCtrl;        // +36 (56 bytes)
    std::int32_t  m_alpha;          // DWORD[23] = +92
    FrameSkip     m_frameSkip;      // DWORD[24..26] (vftable + accum + threshold)
    float         m_fX;             // DWORD[27] = +108
    float         m_fY;             // DWORD[28] = +112
    std::uint8_t  m_bFirstHurt;     // BYTE[116]
    std::uint8_t  m_pad117[3];
    Mini_Speek_Mgr    m_speekMgr;   // +120 (20 bytes)
    std::uint8_t  m_bSpeekUsed;     // BYTE[140]
    std::uint8_t  m_pad141[3];
    Mini_Speek_Thanks m_speekThx;   // +144 (40 bytes)
};

// mofclient.c 還原：CBedstead — 一張病床（座標 + 一個 CPatient）
class CBedstead {
public:
    CBedstead();
    ~CBedstead();

    void InitBedstead(int bedKind, float x, float y);
    void PatientRecall();
    bool IsPatient();          // 病人正在等待治療（state ∈ {2,3,4,6}）
    int  IsPatientState();     // 直接回傳 patient.state
    void SelectBedstead(bool selected);
    int  HealPatient(int medicalKind);
    void Process(float dt);
    void Render();

public:
    std::int32_t m_bedKind;     // DWORD[0]
    float        m_fX;          // DWORD[1]
    float        m_fY;          // DWORD[2]
    CPatient     m_patient;     // +12 (約 184 bytes)
    std::uint8_t m_bSelected;   // BYTE[196]
    std::uint8_t m_pad197[3];
};
