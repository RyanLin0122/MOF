#include "UI/CControlCountScrollButton.h"
#include "global.h"

//------------------------------------------------------------------------------
// ctor / dtor（對齊反編譯的成員初始）
//------------------------------------------------------------------------------
CControlCountScrollButton::CControlCountScrollButton()
    : CControlBase()
    , m_Button()
    , m_Layer()
{
    // 反編譯：ctor 內把 m_Button、m_Layer 掛到自己底下
    m_Button.Create(this);
    m_Layer.Create(this);

    // 其餘狀態清 0（+225、+241..+244）
    m_Cur = 0;
    m_Min = 0;
    m_Max = 0;
    m_Dragging = 0;
}

CControlCountScrollButton::~CControlCountScrollButton()
{
    // 成員依序自動解構：m_Layer、m_Button、再基底
}

//------------------------------------------------------------------------------
// 子件事件：只處理來自 m_Button 的拖曳行為
// msg: 0=Down, 2=Move, 3=Up（依反編譯）
//------------------------------------------------------------------------------
void CControlCountScrollButton::ChildKeyInputProcess(int msg, CControlBase* child,
    int x, int y, int /*a6*/, int /*a7*/)
{
    CControlBase* btn = &m_Button;

    if (child == btn && msg == 0 && m_Dragging == 0) {
        m_Dragging = 1;
        CalcCurCount(x);
        g_LButtonUp = 0;
    }

    if (child == btn) {
        if (msg == 2 && m_Dragging == 1) {
            CalcCurCount(x);
        }
        if (msg == 3 && m_Dragging == 1) {
            m_Dragging = 0;
            g_LButtonUp = 1;
        }
    }
}

//------------------------------------------------------------------------------
// 位置/尺寸
//------------------------------------------------------------------------------
void CControlCountScrollButton::SetButtonPos(int x, int y, int16_t w, int16_t h)
{
    SetPos(x, y);
    SetSize(w, h);
    m_Button.SetPos(0, 0); // 反編譯：按鈕相對本控制 (0,0)
}

void CControlCountScrollButton::SetCountLayer(int /*x*/, int /*y*/, int16_t p1, int16_t p2)
{
    // 反編譯：把滑軌層相對放在 (0,0)，保存兩個 16 位參數
    m_Layer.SetPos(0, 0);
    m_Param1 = p1;
    m_Param2 = p2;
}

//------------------------------------------------------------------------------
// 資料設定
//------------------------------------------------------------------------------
void CControlCountScrollButton::SetCountInfo(int a2_max, int a3_cur, int a4_min)
{
    m_Cur = a3_cur;
    m_Max = a2_max;
    m_Min = a4_min;
    ControlRebuild();
}

//------------------------------------------------------------------------------
// 依 Cur/Max 更新按鈕絕對位置
//------------------------------------------------------------------------------
void CControlCountScrollButton::ControlRebuild()
{
    // Layer 寬度 * (Cur/Max)
    const uint16_t layerW = m_Layer.GetWidth();
    double advance = 0.0;
    if (m_Max != 0) {
        advance = double(layerW) * (double(m_Cur) / double(m_Max));
    }

    const int baseX = m_Layer.GetAbsX();
    m_Button.SetAbsX(baseX + int(advance));
}

//------------------------------------------------------------------------------
// 將滑鼠絕對 X 轉換為 Cur，並重建
// 反編譯：Cur = clamp( floor( x / (W/Max) + 1.0 ), 1, Max ) - 1
//------------------------------------------------------------------------------
void CControlCountScrollButton::CalcCurCount(int mouseAbsX)
{
    const int layerAbsX = m_Layer.GetAbsX();
    const int layerW = m_Layer.GetWidth();

    // 不在滑軌層範圍內則忽略
    if (mouseAbsX < layerAbsX) return;
    if (mouseAbsX > layerAbsX + layerW) return;

    const int rel = mouseAbsX - layerAbsX;
    if (m_Max <= 0) return;

    // 每一單位的像素寬
    const double unitW = double(layerW) / double(m_Max);

    // 反編譯公式：floor(rel / unitW + 1.0)，夾到 [1, Max]，最後再 -1
    int cur = int((double)rel / unitW + 1.0);
    if (cur < 1) cur = 1;
    if (cur > m_Max) cur = m_Max;
    cur -= 1;

    m_Cur = cur;
    ControlRebuild();
}
