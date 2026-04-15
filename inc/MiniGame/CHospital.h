#pragma once

#include <cstdint>

#include "MiniGame/CMedical.h"
#include "MiniGame/PatientRecallMgr.h"
#include "MiniGame/Mini_AniCtrl.h"
#include "MiniGame/Mini_Timer.h"
#include "MiniGame/Mini_IMG_Number.h"

// mofclient.c 還原：CHospital — 醫院場景控制器，掌管 3x3 病床網格、
// 治療包選單、自動補入病人計時器，以及右上角的「剩餘秒數」數字。
//
// 對應 mofclient.c 的欄位佈局（32-bit 原始 2300 bytes，本還原以邏輯成員
// 重新表達；行為等價）：
//   DWORD[0]      m_timeRemaining   倒數總秒數
//   DWORD[1]      m_state           0=結束 1=操作 2=開治療包選單
//   +8            m_beds[9]         3x3，row-major
//   +1808         m_medical
//   +2064         m_recallMgr
//   +2172         m_aniCtrl         選中游標
//   +2228         m_timer           讀秒
//   +2252         m_imgNumber       右上角數字
//   DWORD[513]    m_selectRow
//   DWORD[514]    m_selectCol
//   DWORD[515]    m_difficulty
class CHospital {
public:
    CHospital();
    ~CHospital();

    void InitHospital(int difficulty);
    void SelectBedstead_Move_Up();
    void SelectBedstead_Move_Down();
    void SelectBedstead_Move_Left();
    void SelectBedstead_Move_Right();
    int  GetGameScore();
    int  Process();
    void Render();

public:
    int               m_timeRemaining;
    int               m_state;
    CBedstead         m_beds[9];
    CMedical          m_medical;
    PatientRecallMgr  m_recallMgr;
    Mini_AniCtrl      m_aniCtrl;
    Mini_Timer        m_timer;
    Mini_IMG_Number   m_imgNumber;
    int               m_selectRow;
    int               m_selectCol;
    int               m_difficulty;
};
