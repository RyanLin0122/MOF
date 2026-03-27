#include <cstdio>
#include <cstring>
#include <windows.h>
#include "UI/CControlBoxCashShop.h"
#include "Logic/cltCashShopItem.h"
#include "global.h"

// 定義在 CControlText.cpp 中的工具函式
extern void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int flagUnused);

// ====================================================================
// 建構/解構（對齊 00416A70 / 00416B40）
// ====================================================================
CControlBoxCashShop::CControlBoxCashShop()
    : CControlBoxBase()
    , m_BtnBuy()
    , m_BtnGift()
    , m_BtnWish()
    , m_BtnPackageDetail()
    , m_TextName()
    , m_TextPrice()
    , m_AlphaBox()
    , m_ImgQuantityBg()
    , m_NumberImage()
    , m_ImgFrame()
{
    // 反編譯：*((_DWORD *)this + 79) = 1; *((_DWORD *)this + 80) = 0; *((_DWORD *)this + 78) = 0;
    m_canEquip   = 1;
    m_isWishList = 0;
    m_pItemList  = nullptr;

    Init();
    CreateChildren();
}

CControlBoxCashShop::~CControlBoxCashShop()
{
    // C++ 解構順序（反向宣告順序）等價於反編譯手動解構順序：
    // ~m_ImgFrame → ~m_NumberImage → ~m_ImgQuantityBg → ~m_AlphaBox
    // → ~m_TextPrice → ~m_TextName → ~m_BtnPackageDetail → ~m_BtnWish
    // → ~m_BtnGift → ~m_BtnBuy → ~CControlBoxBase（內含 ~m_Background + ~CControlBase）
}

// ====================================================================
// Init（對齊 00416C30）
// ====================================================================
void CControlBoxCashShop::Init()
{
    CControlBoxBase::Init();
    // 反編譯：*((_DWORD *)this + 13) = 0 → 初始隱藏
    SetVisible(false);
}

