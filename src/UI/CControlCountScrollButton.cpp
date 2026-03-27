#include "UI/CControlCountScrollButton.h"
#include "global.h"

// ==========================
// 建構
// 反編譯：
//   CControlBase::CControlBase(this);
//   CControlButton::CControlButton((char*)this + 120);
//   CControlBase::CControlBase((char*)this + 844);
//   m_Button.Create(this);  this[45] = 1;
//   m_Layer.Create(this);
//   this[225] = 0;
//   this[241] = 0;  // cur
//   this[243] = 0;  // max
//   this[242] = 0;  // min
//   this[244] = 0;  // dragging
// ==========================
CControlCountScrollButton::CControlCountScrollButton()
    : CControlBase()
    , m_Button()
    , m_Layer()
{
    m_Button.Create(this);
    m_Layer.Create(this);

    m_Cur = 0;
    m_Max = 0;
    m_Min = 0;
    m_Dragging = 0;
}

CControlCountScrollButton::~CControlCountScrollButton()
{
    // 反編譯：逆序解構 m_Layer(CControlBase), m_Button(CControlButton), CControlBase
}

// ==========================
// ChildKeyInputProcess
// 反編譯：
//   v8 = (char*)this + 120;
//   if (a3 == v8 && !a2 && !this[244]) {
//     this[244] = 1;
//     CalcCurCount(a4);
//     dword_AFD34C = 0;
//   }
//   if (a3 == v8) {
//     if (a2 == 2 && this[244] == 1)
//       CalcCurCount(a4);
//     if (a3 == v8 && a2 == 3 && this[244] == 1) {
//       this[244] = 0;
//       dword_AFD34C = 1;
//     }
//   }
// ==========================
void CControlCountScrollButton::ChildKeyInputProcess(int msg, CControlBase* child,
    int x, int y, int /*a6*/, int /*a7*/)
{
    CControlBase* btn = &m_Button;

    if (child == btn && msg == 0 && m_Dragging == 0)
    {
        m_Dragging = 1;
        CalcCurCount(x);
        dword_AFD34C = 0;
    }

    if (child == btn)
    {
        if (msg == 2 && m_Dragging == 1)
            CalcCurCount(x);

        if (child == btn && msg == 3 && m_Dragging == 1)
        {
            m_Dragging = 0;
            dword_AFD34C = 1;
        }
    }
}

// ==========================
// SetButtonPos
// 反編譯：
//   SetPos(a2, a3);
//   WORD[16] = a4;  // width
//   WORD[17] = a5;  // height
//   m_Button.SetPos(0, 0);
// ==========================
void CControlCountScrollButton::SetButtonPos(int x, int y, int16_t w, int16_t h)
{
    SetPos(x, y);
    SetSize(static_cast<uint16_t>(w), static_cast<uint16_t>(h));
    m_Button.SetPos(0, 0);
}

// ==========================
// SetCountLayer
// 反編譯：
//   m_Layer.SetPos(0, 0);
//   WORD[438] = a4;
//   WORD[439] = a5;
// ==========================
// 反編譯：
//   m_Layer.SetPos(0, 0);
//   WORD[438] = a4;  → m_Layer.m_usWidth  (byte offset 844+32 = 876, WORD idx 438)
//   WORD[439] = a5;  → m_Layer.m_usHeight (byte offset 844+34 = 878, WORD idx 439)
void CControlCountScrollButton::SetCountLayer(int /*x*/, int /*y*/, int16_t p1, int16_t p2)
{
    m_Layer.SetPos(0, 0);
    m_Layer.SetSize(static_cast<uint16_t>(p1), static_cast<uint16_t>(p2));
}

// ==========================
// SetCountInfo
// 反編譯：
//   this[241] = a3;  // cur
//   this[243] = a2;  // max
//   this[242] = a4;  // min
//   ControlRebuild();
// ==========================
void CControlCountScrollButton::SetCountInfo(int a2_max, int a3_cur, int a4_min)
{
    m_Cur = a3_cur;
    m_Max = a2_max;
    m_Min = a4_min;
    ControlRebuild();
}

// ==========================
// ControlRebuild
// 反編譯：
//   v2 = (char*)this + 844;  // m_Layer
//   v4 = (float)m_Layer.GetWidth() * ((float)this[241] / (float)this[243]);
//   v3 = m_Layer.GetAbsX();
//   m_Button.SetAbsX((int)v4 + v3);
// ==========================
void CControlCountScrollButton::ControlRebuild()
{
    float advance = (float)m_Layer.GetWidth()
        * ((float)m_Cur / (float)m_Max);
    int baseX = m_Layer.GetAbsX();
    m_Button.SetAbsX((int)(int64_t)advance + baseX);
}

// ==========================
// CalcCurCount
// 反編譯：
//   v3 = (char*)this + 844;  // m_Layer
//   if (a2 >= m_Layer.GetAbsX()) {
//     v4 = m_Layer.GetWidth();
//     if (a2 <= m_Layer.GetAbsX() + v4) {
//       v5 = a2 - m_Layer.GetAbsX();
//       v7 = this[243];  // max
//       v10 = (float)v5;
//       v8 = m_Layer.GetWidth();
//       v9 = (int)(v10 / ((double)v8 / (double)v7) + 1.0);
//       this[241] = v9;
//       if (v9 >= v7) this[241] = v7;
//       --this[241];
//       ControlRebuild();
//     }
//   }
// ==========================
void CControlCountScrollButton::CalcCurCount(int mouseAbsX)
{
    if (mouseAbsX >= m_Layer.GetAbsX())
    {
        int layerW = m_Layer.GetWidth();
        if (mouseAbsX <= m_Layer.GetAbsX() + layerW)
        {
            int rel = mouseAbsX - m_Layer.GetAbsX();
            int maxV = m_Max;
            float fRel = (float)rel;
            uint16_t w = m_Layer.GetWidth();
            int64_t cur = (int64_t)(fRel / ((double)w / (double)maxV) + 1.0);
            m_Cur = (int)cur;
            if ((int)cur >= maxV)
                m_Cur = maxV;
            --m_Cur;
            ControlRebuild();
        }
    }
}
