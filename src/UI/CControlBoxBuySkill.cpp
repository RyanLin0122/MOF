#include <cstdio>
#include "UI/CControlBoxBuySkill.h"
#include "global.h"

// 對齊反編譯 0045C980
CControlBoxBuySkill::CControlBoxBuySkill()
    : CControlBase()
    , m_Title()
    , m_Icon()
    , m_NeedPT()
    , m_Left()
    , m_Right()
    , m_BottomDecor()
{
    // *((_DWORD *)this + 13) = 1 → m_bPassKeyInputToParent
    m_bPassKeyInputToParent = true;

    // m_Title (+120)
    m_Title.Create(this);
    m_Title.SetPos(59, 20);
    m_Title.m_isCentered = 1;  // *((_DWORD *)this + 66) = 1

    // m_Icon (+552)
    m_Icon.Create(this);
    m_Icon.SetPos(44, 43);

    // m_NeedPT (+744)
    m_NeedPT.Create(this);
    m_NeedPT.SetControlSetFont("BuySkillNeetPT");
    m_NeedPT.SetPos(59, 80);
    m_NeedPT.SetText(3601);
    m_NeedPT.m_isCentered = 1;                       // *((_DWORD *)this + 222) = 1
    m_NeedPT.m_TextColor = static_cast<DWORD>(-6944247); // *((_DWORD *)this + 223)

    // 4 行左/右欄（y: 99, 113, 127, 141）
    int y = 99;
    for (int i = 0; i < kBuySkillRowCount; ++i)
    {
        // 左欄標籤
        m_Left[i].Create(this);
        m_Left[i].SetPos(30, y);

        const char* key = g_DCTTextManager.GetText(3061 + i);
        char buf[256]{};
        sprintf(buf, "%s :", key);
        m_Left[i].SetText(buf);

        // 右欄數值
        m_Right[i].Create(this);
        m_Right[i].SetPos(66, y);

        y += 14;
    }

    // m_BottomDecor (+4632)
    m_BottomDecor.Create(this);
    m_BottomDecor.SetImage(570425350u, 8);
}

// 對齊反編譯 0045C9F0
CControlBoxBuySkill::~CControlBoxBuySkill()
{
    // 子物件自動逆序解構
}

// 對齊反編譯 0045C9B0
void CControlBoxBuySkill::Show()
{
    CControlBase::Show();
    // vtbl+44 = Hide()
    m_BottomDecor.Hide();
}
