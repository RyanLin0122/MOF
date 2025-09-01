#include <cstdio>
#include <cstring>
#include <algorithm>
#include "UI/CControlText.h"

// ---- 小工具：將數字轉為含逗號字串 ----
static void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int /*flagUnused*/)
{
    char buf[64];
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%u", value);
#else
    std::snprintf(buf, sizeof(buf), "%u", value);
#endif
    std::string s(buf);
    // 插入逗號
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
        s.insert(static_cast<size_t>(i), ",");
#if defined(_MSC_VER)
    strncpy_s(out, outSize, s.c_str(), _TRUNCATE);
#else
    std::snprintf(out, static_cast<size_t>(outSize), "%s", s.c_str());
#endif
}

CControlText::CControlText()
{
    // 預設值已在欄位初始化
}

CControlText::~CControlText()
{
    // std::string 自行釋放
}

void CControlText::ClearData()
{
    CControlBase::ClearData();
    ClearText();
    m_ShadowColor = 0;
    m_OutlineColor = 0;
    m_LineSpacing = 5;
    m_WantSpaceFirstByte = 0;
    m_LineCountDirty = true;
    m_CachedLineCount = 1;
}

void CControlText::EnsureDefaultFontFace()
{
    if (m_FontFaceA[0] == '\0')
    {
        // 與反編譯一致：ID=3264 的字串做為預設字型名稱
        const char* face = g_DCTTextManager.GetText(3264);
        if (face)
        {
#if defined(_MSC_VER)
            strncpy_s(m_FontFaceA, face, _TRUNCATE);
#else
            std::strncpy(m_FontFaceA, face, sizeof(m_FontFaceA) - 1);
            m_FontFaceA[sizeof(m_FontFaceA) - 1] = '\0';
#endif
            SyncWideFace();
        }
    }
}

void CControlText::SyncWideFace()
{
    // 將 ANSI 轉為寬字元（簡單 ACP 轉換）
    if (m_FontFaceA[0] == '\0') { m_FontFaceW[0] = L'\0'; return; }
    int len = MultiByteToWideChar(CP_ACP, 0, m_FontFaceA, -1, nullptr, 0);
    if (len <= 0) { m_FontFaceW[0] = L'\0'; return; }
    len = std::min<int>(len, static_cast<int>(std::size(m_FontFaceW)));
    MultiByteToWideChar(CP_ACP, 0, m_FontFaceA, -1, m_FontFaceW, len);
    m_FontFaceW[std::size(m_FontFaceW) - 1] = L'\0';
}

void CControlText::SetText(const char* text)
{
    EnsureDefaultFontFace();
    if (!text) text = "";
    if (m_Text != text)
    {
        m_Text = text;
        m_LineCountDirty = true; // 文字變更需重算行數
    }
}

void CControlText::SetText(int stringId)
{
    if (stringId >= 0)
        SetText(g_DCTTextManager.GetText(stringId));
}

void CControlText::SetParsedText(int stringId)
{
    if (stringId >= 0)
        SetText(g_DCTTextManager.GetParsedText(stringId, 0, nullptr));
}

void CControlText::ClearText()
{
    if (!m_Text.empty())
    {
        m_Text.clear();
        m_LineCountDirty = true;
    }
}

void CControlText::SetFontHeight(int h)
{
    m_FontHeight = h;
}

void CControlText::SetFontWeight(int w)
{
    m_FontWeight = w;
}

void CControlText::SetControlSetFont(const char* fontKey)
{
    stFontInfo* info = g_MoFFont.GetFontInfo(fontKey);
    if (info)
    {
        m_FontHeight = info->nHeight;
        m_FontWeight = info->nWeight;
        // faceName 為寬字元
        size_t n = std::wcstombs(nullptr, info->wszFaceName, 0);
        if (n != static_cast<size_t>(-1))
        {
            n = min(n, sizeof(m_FontFaceA) - 1);
            std::wcstombs(m_FontFaceA, info->wszFaceName, n);
            m_FontFaceA[n] = '\0';
        }
        else
        {
#if defined(_MSC_VER)
            strcpy_s(m_FontFaceA, "");
#else
            m_FontFaceA[0] = '\0';
#endif
        }
        // 同步寬字元版本
        wcsncpy_s(m_FontFaceW, info->wszFaceName, _TRUNCATE);
    }
}

const char* CControlText::GetFontFace()
{
    return m_FontFaceA;
}

void CControlText::SetMultiLineSpace(int space)
{
    m_LineSpacing = space;
}

void CControlText::SetMultiLineSize(uint16_t width, int height)
{
    m_DefaultMultiWidth = width;
    m_DefaultMultiHeight = static_cast<uint16_t>(std::clamp(height, 0, 0xFFFF));
}

int CControlText::GetCalcedTextBoxHeight(uint16_t width)
{
    if (m_Text.empty()) return 0;
    if (width == 0) width = m_DefaultMultiWidth;
    unsigned char lines = GetMultiTextLineCount(width);
    return (m_FontHeight + m_LineSpacing) * static_cast<int>(lines);
}

void CControlText::SetTextItoa(int value)
{
    char buf[256];
#if defined(_MSC_VER)
    sprintf_s(buf, "%d", value);
#else
    std::snprintf(buf, sizeof(buf), "%d", value);
#endif
    SetText(buf);
}

void CControlText::SetTextMoney(unsigned int value)
{
    char buf[256];
    Numeric2MoneyByComma(value, buf, 256, 0);
    SetText(buf);
}

