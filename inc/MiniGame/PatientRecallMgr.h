#pragma once

#include <cstdint>

#include "MiniGame/Mini_Timer.h"

class CBedstead;

// mofclient.c 還原：PatientRecallMgr — 控制醫院內 9 張病床的病人投放節奏。
//
// 持有 9 個 CBedstead 指標與一個次序計時器，依難度（0/1/2 = Easy/Normal/Hard）
// 在病床空位數量低於門檻時開始累計時間，達到秒數後一次補入 1~2 名新病人。
//
// 對應 mofclient.c 的欄位佈局（32-bit 原始大小 108 bytes，本還原以邏輯成員
// 重新表達；行為等價）：
//   DWORD[0..8]   = m_beds[9]         9 張病床指標
//   DWORD[9..17]  = m_freeIndices[9]  AutoPatientRecall 暫存：當前空床索引
//   +72           = m_timer           Mini_Timer
//   DWORD[24]     = m_difficulty
//   BYTE [100]    = m_isAccumulating  Process 中是否正在累計補人秒數
//   DWORD[26]     = m_dword26         建構子歸零、執行期未實際使用
class PatientRecallMgr {
public:
    PatientRecallMgr();
    ~PatientRecallMgr();

    void InitPatientRecallMgr(int difficulty, CBedstead* firstBed);
    void AutoPatientRecall();
    void Process();

public:
    CBedstead*    m_beds[9];
    int           m_freeIndices[9];
    Mini_Timer    m_timer;
    int           m_difficulty;
    std::uint8_t  m_isAccumulating;
    std::uint8_t  m_pad101[3];
    int           m_dword26;
};