// ====================================================================
// CreateChildren（對齊 00416C50）
// ====================================================================
void CControlBoxCashShop::CreateChildren()
{
    // 基底：建立背景圖 + SetShadeMode(0)
    CControlBoxBase::CreateChildren();

    // 背景圖位置與縮放
    // 反編譯：CControlBase::SetPos((char*)this + 120, 31, 14)
    GetBackground()->SetPos(31, 14);
    // 反編譯：*((_DWORD *)this + 39) = 0x40000000 → scale 2.0f
    GetBackground()->SetScale(2.0f, 2.0f);

    // ---- 名稱文字 (+3220) ----
    // 反編譯：vtable+8 → Create(63, 88, this)
    m_TextName.Create(63, 88, this);
    // 反編譯：*((_DWORD *)this + 841) = 1 → alignment = 1
    m_TextName.SetAlignment(1);

    // ---- 價格文字 (+3652) ----
    // 反編譯：vtable+8 → Create(63, 102, this)
    m_TextPrice.Create(63, 102, this);
    // 反編譯：*((_DWORD *)this + 949) = 1 → alignment = 1
    m_TextPrice.SetAlignment(1);
    // 反編譯：*((_DWORD *)this + 950) = -65536 → textColor = 0xFFFF0000 (紅色)
    m_TextPrice.SetTextColor(0xFFFF0000);

    // ---- 購買按鈕 (+324) ----
    // 反編譯：vtable+12 → Create(this)
    m_BtnBuy.Create(this);
    m_BtnBuy.SetPos(11, 116);
    // 反編譯：vtable+88 → SetText(3998)
    m_BtnBuy.SetText(3998);
    // 反編譯：vtable+112 → SetImage(536871043, 12, 13, 14, 15)
    m_BtnBuy.SetImage(536871043u, 12, 13, 14, 15);

    // ---- 贈送按鈕 (+1048) ----
    // 反編譯：vtable+12 → Create(this)
    m_BtnGift.Create(this);
    m_BtnGift.SetPos(46, 116);
    // 反編譯：vtable+88 → SetText(3999)
    m_BtnGift.SetText(3999);
    // 反編譯：SetImage 從購買按鈕複製 → 使用相同圖片參數
    m_BtnGift.SetImage(536871043u, 12, 13, 14, 15);

    // ---- 願望按鈕 (+1772) ----
    // 反編譯：vtable+12 → Create(this)
    m_BtnWish.Create(this);
    m_BtnWish.SetPos(81, 116);
    // 反編譯：vtable+88 → SetText(4000)
    m_BtnWish.SetText(4000);
    // 反編譯：SetImage 從購買按鈕複製 → 使用相同圖片參數
    m_BtnWish.SetImage(536871043u, 12, 13, 14, 15);

    // ---- 組合包詳情按鈕 (+2496) ----
    // 反編譯：vtable+12 → Create(this)
    m_BtnPackageDetail.Create(this);
    m_BtnPackageDetail.SetPos(97, 63);
    // 反編譯：vtable+92 → SetText(const char*)
    m_BtnPackageDetail.SetText("");
    // 反編譯：vtable+112 → SetImage(536871042, 17, 18, 19, 20)
    m_BtnPackageDetail.SetImage(536871042u, 17, 18, 19, 20);

    // ---- 數量圖 (+4292) ----
    // 反編譯：vtable+12 → Create 掛在背景圖底下
    m_ImgQuantityBg.Create(GetBackground());
    m_ImgQuantityBg.SetPos(m_ImgQuantityBg.GetX() + 50, m_ImgQuantityBg.GetY());
    // 反編譯：SetImage(0x180001AB, 0)
    m_ImgQuantityBg.SetImage(0x180001ABu, 0);

    // ---- 數量數字 (+4484) ----
    // 反編譯：Create(0, 0, 5, background)
    m_NumberImage.Create(0, 0, 5, GetBackground());

    // ---- 外框圖 (+8456) ----
    // 反編譯：vtable+12 → Create(this)
    m_ImgFrame.Create(this);
    m_ImgFrame.SetPos(-6, -6);
    // 反編譯：SetImage(0x2200002E, 0)
    m_ImgFrame.SetImage(0x2200002Eu, 0);

    // ---- AlphaBox (+4084) ----
    // 反編譯：Create(15, 106, 1, 1, 1.0f, 0.0f, 0.0f, 1.0f, this)
    // 1x1 紅色方塊，用作價格標記指示器
    m_AlphaBox.Create(15, 106, 1, 1, 1.0f, 0.0f, 0.0f, 1.0f, this);
}

// ====================================================================
// ShowChildren（對齊 00417660）
// ====================================================================
void CControlBoxCashShop::ShowChildren()
{
    CControlBoxBase::ShowChildren();
    // 反編譯：vtable+52 on background → Show()
    GetBackground()->Show();
}

