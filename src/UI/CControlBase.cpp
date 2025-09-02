#include "UI/CControlBase.h"
#include "global.h"
#include "Windows.h"

// 開區間矩形判定（符合 Win32 PtInRect 行為）
static inline bool PtInRectOpen(const RECT& rc, stPoint pt)
{
    return (pt.x >= rc.left) && (pt.x < rc.right) &&
        (pt.y >= rc.top) && (pt.y < rc.bottom);
}

// ======================================================================
// 建構/解構
// ======================================================================
CControlBase::CControlBase()
{
    m_ToolTip.Init();
}

CControlBase::~CControlBase()
{
    // stToolTipData 析構器會自行清理
}

// ======================================================================
// 建立流程
// ======================================================================
void CControlBase::Create(CControlBase* pParent)
{
    if (!m_pParent && pParent)
    {
        m_pParent = pParent;
        m_pParent->AddChild(this);
    }
}

void CControlBase::Create(int x, int y, CControlBase* pParent)
{
    Create(pParent);
    SetAbsPos(x, y);
}

void CControlBase::Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent)
{
    m_usWidth = w;
    m_usHeight = h;
    Create(x, y, pParent);
}

// ======================================================================
// 子物件管理
// ======================================================================
void CControlBase::AddChild(CControlBase* pChild)
{
    if (!pChild) return;
    if (m_pLastChild)
    {
        m_pLastChild->m_pNext = pChild;
        pChild->m_pPrev = m_pLastChild;
        m_pLastChild = pChild;
    }
    else
    {
        m_pFirstChild = m_pLastChild = pChild;
    }
}

void CControlBase::DeleteChild()
{
    m_pFirstChild = nullptr;
    m_pLastChild = nullptr;
}

void CControlBase::DelLastChild()
{
    if (!m_pLastChild) return;
    if (m_pLastChild == m_pFirstChild)
    {
        m_pFirstChild = m_pLastChild = nullptr;
    }
    else
    {
        CControlBase* prev = m_pLastChild->m_pPrev;
        m_pLastChild = prev;
        if (prev) prev->m_pNext = nullptr;
    }
}

CControlBase* CControlBase::GetChild(int index)
{
    CControlBase* cur = m_pFirstChild;
    int i = 0;
    while (cur && i < index)
    {
        cur = cur->m_pNext;
        ++i;
    }
    return (i == index) ? cur : nullptr;
}

// ======================================================================
// 位置/尺寸
// ======================================================================
void CControlBase::SetPos(int x, int y)
{
    m_x = x;
    m_y = y;
}

void CControlBase::SetPos(stPoint pt)
{
    SetPos(pt.x, pt.y);
}

void CControlBase::SetX(int x) { m_x = x; }
void CControlBase::SetY(int y) { m_y = y; }

void CControlBase::SetAbsPos(int absX, int absY)
{
    int px = 0, py = 0;
    for (CControlBase* p = m_pParent; p; p = p->m_pParent)
    {
        px += p->GetX();
        py += p->GetY();
    }
    m_x = absX - px;
    m_y = absY - py;
}

void CControlBase::SetAbsPos(stPoint pt)
{
    SetAbsPos(pt.x, pt.y);
}

void CControlBase::SetAbsX(int absX)
{
    int px = 0;
    for (CControlBase* p = m_pParent; p; p = p->m_pParent)
        px += p->GetX();
    m_x = absX - px;
}

void CControlBase::SetAbsY(int absY)
{
    int py = 0;
    for (CControlBase* p = m_pParent; p; p = p->m_pParent)
        py += p->GetY();
    m_y = absY - py;
}

void CControlBase::GetAbsPos(int& outAbsX, int& outAbsY) const
{
    outAbsX = m_x;
    outAbsY = m_y;
    for (CControlBase* p = m_pParent; p; p = p->m_pParent)
    {
        outAbsX += p->GetX();
        outAbsY += p->GetY();
    }
}

int CControlBase::GetAbsX() const
{
    int ax, ay;
    GetAbsPos(ax, ay);
    return ax;
}

