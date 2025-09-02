#include "UI/CControlCountBar.h"

//------------------------------------------------------------------------------
// 建構 / 解構：建立所有子控制並掛在自己底下
//------------------------------------------------------------------------------
CControlCountBar::CControlCountBar()
{
    // 兩顆箭頭按鈕
    m_btnLeft.Create(this);
    m_btnRight.Create(this);

    // 三段圖
    m_imgLeft.Create(this);
    m_imgMid.Create(this);
    m_imgRight.Create(this);

    // 軌道區（僅作為命中/拖曳判定區）
    m_track.Create(this);

    // 初始狀態
    m_cur = m_max = m_min = 0;
    m_dragging = false;
    m_scale = 0.0f;
}

CControlCountBar::~CControlCountBar()
{
    // 成員自動解構（順序：m_track → m_imgRight → m_imgMid → m_imgLeft → m_btnRight → m_btnLeft → base）
}

//------------------------------------------------------------------------------
// 事件處理（對照反編譯 ControlKeyInputProcess）
// msg: 0=按下（在本系統中常見為鼠鍵觸發），2=移動，3=彈起，7=滑入（未用到）
// x,y 為當前滑鼠座標（以螢幕/全域座標）
//------------------------------------------------------------------------------
int* CControlCountBar::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    if (msg) {
        if (msg == 2) {
            // mouse move
            if (m_dragging)
                CalcCurCount(x);
        }
        else if (msg == 3) {
            // mouse up
            if (m_dragging)
                m_dragging = false;
            // g_LButtonUp = 1; // 全域旗標，對外觀無影響，略
        }
    }
    else {
        // mouse down
        if (m_track.PtInCtrl({ x, y })) {
            m_dragging = true;
            CalcCurCount(x);
            // g_LButtonUp = 0;
        }
    }

    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

//------------------------------------------------------------------------------
// 子控制事件：左右箭頭按下（msg==3）觸發 +/-1（對照反編譯 ChildKeyInputProcess）
//------------------------------------------------------------------------------
void CControlCountBar::ChildKeyInputProcess(int msg, CControlBase* child, int a4, int a5, int a6, int a7)
{
    if (child == &m_btnLeft && msg == 3) {
        ArrowPosLeft(1);
        // g_LButtonUp = 1;
    }
    else {
        if (child == &m_btnRight && msg == 3) {
            ArrowPosRight(1);
        }
        // g_LButtonUp = 1;
    }
}

//------------------------------------------------------------------------------
// 反編譯 SetArrowPos：設定本控制大小、左/右箭頭相對位置
//------------------------------------------------------------------------------ 
void CControlCountBar::SetArrowPos(int xLeft, int yTop, int xRight, int /*unused*/)
{
    // 設定自己左上
    CControlBase::SetPos(xLeft, yTop);

    // 高度取左鍵高度；寬度 = (rightX - leftX) + 右鍵寬
    const uint16_t hBtnL = m_btnLeft.GetHeight();
    const uint16_t wBtnR = m_btnRight.GetWidth();
    const uint16_t width = static_cast<uint16_t>((xRight - xLeft) + wBtnR);
    const uint16_t height = hBtnL;

    CControlBase::SetSize(width, height);

    // 左鍵置於 (0,0)
    m_btnLeft.SetPos(0, 0);

    // 右鍵置於 (本控制寬 - 右鍵寬, 0)
    const uint16_t wSelf = GetWidth();
    m_btnRight.SetPos(wSelf - wBtnR, 0);
}

