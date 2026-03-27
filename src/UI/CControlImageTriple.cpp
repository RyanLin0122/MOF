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
// 完全對齊反編譯 0041C6B0
// -----------------------------------------------------------------------------
void CControlImageTriple::SetSize(uint16_t totalPrimary)
{
    uint16_t v11; // 左(或上)段的主軸尺寸
    uint16_t v13; // 中段的主軸尺寸
    int16_t  v14; // 右(或下)段的主軸尺寸
    int      v12; // v11 的 int 副本

    if (m_bHorizontal)
    {
        v11 = m_LeftOrTop.GetWidth();
        v13 = m_Mid.GetWidth();
        v14 = m_RightOrBottom.GetWidth();
        v12 = v11;
        m_Mid.SetX(v11);
    }
    else
    {
        v11 = m_LeftOrTop.GetHeight();
        v13 = m_Mid.GetHeight();
        v14 = m_RightOrBottom.GetHeight();
        v12 = v11;
        m_Mid.SetY(v11);
    }

    uint16_t v6 = totalPrimary;
    if (!totalPrimary)
    {
        if (m_bHorizontal)
            m_RightOrBottom.SetX(v12 + v13);
        else
            m_RightOrBottom.SetY(v12 + v13);
        v6 = totalPrimary;
    }

    if (v13)
    {
        double v7 = (double)(uint16_t)(v6 - v14 - v11) / (double)v13;
        if (m_bHorizontal)
        {
            m_Mid.SetScale(static_cast<float>(v7), m_Mid.GetScaleY());
            int v9 = static_cast<int>((double)m_Mid.GetWidth() * m_Mid.GetScaleX());
            m_RightOrBottom.SetX(v12 + static_cast<uint16_t>(v9));
            m_usWidth  = static_cast<uint16_t>(v11 + v14 + v9);
            m_usHeight = m_LeftOrTop.GetHeight();
        }
        else
        {
            m_Mid.SetScale(m_Mid.GetScaleX(), static_cast<float>(v7));
            int v8 = static_cast<int>((double)m_Mid.GetHeight() * m_Mid.GetScaleY());
            m_RightOrBottom.SetY(v12 + static_cast<uint16_t>(v8));
            m_usWidth  = m_LeftOrTop.GetWidth();
            m_usHeight = static_cast<uint16_t>(v11 + v14 + v8);
        }
    }
}
