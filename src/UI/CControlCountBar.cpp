#include "UI/CControlCountBar.h"
#include "global.h"

// ==========================
// 建構
// 反編譯：
//   CControlBase::CControlBase(this);
//   CControlButton::CControlButton((char*)this + 120);
//   CControlButton::CControlButton((char*)this + 844);
//   CControlImage::CControlImage((char*)this + 1568);
//   CControlImage::CControlImage((char*)this + 1760);
//   CControlImage::CControlImage((char*)this + 1952);
//   CControlBase::CControlBase((char*)this + 2144);
//   // Create children to this:
//   ((char*)this + 120)->Create(this);  this[45] = 1;
//   ((char*)this + 844)->Create(this);  this[226] = 1;
//   ((char*)this + 1568)->Create(this);
//   ((char*)this + 1760)->Create(this);
//   ((char*)this + 1952)->Create(this);
//   ((char*)this + 2144)->Create(this);
//   this[406]=0; this[454]=0; this[502]=0; this[550]=0;
//   this[566]=0; this[567]=0; this[568]=0;
//   BYTE[2276] = 0;
// ==========================
CControlCountBar::CControlCountBar()
    : CControlBase()
    , m_btnLeft()
    , m_btnRight()
    , m_imgLeft()
    , m_imgMid()
    , m_imgRight()
    , m_track()
{
    m_btnLeft.Create(this);
    // 對齊反編譯：*((_DWORD *)this + 45) = 1; → byte offset 60 within m_btnLeft
    reinterpret_cast<int*>(&m_btnLeft)[15] = 1;

    m_btnRight.Create(this);
    // 對齊反編譯：*((_DWORD *)this + 226) = 1; → byte offset 60 within m_btnRight
    reinterpret_cast<int*>(&m_btnRight)[15] = 1;

    m_imgLeft.Create(this);
    m_imgMid.Create(this);
    m_imgRight.Create(this);
    m_track.Create(this);

    // 對齊反編譯：*((_DWORD *)this + 406/454/502/550) = 0;
    // → byte offset 56 within each image/track sub-control
    reinterpret_cast<int*>(&m_imgLeft)[14] = 0;
    reinterpret_cast<int*>(&m_imgMid)[14] = 0;
    reinterpret_cast<int*>(&m_imgRight)[14] = 0;
    reinterpret_cast<int*>(&m_track)[14] = 0;

    m_cur = 0;
    m_max = 0;
    m_min = 0;
    m_dragging = 0;
}

CControlCountBar::~CControlCountBar()
{
    // 反編譯：逆序解構
    // CControlBase(track), CControlImage×3, CControlButton×2, CControlBase
}

// ==========================
// ControlKeyInputProcess
// 反編譯完全對照
// ==========================
int* CControlCountBar::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    if (msg)
    {
        if (msg == 2)
        {
            // mouse move
            if (m_dragging == 1)
                CalcCurCount(x);
        }
        else if (msg == 3)
        {
            // mouse up
            if (m_dragging == 1)
                m_dragging = 0;
            dword_AFD34C = 1;
        }
    }
    else
    {
        // mouse down (msg == 0)
        if (m_track.PtInCtrl({ x, y }))
        {
            m_dragging = 1;
            CalcCurCount(x);
            dword_AFD34C = 0;
        }
    }

    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

// ==========================
// ChildKeyInputProcess
// 反編譯：
//   if (a3 == (char*)this+120 && a2 == 3) {
//     ArrowPosLeft(1);
//     dword_AFD34C = 1;
//   } else {
//     if (a3 == (char*)this+844 && a2 == 3)
//       ArrowPosRight(1);
//     dword_AFD34C = 1;
//   }
// ==========================
void CControlCountBar::ChildKeyInputProcess(int msg, CControlBase* child, int a4, int a5, int a6, int a7)
{
    if (child == &m_btnLeft && msg == 3)
    {
        ArrowPosLeft(1);
        dword_AFD34C = 1;
    }
    else
    {
        if (child == &m_btnRight && msg == 3)
            ArrowPosRight(1);
        dword_AFD34C = 1;
    }
}

// ==========================
// SetArrowPos
// 反編譯：
//   SetPos(a2, a3);
//   v11 = m_btnLeft.GetHeight();  // vtbl+60
//   WORD[16] = a4 - a2 + m_btnRight.GetWidth();  // vtbl+56
//   WORD[17] = v11;
//   m_btnLeft.SetPos(0, 0);
//   v7 = this.GetWidth();  // vtbl+56
//   v10 = m_btnRight.GetWidth();
//   m_btnRight.SetPos(v7 - v10, 0);
// ==========================
void CControlCountBar::SetArrowPos(int xLeft, int yTop, int xRight, int /*unused*/)
{
    CControlBase::SetPos(xLeft, yTop);

    uint16_t hBtnL = m_btnLeft.GetHeight();
    uint16_t wBtnR = m_btnRight.GetWidth();

    SetSize(static_cast<uint16_t>(xRight - xLeft + wBtnR), hBtnL);

    m_btnLeft.SetPos(0, 0);

    uint16_t wSelf = GetWidth();
    uint16_t wRight = m_btnRight.GetWidth();
    m_btnRight.SetPos(wSelf - wRight, 0);
}

