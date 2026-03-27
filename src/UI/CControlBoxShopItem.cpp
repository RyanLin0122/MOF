#include "UI/CControlBoxShopItem.h"
#include "global.h"

//--------------------------------------------------
// ctor (00419B60)
//--------------------------------------------------
CControlBoxShopItem::CControlBoxShopItem()
    : CControlBoxBase()
    , m_noTradeIcon()
    , m_frameL()
    , m_frameR()
    , m_countBox()
    , m_nameText()
    , m_descText()
    , m_sealOverlay()
{
    CreateChildren();
    CControlBoxBase::Init();
    m_noTrade = false;
    m_fxState = 0;
}

//--------------------------------------------------
// dtor (00419C60)
//--------------------------------------------------
CControlBoxShopItem::~CControlBoxShopItem()
{
    // 成員自動解構
}

//--------------------------------------------------
// CreateChildren (00419D50)
//--------------------------------------------------
void CControlBoxShopItem::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // 數量盒 (+888) 掛在背景圖上
    m_countBox.Create(GetBackground());

    // 不可交易圖示 (+312)
    m_noTradeIcon.Create(this);
    m_noTradeIcon.SetImage(0x20000013u, 0x18u);

    // 名稱文字 (+1648)
    m_nameText.Create(44, 2, this);

    // 描述文字 (+2080)
    m_descText.Create(44, 16, this);
    m_descText.SetTextColor(0xFFFF0000);  // -65536

    // 封印覆蓋色塊 (+2512) 32×32
    m_sealOverlay.Create(this);
    m_sealOverlay.SetSize(32, 32);

    // 左右邊框圖 (+504, +696)
    m_frameL.Create(-3, -3, 570425419u, 6, this);
    m_frameR.Create(-3, -3, 570425419u, 7, this);
}

//--------------------------------------------------
// SetBoxItemCountSetNumber (00419E20)
//--------------------------------------------------
void CControlBoxShopItem::SetBoxItemCountSetNumber(char flagShow, int count)
{
    if (flagShow == 1 || count <= 0)
    {
        m_countBox.Hide();
    }
    else
    {
        m_countBox.SetNumber(count);
        m_countBox.Show();
    }
}

//--------------------------------------------------
// NoTrade (00419E60)
//--------------------------------------------------
void CControlBoxShopItem::NoTrade(bool yes)
{
    m_noTrade = yes;
    if (yes)
        m_noTradeIcon.Show();
    else
        m_noTradeIcon.Hide();
}

//--------------------------------------------------
// IsNoTrade (00419E90)
// 反編譯：return *((_DWORD *)this + 90) != 0;
// 對應 m_noTradeIcon 的可見性旗標
//--------------------------------------------------
bool CControlBoxShopItem::IsNoTrade() const
{
    return m_noTrade;
}

//--------------------------------------------------
// PrepareDrawing (00419EA0)
//--------------------------------------------------
void CControlBoxShopItem::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    if (m_fxState > 0 && m_fxState <= 2)
    {
        GetBackground()->PrepareDrawing();
        m_frameL.PrepareDrawing();
        m_frameR.PrepareDrawing();
    }
}

//--------------------------------------------------
// Draw (00419EE0)
//--------------------------------------------------
void CControlBoxShopItem::Draw()
{
    CControlBoxBase::Draw();

    if (m_fxState > 0 && m_fxState <= 2)
    {
        GetBackground()->Draw();
        m_frameL.Draw();
        m_frameR.Draw();
    }
}

//--------------------------------------------------
// SetSealStatus (00419F20)
//--------------------------------------------------
int CControlBoxShopItem::SetSealStatus(int status)
{
    m_fxState = status;

    if (status == 1)
    {
        // 黃色
        m_sealOverlay.SetColor(1.0f, 1.0f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_frameL.Show();
        m_frameR.Hide();
    }
    else if (status == 2)
    {
        // 綠色
        m_sealOverlay.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_frameL.Hide();
        m_frameR.Show();
    }
    else
    {
        // 全隱藏
        m_sealOverlay.Hide();
        m_frameL.Hide();
        m_frameR.Hide();
    }

    return status;
}

//--------------------------------------------------
// ShowChildren (0041A020)
//--------------------------------------------------
void CControlBoxShopItem::ShowChildren()
{
    CControlBoxBase::ShowChildren();
    m_sealOverlay.Hide();
    m_noTradeIcon.Hide();
    m_frameL.Hide();
    m_frameR.Hide();
    m_countBox.Hide();
    m_fxState = 0;
}

//--------------------------------------------------
// HideChildren (0041A080)
//--------------------------------------------------
void CControlBoxShopItem::HideChildren()
{
    CControlBoxBase::HideChildren();
    m_sealOverlay.Hide();
    m_noTradeIcon.Hide();
    m_frameL.Hide();
    m_frameR.Hide();
    m_countBox.Hide();
    m_fxState = 0;
}
