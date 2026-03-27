#include "UI/CControlMoveText.h"
#include "Font/MoFFont.h"
#include "Text/DCTTextManager.h"
#include <cstring>
#include <cstdlib>
#include <new>

extern MoFFont g_MoFFont;
extern DCTTextManager g_DCTTextManager;

// ============================================================================
// 建構 / 解構 — 對齊 00421130
// ============================================================================
CControlMoveText::CControlMoveText()
    : CControlText()
{
    m_head = nullptr;
    m_tail = nullptr;
    m_cur  = nullptr;
    m_loop = 0;
    m_cutL = -1;
    m_cutR = -1;
}

CControlMoveText::~CControlMoveText()
{
    DeleteAll();
}

// ============================================================================
// Add — 對齊 00421780
// ============================================================================
MTSInfo* CControlMoveText::Add()
{
    MTSInfo* v3 = static_cast<MTSInfo*>(::operator new(0x30));
    if (!v3) return nullptr;
    std::memset(v3, 0, 0x30);

    if (m_head || m_tail)
    {
        if (m_head == m_tail)
            m_head->next = v3;
        m_tail->next = v3;
        v3->prev = m_tail;
        m_tail = v3;
    }
    else
    {
        m_tail = v3;
        m_head = v3;
    }
    return v3;
}

// ============================================================================
// InsertCurrent — 對齊 00421820
// ============================================================================
MTSInfo* CControlMoveText::InsertCurrent()
{
    MTSInfo* v3 = static_cast<MTSInfo*>(::operator new(0x30));
    if (!v3) return nullptr;
    std::memset(v3, 0, 0x30);

    if (m_head || m_tail)
    {
        if (m_cur == m_tail)
        {
            // 插到尾巴等同 Add — 注意：此處 v3 洩漏（符合原始行為）
            ::operator delete(v3);
            return Add();
        }
        else
        {
            v3->next = m_cur->next;
            m_cur->next = v3;
            MTSInfo* v6 = v3->next;
            v3->prev = m_cur;
            v6->prev = v3;
            return v3;
        }
    }
    else
    {
        m_tail = v3;
        m_head = v3;
        return v3;
    }
}

// ============================================================================
// DeleteAll — 對齊 004218D0
// ============================================================================
void CControlMoveText::DeleteAll()
{
    MTSInfo* p = m_head;
    while (p)
    {
        MTSInfo* nxt = p->next;
        if (p->text)
        {
            ::operator delete(p->text);
            p->text = nullptr;
        }
        ::operator delete(p);
        p = nxt;
    }
    m_cur  = nullptr;
    m_tail = nullptr;
    m_head = nullptr;

    ClearText();
}

// ============================================================================
// DeleteHeadText — 對齊 004219C0
// ============================================================================
int CControlMoveText::DeleteHeadText()
{
    MTSInfo* v3 = m_head;
    MTSInfo* v4 = v3->next;
    if (v3)
    {
        if (v3->text)
        {
            ::operator delete(v3->text);
            v3->text = nullptr;
        }
        ::operator delete(v3);
    }
    m_head = v4;
    if (v4)
    {
        m_cur = v4;
        return 1;
    }
    else
    {
        m_cur  = nullptr;
        m_tail = nullptr;
        return 0;
    }
}

// ============================================================================
// DeleteText — 對齊 00421A20
// ============================================================================
int CControlMoveText::DeleteText()
{
    MTSInfo* v1 = m_head;
    MTSInfo* v2 = m_tail;
    MTSInfo* v3 = m_cur;

    if (v1 == v2)
    {
        DeleteAll();
        return 0;
    }

    if (v3 == v2) // cur 是尾巴
    {
        m_tail = v3->prev;
        m_tail->next = nullptr;
        m_cur = m_head;
    }
    else if (v3 == v1) // cur 是頭
    {
        MTSInfo* v6 = v3->next;
        m_cur  = v6;
        m_head = v6;
        v6->prev = nullptr;
    }
    else // 中間節點
    {
        v3->prev->next = v3->next;
        MTSInfo* v8 = m_cur->next;
        m_cur = v8;
        v8->prev = v3->prev;
    }

    // 釋放被刪除的節點
    if (v3)
    {
        if (v3->text)
        {
            ::operator delete(v3->text);
            v3->text = nullptr;
        }
        ::operator delete(v3);
    }
    return 1;
}

// ============================================================================
// SetTextCutPos — 對齊 00421460
// ============================================================================
void CControlMoveText::SetTextCutPos(int cutL, int cutR)
{
    m_cutL = cutL;
    m_cutR = cutR;
}

