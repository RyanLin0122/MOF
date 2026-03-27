#include "UI/CControlPageScroll.h"
#include "global.h"
#include <cstdio>

CControlPageScroll::CControlPageScroll()
    : CControlBase()
    , m_text()
    , m_btnPrev()
    , m_btnNext()
{
    CreateChildren();
    Init();
}

CControlPageScroll::~CControlPageScroll()
{
}

void CControlPageScroll::CreateChildren()
{
    // 文字
    m_text.Create(this);
    m_text.SetControlSetFont("ScrollText");
    m_text.m_isCentered = 1;  // 對齊反編譯：*((_DWORD *)this + 68) = 1
    m_text.SetPos(48, 7);

    // 左鍵（上一頁）— 對齊反編譯：SetImage(0x20000013, 25, 27, 29, 31)
    m_btnPrev.Create(this);
    m_btnPrev.SetImage(0x20000013u, 25, 27, 29, 31);
    m_btnPrev.SetPassKeyInputToParent(true);

    // 右鍵（下一頁）— 對齊反編譯：SetImage(0x20000013, 26, 28, 30, 32)
    m_btnNext.Create(this);
    m_btnNext.SetX(69);
    m_btnNext.SetImage(0x20000013u, 26, 28, 30, 32);
    m_btnNext.SetPassKeyInputToParent(true);

    // 依兩側按鈕設定整體控件大小（寬=右鍵X+右鍵寬；高=右鍵高）
    SetHeight(static_cast<uint16_t>(m_btnNext.GetHeight()));
    SetWidth(static_cast<uint16_t>(m_btnNext.GetX() + m_btnNext.GetWidth()));
}

void CControlPageScroll::Init()
{
    m_changed = 0;
    m_maxPage = 1;
    m_curPage = 1;
    m_minPage = 1;
}

void CControlPageScroll::ChildKeyInputProcess(int a2, CControlBase* a3, int /*a4*/, int /*a5*/, int /*a6*/, int /*a7*/)
{
    if (a2 == 3)
    {
        if (a3 == &m_btnPrev)
        {
            if (m_curPage > m_minPage)
            {
                ChangeCurPage(-1);
                dword_AFD34C = 1;
                return;
            }
        }
        else if (a3 == &m_btnNext && m_curPage < m_maxPage)
        {
            ChangeCurPage(+1);
        }
        dword_AFD34C = 1;
    }
}

void CControlPageScroll::AutoKeyInput(int a2)
{
    if (a2)
    {
        if (m_curPage >= m_maxPage)
            SetCurPage(static_cast<char>(m_minPage));
        else
            ChangeCurPage(+1);
    }
    else
    {
        if (m_curPage <= m_minPage)
            SetCurPage(static_cast<char>(m_maxPage));
        else
            ChangeCurPage(-1);
    }
}

void CControlPageScroll::SetPageRange(uint16_t itemsPerPage, uint16_t totalItems, int resetToMin)
{
    if (itemsPerPage && totalItems && totalItems >= itemsPerPage)
    {
        uint8_t pages = static_cast<uint8_t>(totalItems / itemsPerPage);
        if (totalItems % itemsPerPage) ++pages;
        m_maxPage = pages;
    }
    else
    {
        m_maxPage = m_minPage;
    }

    if (resetToMin)
        m_curPage = m_minPage;

    if (m_curPage > m_maxPage)
        m_curPage = m_maxPage;

    DecideButtonActive();
    PrintTextPage();
}

void CControlPageScroll::ChangeCurPage(char delta)
{
    int next = static_cast<int>(m_curPage) + static_cast<int>(delta);
    if (next <= static_cast<int>(m_maxPage) && next >= static_cast<int>(m_minPage))
    {
        m_changed = 1;
        m_curPage = static_cast<uint8_t>(next);
        DecideButtonActive();
        PrintTextPage();
    }
}

void CControlPageScroll::DecideButtonActive()
{
    if (m_curPage == m_maxPage) m_btnNext.NoneActive();
    else                        m_btnNext.Active();

    if (m_curPage == m_minPage) m_btnPrev.NoneActive();
    else                        m_btnPrev.Active();
}

int CControlPageScroll::IsChangedCurPage()
{
    int ret = m_changed;
    m_changed = 0;
    return ret;
}

void CControlPageScroll::PrintTextPage()
{
    // 對齊反編譯：_sprintf(Buffer, "%d/%d", curPage, maxPage)
    char buf[256] = {};
    std::snprintf(buf, sizeof(buf), "%d/%d",
        static_cast<int>(m_curPage),
        static_cast<int>(m_maxPage));
    m_text.SetText(buf);
}

void CControlPageScroll::SetCurPage(char page)
{
    m_curPage = static_cast<uint8_t>(page);
    DecideButtonActive();
    PrintTextPage();
}
