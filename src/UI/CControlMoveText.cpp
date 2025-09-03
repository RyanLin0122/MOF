#include "UI/CControlMoveText.h"
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include <cstring>
#include <cstdlib>

// 你的專案裡 MoFFont 的全域單例＆ API：
// void GetTextLength(int* w, int* h, const char* fontKey, const char* text);
// （你提供的簽名就用這個）
extern class MoFFont g_MoFFont;

CControlMoveText::CControlMoveText()
    : CControlText()
{
    // vftable 已由 C++ 自動設好
    m_head = m_tail = m_cur = nullptr;
    m_loop = 0;
    m_cutL = m_cutR = -1;
}

CControlMoveText::~CControlMoveText()
{
    DeleteAll();
}

// ---------- 內部小工具 ----------

int CControlMoveText::MeasureTextWidth(const char* text) const
{
    if (!text || !*text) return 0;
    int w = 0, h = 0;
    // CControlText 內部一般會保存一個 font key / style，若你有 GetFontKey() 就用；否則用目前設定的字型 key。
    // 反編譯在很多地方用 "ChatBallonText" 或由 DCT 取 key；為了還原性，這裡用 m_fontKey（你可改成固定 key）。
    GetTextPixelSize(&w, &h, text);
    return w;
}

// ---------- 佇列操作 ----------

MTSInfo* CControlMoveText::Add()
{
    auto* n = static_cast<MTSInfo*>(std::malloc(sizeof(MTSInfo)));
    if (!n) return nullptr;
    std::memset(n, 0, sizeof(MTSInfo));

    // 串到尾巴
    n->prev = m_tail;
    n->next = nullptr;
    if (m_tail) m_tail->next = n;
    m_tail = n;
    if (!m_head) m_head = n;

    return n;
}

MTSInfo* CControlMoveText::InsertCurrent()
{
    if (!m_head || !m_tail)
        return Add();

    if (m_cur == m_tail)
        return Add();

    auto* n = static_cast<MTSInfo*>(std::malloc(sizeof(MTSInfo)));
    if (!n) return nullptr;
    std::memset(n, 0, sizeof(MTSInfo));

    n->prev = m_cur;
    n->next = m_cur->next;
    m_cur->next->prev = n;
    m_cur->next = n;

    return n;
}

void CControlMoveText::DeleteAll()
{
    auto* p = m_head;
    while (p)
    {
        auto* nxt = p->next;
        if (p->text) { std::free(p->text);    p->text = nullptr; }
        if (p->altText) { std::free(p->altText); p->altText = nullptr; }
        std::free(p);
        p = nxt;
    }
    m_head = m_tail = m_cur = nullptr;

    // 清掉顯示文字（反編譯最後是把 std::string 清空）
    ClearText();
}

int CControlMoveText::DeleteHeadText()
{
    if (!m_head) {
        m_cur = m_tail = nullptr;
        return 0;
    }

    auto* old = m_head;
    m_head = old->next;
    if (m_head) m_head->prev = nullptr;
    else        m_tail = nullptr;

    if (old->text)    std::free(old->text);
    if (old->altText) std::free(old->altText);
    std::free(old);

    if (m_head) {
        m_cur = m_head;
        return 1;
    }
    m_cur = nullptr;
    return 0;
}

int CControlMoveText::DeleteText()
{
    if (!m_head || !m_cur) return 0;

    // 只有一個節點
    if (m_head == m_tail) {
        DeleteAll();
        return 0;
    }

    auto* victim = m_cur;

    // 砍尾：移動 tail，cur 回到 head
    if (victim == m_tail) {
        m_tail = m_tail->prev;
        m_tail->next = nullptr;
        m_cur = m_head;
    }
    // 砍頭：移動 head，cur 指到新 head
    else if (victim == m_head) {
        m_head = m_head->next;
        m_head->prev = nullptr;
        m_cur = m_head;
    }
    else {
        // 一般中間節點
        victim->prev->next = victim->next;
        victim->next->prev = victim->prev;
        m_cur = victim->next;
    }

    if (victim->text)    std::free(victim->text);
    if (victim->altText) std::free(victim->altText);
    std::free(victim);
    return 1;
}

// ---------- 設定/更新 ----------

