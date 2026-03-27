#include "UI/CControlScrollBar.h"
#include "global.h"

// 對齊反編譯：static local 變數
static int s_nDeltaY = 0;

// ========================================
// 建構 / 解構
// ========================================
CControlScrollBar::CControlScrollBar()
{
    // 對齊反編譯：預設值已在欄位初始化
}

CControlScrollBar::~CControlScrollBar()
{
    // C++ 自動解構成員
}

// ========================================
// Create
// ========================================
void CControlScrollBar::Create(int a2, int a3, unsigned short a4, unsigned short a5,
                                 CControlBase* a6, char a7)
{
    m_mode = a7; // 對齊反編譯：*(BYTE*)this+140 = a7
    CControlBase::Create(a2, a3, a4, a5, a6);
    m_nClassId = 100; // 對齊反編譯：this[29] = 100

    // 建立子控制項
    m_Background.Create(this);
    m_ArrowUp.Create(this);
    m_Thumb.Create(this);
    m_ArrowDown.Create(this);

    // 設定預設子控制屬性
    // 對齊反編譯：this[238]=1, this[600]=1, this[419]=1
    // 這些是按鈕的 passKeyInputToParent 旗標
    m_ArrowUp.SetPassKeyInputToParent(true);
    m_Thumb.SetPassKeyInputToParent(true);
    m_ArrowDown.SetPassKeyInputToParent(true);

    // 設定預設圖像
    m_Background.SetImage(0x20000013u, 0x31, 0x32, 0x33, 0);
    m_Background.SetSize(a5);

    SetArrowUpImage(0x20000013u, 41, 43, 45, 47);
    SetThumbImage(0x20000013u, 37, 38, 39, 40);
    SetArrowDownImage(0x20000013u, 42, 44, 46, 48);
}

// ========================================
// SetHeight
// ========================================
void CControlScrollBar::SetHeight(unsigned short a2)
{
    m_usHeight = a2;
    unsigned short downH = m_ArrowDown.GetHeight();
    m_ArrowDown.SetY(a2 - downH);
}

// ========================================
// 圖像設定
// ========================================
void CControlScrollBar::SetArrowUpImage(unsigned int a2, int a3, int a4, int a5, int a6)
{
    m_ArrowUp.SetImage(a2, static_cast<uint16_t>(a3), static_cast<uint16_t>(a4),
                        static_cast<uint16_t>(a5), static_cast<uint16_t>(a6));
}

void CControlScrollBar::SetThumbImage(unsigned int a2, int a3, int a4, int a5, int a6)
{
    m_Thumb.SetImage(a2, static_cast<uint16_t>(a3), static_cast<uint16_t>(a4),
                      static_cast<uint16_t>(a5), static_cast<uint16_t>(a6));
    unsigned short upH = m_ArrowUp.GetHeight();
    m_Thumb.SetY(upH);
}

void CControlScrollBar::SetArrowDownImage(unsigned int a2, int a3, int a4, int a5, int a6)
{
    m_ArrowDown.SetImage(a2, static_cast<uint16_t>(a3), static_cast<uint16_t>(a4),
                          static_cast<uint16_t>(a5), static_cast<uint16_t>(a6));
    unsigned short downH = m_ArrowDown.GetHeight();
    unsigned short totalH = GetHeight();
    m_ArrowDown.SetY(totalH - downH);
}

// ========================================
// 回呼
// ========================================
void CControlScrollBar::SetCallFunc(int obj, int funcPtr, int param1, int param2, int param3)
{
    m_callbackObj = obj;
    m_callbackFunc = funcPtr;
    m_callbackOffset = param1;
    m_callbackParam1 = param2;
    m_callbackParam2 = param3;
}

void CControlScrollBar::CallScrollFunc()
{
    if (m_callbackObj && m_callbackFunc)
    {
        typedef void (__thiscall *CallbackFn)(int);
        CallbackFn fn = reinterpret_cast<CallbackFn>(m_callbackFunc);
        fn(m_callbackObj + m_callbackOffset);
    }
}

