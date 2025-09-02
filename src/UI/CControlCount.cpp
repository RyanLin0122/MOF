#include "UI/CControlCount.h"

//------------------------------------------------------------------------------
// Create
//------------------------------------------------------------------------------
void CControlCount::Create(int x, int y, int totalWidth,
    unsigned int group, int idLeft, int idMid, int idRight,
    CControlBase* pParent)
{
    CControlBase::Create(x, y, pParent);

    // 子圖片掛載在本控制下
    m_Left.Create(this);
    m_Mid.Create(this);
    m_Right.Create(this);

    // 設定圖片與初始水平位置
    SetImage(group, idLeft, idMid, idRight);

    // 控制項大小：寬=傳入 totalWidth；高取左端圖高度（反編譯：*((WORD*)this+17) = left.GetHeight()）
    SetSize(static_cast<uint16_t>(totalWidth),
        static_cast<uint16_t>(m_Left.GetHeight()));
}

//------------------------------------------------------------------------------
// SetImage：三張圖水平相鄰排列（left | mid | right）
//------------------------------------------------------------------------------
void CControlCount::SetImage(unsigned int group, int idLeft, int idMid, int idRight)
{
    m_Left.SetImage(group, static_cast<uint16_t>(idLeft));
    m_Mid.SetImage(group, static_cast<uint16_t>(idMid));
    m_Right.SetImage(group, static_cast<uint16_t>(idRight));

    // 依實際寬度排版
    const uint16_t wL = m_Left.GetWidth();
    const uint16_t wM = m_Mid.GetWidth();

    m_Mid.SetX(wL);            // mid 起點緊接 left
    m_Right.SetX(m_Mid.GetX() + wM); // right 起點緊接 mid
}

//------------------------------------------------------------------------------
// 設定範圍（依反編譯順序：cur, max, min），並立即套用
//------------------------------------------------------------------------------
void CControlCount::SetCountRange(int64_t cur, int64_t max, int64_t min)
{
    m_Min = min;   // *((_QWORD*)this + 87) = a4
    m_Cur = cur;   // *((_QWORD*)this + 88) = a2
    m_Max = max;   // *((_QWORD*)this + 89) = a3
    SetupCurCount();
}

//------------------------------------------------------------------------------
// SetupCurCount：依 cur/max 決定顯示/隱藏與右端位置
// 反編譯流程對照：
// 1) 夾制 m_Cur ∈ [m_Min, m_Max]
// 2) 取 left/mid/right 寬度：sumCaps = leftW + rightW
// 3) desired = controlWidth * (m_Cur / m_Max)
//    - desired > 1.0 時顯示三圖，否則全隱藏
//    - 若 controlWidth >= desired：midScale = max(0, (desired - sumCaps) / midW)
//      否則 midScale = max(0, (controlWidth - sumCaps) / midW)
// 4) 以 mid.AbsX + midW * midScale 設定 right 的 AbsX
//------------------------------------------------------------------------------
void CControlCount::SetupCurCount()
{
    // 夾制當前值（反編譯的兩段比較實質上是在做夾制）
    if (m_Max < m_Min) std::swap(m_Min, m_Max);
    if (m_Cur > m_Max) m_Cur = m_Max;
    else if (m_Cur < m_Min) m_Cur = m_Min;

    const uint16_t leftW = m_Left.GetWidth();
    const uint16_t midW = m_Mid.GetWidth();
    const uint16_t rightW = m_Right.GetWidth();
    const int      sumCaps = int(leftW) + int(rightW);

    const uint16_t ctrlW = GetWidth();

    if (m_Max != 0) {
        // 需要填滿的總像素
        const double desired = double(ctrlW) * (double(m_Cur) / double(m_Max));

        if (desired > 1.0) {
            // 顯示三張圖
            m_Left.Show();
            m_Mid.Show();
            m_Right.Show();

            double fillMidPixels;
            if (double(ctrlW) >= desired) {
                // 未超出控制寬
                fillMidPixels = max(0.0, desired - double(sumCaps));
            }
            else {
                // 需求超過控制寬，退而以控制寬為上限
                fillMidPixels = max(0.0, double(ctrlW) - double(sumCaps));
            }

            // 轉為縮放係數（反編譯最後用 midW * scale 來偏移 right 的 AbsX）
            m_MidScale = (midW > 0) ? float(fillMidPixels / double(midW)) : 0.0f;

            // 右端圖的絕對 X = mid.AbsX + midW * scale
            const double advance = double(midW) * double(m_MidScale);
            const int rightAbsX = m_Mid.GetAbsX() + int(advance + 0.5);
            m_Right.SetAbsX(rightAbsX);
        }
        else {
            // 沒有有效填充：全隱藏
            m_Left.Hide();
            m_Mid.Hide();
            m_Right.Hide();
            m_MidScale = 0.0f;
        }
    }
}
