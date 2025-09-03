#include "UI/CControlBoxShopItem.h"
#include "global.h"

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
    // vftable 由編譯器處理
    CreateChildren();
    CControlBoxBase::Init();

    // 反編譯裡還清了幾個欄位（642、140、188）；這裡沒有語意就略過
    m_noTrade = false;
    m_fxState = 0;
}

CControlBoxShopItem::~CControlBoxShopItem()
{
    // RAII：子物件自動解構
}

void CControlBoxShopItem::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // NumberBox 掛在背景（反編譯把 parent 指向 this+120 的 BoxBase 背景圖）
    if (auto* bg = GetBackground()) {
        m_countBox.Create(bg);
    }
    else {
        // 沒有背景 accessor 的話就先掛在自己身上
        m_countBox.Create(this);
    }

    // no-trade 圖示（+312）
    m_noTradeIcon.Create(this);
    m_noTradeIcon.SetImage(0x20000013u, 0x18u); // 資源&影格
    m_noTradeIcon.Hide();                       // 初始不顯示

    // 名稱/描述文字
    m_nameText.Create(44, 2, this);
    m_descText.Create(44, 16, this);
    m_descText.SetTextColor(-65536); // *((_DWORD*)this + 557) = -65536;

    // 封印/選取的覆蓋色塊（+2512）
    m_sealOverlay.Create(this);
    // 反編譯：*((WORD*)this + 1272/1273) = 32 -> 寬高 32
    // 你的 AlphaBox 若沒有 SetWH，請用 SetSize 或等效 API
    m_sealOverlay.SetSize(32, 32);
    m_sealOverlay.Hide();

    // 左/右邊框（+504 / +696），位置為 (-3,-3)，同資源不同 frame
    m_frameL.Create(this);
    m_frameL.SetPos(-3, -3);
    m_frameL.SetImage(570425419u, 6);
    m_frameL.Hide();

    m_frameR.Create(this);
    m_frameR.SetPos(-3, -3);
    m_frameR.SetImage(570425419u, 7);
    m_frameR.Hide();
}

void CControlBoxShopItem::SetBoxItemCountSetNumber(char flagShow, int count)
{
    if (flagShow == 1 || count <= 0) {
        m_countBox.Hide();
    }
    else {
        m_countBox.SetNumber(count);
        m_countBox.Show();
    }
}

void CControlBoxShopItem::NoTrade(bool yes)
{
    m_noTrade = yes;
    if (yes)
        m_noTradeIcon.Show();
    else
        m_noTradeIcon.Hide();
}

bool CControlBoxShopItem::IsNoTrade() const
{
    return m_noTrade;
}

void CControlBoxShopItem::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    // 反編譯：if (fxState > 0 && fxState <= 2) 就把 背景(+120)、frameL(+504)、frameR(+696) 進入 prepare
    if (m_fxState > 0 && m_fxState <= 2) {
        if (auto* bg = GetBackground()) bg->PrepareDrawing();
        m_frameL.PrepareDrawing();
        m_frameR.PrepareDrawing();
    }
}

void CControlBoxShopItem::Draw()
{
    CControlBoxBase::Draw();

    if (m_fxState > 0 && m_fxState <= 2) {
        if (auto* bg = GetBackground()) bg->Draw();
        m_frameL.Draw();
        m_frameR.Draw();
    }
}

int CControlBoxShopItem::SetSealStatus(int status)
{
    // 00419F20：三種狀態
    // 1: 黃色(1,1,0,0.392...)，顯示 overlay + 左框，隱藏右框
    // 2: 綠色(0.3529,0.8706,0,0.392...)，顯示 overlay + 右框，隱藏左框
    // 0: 全隱
    switch (status) {
    case 1:
        m_sealOverlay.SetColor(1.0f, 1.0f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_frameL.Show();
        m_frameR.Hide();
        break;
    case 2:
        m_sealOverlay.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_sealOverlay.Show();
        m_frameL.Hide();
        m_frameR.Show();
        break;
    default:
        m_sealOverlay.Hide();
        m_frameL.Hide();
        m_frameR.Hide();
        break;
    }
    // 反編譯會寫到 *((_DWORD*)this + 680) = status；用 m_fxState 保存
    m_fxState = status;
    return status;
}

void CControlBoxShopItem::ShowChildren()
{
    CControlBoxBase::ShowChildren();
    // 反編譯在 ShowChildren 之後，確保這些初始為隱藏
    m_sealOverlay.Hide();
    m_noTradeIcon.Hide();
    m_frameL.Hide();
    m_frameR.Hide();
    m_countBox.Hide();
    m_fxState = 0;
}

void CControlBoxShopItem::HideChildren()
{
    CControlBoxBase::HideChildren();
    // 同 ShowChildren：全部收起
    m_sealOverlay.Hide();
    m_noTradeIcon.Hide();
    m_frameL.Hide();
    m_frameR.Hide();
    m_countBox.Hide();
    m_fxState = 0;
}