// ============================================================================
// SetMoveText — 對齊 004211E0
// ============================================================================
void CControlMoveText::SetMoveText(
    const char* lpString,
    char* altStr,
    int x0, int y0, int x1, int y1,
    int useCut,
    int doLoop,
    char shadowStyle)
{
    if (!lpString) return;

    MTSInfo* v11 = Add();

    size_t len = std::strlen(lpString) + 1;
    v11->style = static_cast<int>(shadowStyle); // style 低位元組

    // 設定字重
    SetFontWeight(700);
    if (static_cast<unsigned char>(g_MoFFont.GetNationCode()) == 2)
    {
        stFontInfo* fi = g_MoFFont.GetFontInfo("ChatBallonText");
        if (fi)
            SetFontWeight(reinterpret_cast<int*>(fi)[65]);
    }

    // 複製主字串
    char* buf = static_cast<char*>(std::malloc(len));
    v11->text = buf;
    std::memcpy(buf, lpString, len);

    // 若 style 非 0，複製 altText
    if (shadowStyle)
    {
        size_t len2 = std::strlen(altStr) + 1;
        char* buf2 = static_cast<char*>(std::malloc(len2));
        v11->altText = buf2;
        std::memcpy(buf2, altStr, len2);
    }

    // 取得文字像素寬度
    const char* fontKey = g_DCTTextManager.GetText(3264);
    int textSize[2] = { 0, 0 };
    g_MoFFont.GetTextLength(&textSize[0], &textSize[1],
        GetFontHeight(), reinterpret_cast<const wchar_t*>(fontKey),
        lpString, GetFontWeight());
    int textW = textSize[0];

    // 處理裁切
    if (useCut)
    {
        if (x0 <= x1)
        {
            if (x0 < x1)
            {
                v11->xEnd   = x1;
                v11->xStart = x0 - textW;
                SetTextCutPos(x0, x1);
            }
        }
        else
        {
            v11->xStart = x0;
            v11->xEnd   = x1 - textW;
            SetTextCutPos(x1, x0);
        }
    }
    else
    {
        v11->xStart = x0;
        v11->xEnd   = x1;
    }

    v11->yStart   = y0;
    v11->yEnd     = y1;
    v11->counter  = 0;
    v11->timerLeft = 3;
    m_loop = doLoop;

    if (!m_cur)
    {
        SetAbsPos(v11->xStart, v11->yStart);
        m_cur = m_head;
    }
}

// ============================================================================
// SetMoveTextReturnValue — 對齊 00421AD0
// ============================================================================
MTSInfo* CControlMoveText::SetMoveTextReturnValue(
    const char* lpString,
    int x0, int y0, int x1, int y1,
    int useCut,
    int doLoop,
    char shadowStyle)
{
    if (!lpString) return nullptr;

    MTSInfo* v11 = InsertCurrent();

    size_t len = std::strlen(lpString) + 1;
    *(reinterpret_cast<char*>(v11) + 40) = shadowStyle;

    SetFontWeight(700);
    if (static_cast<unsigned char>(g_MoFFont.GetNationCode()) == 2)
    {
        stFontInfo* fi = g_MoFFont.GetFontInfo("ChatBallonText");
        if (fi)
            SetFontWeight(reinterpret_cast<int*>(fi)[65]);
    }

    char* buf = static_cast<char*>(std::malloc(len));
    v11->text = buf;
    std::memcpy(buf, lpString, len);

    // 取得文字像素寬度
    const char* fontKey = g_DCTTextManager.GetText(3264);
    int textSize[2] = { 0, 0 };
    g_MoFFont.GetTextLength(&textSize[0], &textSize[1],
        GetFontHeight(), reinterpret_cast<const wchar_t*>(fontKey),
        lpString, GetFontWeight());
    int textW = textSize[0];

    if (useCut)
    {
        if (x0 <= x1)
        {
            if (x0 < x1)
            {
                v11->xEnd   = x1;
                v11->xStart = x0 - textW;
                SetTextCutPos(x0, x1);
            }
        }
        else
        {
            v11->xStart = x0;
            v11->xEnd   = x1 - textW;
            SetTextCutPos(x1, x0);
        }
    }
    else
    {
        v11->xStart = x0;
        v11->xEnd   = x1;
    }

    v11->yStart    = y0;
    v11->yEnd      = y1;
    v11->counter   = 0;
    v11->timerLeft = 3;
    v11->retFlag   = 1;
    m_loop = doLoop;

    if (!m_cur)
    {
        SetAbsPos(v11->xStart, v11->yStart);
        m_cur = m_head;
    }
    return v11;
}

