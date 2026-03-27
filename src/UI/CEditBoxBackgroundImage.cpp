#include "UI/CEditBoxBackgroundImage.h"
#include "UI/CControlImage.h"
#include "UI/CControlBase.h"

// ========================================
// 建構 / 解構
// ========================================
CEditBoxBackgroundImage::CEditBoxBackgroundImage()
{
    CreateChildren();
}

CEditBoxBackgroundImage::~CEditBoxBackgroundImage()
{
}

// ========================================
// CreateChildren
// ========================================
void CEditBoxBackgroundImage::CreateChildren()
{
    // 對齊反編譯：每個 image Create(this), SetImageID(536870931, i+67)
    for (int i = 0; i < 9; ++i)
    {
        m_Images[i].Create(this);
        m_Images[i].SetImageID(536870931u, static_cast<unsigned short>(i + 67));
    }

    // 對齊反編譯：根據左上角尺寸設定子控制位置
    unsigned short v5 = m_Images[0].GetWidth();
    unsigned short v6 = m_Images[0].GetHeight();

    // m_Images[1] (中上) → SetX(v5)
    m_Images[1].SetX(v5);
    // m_Images[3] (左中) → SetY(v6)
    m_Images[3].SetY(v6);
    // m_Images[4] (中中) → SetPos(v5, v6)
    m_Images[4].SetPos(v5, v6);
    // m_Images[5] (右中) → SetY(v6)
    m_Images[5].SetY(v6);
    // m_Images[7] (中下) → SetX(v5)
    m_Images[7].SetX(v5);
}

// ========================================
// Create (5 params)
// ========================================
void CEditBoxBackgroundImage::Create(int a2, int a3, int a4, CControlBase* a5, int a6)
{
    // 對齊反編譯：
    // vtbl+88 → SetPos(a2, a3)
    // vtbl+84 → SetSize(a4, a6)  (SetImageID in decompiled but actually SetSize context)
    // vtbl+12 → Create(a5)
    SetPos(a2, a3);
    SetSize(static_cast<unsigned short>(a4), static_cast<unsigned short>(a6));
    CControlBase::Create(a5);
}

// ========================================
// Create (parent only)
// ========================================
void CEditBoxBackgroundImage::Create(CControlBase* a2)
{
    CControlBase::Create(a2);
}

// ========================================
// SetPos
// ========================================
void CEditBoxBackgroundImage::SetPos(int a2, int a3)
{
    CControlBase::SetPos(a2, a3);
}

// ========================================
// SetSize
// ========================================
void CEditBoxBackgroundImage::SetSize(unsigned short a2, unsigned short a3)
{
    // 對齊反編譯：完整的 SetSize 邏輯
    unsigned short v3 = a2;
    if (!a2)
        return;

    unsigned short v5 = a3;
    if (a3)
        v3 = a2 + 10;

    // 取得角塊原始尺寸
    unsigned short cornerW = m_Images[0].GetWidth();
    unsigned short cornerH = m_Images[0].GetHeight();

    // 中段目標寬度
    unsigned short v7 = v3 - 2 * cornerW;

    // 取得中中塊原始尺寸
    unsigned short origMidW = m_Images[4].GetWidth();
    unsigned short origMidH = m_Images[4].GetHeight();

    if (!v5)
        v5 = origMidH;

    if (!origMidW || !origMidH)
        return;

    // 計算縮放比
    float scaleX = static_cast<float>(v7) / static_cast<float>(origMidW);
    float scaleY = static_cast<float>(v5) / static_cast<float>(origMidH);

    // 計算縮放後中段尺寸
    unsigned short scaledMidW = static_cast<unsigned short>(
        static_cast<float>(m_Images[4].GetWidth()) * scaleX);
    unsigned short scaledMidH = static_cast<unsigned short>(
        static_cast<float>(m_Images[4].GetHeight()) * scaleY);

    // 設定整體大小
    m_usWidth = scaledMidW + 2 * cornerW;
    m_usHeight = scaledMidH + 2 * cornerH;

    // m_Images[1] (中上) → 縮放 X
    unsigned short img1W = m_Images[1].GetWidth();
    if (img1W)
    {
        m_Images[1].SetScale(static_cast<float>(scaledMidW) / static_cast<float>(img1W), 1.0f);

        // m_Images[2] (右上) → SetX
        m_Images[2].SetX(cornerW + scaledMidW);

        // m_Images[3] (左中) → 縮放 Y
        unsigned short img3H = m_Images[3].GetHeight();
        if (img3H)
        {
            m_Images[3].SetScale(1.0f, static_cast<float>(scaledMidH) / static_cast<float>(img3H));

            // m_Images[5] (右中) → SetX, 縮放 Y
            int rightX = m_Images[2].GetX();
            m_Images[5].SetX(rightX);

            unsigned short img5H = m_Images[5].GetHeight();
            if (img5H)
            {
                m_Images[5].SetScale(1.0f, static_cast<float>(scaledMidH) / static_cast<float>(img5H));

                // m_Images[6] (左下) → SetY
                m_Images[6].SetY(scaledMidH + cornerH);

                // m_Images[8] (右下) → SetPos
                unsigned short scaledImg0H = static_cast<unsigned short>(
                    static_cast<float>(m_Images[0].GetHeight()) * m_Images[3].GetScaleY());
                m_Images[8].SetPos(rightX, scaledMidH + scaledImg0H);

                // m_Images[7] (中下) → 縮放 X, SetY
                unsigned short img7W = m_Images[7].GetWidth();
                if (img7W)
                {
                    m_Images[7].SetScale(static_cast<float>(scaledMidW) / static_cast<float>(img7W), 1.0f);
                    m_Images[7].SetY(
                        scaledMidH + static_cast<unsigned short>(
                            static_cast<float>(m_Images[0].GetHeight()) * m_Images[1].GetScaleY()));

                    // 最終更新整體 size
                    CControlBase::SetSize(m_usWidth, m_usHeight);
                }
            }
        }
    }
}