//------------------------------------------------------------------------------
// 反編譯 SetCountLayer：設定軌道區與三段圖的佈局
//------------------------------------------------------------------------------
void CControlCountBar::SetCountLayer(int x, int y, int16_t width, int height)
{
    // 軌道區位置與尺寸
    m_track.SetPos(x, y);
    m_track.SetSize(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    m_layerW = static_cast<uint16_t>(width);
    m_layerH = static_cast<uint16_t>(height);

    // 三段圖貼齊排列：Left | Mid | Right
    m_imgLeft.SetPos(x, y);
    const uint16_t wL = m_imgLeft.GetWidth();

    m_imgMid.SetPos(x + wL, y);
    const uint16_t wM = m_imgMid.GetWidth();

    m_imgRight.SetPos(x + wL + wM, y);
}

//------------------------------------------------------------------------------
// 反編譯 SetCountInfo：a2=max, a3=cur, a4=min，並立即重建
//------------------------------------------------------------------------------
void CControlCountBar::SetCountInfo(int maxValue, int curValue, int minValue)
{
    m_cur = curValue;
    m_max = maxValue;
    m_min = minValue;
    ControlRebuild();
}

//------------------------------------------------------------------------------
// 反編譯 ArrowPosLeft：cur != min 且 cur>0 時，cur -= step
//------------------------------------------------------------------------------
void CControlCountBar::ArrowPosLeft(int step)
{
    int v = m_cur;
    if (v != m_min) {
        if (v) {
            m_cur = v - step;
            if (m_cur < m_min) m_cur = m_min;
            ControlRebuild();
        }
    }
}

//------------------------------------------------------------------------------
// 反編譯 ArrowPosRight：cur != max 時，cur += step
//------------------------------------------------------------------------------
void CControlCountBar::ArrowPosRight(int step)
{
    int v = m_cur;
    if (v != m_max) {
        m_cur = v + step;
        if (m_cur > m_max) m_cur = m_max;
        ControlRebuild();
    }
}

//------------------------------------------------------------------------------
// 反編譯 ControlRebuild：依 cur/max 決定顯示與中段縮放，設定右帽 AbsX
//------------------------------------------------------------------------------
void CControlCountBar::ControlRebuild()
{
    // 夾制
    if (m_max < m_min) std::swap(m_min, m_max);
    if (m_cur < m_min) m_cur = m_min;
    if (m_cur > m_max) m_cur = m_max;

    const uint16_t capL = m_imgLeft.GetWidth();
    const uint16_t capR = m_imgRight.GetWidth();
    const int caps = int(capL) + int(capR);

    const uint16_t trackW = m_track.GetWidth();
    const double desired = (m_max > 0)
        ? double(trackW) * (double(m_cur) / double(m_max))
        : 0.0;

    if (desired > 1.0) {
        // 顯示三段
        m_imgLeft.Show();
        m_imgMid.Show();
        m_imgRight.Show();

        // 需要填滿的中段像素
        double fillMidPixels = desired - double(caps);
        if (fillMidPixels < 0.0) fillMidPixels = 0.0;

        const uint16_t wM = m_imgMid.GetWidth();
        m_scale = (wM > 0) ? float(fillMidPixels / double(wM)) : 0.0f;
    }
    else if (m_cur < m_min) {
        // 小於最小：隱藏
        m_imgLeft.Hide();
        m_imgMid.Hide();
        m_imgRight.Hide();
        m_scale = 0.0f;
    }
    else {
        // 最低顯示：三段皆顯示，中段縮放為 1.0（右帽緊接中段原寬）
        m_imgLeft.Show();
        m_imgMid.Show();
        m_imgRight.Show();
        m_scale = 1.0f;
    }

    // 右帽的絕對 X = mid.AbsX + mid.Width * scale
    const int midAbsX = m_imgMid.GetAbsX();
    const uint16_t wM = m_imgMid.GetWidth();
    const int rightAbsX = midAbsX + int(double(wM) * double(m_scale));
    m_imgRight.SetAbsX(rightAbsX);
}

//------------------------------------------------------------------------------
// 反編譯 CalcCurCount：把滑鼠在軌道中的 X 映射到 [1..max]，再夾到 max
//------------------------------------------------------------------------------
void CControlCountBar::CalcCurCount(int mouseAbsX)
{
    const int trackAbsX = m_track.GetAbsX();
    if (mouseAbsX < trackAbsX) return;

    const uint16_t trackW = m_track.GetWidth();
    if (mouseAbsX > trackAbsX + trackW) return;

    const int dx = mouseAbsX - trackAbsX;
    const int maxV = (m_max > 0) ? m_max : 0;

    if (maxV > 0) {
        // 反編譯：cur = ( dx / (trackW / max) ) + 1.0
        const double unit = double(trackW) / double(maxV);
        int cur = int(double(dx) / unit + 1.0);
        if (cur >= maxV) cur = maxV;
        m_cur = cur;
        ControlRebuild();
    }
}
