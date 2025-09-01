#include "UI/CEditBoxBackgroundImage.h"

// ======================================================================
// 建構/解構
// ======================================================================

CEditBoxBackgroundImage::CEditBoxBackgroundImage()
{
    // C++ 會自動呼叫 CControlBase 的建構函式
    // m_Images 陣列的成員也會自動呼叫 CControlImage 的建構函式
    CreateChildren();
}

CEditBoxBackgroundImage::~CEditBoxBackgroundImage()
{
    // C++ 會自動呼叫成員和基底類別的解構函式，無需手動處理
}

// ======================================================================
// 建立流程
// ======================================================================

void CEditBoxBackgroundImage::CreateChildren()
{
    // GI_GROUP 536870931 (0x20000013) 是圖片資源的群組ID
    const unsigned int GI_GROUP = 536870931;

    // 建立並設定九個圖片子控制項
    for (int i = 0; i < 9; ++i)
    {
        m_Images[i].Create(this);
        // BLOCK_ID 從 67 開始，對應九宮格的各個部分
        m_Images[i].SetImageID(GI_GROUP, i + 67);
    }

    // 根據左上角圖片的尺寸，初步設定其他圖片的位置
    unsigned short cornerWidth = m_Images[0].GetWidth();
    unsigned short cornerHeight = m_Images[0].GetHeight();

    // 設定第一行
    m_Images[1].SetX(cornerWidth); // 中上

    // 設定第二行
    m_Images[3].SetY(cornerHeight); // 左中
    m_Images[4].SetPos(cornerWidth, cornerHeight); // 中中

    // 設定第三行
    m_Images[7].SetX(cornerWidth); // 中下

    // 右邊和底下的圖片位置會在 SetSize 中動態計算
}

void CEditBoxBackgroundImage::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);
}

void CEditBoxBackgroundImage::Create(int x, int y, int width, int height, CControlBase* pParent)
{
    // 注意：IDA中的參數順序可能與直觀不同，此處遵循反編譯碼邏輯
    // CEditBoxBackgroundImage::Create(this, x, y, width, pParent, height)
    SetPos(x, y);
    SetSize(static_cast<unsigned short>(width), static_cast<unsigned short>(height));
    CControlBase::Create(pParent);
}

// ======================================================================
// 位置與尺寸
// ======================================================================

void CEditBoxBackgroundImage::SetPos(int x, int y)
{
    CControlBase::SetPos(x, y);
}

void CEditBoxBackgroundImage::SetSize(unsigned short width, unsigned short height)
{
    // 忽略無效的尺寸設定
    if (width == 0 || height == 0)
    {
        return;
    }

    // 根據反編譯碼，寬度似乎有+10的偏移
    width += 10;

    // 獲取邊角和中間圖片的原始尺寸
    unsigned short cornerW = m_Images[0].GetWidth();
    unsigned short cornerH = m_Images[0].GetHeight();
    unsigned short middleW = m_Images[4].GetWidth();
    unsigned short middleH = m_Images[4].GetHeight();

    // 如果原始尺寸無效，則無法進行縮放
    if (middleW == 0 || middleH == 0) return;

    // 計算中間部分需要被拉伸的目標寬度和高度
    unsigned short targetMiddleW = width - 2 * cornerW;
    unsigned short targetMiddleH = height - 2 * cornerH;

    // 計算水平和垂直方向的縮放比例
    float scaleX = static_cast<float>(targetMiddleW) / middleW;
    float scaleY = static_cast<float>(targetMiddleH) / middleH;

    // 設定可拉伸圖片的縮放比例
    m_Images[1].SetScale(scaleX, 1.0f); // 中上
    m_Images[3].SetScale(1.0f, scaleY); // 左中
    m_Images[4].SetScale(scaleX, scaleY); // 中中
    m_Images[5].SetScale(1.0f, scaleY); // 右中
    m_Images[7].SetScale(scaleX, 1.0f); // 中下
    
    // 根據縮放後的尺寸，更新右邊和底部圖片的位置
    unsigned short scaledMiddleW = static_cast<unsigned short>(middleW * scaleX);
    unsigned short scaledMiddleH = static_cast<unsigned short>(middleH * scaleY);
    
    // 右側列
    m_Images[2].SetX(cornerW + scaledMiddleW); // 右上
    m_Images[5].SetX(cornerW + scaledMiddleW); // 右中
    m_Images[8].SetX(cornerW + scaledMiddleW); // 右下

    // 底部行
    m_Images[6].SetY(cornerH + scaledMiddleH); // 左下
    m_Images[7].SetY(cornerH + scaledMiddleH); // 中下
    m_Images[8].SetY(cornerH + scaledMiddleH); // 右下

    // 更新整個控制項的總尺寸
    CControlBase::SetSize(width, height);
}

