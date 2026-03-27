#include "UI/CTextBoxScroll.h"
#include "Font/MoFFont.h"
#include "global.h"
#include <cstring>
#include <cstdlib>

// ========================================
// TBSInfo
// ========================================
TBSInfo::~TBSInfo()
{
    if (pText)
    {
        delete[] pText;
        pText = nullptr;
    }
}

// ========================================
// TextBoxScrollLineInfoListMgr
// ========================================
TextBoxScrollLineInfoListMgr::TextBoxScrollLineInfoListMgr()
{
}

TextBoxScrollLineInfoListMgr::~TextBoxScrollLineInfoListMgr()
{
    DeleteAll();
}

void TextBoxScrollLineInfoListMgr::Add(const char* lineText, float width, DWORD color)
{
    TBSInfo* node = new TBSInfo();
    if (lineText)
    {
        int len = static_cast<int>(strlen(lineText));
        node->pText = new char[len + 1];
        memcpy(node->pText, lineText, len + 1);
    }
    node->fWidth = width;
    node->dwColor = color;

    node->pPrev = m_pTail;
    node->pNext = nullptr;

    if (m_pTail)
        m_pTail->pNext = node;
    else
        m_pHead = node;

    m_pTail = node;
    ++m_nCount;
}

void TextBoxScrollLineInfoListMgr::DeleteAll()
{
    TBSInfo* cur = m_pHead;
    while (cur)
    {
        TBSInfo* next = cur->pNext;
        delete cur;
        cur = next;
    }
    m_pHead = nullptr;
    m_pTail = nullptr;
    m_nCount = 0;
}

// ========================================
// CTextBoxScroll
// ========================================
CTextBoxScroll::CTextBoxScroll()
{
}

CTextBoxScroll::~CTextBoxScroll()
{
    // TextBoxScrollLineInfoListMgr 自動解構
}

void CTextBoxScroll::Reset()
{
    m_LineMgr.DeleteAll();
    m_nFontHeight = 0;
    m_nTotalLines = 0;
    m_nVisibleLines = 0;
    m_nWidth = 0;
    m_nHeight = 0;
    m_nCurrentLine = 0;
    m_nLineHeight = 0;
}

void CTextBoxScroll::SetTextBoxScroll(int cHeight, const char* pszFaceName, const char* lpString,
                                       unsigned short a5, unsigned short a6, int a7,
                                       int cWeight, int a9, int a10)
{
    Reset();
    CreateTextBoxScroll(cHeight, pszFaceName, lpString, a5, a6, a7, cWeight, a9, a10);
}

void CTextBoxScroll::SetLine(int a2)
{
    if (a2 >= 0 && a2 < m_nTotalLines)
        m_nCurrentLine = a2;
}

void CTextBoxScroll::TextBoxDraw(int x, int y, unsigned int color, int lineSpacing,
                                  char alignment, int fontWeight)
{
    // 對齊反編譯：設定字型
    wchar_t faceW[128];
    MultiByteToWideChar(CP_ACP, 0, m_szFontFace, -1, faceW, 128);
    g_MoFFont.SetFont(m_nFontHeight, faceW, fontWeight);

    int lineH = m_nLineHeight;
    if (lineSpacing)
        lineH = lineSpacing + m_nLineHeight;

    TBSInfo* node = m_LineMgr.GetHead();
    int lineIdx = 0;
    int drawCount = 0;

    while (node)
    {
        if (lineIdx >= m_nCurrentLine)
        {
            unsigned int drawColor = color;
            if (!color)
                drawColor = node->dwColor;

            g_MoFFont.SetTextLineA(x, y, drawColor, node->pText, alignment, -1, -1);
            y += lineH;
            if (drawCount >= m_nVisibleLines - 1)
                return;
            ++drawCount;
        }
        else
        {
            ++lineIdx;
        }
        node = node->pNext;
    }
}

void CTextBoxScroll::CreateTextBoxScroll(int cHeight, const char* pszFaceName, const char* lpString,
                                          unsigned short a5, unsigned short a6, int a7,
                                          int cWeight, int a9, int a10)
{
    if (!lpString || !pszFaceName)
        return;

    // 儲存字型資訊
    m_nFontHeight = cHeight;
    strncpy(m_szFontFace, pszFaceName, sizeof(m_szFontFace) - 1);
    m_szFontFace[sizeof(m_szFontFace) - 1] = '\0';

    // 設定字型
    wchar_t faceW[128];
    MultiByteToWideChar(CP_ACP, 0, pszFaceName, -1, faceW, 128);
    g_MoFFont.SetFont(cHeight, faceW, cWeight);

    m_nWidth = a5;
    m_nHeight = a6;

    // 計算行高
    m_nLineHeight = cHeight;
    if (a10)
        m_nLineHeight = cHeight + a10;

    // 計算可見行數
    if (m_nLineHeight > 0 && a6 > 0)
        m_nVisibleLines = a6 / m_nLineHeight;
    else
        m_nVisibleLines = 1;

    if (m_nVisibleLines < 1)
        m_nVisibleLines = 1;

    // 斷行處理：逐字元量測，遇到超過 a5 寬度或 \n 時換行
    HDC hdc = GetWindowDC(NULL);
    HFONT hFont = CreateFontW(cHeight, 0, 0, 0, cWeight, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, faceW);
    HGDIOBJ oldFont = SelectObject(hdc, hFont);

    const char* p = lpString;
    char lineBuffer[4096];
    int lineLen = 0;
    int lineWidth = 0;
    int totalLines = 0;

    while (*p)
    {
        if (*p == '\n')
        {
            lineBuffer[lineLen] = '\0';
            m_LineMgr.Add(lineBuffer, static_cast<float>(lineWidth), 0xFFFFFFFF);
            ++totalLines;
            lineLen = 0;
            lineWidth = 0;
            ++p;
            continue;
        }

        int charLen = IsDBCSLeadByte((unsigned char)*p) ? 2 : 1;
        SIZE charSize;
        GetTextExtentPoint32A(hdc, p, charLen, &charSize);

        if (a5 > 0 && lineWidth + charSize.cx > a5 && lineLen > 0)
        {
            // 換行
            lineBuffer[lineLen] = '\0';
            m_LineMgr.Add(lineBuffer, static_cast<float>(lineWidth), 0xFFFFFFFF);
            ++totalLines;
            lineLen = 0;
            lineWidth = 0;
        }

        for (int i = 0; i < charLen && *p; ++i)
        {
            lineBuffer[lineLen++] = *p++;
        }
        lineWidth += charSize.cx;
    }

    // 最後一行
    if (lineLen > 0)
    {
        lineBuffer[lineLen] = '\0';
        m_LineMgr.Add(lineBuffer, static_cast<float>(lineWidth), 0xFFFFFFFF);
        ++totalLines;
    }

    m_nTotalLines = totalLines;

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
    ReleaseDC(NULL, hdc);

    // 設定起始行
    m_nCurrentLine = a7;
    if (m_nCurrentLine < 0) m_nCurrentLine = 0;
    if (m_nCurrentLine >= m_nTotalLines) m_nCurrentLine = 0;
}
