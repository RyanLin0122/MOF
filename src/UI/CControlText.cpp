#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "UI/CControlText.h"
#include "Font/MoFFont.h"
#include "Font/stFontInfo.h"
#include "Text/DCTTextManager.h"
#include "global.h"

// ---- 小工具：將數字轉為含逗號字串 ----
void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int /*flagUnused*/)
{
    char buf[64];
    sprintf(buf, "%u", value);
    std::string s(buf);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
        s.insert(static_cast<size_t>(i), ",");
    strncpy(out, s.c_str(), outSize - 1);
    out[outSize - 1] = '\0';
}

// ========================================
// 建構 / 解構
// ========================================
CControlText::CControlText()
{
    // 對齊反編譯：預設值已在欄位初始化
    // this[14] = 0 → m_nArrayIndex 或類似（CControlBase 會處理）
}

CControlText::~CControlText()
{
    m_Text.clear();
}

// ========================================
// ClearData
// ========================================
void CControlText::ClearData()
{
    CControlBase::ClearData();
    m_Text.clear();
}

// ========================================
// SetText(char*)
// ========================================
void CControlText::SetText(char* a2)
{
    if (!a2)
    {
        // 對齊反編譯：null 時設為空字串
        m_Text = "";
        return;
    }

    // 對齊反編譯：確保預設字型名稱
    if (!m_FontFaceA[0])
        strcpy(m_FontFaceA, g_DCTTextManager.GetText(3264));

    // 對齊反編譯：比較相同則不更新
    if (m_Text.c_str() && strcmp(a2, m_Text.c_str()) == 0)
        return;

    m_LineCountDirty = 1;
    m_Text = a2;
}

// ========================================
// SetText(int)
// ========================================
void CControlText::SetText(int a2)
{
    if (a2 >= 0)
    {
        char* v3 = g_DCTTextManager.GetText(a2);
        SetText(v3);
    }
}

// ========================================
// SetParsedText(int)
// ========================================
void CControlText::SetParsedText(int a2)
{
    if (a2 >= 0)
    {
        char* v3 = g_DCTTextManager.GetParsedText(a2, 0, 0);
        SetText(v3);
    }
}

// ========================================
// ClearText
// ========================================
void CControlText::ClearText()
{
    m_Text.clear();
}

// ========================================
// 字型設定
// ========================================
void CControlText::SetFontHeight(int a2)
{
    m_FontHeight = a2;
}

void CControlText::SetFontWeight(int a2)
{
    m_FontWeight = a2;
}

void CControlText::SetControlSetFont(const char* a2)
{
    stFontInfo* v3 = g_MoFFont.GetFontInfo(a2);
    if (v3)
    {
        m_FontHeight = v3->nHeight;
        m_FontWeight = v3->nWeight;
        // 對齊反編譯：strcpy((char*)this + 176, (const char*)v3 + 128)
        strcpy(m_FontFaceA, v3->szFaceName);
    }
}

void CControlText::SetMultiLineSpace(int a2)
{
    m_LineSpacing = a2;
}

void CControlText::SetMultiLineSize(short a2, int a3)
{
    m_usWidth = static_cast<uint16_t>(a2);
    m_usHeight = static_cast<uint16_t>(a3);
}

int CControlText::GetMultiLineSpace()
{
    return m_LineSpacing;
}

