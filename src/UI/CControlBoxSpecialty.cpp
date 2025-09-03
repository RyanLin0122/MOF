#include "UI/CControlBoxSpecialty.h"

CControlBoxSpecialty::CControlBoxSpecialty()
    : CControlBoxBase()
    , m_icon()
    , m_title()
    , m_rightTxt()
    , m_frame()
{
    // vftable 由編譯器處理
    CreateChildren();
    CControlBoxBase::Init();
}

CControlBoxSpecialty::~CControlBoxSpecialty()
{
    // 成員自動解構即可；順序與反編譯相符但不需手動呼叫
}

void CControlBoxSpecialty::CreateChildren()
{
    // 背景
    CControlBoxBase::CreateChildren();
    if (auto* bg = GetBackground())
        bg->SetPos(0, 0);

    // +312：主圖示 (資源 0x2200000B, frame 0x17)，位置 (7,10)
    m_icon.Create(this);
    m_icon.SetPos(7, 10);
    m_icon.SetImage(0x2200000Bu, 0x17u);

    // +504：左上文字，位置 (43,6)，顏色 -7590900
    m_title.Create(this);
    m_title.SetPos(43, 6);
    m_title.SetTextColor(-7590900);  // *((_DWORD*)this + 163) = -7590900

    // +936：右側文字，位置 (130,20)，顏色 -7590900
    m_rightTxt.Create(this);
    m_rightTxt.SetPos(130, 20);
    m_rightTxt.SetTextColor(-7590900); // *((_DWORD*)this + 271) = -7590900

    // +1368：外框/底圖，位置 (-3,-3)，資源 0x20000047, frame 0
    m_frame.Create(this);
    m_frame.SetPos(-3, -3);
    m_frame.SetImage(0x20000047u, 0);
}
