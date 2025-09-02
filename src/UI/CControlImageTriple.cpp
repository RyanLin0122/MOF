#include "UI/CControlImageTriple.h"

// -----------------------------------------------------------------------------
// Create
// -----------------------------------------------------------------------------
void CControlImageTriple::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);
    // 對齊反編譯：虛呼 +80 -> CreateChildren()
    CreateChildren();
    // 對齊反編譯：*((DWORD*)this + 13) = 1; 視作建立完成（此處不再額外設旗標）
}

// -----------------------------------------------------------------------------
// CreateChildren
// -----------------------------------------------------------------------------
void CControlImageTriple::CreateChildren()
{
    // 三張圖都掛在自己底下
    m_LeftOrTop.Create(this);
    m_Mid.Create(this);
    m_RightOrBottom.Create(this);
}

// -----------------------------------------------------------------------------
// SetImage
// -----------------------------------------------------------------------------
void CControlImageTriple::SetImage(unsigned int group,
    uint16_t idLeftOrTop,
    uint16_t idMid,
    uint16_t idRightOrBottom,
    uint8_t horizontal)
{
    m_LeftOrTop.SetImage(group, idLeftOrTop);
    m_Mid.SetImage(group, idMid);
    m_RightOrBottom.SetImage(group, idRightOrBottom);
    m_bHorizontal = horizontal ? 1 : 0;
}

// -----------------------------------------------------------------------------
// Clear
// -----------------------------------------------------------------------------
void CControlImageTriple::Clear()
{
    // 反編譯：SetImage(this, 0, 0xFFFF, 0xFFFF, 0xFFFF, 0);
    SetImage(0, 0xFFFFu, 0xFFFFu, 0xFFFFu, 0);
}

// -----------------------------------------------------------------------------
// SetSize：依主軸總長度調整中段縮放與三段位置，並更新自身大小
// -----------------------------------------------------------------------------
void CControlImageTriple::SetSize(uint16_t totalPrimary)
{
    // 先取各張圖的原始尺寸（未乘 scale 的基準寬高）
    const uint16_t wL = m_LeftOrTop.GetWidth();
    const uint16_t hL = m_LeftOrTop.GetHeight();
    const uint16_t wM = m_Mid.GetWidth();
    const uint16_t hM = m_Mid.GetHeight();
    const uint16_t wR = m_RightOrBottom.GetWidth();
    const uint16_t hR = m_RightOrBottom.GetHeight();

    if (m_bHorizontal)
    {
        // ---- 水平三段 ----
        // 中段起點 X = 左段寬
        m_Mid.SetX(wL);

        if (totalPrimary == 0)
        {
            // 不縮放，右段緊接左+中
            m_RightOrBottom.SetX(wL + wM);
            // 更新自身寬高：寬=左+中+右，高=取左段高度（與反編譯一致）
            CControlBase::SetSize(static_cast<uint16_t>(wL + wM + wR), hL);
            return;
        }

        if (wM > 0)
        {
            // 需要的中段長度 = 目標總寬 - 左 - 右
            const int remain = max(0, int(totalPrimary) - int(wL) - int(wR));
            const float sx = remain / float(wM);                 // 反編譯：v7 = (total - right - left) / midWidth
            m_Mid.SetScale(sx, 1.0f);                            // 反編譯：寫入中段物件的 X 向縮放欄位

            const uint16_t midScaledW = static_cast<uint16_t>(max(0, int(wM * sx + 0.5f)));
            // 右段位置 = 左段寬 + 中段縮放後寬
            m_RightOrBottom.SetX(wL + midScaledW);

            // 整體尺寸：寬=左 + 中(縮放) + 右；高沿用左段高度
            CControlBase::SetSize(static_cast<uint16_t>(wL + midScaledW + wR), hL);
        }
        else
        {
            // 中段寬為 0：視作只有左右兩段
            m_Mid.SetScale(1.0f, 1.0f);
            m_RightOrBottom.SetX(wL);
            CControlBase::SetSize(static_cast<uint16_t>(wL + wR), hL);
        }
    }
    else
    {
        // ---- 垂直三段 ----
        // 中段起點 Y = 上段高
        m_Mid.SetY(hL);

        if (totalPrimary == 0)
        {
            // 不縮放，下段緊接上+中
            m_RightOrBottom.SetY(hL + hM);
            // 更新自身寬高：高=上+中+下，寬=取上段寬（與反編譯一致）
            CControlBase::SetSize(wL, static_cast<uint16_t>(hL + hM + hR));
            return;
        }

        if (hM > 0)
        {
            // 需要的中段高度 = 目標總高 - 上 - 下
            const int remain = max(0, int(totalPrimary) - int(hL) - int(hR));
            const float sy = remain / float(hM);                 // 反編譯：v7 = (total - bottom - top) / midHeight
            m_Mid.SetScale(1.0f, sy);                            // 反編譯：寫入中段物件的 Y 向縮放欄位

            const uint16_t midScaledH = static_cast<uint16_t>(max(0, int(hM * sy + 0.5f)));
            // 下段位置 = 上段高 + 中段縮放後高
            m_RightOrBottom.SetY(hL + midScaledH);

            // 整體尺寸：高=上 + 中(縮放) + 下；寬沿用上段寬
            CControlBase::SetSize(wL, static_cast<uint16_t>(hL + midScaledH + hR));
        }
        else
        {
            // 中段高為 0：視作只有上下兩段
            m_Mid.SetScale(1.0f, 1.0f);
            m_RightOrBottom.SetY(hL);
            CControlBase::SetSize(wL, static_cast<uint16_t>(hL + hR));
        }
    }
}