// ============================================================================
// SetCurrentMovePos — 對齊 00421390
// ============================================================================
int CControlMoveText::SetCurrentMovePos(int x0, int y0, int x1, int y1, int useCut)
{
    if (!m_cur) return 0;

    // 取得目前節點文字寬度
    const char* fontKey = g_DCTTextManager.GetText(3264);
    int textSize[2] = { 0, 0 };
    g_MoFFont.GetTextLength(&textSize[0], &textSize[1],
        GetFontHeight(), reinterpret_cast<const wchar_t*>(fontKey),
        m_cur->text, GetFontWeight());
    int textW = textSize[0];

    if (useCut)
    {
        int oldXEnd = m_cur->xEnd;
        m_cur->xStart = x0;
        if (x0 <= oldXEnd)
        {
            if (x0 < oldXEnd)
            {
                m_cur->xStart = x0 - textW;
                m_cur->xEnd   = x1;
            }
        }
        else
        {
            m_cur->xStart = x0;
            m_cur->xEnd   = x1 - textW;
        }
    }
    else
    {
        m_cur->xStart = x0;
        m_cur->xEnd   = x1;
        SetTextCutPos(-1, -1);
    }

    m_cur->yEnd    = y1;
    m_cur->yStart  = y0;
    m_cur->counter = 0;
    SetAbsPos(m_cur->xStart, y0);
    return 1;
}

// ============================================================================
// SetAllPos — 對齊 00421480
// ============================================================================
void CControlMoveText::SetAllPos(int x0, int y0, int x1, int y1)
{
    MTSInfo* savedCur = m_cur;
    m_cur = m_head;
    if (m_cur)
    {
        do
        {
            SetCurrentMovePos(x0, y0, x1, y1, 1);
            m_cur = m_cur->next;
        } while (m_cur);
        m_cur = savedCur;
    }
    else
    {
        m_cur = savedCur;
    }
}

// ============================================================================
// ProcessMoveText — 對齊 004214F0
// ============================================================================
int CControlMoveText::ProcessMoveText(int* pX, int* pY, int step)
{
    int dx = m_cur->xEnd - m_cur->xStart;
    int sgn;
    if (dx > 0) sgn = 1;
    else sgn = (dx >= 0) - 1;  // dx==0 → -1, dx<0 → -1

    int v7 = step * sgn + *pX;
    *pX = v7;
    return v7 > m_cur->xEnd;
}

// ============================================================================
// Poll — 對齊 00421540
// ============================================================================
void CControlMoveText::Poll()
{
    if (!m_cur) return;

    int x = GetAbsX();
    int y = GetAbsY();

    if (ProcessMoveText(&x, &y, 2))
    {
        SetAbsPos(x, y);
        SetText(m_cur->text);
    }
    else if (m_loop)
    {
        --m_cur->timerLeft;
        if (m_cur->timerLeft <= 0)
            m_cur->timerLeft = 0;

        if (m_cur->timerLeft)
        {
            if (!m_cur->retFlag)
            {
                if (m_cur == m_tail)
                    m_cur = m_head;
                else
                    m_cur = m_cur->next;
            }
            if (m_cur)
                SetAbsPos(m_cur->xStart, m_cur->yStart);
        }
        else
        {
            if (DeleteText())
            {
                SetAbsPos(m_cur->xStart, m_cur->yStart);
            }
        }
    }
    else
    {
        if (DeleteHeadText())
        {
            SetAbsPos(m_head->xStart, m_head->yStart);
        }
    }
}

// ============================================================================
// Draw — 對齊 00421680
// ============================================================================
void CControlMoveText::Draw()
{
    if (!IsVisible()) return;
    if (!IsActive())  return;

    // 設定字型
    const char* fontKey = g_DCTTextManager.GetText(3264);
    g_MoFFont.SetFont(GetFontHeight(),
        reinterpret_cast<const wchar_t*>(fontKey),
        GetFontWeight());

    if (!m_cur) return;

    const char* lpString = GetText();
    int x = GetAbsX();
    int y = GetAbsY();

    g_MoFFont.SetTextLineA(x, y, m_TextColor, lpString, m_isCentered, m_cutL, m_cutR);

    char style = static_cast<char>(m_cur->style);
    unsigned int altColor = 0;
    if (style)
    {
        if (style == 1 || style == 3)
            altColor = static_cast<unsigned int>(-1);
        else if (style == 2)
            altColor = static_cast<unsigned int>(-250);

        g_MoFFont.SetTextLineA(x, y, altColor,
            m_cur->altText,
            m_isCentered, m_cutL, m_cutR);
    }
}
