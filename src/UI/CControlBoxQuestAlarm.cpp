#include "UI/CControlBoxQuestAlarm.h"
#include "global.h"

//--------------------------------------------------
// ctor (0041AF60)
//--------------------------------------------------
CControlBoxQuestAlarm::CControlBoxQuestAlarm()
    : CControlBoxBase()
    , m_rootBtn()
    , m_rootText()
    , m_childBtns{}
    , m_childTexts{}
{
    Init();
    CreateChildren();
    m_expandedFlag = 0;
    m_reserved = 0;
}

//--------------------------------------------------
// dtor
//--------------------------------------------------
CControlBoxQuestAlarm::~CControlBoxQuestAlarm()
{
    // 成員自動解構
}

//--------------------------------------------------
// Init (0041B0D0)
//--------------------------------------------------
void CControlBoxQuestAlarm::Init()
{
    CControlBoxBase::Init();
    m_bPassKeyInputToParent = false;  // *((_DWORD *)this + 13) = 0
}

//--------------------------------------------------
// Hide (0041B0F0)
//--------------------------------------------------
void CControlBoxQuestAlarm::Hide()
{
    CControlBase::Hide();
    m_rootBtn.Hide();
    m_rootText.Hide();
    for (int i = 0; i < 5; ++i)
    {
        m_childBtns[i].Hide();
        m_childTexts[i].Hide();
    }
}

//--------------------------------------------------
// Show (0041B150)
//--------------------------------------------------
void CControlBoxQuestAlarm::Show()
{
    CControlBase::Show();

    if (m_expandedFlag)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (!std::strcmp(m_childTexts[i].GetText(), ""))
            {
                m_childBtns[i].Hide();
                m_childTexts[i].Hide();
            }
        }
    }
}

//--------------------------------------------------
// HideChild (0041B1E0)
//--------------------------------------------------
void CControlBoxQuestAlarm::HideChild()
{
    for (int i = 0; i < 5; ++i)
    {
        m_childBtns[i].Hide();
        m_childTexts[i].Hide();
    }
}

//--------------------------------------------------
// CreateChildren (0041B220)
//--------------------------------------------------
void CControlBoxQuestAlarm::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // Root Button (+312)
    m_rootBtn.Create(this);
    m_rootBtn.SetPos(10, 10);
    m_rootBtn.SetText("");
    m_rootBtn.SetImage(335544327u, 23, 23, 23, 23);

    // Root Text (+1036)
    m_rootText.Create(23, 10, this);
    m_rootText.SetControlSetFont("MeritoriousElement");
    m_rootText.SetTextColor(0xFFFFFFFF);  // -1

    // 子項（按鈕 + 文字）× 5
    int y = 25;
    for (int i = 0; i < 5; ++i)
    {
        m_childBtns[i].Create(this);
        m_childBtns[i].SetPos(20, y + 1);
        m_childBtns[i].SetImage(335544327u, 29, 29, 29, 29);
        m_childBtns[i].SetText("");

        m_childTexts[i].Create(33, y, this);
        m_childTexts[i].SetControlSetFont("MeritoriousElement");
        m_childTexts[i].SetTextColor(0xFFFFFFFF);  // -1

        y += 15;
    }
}

//--------------------------------------------------
// GetButtonRootTree (0041B320)
//--------------------------------------------------
CControlButton* CControlBoxQuestAlarm::GetButtonRootTree()
{
    return &m_rootBtn;
}

//--------------------------------------------------
// GetTextRoot (0041B330)
//--------------------------------------------------
CControlText* CControlBoxQuestAlarm::GetTextRoot()
{
    return &m_rootText;
}

//--------------------------------------------------
// GetButtonChild (0041B340)
// 反編譯無邊界檢查，直接做指標運算
//--------------------------------------------------
CControlButton* CControlBoxQuestAlarm::GetButtonChild(int idx)
{
    return &m_childBtns[idx];
}

//--------------------------------------------------
// SetRootName (0041B360)
//--------------------------------------------------
void CControlBoxQuestAlarm::SetRootName(char* name)
{
    CControlBase::Show();
    m_rootText.ClearText();
    m_rootText.SetText(name);
    m_rootText.Show();
    m_rootBtn.Show();
    HideChild();
}

//--------------------------------------------------
// SetChildText (0041B3B0)
//--------------------------------------------------
void CControlBoxQuestAlarm::SetChildText(int idx, char* text)
{
    m_childTexts[idx].ClearText();
    m_childTexts[idx].SetText(text);
}

