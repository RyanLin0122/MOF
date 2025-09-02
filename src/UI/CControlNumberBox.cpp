#include <cstdlib>
#include "UI/CControlNumberBox.h"

CControlNumberBox::CControlNumberBox()
    : CControlBase()
    , m_Frame()
    , m_Text()
{
    // 建立底框到本控制
    m_Frame.Create(this);
    // 反編譯：SetAttr(13, -2, 19, 15, 0,0,0, 0.50196081)
    m_Frame.SetAttr(13, -2, 19, 15, 0.0f, 0.0f, 0.0f, 0.50196081f);

    // 文字掛在底框之下（反編譯第二參數為 +120）
    m_Text.Create(&m_Frame);
    m_Text.SetControlSetFont("NumberBox");
    m_Text.SetPos(9, 2);

    // 反編譯中的兩個欄位
    m_FlagA = 1;   // *((DWORD*)this + 118) = 1
    m_FlagB = -1;  // *((DWORD*)this + 119) = -1
}

CControlNumberBox::~CControlNumberBox()
{
    // 成員自動解構；無額外資源
}

void CControlNumberBox::SetNumber(int value)
{
    // 對齊 0041F190
    m_Text.SetTextItoa(value);
}

int CControlNumberBox::GetNumber()
{
    // 對齊 0041F1B0
    const char* s = m_Text.GetText();
    return std::atoi(s);
}