void CControlText::SetTextMoney(int textIdFmt, unsigned int value)
{
    char money[256];
    Numeric2MoneyByComma(value, money, 256, 0);
    const char* fmt = g_DCTTextManager.GetText(textIdFmt);
    char buf[256];
#if defined(_MSC_VER)
    sprintf_s(buf, fmt ? fmt : "%s", money);
#else
    std::snprintf(buf, sizeof(buf), "%s", money);
    if (fmt) { std::snprintf(buf, sizeof(buf), fmt, money); }
#endif
    SetText(buf);
}

void CControlText::SetParsedTextMoney(int textIdFmt, unsigned int value)
{
    char money[256];
    Numeric2MoneyByComma(value, money, 256, 0);
    const char* fmt = g_DCTTextManager.GetParsedText(textIdFmt, 0, nullptr);
    char buf[256];
#if defined(_MSC_VER)
    sprintf_s(buf, fmt ? fmt : "%s", money);
#else
    std::snprintf(buf, sizeof(buf), "%s", money);
    if (fmt) { std::snprintf(buf, sizeof(buf), fmt, money); }
#endif
    SetText(buf);
}

BOOL CControlText::PtInCtrl(int x, int y)
{
    if (!IsVisible()) return FALSE;

    int w = 0, h = 0;
    if (!m_Text.empty())
        g_MoFFont.GetTextLength(&w, &h, m_FontHeight, m_FontFaceW, m_Text.c_str(), m_FontWeight);

    if (w == 0 || h == 0) return FALSE;

    int ax, ay; GetAbsPos(ax, ay);
    RECT rc{ ax, ay, ax + w, ay + h };
    POINT pt{ x, y };
    return ::PtInRect(&rc, pt);
}

int CControlText::GetCharByteByLine(unsigned char* lineBreakBytes, int maxLines)
{
    if (m_Text.empty() || !lineBreakBytes) return 0;

    unsigned short width = GetWidth();
    if (width == 0) width = m_DefaultMultiWidth; // 若本體寬為 0，使用預設多行寬

    // 需先設定字型，確保量測一致
    g_MoFFont.SetFont(m_FontHeight, m_FontFaceW, m_FontWeight);
    return g_MoFFont.GetCharByteByLine(width, m_Text.c_str(), lineBreakBytes, maxLines);
}

unsigned char CControlText::GetMultiTextLineCount(uint16_t width)
{
    if (m_Text.empty()) return 0;
    if (!m_LineCountDirty && m_CachedLineCount != 0) return m_CachedLineCount;

    if (width == 0) width = m_DefaultMultiWidth;

    unsigned char dummy[512];
    int lines = GetCharByteByLine(dummy, static_cast<int>(std::size(dummy)));
    if (lines < 0) lines = 0; if (lines > 255) lines = 255;
    m_CachedLineCount = static_cast<unsigned char>(lines);
    m_LineCountDirty = false;
    return m_CachedLineCount;
}

void CControlText::SetTextPosToParentCenter()
{
    SetCenterPos();
    // 反編譯裡會將 Y 再往上 5 px
    SetY(GetY() - 5);
}

void CControlText::Draw()
{
    if (!IsVisible() || m_Text.empty()) return;

    const int x = GetAbsX();
    const int y = GetAbsY();

    // 設定字型（以寬字元 face）
    EnsureDefaultFontFace();
    g_MoFFont.SetFont(m_FontHeight, m_FontFaceW, m_FontWeight);

    const char* text = m_Text.c_str();

    const uint16_t boxW = GetWidth();
    const uint16_t boxH = GetHeight();

    if (boxW != 0 && boxH != 0)
    {
        // 多行方框繪製
        RECT rc;
        if (m_ShadowColor)
        {
            ::SetRect(&rc, x + 1, y + 1, boxW, boxH);
            g_MoFFont.SetTextBoxA(&rc, m_ShadowColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);
        }
        else if (m_OutlineColor)
        {
            // 八方向外框：(-1,0),( -1,-1),(0,-1),(1,-1),(1,0),(1,1),(0,1),(-1,1)
            ::SetRect(&rc, x - 1, y + 0, boxW - 1, boxH);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x - 1, y - 1, boxW - 1, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x + 0, y - 1, boxW + 0, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x + 1, y - 1, boxW + 1, boxH - 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x + 1, y + 0, boxW + 1, boxH + 0);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x + 1, y + 1, boxW + 1, static_cast<int>(boxH) + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x + 0, y + 1, boxW + 0, static_cast<int>(boxH) + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);

            ::SetRect(&rc, x - 1, y + 1, boxW - 1, static_cast<int>(boxH) + 1);
            g_MoFFont.SetTextBoxA(&rc, m_OutlineColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);
        }

        // 主文字
        ::SetRect(&rc, x + 0, y + 0, boxW, boxH);
        g_MoFFont.SetTextBoxA(&rc, m_TextColor, text, m_LineSpacing, m_Alignment, m_WantSpaceFirstByte);
    }
    else
    {
        // 單行繪製
        if (m_ShadowColor)
        {
            g_MoFFont.SetTextLineA(x + 1, y + 1, m_ShadowColor, text, m_Alignment, -1, -1);
        }
        else if (m_OutlineColor)
        {
            g_MoFFont.SetTextLineShadow(x, y, m_OutlineColor, text, m_Alignment);
        }
        // 主文字
        g_MoFFont.SetTextLineA(x, y, m_TextColor, text, m_Alignment, -1, -1);
    }
}
