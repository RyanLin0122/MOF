#include "UI/CControlScrollText.h"
#include "global.h"
#include <cstring>
#include <cstdlib>

// 對齊反編譯：static local 變數
static int s_nTempScroll = 0;

// ========================================
// 建構 / 解構
// ========================================
CControlScrollText::CControlScrollText()
{
    // 對齊反編譯
    strcpy(m_szFontFace, "");
    m_pText = nullptr;
    m_TextColor = 0xFF000000;   // -16777216
    m_nShadowFlag = 0;
    m_ShadowColor2 = 0xFF000000;
    m_bHasShadow = 0;
    m_nTextOffsetX = 0;
    m_nTextOffsetY = 0;
    m_usTextW = 0;
    m_usTextH = 0;
    m_nFontHeight = 11;
    m_nFontWeight = 400;
}

CControlScrollText::~CControlScrollText()
{
    if (m_pText)
    {
        delete[] m_pText;
        m_pText = nullptr;
    }
}

// ========================================
// Create
// ========================================
void CControlScrollText::Create(int a2, int a3, unsigned short a4, unsigned short a5,
                                  int a6, int a7, unsigned short a8, unsigned short a9,
                                  unsigned int a10, CControlBase* a11,
                                  int a12, int a13, char* a14, int a15, char* a16)
{
    CControlScrollBar::Create(a2, a3, a4, a5, a11, 0);
    m_nTextOffsetX = a6;
    m_nTextOffsetY = a7;
    SetTextSize(a8, a9);
    m_nFontHeight = a12;
    m_nFontWeight = a13;
    m_TextColor = a10;
    m_nShadowFlag = a15;
    strcpy(m_szFontFace, a14);

    // 對齊反編譯：if (a16 && !strcmp(a16, "")) SetText(a16, 0); else SetText("", 0);
    if (a16 && !strcmp(a16, ""))
        SetText(a16, 0);
    else
        SetText((char*)"", 0);
}

// ========================================
// Show
// ========================================
void CControlScrollText::Show()
{
    CControlScrollBar::Show();
    CControlScrollBar::ClearScroll();
    CControlScrollBar::SetScrollRange(0, m_TextBox.GetTotalLineCount(),
                                       m_TextBox.GetVisibleLineCount(), 1, 0);
}

// ========================================
// Draw
// ========================================
void CControlScrollText::Draw()
{
    // 對齊反編譯：直接檢查 this[12]（IsVisible）
    if (!IsVisible())
        return;

    // 對齊反編譯：直接 strlen，無 nullptr guard
    if (strlen(m_pText))
    {
        // 計算絕對座標
        CControlBase* v2 = GetParent();
        int v3 = m_nTextOffsetX;
        int i = m_nTextOffsetY;
        while (v2)
        {
            v3 += v2->GetX();
            i += v2->GetY();
            v2 = v2->GetParent();
        }

        // 畫陰影
        if (m_bHasShadow)
        {
            m_TextBox.TextBoxDraw(v3 + 1, i + 1, m_ShadowColor2, 5,
                                   static_cast<unsigned char>(m_nShadowFlag), 400);
        }

        // 對齊反編譯：第 6 參數取自 *((_BYTE *)this + 3120) = (unsigned char)m_nShadowFlag
        m_TextBox.TextBoxDraw(v3, i, m_TextColor, 5,
                               static_cast<unsigned char>(m_nShadowFlag), 400);

        CControlBase::Draw();
    }
    else
    {
        CControlBase::Draw();
    }
}

// ========================================
// SetText
// ========================================
void CControlScrollText::SetText(char* a2, int a3)
{
    if (a2)
    {
        if (m_pText)
        {
            if (strcmp(m_pText, a2) == 0)
                return;
            delete[] m_pText;
            m_pText = nullptr;
        }
        CControlScrollBar::ClearScroll();
        m_pText = new char[strlen(a2) + 1];
        strcpy(m_pText, a2);
    }
    else
    {
        if (m_pText)
        {
            delete[] m_pText;
            m_pText = nullptr;
        }
        CControlScrollBar::ClearScroll();
        m_pText = new char[2];
        strcpy(m_pText, "");
    }

    m_TextBox.SetTextBoxScroll(m_nFontHeight, m_szFontFace, m_pText,
                                m_usTextW, m_usTextH,
                                GetScrollPos(),
                                m_nFontWeight, a3, 5);
    CControlScrollBar::SetScrollRange(0, m_TextBox.GetTotalLineCount(),
                                       m_TextBox.GetVisibleLineCount(), 1, 0);
}

// ========================================
// RefreshText
// ========================================
void CControlScrollText::RefreshText(int a2)
{
    CControlScrollBar::ClearScroll();
    m_TextBox.SetTextBoxScroll(m_nFontHeight, m_szFontFace, m_pText,
                                m_usTextW, m_usTextH,
                                GetScrollPos(),
                                m_nFontWeight, a2, 5);
    CControlScrollBar::SetScrollRange(0, m_TextBox.GetTotalLineCount(),
                                       m_TextBox.GetVisibleLineCount(), 1, 0);
}

// ========================================
// ControlKeyInputProcess
// ========================================
int* CControlScrollText::ControlKeyInputProcess(int a2, int a3, int a4, int a5, int a6, int a7)
{
    int* result = CControlScrollBar::ControlKeyInputProcess(a2, a3, a4, a5, a6, a7);
    if (a2 != 7 && a2 != -1)
        ScrollTextSetLine();
    return result;
}

// ========================================
// ChildKeyInputProcess
// ========================================
void CControlScrollText::ChildKeyInputProcess(int a2, int a3, int a4, int a5, int a6, int a7)
{
    CControlScrollBar::ChildKeyInputProcess(a2, a3, a4, a5, a6, a7);
    if (a2 != 7 && a2 != -1)
        ScrollTextSetLine();
}

// ========================================
// ScrollTextSetLine
// ========================================
void CControlScrollText::ScrollTextSetLine()
{
    m_TextBox.SetLine(GetScrollPos());
    s_nTempScroll = GetScrollPos();
}

// ========================================
// SetScrollPosBottom
// ========================================
void CControlScrollText::SetScrollPosBottom()
{
    // 對齊反編譯：使用 this[31]（m_scrollMax），不是 GetScrollMax()
    int v2 = m_scrollMax;
    if (v2 > 0)
    {
        Scroll(4, v2, 1);
        ScrollTextSetLine();
    }
}

// ========================================
// SetShadowColor
// ========================================
void CControlScrollText::SetShadowColor(unsigned int a2)
{
    m_bHasShadow = 1;
    m_ShadowColor2 = a2;
}

// ========================================
// SetTextY / GetTextY
// ========================================
void CControlScrollText::SetTextY(int a2)
{
    m_nTextOffsetY = a2;
}

int CControlScrollText::GetTextY()
{
    return m_nTextOffsetY;
}

// ========================================
// SetTextSize
// ========================================
void CControlScrollText::SetTextSize(unsigned short a2, unsigned short a3)
{
    m_usTextW = a2;
    m_usTextH = a3;
}