int CControlBase::GetAbsY() const
{
    int ax, ay;
    GetAbsPos(ax, ay);
    return ay;
}

void CControlBase::MoveWindow(int dx, int dy)
{
    m_x += dx;
    m_y += dy;
}

void CControlBase::SetChildPosMove(int dx, int dy)
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
        c->MoveWindow(dx, dy);
}

// ======================================================================
// 置中
// ======================================================================
void CControlBase::SetCenterPos(int centerAbsX, int centerAbsY)
{
    int newAbsX = static_cast<int>(centerAbsX - static_cast<int>(m_usWidth * 0.5f));
    int newAbsY = static_cast<int>(centerAbsY - static_cast<int>(m_usHeight * 0.5f));

    // 如需針對 m_bCenterOrigin 做特別微調，可在此加上專案規範
    SetAbsPos(newAbsX, newAbsY);
}

void CControlBase::SetWindowCenterAbsPos()
{
    m_x = static_cast<int>(g_Game_System_Info.ScreenWidth * 0.5 - m_usWidth * 0.5);
    m_y = static_cast<int>(static_cast<double>(g_Game_System_Info.ScreenHeight) * 0.5 - m_usHeight * 0.5);
}

void CControlBase::SetCenterPos()
{
    if (m_pParent)
    {
        SetCenterXToParent();
        SetCenterYToParent();
    }
    else
    {
        SetWindowCenterAbsPos();
    }
}

void CControlBase::SetCenterXToParent()
{
    if (!m_pParent) return;
    m_x = static_cast<int>(m_pParent->GetWidth() * 0.5f - m_usWidth * 0.5f);
}

void CControlBase::SetCenterYToParent()
{
    if (!m_pParent) return;
    m_y = static_cast<int>(m_pParent->GetHeight() * 0.5f - m_usHeight * 0.5f);
}

// ======================================================================
// 顯示/隱藏 與 繪製
// ======================================================================
void CControlBase::Show()
{
    m_bIsVisible = true;
    ShowChildren();
}

void CControlBase::Hide()
{
    m_bIsVisible = false;
    HideChildren();
}

void CControlBase::ShowChildren()
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
        c->Show();
}

void CControlBase::HideChildren()
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
        c->Hide();
}

// 只為可見子物件做準備，不呼叫自身 OnPrepareDrawing()
void CControlBase::PrepareDrawing()
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
    {
        if (c->IsVisible())
            c->PrepareDrawing();
    }
}

// 只遞迴子物件，不呼叫自身 OnDraw()
void CControlBase::Draw()
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
    {
        if (c->IsVisible())
            c->Draw();
    }
}

// ======================================================================
// 事件冒泡
// ======================================================================
int* CControlBase::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 根據反編譯碼，回傳值 result 初始設為父物件指標
    CControlBase* pResult = m_pParent;

    if (m_pParent)
    {
        if (m_bPassKeyInputToParent)
        {
            // 如果旗標為 true，則呼叫父物件的函式，並用其回傳值更新 pResult
            pResult = (CControlBase*)m_pParent->ControlKeyInputProcess(msg, key, x, y, a6, a7);
        }
        // 如果旗標為 false，則不執行任何操作，pResult 保持為 m_pParent 的值
    }
    // 如果 m_pParent 為 nullptr，pResult 從一開始就是 nullptr

    return (int*)pResult;
}


// ======================================================================
// 命中測試（右/下邊界開區間），考慮中心原點與縮放
// ======================================================================
bool CControlBase::PtInCtrl(stPoint pt) const
{
    if (!m_bIsVisible || m_usWidth == 0 || m_usHeight == 0)
        return false;

    int ax, ay;
    GetAbsPos(ax, ay);

    float w = static_cast<float>(m_usWidth) * m_fScaleX;
    float h = static_cast<float>(m_usHeight) * m_fScaleY;

    float left = static_cast<float>(ax);
    float top = static_cast<float>(ay);

    if (m_bCenterOrigin)
    {
        left -= w * 0.5f;
        top -= h * 0.5f;
    }

    RECT rc;
    rc.left = static_cast<int>(left);
    rc.top = static_cast<int>(top);
    rc.right = static_cast<int>(left + w);
    rc.bottom = static_cast<int>(top + h);

    return PtInRectOpen(rc, pt);
}

