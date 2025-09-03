#include "UI/CControlTabMgr.h"
#include <cassert>

CControlTabMgr::CControlTabMgr()
{
    CreateChildren();
    Init();
}

CControlTabMgr::~CControlTabMgr() = default;

void CControlTabMgr::Create(CControlBase* pParent, bool textTabStyle, bool enableShadeMode)
{
    CControlBase::Create(pParent);
    if (textTabStyle)
    {
        for (int i = 0; i < 20; ++i)
        {
            if (!enableShadeMode)
                m_Tabs[i].SetShadeMode(0);
            m_Tabs[i].SetTabTextType();
        }
    }
}

void CControlTabMgr::InitLogIn()
{
    CControlBase::InitLogIn();
    SetFocusTabIndex(0);

    if (m_Tabs[0].GetTabType() == 1)
    {
        // 對齊反編譯：在目前焦點索引上做一次 hover 行為
        const uint8_t idx = m_focusIndex;
        (void)idx; // 如需觸發實際事件流程可在此調用
        m_Tabs[idx].ControlKeyInputProcess(7, 0, 0, 0, 0, 0);
    }
}

void CControlTabMgr::CreateChildren()
{
    for (int i = 0; i < 20; ++i)
    {
        m_Tabs[i].Create(this);
        m_Tabs[i].SetIndex(i);

        m_Seps[i].Create(this);
        m_Seps[i].SetText("|");
        m_Seps[i].SetTextColor(0xFFFFFFFF);
    }

    // 預設整體顏色（對齊反編譯）
    SetTabTextColor(0xFFFFFFFF, 0xFF8C2C0C, 0xFF000000, 0xFFC99273);
}

void CControlTabMgr::Init()
{
    m_tabCount = 0;
    m_focusIndex = 0;
}

void CControlTabMgr::SetTextTabPosition(int wrapWidth)
{
    int baseX = 0, baseY = 0;
    GetAbsPos(baseX, baseY);

    int cursorX = baseX;
    int cursorY = baseY;

    for (int i = 0; i < m_tabCount; ++i)
    {
        // 文字放在目前游標位置
        CControlText* t = m_Tabs[i].GetTextCtrl();
        t->SetAbsPos(cursorX, cursorY);

        int tw = 0, th = 0;
        t->GetTextPixelSize(&tw, &th);
        m_Tabs[i].SetSize(static_cast<uint16_t>(tw), static_cast<uint16_t>(th));

        // 分隔符緊跟其後 + 6px 間距
        int nextX = cursorX + tw + 6;
        m_Seps[i].SetAbsPos(nextX, cursorY);

        int sepW = 0, sepH = 0;
        m_Seps[i].GetTextPixelSize(&sepW, &sepH, "|");

        cursorX = nextX + sepW + 6;

        if (wrapWidth > 0 && (cursorX - baseX) > wrapWidth)
        {
            cursorX = baseX;
            cursorY += 14; // 對齊反編譯：換行高度 14
        }
    }
}

void CControlTabMgr::AddTab(int x, int y,
    unsigned int giid,
    uint16_t blockFocused,
    uint16_t blockUnfocused,
    uint16_t blockHover,
    uint16_t blockNoneActive,
    int textId)
{
    if (m_tabCount >= 20) return;

    if (m_tabCount == 0)
        SetAbsPos(x, y);

    CControlTab& tab = m_Tabs[m_tabCount];
    tab.SetAbsPos(x, y);
    tab.SetImage(giid, blockFocused, blockUnfocused, blockHover, blockNoneActive);
    if (textId)
        tab.SetText(textId);

    UpdateBoundingSizeByTab(m_tabCount);

    if (m_tabCount > 0)
        tab.SetFocus(false);

    // 對齊反編譯：把索引寫進子物件（便於事件回傳）
    tab.SetIndex(m_tabCount);

    ++m_tabCount;
}

void CControlTabMgr::AddTab(int x, int y,
    unsigned int giid,
    uint16_t blockFocused,
    uint16_t blockUnfocused,
    int textId)
{
    AddTab(x, y, giid, blockFocused, blockUnfocused, 0xFFFF, 0xFFFF, textId);
}

void CControlTabMgr::AddTab(int x, int y, char type, int textId)
{
    unsigned int giid = 0;
    uint16_t b0 = 0xFFFF, b1 = 0xFFFF;

    if (type == 0) { giid = 536870949u; b0 = 61; b1 = 62; }
    else if (type == 1) { giid = 536870931u; b0 = 33; b1 = 34; }
    else if (type == 2) { giid = 536870931u; b0 = 35; b1 = 36; }
    else return;

    AddTab(x, y, giid, b0, b1, textId);
}