// ==========================
// SetCountLayer
// 反編譯：
//   m_track.SetPos(a2, a3);
//   WORD[1089] = a5;
//   WORD[1088] = a4;
//   m_imgLeft.SetPos(a2, a3);
//   v6 = m_imgLeft.GetWidth();
//   m_imgMid.SetPos(a2 + v6, a3);
//   v7 = m_imgMid.GetWidth();
//   v8 = m_imgMid.GetX();
//   m_imgRight.SetPos(v8 + v7, a3);
// ==========================
void CControlCountBar::SetCountLayer(int x, int y, int16_t width, int height)
{
    // 反編譯：
    //   m_track.SetPos(a2, a3);
    //   WORD[1089] = a5;  → track.m_usHeight (byte offset 2144+34 = 2178, WORD idx 1089)
    //   WORD[1088] = a4;  → track.m_usWidth  (byte offset 2144+32 = 2176, WORD idx 1088)
    m_track.SetPos(x, y);
    m_track.SetSize(static_cast<uint16_t>(width), static_cast<uint16_t>(height));

    m_imgLeft.SetPos(x, y);
    uint16_t wL = m_imgLeft.GetWidth();

    m_imgMid.SetPos(x + wL, y);
    uint16_t wM = m_imgMid.GetWidth();
    int midX = m_imgMid.GetX();

    m_imgRight.SetPos(midX + wM, y);
}

// ==========================
// SetCountInfo
// 反編譯：
//   this[566] = a3;  // cur
//   this[567] = a2;  // max
//   this[568] = a4;  // min
//   ControlRebuild();
// ==========================
void CControlCountBar::SetCountInfo(int maxValue, int curValue, int minValue)
{
    m_cur = curValue;
    m_max = maxValue;
    m_min = minValue;
    ControlRebuild();
}

// ==========================
// ArrowPosLeft
// 反編譯：
//   v2 = this[566];
//   if (v2 != this[568]) {    // cur != min
//     if (v2) {                // cur != 0
//       this[566] = v2 - a2;
//       ControlRebuild();
//     }
//   }
// ==========================
void CControlCountBar::ArrowPosLeft(int step)
{
    int v = m_cur;
    if (v != m_min)
    {
        if (v)
        {
            m_cur = v - step;
            ControlRebuild();
        }
    }
}

// ==========================
// ArrowPosRight
// 反編譯：
//   v2 = this[566];
//   if (v2 != this[567]) {    // cur != max
//     this[566] = a2 + v2;
//     ControlRebuild();
//   }
// ==========================
void CControlCountBar::ArrowPosRight(int step)
{
    int v = m_cur;
    if (v != m_max)
    {
        m_cur = step + v;
        ControlRebuild();
    }
}

// ==========================
// ControlRebuild
// 反編譯完全對照
// 計算：v6 = trackW * (cur/max) - caps
//   if v6 > 1.0: Show, scale = v6
//   elif cur < min: Hide
//   else: Show, scale = 1.0
//   rightAbsX = midAbsX + midW * scale
// ==========================
void CControlCountBar::ControlRebuild()
{
    uint16_t rightW = m_imgRight.GetWidth();
    uint16_t leftW = m_imgLeft.GetWidth();
    int caps = leftW + rightW;

    int cur = m_cur;
    double v6 = (double)m_track.GetWidth()
        * ((double)cur / (double)m_max)
        - (double)caps;

    if (v6 > 1.0)
    {
        m_imgLeft.Show();
        m_imgMid.Show();
        m_imgRight.Show();
        m_scale = (float)v6;
    }
    else if (cur < m_min)
    {
        m_imgLeft.Hide();
        m_imgMid.Hide();
        m_imgRight.Hide();
    }
    else
    {
        m_imgLeft.Show();
        m_imgMid.Show();
        m_imgRight.Show();
        m_scale = 1.0f;  // 1065353216 = float 1.0
    }

    // 反編譯：
    //   v9 = float[449];  (m_scale)
    //   v10 = (float)m_imgMid.GetWidth() * v9;
    //   v8 = (double)m_imgMid.GetAbsX() + v10;
    //   m_imgRight.SetAbsX((int)v8);
    float scale = m_scale;
    float advance = (float)m_imgMid.GetWidth() * scale;
    double rightAbsX = (double)m_imgMid.GetAbsX() + (double)advance;
    m_imgRight.SetAbsX((int)(int64_t)rightAbsX);
}

// ==========================
// CalcCurCount
// 反編譯：
//   if (a2 >= m_track.GetAbsX()) {
//     v4 = m_track.GetWidth();
//     if (a2 <= m_track.GetAbsX() + v4) {
//       v8 = a2 - m_track.GetAbsX();
//       v5 = this[567];  // max
//       v7 = (float)v8;
//       v6 = (int)(v7 / ((double)m_track.GetWidth() / (double)v5) + 1.0);
//       this[566] = v6;
//       if (v6 >= v5) this[566] = v5;
//       ControlRebuild();
//     }
//   }
// ==========================
void CControlCountBar::CalcCurCount(int mouseAbsX)
{
    if (mouseAbsX >= m_track.GetAbsX())
    {
        int trackW = m_track.GetWidth();
        if (mouseAbsX <= m_track.GetAbsX() + trackW)
        {
            int dx = mouseAbsX - m_track.GetAbsX();
            int maxV = m_max;
            float fDx = (float)dx;
            int cur = (int)(int64_t)(fDx / ((double)m_track.GetWidth() / (double)maxV) + 1.0);
            m_cur = cur;
            if (cur >= maxV)
                m_cur = maxV;
            ControlRebuild();
        }
    }
}