// ======================================================================
// ToolTip/資料初始化
// ======================================================================
void CControlBase::InitLogIn()
{
    if (m_bToolTipEnabled)
        m_ToolTip.Init();

    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
        c->InitLogIn();
}

void CControlBase::ClearData()
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
        c->ClearData();
}

// ------------------------------
// FindClickedChild
// ------------------------------
CControlBase* CControlBase::FindClickedChild(stPoint pt)
{
    CControlBase* node = this;

    while (true)
    {
        // 走訪 node 的子鏈（由 first → next）
        CControlBase* i = node->GetFirstChild();
        for (; i; i = i->m_pNext)
        {
            if (i->m_bIsVisible && i->IsActive() && i->PtInCtrl(stPoint{ pt.x, pt.y }))
                break;
        }

        // 沒找到，回傳目前節點（反編譯：return this）
        if (!i)
            return node;

        // 若不把鍵盤事件傳回父層，且 i 有子結點，往下繼續找
        if (!i->m_bPassKeyInputToParent && i->m_pFirstChild)
        {
            node = i;
            continue;
        }

        // 否則回傳 i（反編譯：return i）
        return i;
    }
}

// ------------------------------
// FindScrollBarCtrlChild
// ------------------------------
CControlBase* CControlBase::FindScrollBarCtrlChild(int /*a2*/, int /*a3*/)
{
    for (CControlBase* c = m_pFirstChild; c; c = c->m_pNext)
    {
        if (c->m_bIsVisible && c->IsActive())
        {
            // 反編譯：if (result[29] == 100) return result;
            if (c->m_nClassId == 100)
                return c;

            // 若有子，遞迴找
            if (c->m_pFirstChild)
            {
                if (CControlBase* hit = c->FindScrollBarCtrlChild(0, 0))
                    return hit;
            }
        }
    }
    return nullptr;
}

// ------------------------------
// ToolTip：Kind/Type 版本
// 反編譯：stToolTipData::SetKindType(this+18, a3, a2, a4, a5, a6, a7, a8);
// ------------------------------
void CControlBase::SetToolTipData(int16_t a2, int a3, int a4, int a5, char a6, int16_t a7, int a8)
{
    m_ToolTip.SetKindType(a3, a2, a4, a5, a6, a7, a8);
}

// ------------------------------
// ToolTip：字串版本
// 反編譯：stToolTipData::SetStringType((this+72), a2, a3);
// ------------------------------
void CControlBase::SetToolTipDataString(char* a2, int a3)
{
    m_ToolTip.SetStringType(a2, a3);
}

// ------------------------------
// ToolTip：描述版本（kind = 17）
// 反編譯：*((DWORD*)this + 18) = 17; *((WORD*)this + 40) = a2;
// 這裡以 SetKindType 形式表達，避免直接觸碰欄位名稱。
// ------------------------------
void CControlBase::SetToolTipDataDesc(uint16_t a2)
{
    // kind=17，僅帶入 desc id，其餘參數填 0
    (void)m_ToolTip.SetKindType(/*kind*/17, /*id*/(int16_t)a2,
        /*a4*/0, /*a5*/0, /*a6*/0,
        /*a7*/0, /*a8*/0);
}

// ------------------------------
// Active 狀態
// 反編譯：Active()   => *((DWORD*)this + 11) = 1;
//        NoneActive()=> *((DWORD*)this + 11) = 0;
//        IsActive() => 回傳上述值
// ------------------------------
void CControlBase::Active()
{
    m_bActive = true;
}
void CControlBase::NoneActive()
{
    m_bActive = false;
}
int CControlBase::IsActive() const
{
    return m_bActive ? 1 : 0;
}
