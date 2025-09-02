#include "UI/CControlPageScroll.h"
#include <cstdio>

// 來自其他模組的定義
int g_MouseCaptureDirty = 0;

CControlPageScroll::CControlPageScroll()
    : CControlBase()
    , m_text()
    , m_btnPrev()
    , m_btnNext()
{
    // vftable 由編譯器處理
    CreateChildren();
    Init();
}

CControlPageScroll::~CControlPageScroll()
{
    // 成員自動解構，呼叫順序與 IDA 列出的一致性不影響行為
}

void CControlPageScroll::CreateChildren()
{
    // 文字
    m_text.Create(this);
    m_text.SetControlSetFont("ScrollText");
    // *((DWORD*)this + 68) = 1; // 反編譯寫到 this 的旗標—此處框架內部旗標，不特別外露
    m_text.SetPos(48, 7);

    // 左鍵（上一頁）
    m_btnPrev.Create(this);
    // ID: 0x20000003 (=536870931)，四態（normal/hover/press/disable）：25/27/29/31
    m_btnPrev.SetImage(0x20000003u, 25, 0x20000003u, 27, 0x20000003u, 29, 0x20000003u, 31);
    // *((DWORD*)this + 155) = 1;

    // 右鍵（下一頁）
    m_btnNext.Create(this);
    m_btnNext.SetX(69);
    // 四態：26/28/30/32
    m_btnNext.SetImage(0x20000003u, 26, 0x20000003u, 28, 0x20000003u, 30, 0x20000003u, 32);
    // *((DWORD*)this + 336) = 1;

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

void CControlPageScroll::ChildKeyInputProcess(int a2, CControlBase* a3, int a4, int /*a5*/, int /*a6*/, int /*a7*/)
{
    // a2 == 3 : 放開（click-up）
    if (a2 == 3)
    {
        if (a3 == &m_btnPrev)
        {
            if (m_curPage > m_minPage)
            {
                ChangeCurPage(-1);
                g_MouseCaptureDirty = 1;
                return;
            }
        }
        else if (a3 == &m_btnNext)
        {
            if (m_curPage < m_maxPage)
            {
                ChangeCurPage(+1);
            }
            g_MouseCaptureDirty = 1;
        }
    }
}

void CControlPageScroll::AutoKeyInput(int a2)
{
    // a2 != 0: 向右；到最大頁時回到最小頁
    // a2 == 0: 向左；到最小頁時跳到最大頁
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
    // 計算最大頁數（ceil(total / perPage)），錯誤輸入時退回 minPage
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
    // 右鍵：到最大頁就 NoneActive，否則 Active
    if (m_curPage == m_maxPage) m_btnNext.NoneActive();
    else                        m_btnNext.Active();

    // 左鍵：到最小頁就 NoneActive，否則 Active
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
    char buf[256] = {};
    std::snprintf(buf, sizeof(buf), "%u/%u",
        static_cast<unsigned>(m_curPage),
        static_cast<unsigned>(m_maxPage));
    m_text.SetText(buf);
}

void CControlPageScroll::SetCurPage(char page)
{
    m_curPage = static_cast<uint8_t>(page);
    DecideButtonActive();
    PrintTextPage();
}