// ====================================================================
// SetData（對齊 00417680）
// ====================================================================
void CControlBoxCashShop::SetData(unsigned int a2, uint16_t a3, char* a4,
                                   int Value, int Value_4,
                                   uint16_t a7, uint16_t a8, uint8_t a9)
{
    char Buffer[256];
    char v24[256];
    char v25[256];
    char v26[256];

    // 反編譯：CControlImage::SetImageID((char*)this + 120, 3, a2, a3)
    GetBackground()->SetImageID(3u, a2, a3);

    // 反編譯：CControlText::SetText((char*)this + 3220, a4)
    m_TextName.SetText(a4);

    // 反編譯：Numeric2MoneyByComma(Value, v25, 256, 0)
    Numeric2MoneyByComma(static_cast<unsigned int>(Value), v25, 256, 0);

    // 反編譯：v11 = DCTTextManager::GetText(&g_DCTTextManager, 4010)
    const char* fmt = g_DCTTextManager.GetText(4010);
    std::sprintf(Buffer, fmt, v25);

    // 反編譯：vtable+44 on AlphaBox → Hide()
    m_AlphaBox.Hide();

    // 反編譯：if (a9 == 2 && Value_4 > 0) → 折扣價格處理
    if (a9 == 2 && Value_4 > 0)
    {
        Numeric2MoneyByComma(static_cast<unsigned int>(Value_4), v24, 256, 0);
        wsprintfA(v26, "%s -> %s", v24, v25);

        const char* fmt2 = g_DCTTextManager.GetText(4010);
        std::sprintf(Buffer, fmt2, v26);

        // 反編譯：vtable+40 on AlphaBox → Show()
        m_AlphaBox.Show();

        // 反編譯：計算 AlphaBox 寬度與位置
        int origW = 0, origH = 0;
        g_MoFFont.GetTextLength(&origW, &origH, "CashShopSTitle", v24);
        // 反編譯：*((_WORD *)this + 2058) = width - 4; *((_WORD *)this + 2059) = 2;
        m_AlphaBox.SetSize(static_cast<uint16_t>(origW - 4), 2);

        int fullW = 0, fullH = 0;
        g_MoFFont.GetTextLength(&fullW, &fullH, "CashShopSTitle", Buffer);
        int alphaX = 8 - fullW / 2;

        // 反編譯：v14 = GetPos(m_TextPrice); SetPos(AlphaBox, v14[0]+alphaX, 106)
        m_AlphaBox.SetPos(m_TextPrice.GetX() + alphaX, 106);
    }

    // 反編譯：組合包按鈕顯示/隱藏
    BOOL isPackage = g_clCashShopItem.IsPackageItem(a9);
    if (isPackage)
        m_BtnPackageDetail.Show();
    else
        m_BtnPackageDetail.Hide();

    // 反編譯：CControlText::SetText((char*)this + 3652, Buffer)
    m_TextPrice.SetText(Buffer);

    // 反編譯：CControlImage::SetBlockID((char*)this + 8456, a8)
    m_ImgFrame.SetBlockID(a8);

    // 反編譯：*((_DWORD *)this + 2128) = 0 → 某內部旗標重置

    // 反編譯：if (a7 > 1) → 數量大於 1 時顯示數量
    if (a7 > 1u)
    {
        uint16_t qtyBgWidth = m_ImgQuantityBg.GetWidth();
        int qtyBgX = m_ImgQuantityBg.GetX();
        int qtyBgY = m_ImgQuantityBg.GetY();

        m_NumberImage.SetPos(qtyBgX + qtyBgWidth, qtyBgY);
        m_NumberImage.SetNumber(static_cast<long long>(a7));

        // 反編譯：vtable+48 on background → Show background
        GetBackground()->Show();
    }
}

// ====================================================================
// 商品清單（對齊 004178E0 / 004178F0）
// ====================================================================
void CControlBoxCashShop::SetCashShopItemList(stCashShopItemList* a2)
{
    m_pItemList = a2;
}

stCashShopItemList* CControlBoxCashShop::GetCashShopItemList() const
{
    return m_pItemList;
}

// ====================================================================
// 可裝備狀態（對齊 00417900 / 00417930）
// ====================================================================
void CControlBoxCashShop::SetCanEquip(int a2)
{
    m_canEquip = a2;
    // 反編譯：*((_DWORD *)this + 16) = a2 ? 1 : 0
    if (a2)
        m_State16 = 1;
    else
        m_State16 = 0;
}

int CControlBoxCashShop::CanEquip() const
{
    return m_canEquip;
}

// ====================================================================
// 願望清單（對齊 00417940 / 00417980）
// ====================================================================
void CControlBoxCashShop::SetWishList(int a2)
{
    m_isWishList = a2;
    // 反編譯：if (a2) SetText(4002) else SetText(4000)
    if (a2)
        m_BtnWish.SetText(4002);
    else
        m_BtnWish.SetText(4000);
}

int CControlBoxCashShop::IsWishList() const
{
    return m_isWishList;
}

// ====================================================================
// 按鈕取得（對齊 00417990 ~ 004179C0）
// ====================================================================
CControlButton* CControlBoxCashShop::GetButtonBuy()
{
    return &m_BtnBuy;
}

CControlButton* CControlBoxCashShop::GetButtonGift()
{
    return &m_BtnGift;
}

CControlButton* CControlBoxCashShop::GetButtonWish()
{
    return &m_BtnWish;
}

CControlButton* CControlBoxCashShop::GetButtonPackageDetail()
{
    return &m_BtnPackageDetail;
}
