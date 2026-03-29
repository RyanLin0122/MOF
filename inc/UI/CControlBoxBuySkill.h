#pragma once

#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"

// 反編譯：4 組欄位（y: 99, 113, 127, 141）
static constexpr int kBuySkillRowCount = 4;

// 買技能資訊盒
// 反編譯對照：
//   this+120:  CControlText  m_Title
//   this+552:  CControlImage m_Icon
//   this+744:  CControlText  m_NeedPT
//   this+1176: CControlText  m_Left[4]  (每個 0x1B0=432 bytes)
//   this+2904: CControlText  m_Right[4]
//   this+4632: CControlImage m_BottomDecor
class CControlBoxBuySkill : public CControlBase
{
public:
    CControlBoxBuySkill();
    virtual ~CControlBoxBuySkill();

    // 對齊反編譯 0045C9B0
    virtual void Show() override;

    CControlText  m_Title;                        // +120
    CControlImage m_Icon;                         // +552
    CControlText  m_NeedPT;                       // +744
    CControlText  m_Left[kBuySkillRowCount];      // +1176
    CControlText  m_Right[kBuySkillRowCount];     // +2904
    CControlImage m_BottomDecor;                  // +4632
};