void CControlTabMgr::SetAllAlpha(int a)
{
    for (int i = 0; i < m_tabCount; ++i)
        m_Tabs[i].SetTabAlpha(a);
}

void CControlTabMgr::SetTabCount(uint8_t count)
{
    m_tabCount = std::min<uint8_t>(count, 20);
    UpdateSeparatorsVisibility();
}

void CControlTabMgr::SetTabTextColor(uint32_t colFocused,
    uint32_t colUnfocused,
    uint32_t colHover,
    uint32_t colNoneActive)
{
    for (int i = 0; i < 20; ++i)
        m_Tabs[i].SetTextColor(colFocused, colUnfocused, colHover, colNoneActive);
}

void CControlTabMgr::ChildKeyInputProcess(int msg, int a3, int x, int y, int a6, int a7)
{
    if (msg == 0)
    {
        // a3 反編譯作為子物件位址，+68 取索引；在本還原用 GetIndex()
        for (int i = 0; i < m_tabCount; ++i)
        {
            if (&m_Tabs[i] == reinterpret_cast<CControlTab*>(a3))
            {
                SetFocusTabIndex(static_cast<uint8_t>(m_Tabs[i].GetIndex()));
                break;
            }
        }
    }
}

void CControlTabMgr::Show()
{
    CControlBase::Show();

    // 隱藏多餘的 Tab
    for (int i = m_tabCount; i < 20; ++i)
        m_Tabs[i].Hide();

    // 依樣式控制分隔符顯示
    UpdateSeparatorsVisibility();
}

void CControlTabMgr::SetFocusTabIndex(uint8_t idx)
{
    if (idx >= m_tabCount) return;
    m_Tabs[m_focusIndex].SetFocus(false);
    m_focusIndex = idx;
    m_Tabs[m_focusIndex].SetFocus(true);
}

void CControlTabMgr::ActiveTab(uint8_t idx)
{
    if (idx < m_tabCount) m_Tabs[idx].Active();
}

int CControlTabMgr::IsActiveTab(uint8_t idx)
{
    if (idx >= m_tabCount) return 0;
    return m_Tabs[idx].IsActive();
}

void CControlTabMgr::NoneActiveTab(uint8_t idx)
{
    if (idx < m_tabCount) m_Tabs[idx].NoneActive();
}

void CControlTabMgr::Draw()
{
    // 先畫非焦點，最後畫當前焦點
    for (int i = m_tabCount - 1; i >= 0; --i)
    {
        if (i != m_focusIndex)
            m_Tabs[i].Draw();
    }

    if (m_tabCount > 0)
        m_Tabs[m_focusIndex].Draw();

    // 文字 Tab：畫出前 (count-1) 個分隔符
    if (m_Tabs[0].GetTabType() == 1)
    {
        for (int i = 0; i < max(0, (int)m_tabCount - 1); ++i)
            m_Seps[i].Draw();
    }
    else
    {
        // 非文字 Tab：全部隱藏
        for (int i = 0; i < 20; ++i)
            m_Seps[i].Hide();
    }
}

void CControlTabMgr::UpdateBoundingSizeByTab(uint8_t idx)
{
    // 用第一個 Tab 的相對座標 + 尺寸，更新整體包框
    const int baseX = m_Tabs[0].GetX();
    const int baseY = m_Tabs[0].GetY();

    const int x = m_Tabs[idx].GetX();
    const int y = m_Tabs[idx].GetY();

    const int w = m_Tabs[idx].GetWidth();
    const int h = m_Tabs[idx].GetHeight();

    const uint16_t boxW = static_cast<uint16_t>((x - baseX) + w);
    const uint16_t boxH = static_cast<uint16_t>((y - baseY) + h);
    SetSize(boxW, boxH);
}

void CControlTabMgr::UpdateSeparatorsVisibility()
{
    if (m_Tabs[0].GetTabType() == 1)
    {
        for (int i = 0; i < 20; ++i)
        {
            if (i >= (int)m_tabCount - 1) m_Seps[i].Hide();
            else                          m_Seps[i].Show();
        }
    }
    else
    {
        for (int i = 0; i < 20; ++i) m_Seps[i].Hide();
    }
}