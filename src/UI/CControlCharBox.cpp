#include "UI/CControlCharBox.h"
#include "global.h"

// 對齊反編譯 0041F830
CControlCharBox::CControlCharBox()
    : CControlBase()
    , m_TextName()
    , m_TextClass()
    , m_TextLevel()
    , m_CA()
    , m_Button()
    , m_PasyImage()
{
    CreateChildren();
    m_bSelected = 0;
    m_lastAnimTick = timeGetTime();
    m_AnimFrame = 0;
}

// 對齊反編譯 0041F920
CControlCharBox::~CControlCharBox()
{
    // 子物件自動逆序解構
    // 反編譯順序：m_PasyImage, m_Button, CCA(m_CA+120), CControlBase(m_CA), m_TextLevel, m_TextClass, m_TextName, CControlBase(this)
}

// 對齊反編譯 0041F9E0
void CControlCharBox::CreateChildren()
{
    // m_CA (+1416): Create(this) + SetPos(28, 97)
    m_CA.Create(this);
    m_CA.SetPos(28, 97);

    // m_TextName (+120): Create(this) + m_isCentered + SetTextColor + SetPos
    m_TextName.Create(this);
    m_TextName.m_isCentered = 1;                         // *((_DWORD *)this + 66) = 1
    m_TextName.m_TextColor = static_cast<DWORD>(-7590900); // *((_DWORD *)this + 67)
    m_TextName.SetPos(114, 15);

    // m_TextClass (+552): Create(this) + m_isCentered + SetTextColor + SetPos
    m_TextClass.Create(this);
    m_TextClass.m_isCentered = 1;                         // *((_DWORD *)this + 174) = 1
    m_TextClass.m_TextColor = static_cast<DWORD>(-7590900); // *((_DWORD *)this + 175)
    m_TextClass.SetPos(114, 32);

    // m_TextLevel (+984): Create(this) + m_isCentered + SetTextColor + SetPos
    m_TextLevel.Create(this);
    m_TextLevel.m_isCentered = 1;                         // *((_DWORD *)this + 282) = 1
    m_TextLevel.m_TextColor = static_cast<DWORD>(-7590900); // *((_DWORD *)this + 283)
    m_TextLevel.SetPos(114, 50);

    // m_Button (+1828): Create(this) + SetPos + SetImage + SetText + SetButtonTextColor + SetChildMoveByClick
    m_Button.Create(this);
    m_Button.SetPos(98, 78);
    // SetImage(imageId=0x2000002C=536870956, blockN=0, blockH=1, blockP=2, blockD=3)
    m_Button.SetImage(536870956u, 0, 1, 2, 3);
    // SetText(stringId=3466)
    m_Button.SetText(3466);
    m_Button.SetButtonTextColor(0xFF000000, 0xFFEAFF00, 0xFFEAFF00, 0xFFC99273);
    m_Button.SetChildMoveByClick(1, 1);

    // m_PasyImage (+2552): Create(this) + SetPos(41, 90)
    m_PasyImage.Create(this);
    m_PasyImage.SetPos(41, 90);
}

// 對齊反編譯 0041FB20
void CControlCharBox::NoExistChar()
{
    m_TextName.SetText(3467);
    m_TextClass.SetText(3468);
    m_TextLevel.SetText(3469);
    m_CA.Hide();           // vtbl+44
    m_PasyImage.Show();    // vtbl+40
}

// 對齊反編譯 0041FB70
void CControlCharBox::ExistChar()
{
    m_TextLevel.Show();    // vtbl+40
    m_TextClass.Show();    // vtbl+40
    m_TextName.Show();     // vtbl+40
    m_CA.Show();           // vtbl+40
    m_PasyImage.Hide();    // vtbl+44
}

// 對齊反編譯 0041FBF0
void CControlCharBox::SelectChar()
{
    m_bSelected = 1;
    if (m_CA.IsVisible())  // *((_DWORD *)this + 366) = m_CA.m_bIsVisible
        m_CA.SetCharMotion(0x0F);  // motion 15
}

// 對齊反編譯 0041FC20（不是0041FBF0）
void CControlCharBox::SelectCancelChar()
{
    m_bSelected = 0;
    if (m_CA.IsVisible())  // *((_DWORD *)this + 366) = m_CA.m_bIsVisible
        m_CA.SetCharMotion(1);  // motion 1
}

// 對齊反編譯 0041FC20
int CControlCharBox::IsSelect()
{
    return m_bSelected;
}

// 對齊反編譯 0041FC30
void CControlCharBox::PasyImageDraw()
{
    DWORD now = timeGetTime();
    if (now - m_lastAnimTick > 0x46)  // 70ms
    {
        m_lastAnimTick = now;

        unsigned int imageId;
        if (m_bSelected)
            imageId = 201327297u;  // 0x0C000741 — 選取時的圖
        else
            imageId = 201327245u;  // 0x0C00070D — 未選取時的圖

        m_PasyImage.SetImage(imageId, m_AnimFrame);

        m_AnimFrame++;
        if (m_AnimFrame == 8)
            m_AnimFrame = 0;
    }
}

// 對齊反編譯 0041FCA0
void CControlCharBox::PrepareDrawing()
{
    // *((_DWORD *)this + 650) 對應 m_PasyImage 是否 visible
    if (m_PasyImage.IsVisible())
        PasyImageDraw();
    CControlBase::PrepareDrawing();
}