// ========================================
// Draw
// ========================================
void CControlText::Draw()
{
    if (!IsVisible() || m_Text.empty())
        return;

    int v2 = GetAbsX();
    int v3 = GetAbsY();

    // 對齊反編譯：SetFont(fontHeight, (LPCSTR)this+176, fontWeight)
    g_MoFFont.SetFont(m_FontHeight, m_FontFaceA, m_FontWeight);

    const char* v4 = m_Text.c_str();
    int v5 = (static_cast<int>(m_usHeight) << 16) | m_usWidth; // packed size

    if (m_usWidth != 0 && m_usHeight != 0)
    {
        // 多行方框繪製
        RECT rc;
        unsigned short boxW = m_usWidth;
        unsigned short boxH = m_usHeight;

        if (m_ShadowColor)
        {
            // 陰影偏移 +1, +1
            SetRect(&rc, v2 + 1, v3 + 1, boxW, boxH);
            g_MoFFont.SetTextBoxA(&rc, m_ShadowColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);
        }
        else if (m_OutlineColor)
        {
            // 八方向外框
            int xRight = boxW - 1;
            SetRect(&rc, v2 - 1, v3, xRight, boxH);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2 - 1, v3 - 1, xRight, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2, v3 - 1, boxW, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2 + 1, v3 - 1, boxW + 1, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2 + 1, v3, boxW + 1, boxH);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2 + 1, v3 + 1, boxW + 1, boxH + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2, v3 + 1, boxW, boxH + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);

            SetRect(&rc, v2 - 1, v3 + 1, xRight, boxH + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);
        }

        // 主文字
        SetRect(&rc, v2, v3, boxW, boxH);
        g_MoFFont.SetTextBoxA(&rc, m_TextColor, v4, m_LineSpacing, (char)m_isCentered, m_WantSpaceFirstByte);
    }
    else
    {
        // 單行繪製
        if (m_ShadowColor)
        {
            g_MoFFont.SetTextLineA(v2 + 1, v3 + 1, m_ShadowColor, v4, (char)m_isCentered, -1, -1);
        }
        else if (m_OutlineColor)
        {
            g_MoFFont.SetTextLineShadow(v2, v3, m_OutlineColor, v4, (char)m_isCentered);
        }
        g_MoFFont.SetTextLineA(v2, v3, m_TextColor, v4, (char)m_isCentered, -1, -1);
    }
}

// ========================================
// GetTextLength
// ========================================
DWORD* CControlText::GetTextLength(DWORD* a2)
{
    if (!m_Text.empty())
    {
        const char* v3 = m_Text.c_str();

        // 對齊反編譯：直接傳 LPCSTR fontFace，不轉 wchar_t
        int w = 0, h = 0;
        g_MoFFont.GetTextLength(&w, &h, m_FontHeight, m_FontFaceA, v3, m_FontWeight);

        // 對齊反編譯：回傳 packed {WORD width, WORD height}
        a2[0] = (static_cast<DWORD>(h & 0xFFFF) << 16) | static_cast<DWORD>(w & 0xFFFF);
    }
    else
    {
        a2[0] = 0;
        a2[1] = 0;
    }
    return a2;
}

// ========================================
// GetCalcedTextBoxHeight
// ========================================
int CControlText::GetCalcedTextBoxHeight(unsigned short a2)
{
    if (m_Text.empty())
        return 0;
    unsigned short v3 = a2;
    if (!a2)
        v3 = m_usWidth;
    return (m_FontHeight + m_LineSpacing) * static_cast<int>(GetMultiTextLineCount(v3));
}

// ========================================
// SetTextItoa
// ========================================
void CControlText::SetTextItoa(int Value)
{
    // 對齊反編譯：不設 m_LineCountDirty
    char Buffer[256];
    strcpy(Buffer, "");
    _itoa(Value, Buffer, 10);
    m_Text = Buffer;
}

// ========================================
// SetTextMoney
// ========================================
void CControlText::SetTextMoney(unsigned int Value)
{
    char v3[256];
    Numeric2MoneyByComma(Value, v3, 256, 0);
    SetText(v3);
}

void CControlText::SetTextMoney(int a2, unsigned int Value)
{
    char v5[256];
    char Buffer[256];
    Numeric2MoneyByComma(Value, v5, 256, 0);
    const char* v4 = g_DCTTextManager.GetText(a2);
    sprintf(Buffer, v4, v5);
    SetText(Buffer);
}

void CControlText::SetParsedTextMoney(int a2, unsigned int Value)
{
    char v5[256];
    char Buffer[256];
    Numeric2MoneyByComma(Value, v5, 256, 0);
    const char* v4 = g_DCTTextManager.GetParsedText(a2, 0, 0);
    sprintf(Buffer, v4, v5);
    SetText(Buffer);
}