void CControlMoveText::SetTextCutPos(int cutL, int cutR)
{
    m_cutL = cutL;
    m_cutR = cutR;
}

void CControlMoveText::SetMoveText(
    const char* lpString,
    const char* altStr,
    int x0, int y0, int x1, int y1,
    bool useCut,
    bool doLoop,
    unsigned char shadowStyle)
{
    if (!lpString) return;

    MTSInfo* n = Add();
    if (!n) return;

    // 設 style / 字重（反編譯會依國別調整，這裡保留整體行為）
    n->style = (n->style & ~0xFF) | shadowStyle;

    // 存主字串
    const size_t len = std::strlen(lpString) + 1;
    n->text = static_cast<char*>(std::malloc(len));
    if (n->text) std::memcpy(n->text, lpString, len);

    // 需要第二份描邊/陰影字串就複製
    if (shadowStyle && altStr) {
        const size_t len2 = std::strlen(altStr) + 1;
        n->altText = static_cast<char*>(std::malloc(len2));
        if (n->altText) std::memcpy(n->altText, altStr, len2);
    }

    // 依 useCut 決定是否修正起訖（讓整段字完整穿越裁切視窗）
    // 反編譯邏輯：若 x0<x1，起點 = x0 - textW；終點 = x1；cut=[x0,x1]
    // 若 x0>x1，則相反；不使用裁切時直接用 (x0,y0)->(x1,y1)
    const int textW = MeasureTextWidth(lpString);

    if (useCut) {
        if (x0 <= x1) {
            n->xStart = (x0 < x1) ? (x0 - textW) : x0;
            n->xEnd = x1;
            SetTextCutPos(x0, x1);
        }
        else {
            n->xStart = x0;
            n->xEnd = (x1 - textW);
            SetTextCutPos(x1, x0);
        }
    }
    else {
        n->xStart = x0;
        n->xEnd = x1;
        SetTextCutPos(-1, -1);
    }
    n->yStart = y0;
    n->yEnd = y1;

    n->counter = 0;
    n->timerLeft = 3;   // 反編譯設成 3

    m_loop = doLoop ? 1 : 0;

    // 第一次加進來，立刻把控制移到起點，並把 m_cur 指到 head
    if (!m_cur) {
        SetAbsPos(n->xStart, n->yStart);
        m_cur = m_head;
    }
}

MTSInfo* CControlMoveText::SetMoveTextReturnValue(
    const char* lpString,
    int x0, int y0, int x1, int y1,
    bool useCut,
    bool doLoop,
    unsigned char shadowStyle)
{
    if (!lpString) return nullptr;

    MTSInfo* n = InsertCurrent();
    if (!n) return nullptr;

    n->style = (n->style & ~0xFF) | shadowStyle;
    const size_t len = std::strlen(lpString) + 1;
    n->text = static_cast<char*>(std::malloc(len));
    if (n->text) std::memcpy(n->text, lpString, len);

    const int textW = MeasureTextWidth(lpString);
    if (useCut) {
        if (x0 <= x1) {
            n->xStart = (x0 < x1) ? (x0 - textW) : x0;
            n->xEnd = x1;
            SetTextCutPos(x0, x1);
        }
        else {
            n->xStart = x0;
            n->xEnd = (x1 - textW);
            SetTextCutPos(x1, x0);
        }
    }
    else {
        n->xStart = x0;
        n->xEnd = x1;
        SetTextCutPos(-1, -1);
    }
    n->yStart = y0;
    n->yEnd = y1;

    n->counter = 0;
    n->timerLeft = 3;
    n->retFlag = 1;  // 反編譯：+44 設為 1

    m_loop = doLoop ? 1 : 0;

    if (!m_cur) {
        SetAbsPos(n->xStart, n->yStart);
        m_cur = m_head;
    }
    return n;
}