void CControlScrollBar::HideBackground()
{
    m_Background.Clear();
}

// ========================================
// SetScrollRange
// ========================================
void CControlScrollBar::SetScrollRange(int a2, int a3, int a4, int a5, int a6)
{
    m_scrollMin = a2;
    m_scrollMax = a3;
    m_pageSize = a4;
    m_lineStep = a5;

    short v9 = 2 * m_ArrowUp.GetHeight();
    short v18 = GetHeight() - v9;
    short v10 = m_Thumb.GetHeight();
    m_trackLen = v18 - v10;
    m_thumbMinY = m_ArrowUp.GetHeight();
    m_thumbMaxY = m_ArrowDown.GetY() - m_Thumb.GetHeight();

    if (a3 <= a4)
    {
        // 範圍不足：停用
        m_ArrowUp.NoneActive();
        m_ArrowDown.NoneActive();
        m_Thumb.Hide();
        m_bEnabled = 0;
        // 對齊反編譯：disable path → goto LABEL_9
        if (a6)
            ClearScroll();
        return;
    }

    // 啟用
    m_ArrowUp.Active();
    m_ArrowDown.Active();
    m_Thumb.Show();
    m_bEnabled = 1;

    int v12 = m_lineStep;
    if (v12)
    {
        int range = m_scrollMax - m_pageSize;
        int v14 = range / v12;
        if (range % v12)
            ++v14;
        if (v14)
        {
            unsigned short tl = m_trackLen;
            m_pixelsPerStep = static_cast<double>(tl) / static_cast<double>(v14);
            if (tl)
            {
                m_stepsPerPixel = static_cast<double>(v14) / static_cast<double>(tl);
                // 對齊反編譯：只有 lineStep!=0 && numSteps!=0 && trackLen!=0 時才 ClearScroll
                if (a6)
                    ClearScroll();
                return;
            }
        }
    }
    // 對齊反編譯：lineStep==0 || numSteps==0 || trackLen==0 時不呼叫 ClearScroll
}

// ========================================
// Scroll
// ========================================
void CControlScrollBar::Scroll(int a2, int a3, int a4)
{
    if (!m_bEnabled)
        return;

    int v5;
    switch (a2)
    {
    case 0: v5 = -m_lineStep; break;
    case 1: v5 = m_lineStep; break;
    case 2: v5 = m_lineStep - m_pageSize; break;
    case 3: v5 = m_pageSize - m_lineStep; break;
    case 4: v5 = a3 - m_scrollPos; break;
    default: v5 = a2; break;
    }

    int v6 = v5 + m_scrollPos;

    if (v6 < m_scrollMin)
        v6 = m_scrollMin;
    else if (v6 > m_scrollMax)
        v6 = m_scrollMax;
    else
    {
        int v8 = m_lineStep;
        if (!v8) return;
        int remaining = m_scrollMax - v6;
        int pagesRemaining = remaining / v8;
        if (remaining % v8) ++pagesRemaining;
        int pagesVisible = m_pageSize / v8;
        if (pagesVisible > pagesRemaining)
            v6 -= v8 * (pagesVisible - pagesRemaining);
    }

    if (v6 != m_scrollPos)
    {
        m_scrollPos = v6;
        CallScrollFunc();
        if (a4)
            SetThumbPosition(v6);
    }
}

// ========================================
// ProcessMoveThumb
// ========================================
void CControlScrollBar::ProcessMoveThumb(int a2)
{
    m_Thumb.SetAbsY(a2);

    int thumbY = m_Thumb.GetY();
    if (thumbY < m_thumbMinY || thumbY > m_thumbMaxY)
    {
        int clampY = (thumbY < m_thumbMinY) ? m_thumbMinY : m_thumbMaxY;
        m_Thumb.SetY(clampY);
    }

    if (m_lineStep)
    {
        int y = m_Thumb.GetY();
        if (y == m_thumbMinY)
        {
            Scroll(4, m_scrollMin, 0);
            return;
        }
        int v11;
        if (y == m_thumbMaxY)
        {
            int range = m_scrollMax - m_pageSize;
            if (m_scrollMax % m_lineStep)
            {
                Scroll(4, m_lineStep * (range / m_lineStep + 1), 0);
                return;
            }
            v11 = range;
        }
        else
        {
            v11 = static_cast<int>(m_lineStep * static_cast<double>(y - m_thumbMinY) * m_stepsPerPixel);
        }
        Scroll(4, v11, 0);
    }
}

