#pragma once
//
// UIMSG — mofclient.c 還原（位址 0x455D30）
//
// 6 × DWORD 的 UI event 資料包，主要被 CUI*::OnEvent_* 系列當作參數陣列。
// 由 ground truth：
//   - this[0]  ← (int)-1     // event ID 預設 -1
//   - this[1]  ← CControlBase* 來源控制項
//   - this[2..5]              // payload，預設 0
//
#include <cstdint>

class CControlBase;

class UIMSG {
public:
    UIMSG(CControlBase* sender);

public:
    int           m_nEventID;     // +0
    CControlBase* m_pSender;      // +4
    int           m_aParam[4];    // +8..+23
};
