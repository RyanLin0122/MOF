#include "UI/CControlTabMgr.h"
#include "global.h"

CControlTabMgr::CControlTabMgr()
{
    // 對齊反編譯：CControlBase ctor 自動呼叫
    // m_Tabs[20] 和 m_Seps[20] 由 C++ 自動建構
    CreateChildren();
    Init();
}

CControlTabMgr::~CControlTabMgr()
{
    // 對齊反編譯：m_Seps、m_Tabs、CControlBase 自動解構
}

void CControlTabMgr::Create(CControlBase* a2, char a3, int a4)
{
    CControlBase::Create(a2);
    if (a3 == 1)
    {
        for (int i = 0; i < 20; ++i)
        {
            if (!a4)
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
        // 對齊反編譯：在目前焦點 Tab 上觸發一次 msg=3 (MouseDown) 事件
        uint8_t idx = m_focusIndex;
        m_Tabs[idx].ControlKeyInputProcess(3, 0, 0, 0, 0, 0);
    }
}

void CControlTabMgr::CreateChildren()
{
    for (int i = 0; i < 20; ++i)
    {
        // 對齊反編譯：(vtbl+12)(tab, this) → Create(this)
        m_Tabs[i].Create(this);
        // 對齊反編譯：(vtbl+12)(sep, this) → Create(this)
        m_Seps[i].Create(this);
        m_Seps[i].SetText((char*)"|");
        m_Seps[i].m_TextColor = 0xFFFFFFFF; // this[37] = -1
    }
    SetTabTextColor(0xFFFFFFFF, 0xFF8C2C0C, 0xFF000000, 0xFFC99273);
}

void CControlTabMgr::Init()
{
    m_tabCount = 0;
    m_focusIndex = 0;
}

void CControlTabMgr::SetTextTabPosition(int a2)
{
    int v11[2];
    CControlBase::GetAbsPos(v11[0], v11[1]);
    int v3 = v11[0];
    int v10 = v11[1];
    int v7 = 0;

    if (m_tabCount)
    {
        for (int i = 0; i < m_tabCount; ++i)
        {
            CControlTab& tab = m_Tabs[i];
            CControlText& sep = m_Seps[i];

            // 設定 Tab 位置
            tab.SetAbsPos(v3, v10);

            // 取得 Tab 文字尺寸，設定 Tab 大小
            DWORD v12[2], v13[2];
            short v8 = static_cast<short>(*reinterpret_cast<WORD*>(
                reinterpret_cast<char*>(tab.GetTextCtrl()->GetTextLength(v12)) + 2));
            short tabW = static_cast<short>(*reinterpret_cast<WORD*>(
                tab.GetTextCtrl()->GetTextLength(v13)));
            tab.SetSize(static_cast<uint16_t>(tabW), static_cast<uint16_t>(v8));

            int v6 = v3 + tab.GetWidth() + 6;
            sep.SetAbsPos(v6, v10);

            DWORD v14[2];
            int sepW = *reinterpret_cast<WORD*>(sep.GetTextLength(v14));
            v3 = v6 + sepW + 6;

            if (a2)
            {
                if (a2 < v3 - v11[0])
                {
                    v3 = v11[0];
                    v10 += 14;
                }
            }
        }
    }
}

void CControlTabMgr::AddTab(int a2, int a3, unsigned int a4, uint16_t a5, uint16_t a6,
                              uint16_t a7, uint16_t a8, int a9)
{
    if (m_tabCount >= 20)
        return;

    if (m_tabCount == 0)
        SetAbsPos(a2, a3);

    CControlTab& tab = m_Tabs[m_tabCount];
    tab.SetAbsPos(a2, a3);
    tab.SetImage(a4, a5, a6, a7, a8);

    if (a9)
        tab.SetText(a9);

    // 對齊反編譯：計算包框大小
    short v14 = static_cast<short>(tab.GetX() - m_Tabs[0].GetX());
    uint16_t totalW = static_cast<uint16_t>(tab.GetWidth() + v14);
    short v17 = static_cast<short>(tab.GetY() - m_Tabs[0].GetY());
    uint16_t totalH = static_cast<uint16_t>(tab.GetHeight() + v17);
    SetSize(totalW, totalH);

    if (m_tabCount > 0)
        tab.SetFocus(0);

    // 對齊反編譯：SetArrayIndex(tabCount)
    tab.SetIndex(m_tabCount);

    ++m_tabCount;
}

void CControlTabMgr::AddTab(int a2, int a3, unsigned int a4, uint16_t a5, uint16_t a6, int a7)
{
    AddTab(a2, a3, a4, a5, a6, 0xFFFF, 0xFFFF, a7);
}

void CControlTabMgr::AddTab(int a2, int a3, char a4, int a5)
{
    unsigned int v5;
    uint16_t v6, v7;

    if (a4 == 0)
    {
        v5 = 536870949u; // 0x20000025
        v6 = 61;
        v7 = 62;
    }
    else if (a4 == 1)
    {
        v5 = 536870931u; // 0x20000013
        v6 = 33;
        v7 = 34;
    }
    else if (a4 == 2)
    {
        v5 = 536870931u;
        v6 = 35;
        v7 = 36;
    }
    else
    {
        return;
    }

    AddTab(a2, a3, v5, v6, v7, a5);
}

void CControlTabMgr::SetAllAlpha(int a2)
{
    for (int i = 0; i < m_tabCount; ++i)
        m_Tabs[i].SetTabAlpha(a2);
}

void CControlTabMgr::SetTabCount(uint8_t a2)
{
    m_tabCount = a2;
    if (m_Tabs[0].GetTabType() == 1)
    {
        for (int i = 0; i < 20; ++i)
        {
            if (i >= m_tabCount - 1)
                m_Seps[i].Hide();
            else
                m_Seps[i].Show();
        }
    }
}

void CControlTabMgr::SetTabTextColor(uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    for (int i = 0; i < 20; ++i)
        m_Tabs[i].SetTextColor(a2, a3, a4, a5);
}

void CControlTabMgr::ChildKeyInputProcess(int msg, int a3, int /*x*/, int /*y*/, int /*a6*/, int /*a7*/)
{
    // 對齊反編譯：msg==0 時，a3 是子物件指標，+68 取 index
    if (msg == 0)
    {
        CControlBase* child = reinterpret_cast<CControlBase*>(a3);
        int idx = child->GetArrayIndex();
        SetFocusTabIndex(static_cast<uint8_t>(idx));
    }
}

void CControlTabMgr::Show()
{
    CControlBase::Show();

    // 隱藏超出數量的 Tab
    if (m_tabCount < 20)
    {
        for (int i = m_tabCount; i < 20; ++i)
            m_Tabs[i].Hide();
    }

    if (m_Tabs[0].GetTabType() == 1)
    {
        // 文字 Tab：控制分隔符顯隱
        for (int i = 0; i < 20; ++i)
        {
            if (i >= m_tabCount - 1)
                m_Seps[i].Hide();
            else
                m_Seps[i].Show();
        }
    }
    else
    {
        // 非文字 Tab：全部隱藏分隔符
        for (int i = 0; i < 20; ++i)
            m_Seps[i].Hide();
    }
}

void CControlTabMgr::SetFocusTabIndex(uint8_t a2)
{
    if (a2 < m_tabCount)
    {
        m_Tabs[m_focusIndex].SetFocus(0);
        m_focusIndex = a2;
        m_Tabs[a2].SetFocus(1);
    }
}

uint8_t CControlTabMgr::GetFocusTabIndex()
{
    return m_focusIndex;
}

void CControlTabMgr::ActiveTab(uint8_t a2)
{
    if (a2 < 20)
        m_Tabs[a2].Active();
}

int CControlTabMgr::IsActiveTab(uint8_t a2)
{
    if (a2 < 20)
        return m_Tabs[a2].IsActive();
    return 0;
}

void CControlTabMgr::NoneActiveTab(uint8_t a2)
{
    if (a2 < 20)
        m_Tabs[a2].NoneActive();
}

void CControlTabMgr::Draw()
{
    // 對齊反編譯：先畫非焦點（從後往前），再畫焦點
    int v2 = static_cast<int>(m_tabCount) - 1;
    for (int i = v2; i >= 0; --i)
    {
        if (i != m_focusIndex)
            m_Tabs[i].Draw();
    }

    // 畫焦點 Tab
    if (m_tabCount > 0)
        m_Tabs[m_focusIndex].Draw();

    // 文字 Tab：畫分隔符
    if (m_Tabs[0].GetTabType() == 1)
    {
        int sepCount = static_cast<int>(m_tabCount) - 1;
        for (int i = 0; i < sepCount; ++i)
            m_Seps[i].Draw();
    }
}