// ========================================
// SetThumbPosition
// ========================================
void CControlScrollBar::SetThumbPosition(int a2)
{
    int v3;
    if (a2 == m_scrollMin)
    {
        v3 = 0;
    }
    else if (a2 == m_scrollMax)
    {
        v3 = m_trackLen;
    }
    else
    {
        int v4 = m_lineStep;
        if (!v4) return;
        int v6 = m_scrollPos / v4;
        if (m_scrollPos % v4) ++v6;
        v3 = static_cast<int>(static_cast<double>(v6) * m_pixelsPerStep);
    }

    unsigned short upH = m_ArrowUp.GetHeight();
    m_Thumb.SetY(v3 + upH);
}

// ========================================
// GetScrollPos / GetScrollMax
// ========================================
int CControlScrollBar::GetScrollPos()
{
    return m_scrollPos;
}

int CControlScrollBar::GetScrollMax()
{
    return m_scrollMax + m_pageSize;
}

// ========================================
// IsInViewLegion
// ========================================
BOOL CControlScrollBar::IsInViewLegion(unsigned short a2)
{
    return m_scrollPos <= a2 && m_scrollPos + m_pageSize > a2;
}

// ========================================
// ClearScroll
// ========================================
void CControlScrollBar::ClearScroll()
{
    if (m_bEnabled)
    {
        Scroll(4, m_scrollMin, 1);
    }
    else
    {
        m_scrollPos = m_scrollMin;
        SetThumbPosition(m_scrollMin);
    }
}

// ========================================
// ControlKeyInputProcess
// ========================================
int* CControlScrollBar::ControlKeyInputProcess(int a2, int a3, int a4, int a5, int a6, int a7)
{
    if (a2 == 0)
    {
        // Click：check if click is above or below thumb for page scroll
        if (!m_bEnabled)
            return nullptr;

        int thumbAbsY = m_Thumb.GetAbsY();
        unsigned short thumbH = m_Thumb.GetHeight();

        if (a5 < thumbAbsY)
        {
            Scroll(2, 0, 1); // page up
        }
        else if (a5 > thumbAbsY + thumbH)
        {
            Scroll(3, 0, 1); // page down
        }
    }
    else if (a2 == 8)
    {
        Scroll(0, 0, 1); // scroll up
    }
    else if (a2 == 9)
    {
        Scroll(1, 0, 1); // scroll down
    }

    return CControlBase::ControlKeyInputProcess(a2, a3, a4, a5, a6, a7);
}

// ========================================
// ChildKeyInputProcess
// ========================================
void CControlScrollBar::ChildKeyInputProcess(int a2, int a3, int /*a4*/, int a5, int /*a6*/, int /*a7*/)
{
    CControlBase* child = reinterpret_cast<CControlBase*>(a3);

    if (child == &m_ArrowUp && a2 == 3)
    {
        Scroll(0, 0, 1);
        dword_AFD34C = 1;
    }
    else if (child == &m_ArrowDown && a2 == 3)
    {
        Scroll(1, 0, 1);
        dword_AFD34C = 1;
    }
    else if (child == &m_Thumb)
    {
        if (a2 == 0)
        {
            int v7 = m_Thumb.GetAbsY();
            dword_AFD34C = 0;
            s_nDeltaY = a5 - v7;
        }
        else if (a2 == 2)
        {
            ProcessMoveThumb(a5 - s_nDeltaY);
        }
    }
}

// ========================================
// Show
// ========================================
void CControlScrollBar::Show()
{
    CControlBase::Show();
}