//--------------------------------------------------
// SetChildTextColor (0041B3E0)
// 將 0~1 的 RGBA 浮點數轉為 ARGB 整數色碼
//--------------------------------------------------
int CControlBoxQuestAlarm::SetChildTextColor(int idx, float r, float g, float b, float a)
{
    auto to8 = [](float v) -> int {
        if (v >= 1.0f) return 255;
        if (v <= 0.0f) return 0;
        return static_cast<int>(v * 255.0f + 0.5f);
    };

    int R = to8(r);
    int G = to8(g);
    int B = to8(b);
    int A = to8(a);

    // 反編譯位運算：B | ((G | ((R | (A << 8)) << 8)) << 8) = ARGB
    int color = B | ((G | ((R | (A << 8)) << 8)) << 8);
    m_childTexts[idx].SetTextColor(color);
    return color;
}

//--------------------------------------------------
// SetRootTextColor (004CCF90)
// 與 SetChildTextColor 相同邏輯，但作用於 m_rootText
//--------------------------------------------------
int CControlBoxQuestAlarm::SetRootTextColor(float r, float g, float b, float a)
{
    auto to8 = [](float v) -> int {
        if (v >= 1.0f) return 255;
        if (v <= 0.0f) return 0;
        return static_cast<int>(v * 255.0f + 0.5f);
    };

    int R = to8(r);
    int G = to8(g);
    int B = to8(b);
    int A = to8(a);

    int color = B | ((G | ((R | (A << 8)) << 8)) << 8);
    m_rootText.SetTextColor(color);
    return color;
}

//--------------------------------------------------
// ClearTextData(int idx) (0041B520)
//--------------------------------------------------
void CControlBoxQuestAlarm::ClearTextData(int idx)
{
    m_childTexts[idx].ClearText();
    m_childTexts[idx].Hide();
}

//--------------------------------------------------
// ClearTextData() (0041B550)
// 反編譯：對所有控件呼叫 ClearData (vtable+36)
//--------------------------------------------------
void CControlBoxQuestAlarm::ClearTextData()
{
    m_rootBtn.ClearData();
    m_rootText.ClearData();

    for (int i = 0; i < 5; ++i)
    {
        m_childBtns[i].ClearData();
        m_childTexts[i].ClearData();
    }
}

//--------------------------------------------------
// GetMaxLength (0041B5B0)
// 反編譯：使用 GetTextPixelSize 取得像素寬度
//--------------------------------------------------
int CControlBoxQuestAlarm::GetMaxLength()
{
    int w = 0, h = 0;
    m_rootText.GetTextPixelSize(&w, &h);
    int maxLen = (w >= 0) ? w : 0;

    for (int i = 0; i < 5; ++i)
    {
        const char* s = m_childTexts[i].GetText();
        if (s && std::strcmp(s, "") != 0 && m_childTexts[i].IsStringData())
        {
            int cw = 0, ch = 0;
            m_childTexts[i].GetTextPixelSize(&cw, &ch);
            if (maxLen <= cw)
                maxLen = cw;
        }
    }
    return maxLen;
}

//--------------------------------------------------
// GetMaxHeight (0041B660)
//--------------------------------------------------
int CControlBoxQuestAlarm::GetMaxHeight()
{
    int h = 20;
    if (!m_expandedFlag)
        return h;

    for (int i = 0; i < 5; ++i)
    {
        const char* s = m_childTexts[i].GetText();
        if (s && std::strcmp(s, "") != 0)
            h += 15;
    }
    return h;
}

//--------------------------------------------------
// OnClickedButtonRoot (0041B6D0)
//--------------------------------------------------
void CControlBoxQuestAlarm::OnClickedButtonRoot()
{
    if (m_expandedFlag)
    {
        // 展開 → 收起
        m_expandedFlag = 0;
        for (int i = 0; i < 5; ++i)
        {
            m_childTexts[i].SetText("");
            m_childBtns[i].Hide();
            m_childTexts[i].Hide();
        }
        m_rootBtn.SetImage(335544327u, 23, 23, 23, 23);
    }
    else
    {
        // 收起 → 展開
        m_expandedFlag = 1;
        for (int i = 0; i < 5; ++i)
        {
            const char* s = m_childTexts[i].GetText();
            if (s && std::strcmp(s, "") != 0)
            {
                m_childBtns[i].Show();
                m_childTexts[i].Show();
            }
        }
        m_rootBtn.SetImage(335544327u, 25, 25, 25, 25);
    }
}
