#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"

// 依反編譯：4 組欄位
static constexpr int kBuySkillRowCount = 4;

/**
 * 買技能資訊盒
 * 版面：標題、圖示、需求點數文字、4 行(左標籤/右數值)、底部裝飾線
 */
class CControlBoxBuySkill : public CControlBase
{
public:
    CControlBoxBuySkill();
    virtual ~CControlBoxBuySkill();

    // 顯示時需同步隱藏底部裝飾（對齊反編譯 Show 內的 vtbl+44 呼叫）
    virtual void Show() override;

    // 取用子件（若外部要填右欄數值）
    CControlText& Title() { return m_Title; }
    CControlImage& Icon() { return m_Icon; }
    CControlText& NeedPT() { return m_NeedPT; }
    CControlText& Left(int i) { return m_Left[i]; }   // 0..3
    CControlText& Right(int i) { return m_Right[i]; }  // 0..3

private:
    void CreateChildren(); // 於 ctor 內呼叫（對齊反編譯）

private:
    CControlText  m_Title;                        // +120
    CControlImage m_Icon;                         // +552
    CControlText  m_NeedPT;                       // +744
    CControlText  m_Left[kBuySkillRowCount];      // +1176 起，每個 0x1B0
    CControlText  m_Right[kBuySkillRowCount];     // +2904 起
    CControlImage m_BottomDecor;                  // +4632
};