int CControlMoveText::SetCurrentMovePos(int x0, int y0, int x1, int y1, bool useCut)
{
    if (!m_cur) return 0;

    const int textW = MeasureTextWidth(m_cur->text ? m_cur->text : "");
    if (useCut) {
        if (x0 <= x1) {
            m_cur->xStart = (x0 < x1) ? (x0 - textW) : x0;
            m_cur->xEnd = x1;
            SetTextCutPos(x0, x1);
        }
        else {
            m_cur->xStart = x0;
            m_cur->xEnd = (x1 - textW);
            SetTextCutPos(x1, x0);
        }
    }
    else {
        m_cur->xStart = x0;
        m_cur->xEnd = x1;
        SetTextCutPos(-1, -1);
    }
    m_cur->yStart = y0;
    m_cur->yEnd = y1;
    m_cur->counter = 0;

    SetAbsPos(m_cur->xStart, m_cur->yStart);
    return 1;
}

void CControlMoveText::SetAllPos(int x0, int y0, int x1, int y1)
{
    // 反編譯是把 m_cur 暫存，走訪全串列逐一呼叫 SetCurrentMovePos(..., true)，最後還原 m_cur
    auto* savedCur = m_cur;
    m_cur = m_head;
    while (m_cur) {
        SetCurrentMovePos(x0, y0, x1, y1, true);
        m_cur = m_cur->next;
    }
    m_cur = savedCur;
}

bool CControlMoveText::ProcessMoveText(int& x, int& y, int step)
{
    if (!m_cur) return false;
    const int dx = (m_cur->xEnd - m_cur->xStart);
    const int sgn = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    x += step * sgn;
    // 反編譯回傳條件是 (x > endX)；保留相同行為
    return x > m_cur->xEnd;
}

void CControlMoveText::Poll()
{
    if (!m_cur) return;

    int x = GetAbsX();
    int y = GetAbsY();

    // 每幀移動 2 px
    if (ProcessMoveText(x, y, 2)) {
        SetAbsPos(x, y);
        // 到達時把本控件顯示字串換成節點文字（反編譯行為）
        SetText(m_cur->text ? m_cur->text : "");
    }
    else if (m_loop) {
        // 有循環/接續行為：遞減 timerLeft
        if (--m_cur->timerLeft < 0) m_cur->timerLeft = 0;

        if (m_cur->timerLeft) {
            // 尚在等待階段：如果本節點不是「return」型 (retFlag==0)
            // 就「前進到下一個；若目前是尾巴，跳回頭」
            if (m_cur->retFlag == 0) {
                if (m_cur == m_tail) m_cur = m_head;
                else                 m_cur = m_cur->next;
            }
            if (m_cur) SetAbsPos(m_cur->xStart, m_cur->yStart);
        }
        else {
            // 等待完畢：刪掉目前節點；若還有節點，重設位置到新 m_cur
            if (DeleteText() && m_cur) {
                SetAbsPos(m_cur->xStart, m_cur->yStart);
            }
        }
    }
    else {
        // 不循環：刪頭節點並把位置移到新頭
        if (DeleteHeadText() && m_head) {
            SetAbsPos(m_head->xStart, m_head->yStart);
        }
    }
}

void CControlMoveText::Draw()
{
    if (!IsVisible()) return;
    if (!IsActive())  return;
    if (!m_cur)       return;

    // 設字型（反編譯：用 DCT 取 key，再用 MoFFont::SetFont；這裡沿用 CControlText 既有設定）
    const char* textMain = GetText();
    const int   x = GetAbsX();
    const int   y = GetAbsY();

    // 直接用 CControlText 的繪字後端（若你一定要走 MoFFont::SetTextLineA，就把下列改成你專案版）
    // 為了保留裁切行為，這裡假設 CControlText::Draw 會尊重 m_cutL/m_cutR（若沒有，你可在 CControlText 增加支持）
    // 先畫主字
    g_MoFFont.SetTextLineA(x, y, m_TextColor, textMain, m_Alignment, m_cutL, m_cutR);

    // 依 style 決定是否再畫 alt（陰影/描邊）
    const unsigned char style = static_cast<unsigned char>(m_cur->style & 0xFF);
    if (style && m_cur->altText) {
        unsigned int altColor = 0;
        if (style == 1 || style == 3)      altColor = 0xFFFFFFFFu; // -1
        else if (style == 2)               altColor = 0xFFFFFF06u; // -250（保留原值語意；實際顏色你可對應）
        g_MoFFont.SetTextLineA(x, y, altColor, m_cur->altText, m_Alignment, m_cutL, m_cutR);
    }
}