// ========================================
// PtInCtrl
// ========================================
BOOL CControlText::PtInCtrl(int pt, int pt_4)
{
    if (!IsVisible())
        return FALSE;

    DWORD v5[2] = { 0, 0 };
    GetTextLength(v5);
    int w = v5[0] & 0xFFFF;
    int h = (v5[0] >> 16) & 0xFFFF;
    if (!w || !h)
        return FALSE;

    int xLeft, yTop;
    GetAbsPos(xLeft, yTop);

    RECT rc;
    SetRect(&rc, xLeft, yTop, xLeft + w, yTop + h);
    POINT point = { pt, pt_4 };
    return PtInRect(&rc, point);
}

// ========================================
// GetText
// ========================================
char* CControlText::GetText()
{
    return const_cast<char*>(m_Text.c_str());
}

// ========================================
// SetTextPosToParentCenter
// ========================================
void CControlText::SetTextPosToParentCenter()
{
    SetCenterPos();
    m_isCentered = 1;
    SetY(GetY() - 5);
}

// ========================================
// GetFontHeight / GetFontFace
// ========================================
int CControlText::GetFontHeight()
{
    return m_FontHeight;
}

char* CControlText::GetFontFace()
{
    return m_FontFaceA;
}

// ========================================
// GetCharByteByLine
// ========================================
int CControlText::GetCharByteByLine(unsigned char* a2, int a3)
{
    if (m_Text.empty())
        return 0;

    const char* v4 = m_Text.c_str();
    unsigned short v5 = GetWidth();

    return g_MoFFont.GetCharByteByLine(v5, v4, a2, a3);
}

// ========================================
// SetWantSpaceFirstByte
// ========================================
void CControlText::SetWantSpaceFirstByte(int a2)
{
    m_WantSpaceFirstByte = a2;
}

// ========================================
// IsStringData
// ========================================
BOOL CControlText::IsStringData()
{
    return !m_Text.empty() ? TRUE : FALSE;
}

// ========================================
// GetMultiTextLineCount
// ========================================
unsigned char CControlText::GetMultiTextLineCount(unsigned short a2)
{
    if (m_Text.empty())
        return 0;

    if (!m_LineCountDirty)
        return m_CachedLineCount;

    unsigned short v4 = a2;
    m_LineCountDirty = 0;
    if (!a2)
        v4 = m_usWidth;

    const char* v5 = m_Text.c_str();

    // 對齊反編譯：MoFFont::GetLineCountByWidth(width, text, fontFace, fontH, fontW)
    m_CachedLineCount = g_MoFFont.GetLineCountByWidth(v4, v5, m_FontFaceA, m_FontHeight, m_FontWeight);
    return m_CachedLineCount;
}

// ========================================
// SetText(const char*)
// ========================================
void CControlText::SetText(const char* a2)
{
    SetText(const_cast<char*>(a2));
}

// ========================================
// GetText() const
// ========================================
const char* CControlText::GetText() const
{
    return m_Text.c_str();
}

// ========================================
// GetTextPixelSize
// ========================================
void CControlText::GetTextPixelSize(int* pWidth, int* pHeight)
{
    if (m_Text.empty())
    {
        if (pWidth)  *pWidth = 0;
        if (pHeight) *pHeight = 0;
        return;
    }

    // 對齊反編譯：直接傳 char* fontFace
    g_MoFFont.GetTextLength(pWidth, pHeight, m_FontHeight, m_FontFaceA, m_Text.c_str(), m_FontWeight);
}

// ========================================
// SetTextFmtW
// ========================================
void CControlText::SetTextFmtW(const wchar_t* format, ...)
{
    wchar_t wBuf[2048];
    va_list args;
    va_start(args, format);
    _vsnwprintf(wBuf, sizeof(wBuf) / sizeof(wBuf[0]) - 1, format, args);
    va_end(args);
    wBuf[sizeof(wBuf) / sizeof(wBuf[0]) - 1] = L'\0';

    // wchar_t -> multibyte
    char mbBuf[4096];
    WideCharToMultiByte(CP_ACP, 0, wBuf, -1, mbBuf, sizeof(mbBuf), NULL, NULL);
    SetText(mbBuf);
}
