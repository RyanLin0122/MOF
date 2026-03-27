#include "UI/CControlBoxPrivateShop.h"
#include "Info/cltItemKindInfo.h"
#include "Text/DCTTextManager.h"
#include "System/CPrivateMarketSystem.h"  // stSellingItem
#include "global.h"
#include <cstdio>

// 外部函式（定義於 CControlText.cpp）
extern void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int flagUnused);

//----------------------------------------------------------------
// ctor / dtor  (對齊 00416610)
//----------------------------------------------------------------
CControlBoxPrivateShop::CControlBoxPrivateShop()
    : CControlBoxBase()
    , m_NumberBox()
    , m_NameText()
    , m_PriceText()
{
    CControlBoxBase::Init();
    CreateChildren();
    m_bNotOpen = 0;
}

CControlBoxPrivateShop::~CControlBoxPrivateShop()
{
    // 成員自動解構，順序與反編譯一致：
    // m_PriceText(+1504) -> m_NameText(+1072) -> CControlNumberBox 內部(+640,+432,+312)
    // -> m_Background(+120) -> CControlBase
}

//----------------------------------------------------------------
// CreateChildren  (對齊 00416780)
//----------------------------------------------------------------
void CControlBoxPrivateShop::CreateChildren()
{
    // 先建立 BoxBase 背景圖(+120)
    CControlBoxBase::CreateChildren();

    // m_NameText(+1072)：Create(parent=this)，定位 (42, 2)
    m_NameText.Create(this);
    m_NameText.SetPos(42, 2);

    // m_PriceText(+1504)：Create(parent=this)，定位 (42, 16)
    m_PriceText.Create(this);
    m_PriceText.SetPos(42, 16);

    // m_NumberBox(+312)：Create(parent=m_Background)
    // 反編譯：(*(vtbl+12))((char*)this+312, (char*)this+120)
    m_NumberBox.Create(GetBackground());
}

//----------------------------------------------------------------
// SetNotOpenBox  (對齊 004167E0)
//----------------------------------------------------------------
void CControlBoxPrivateShop::SetNotOpenBox(unsigned int giid, unsigned short block)
{
    m_bNotOpen = 1;
    // 設定背景圖為指定的圖片
    GetBackground()->SetImageID(3u, giid, block);
    // 隱藏 NumberBox
    m_NumberBox.Hide();
    // m_State16 = 0
    m_State16 = 0;
}

//----------------------------------------------------------------
// SetBoxData  (對齊 00416820)
//----------------------------------------------------------------
void CControlBoxPrivateShop::SetBoxData(unsigned short itemKind, unsigned short qty, unsigned int price)
{
    if (itemKind && qty && price)
    {
        stItemKindInfo* pInfo = g_clItemKindInfo.GetItemKindInfo(itemKind);
        if (pInfo)
        {
            // 設定背景圖為物品圖示
            GetBackground()->SetImageID(3u, pInfo->m_dwIconResID, pInfo->m_wIconBlockID);

            // 設定物品名稱（使用物品文字代碼）
            m_NameText.SetText(static_cast<int>(pInfo->m_wTextCode));

            // 格式化價格文字
            char moneyBuf[256];
            Numeric2MoneyByComma(price, moneyBuf, 256, 0);

            const char* unitText = g_DCTTextManager.GetText(3028);
            const char* priceLabel = g_DCTTextManager.GetText(4628);

            char buffer[256];
            sprintf(buffer, "%s : %s %s", priceLabel, moneyBuf, unitText);
            m_PriceText.SetText(buffer);

            // 設定數量
            m_NumberBox.SetNumber(qty);
        }
    }
}

//----------------------------------------------------------------
// SetItemData  (對齊 00416920)
//----------------------------------------------------------------
void CControlBoxPrivateShop::SetItemData(stSellingItem* a2)
{
    if (a2)
    {
        m_wItemKind  = a2->itemKind;
        m_wInvenSlot = a2->invenSlot;
        m_wQty       = a2->qty;
        m_dwPrice    = a2->price;
        m_dwParam    = a2->param;

        if (m_wItemKind)
        {
            SetBoxData(m_wItemKind, m_wQty, m_dwPrice);
            // vtbl+48 = ShowChildren (即 CControlBoxBase::ShowChildren)
            ShowChildren();
        }
        else
        {
            // vtbl+52 = HideChildren
            HideChildren();
        }
    }
}

//----------------------------------------------------------------
// SetItemQty  (對齊 004169A0)
//----------------------------------------------------------------
int CControlBoxPrivateShop::SetItemQty(unsigned short qty)
{
    if (qty)
    {
        m_wQty = qty;
        SetBoxData(m_wItemKind, qty, m_dwPrice);
        return 1;
    }
    else
    {
        ClearItemData();
        return 0;
    }
}

//----------------------------------------------------------------
// ClearItemData  (對齊 004169E0)
//----------------------------------------------------------------
void CControlBoxPrivateShop::ClearItemData()
{
    // 反編譯：將偏移 +1936 起 16 bytes 歸零
    m_wItemKind  = 0;
    m_wInvenSlot = 0;
    m_wQty       = 0;
    m_wPad       = 0;
    m_dwPrice    = 0;
    m_dwParam    = 0;
}

//----------------------------------------------------------------
// IsData  (對齊 00416A00)
//----------------------------------------------------------------
BOOL CControlBoxPrivateShop::IsData()
{
    return m_wItemKind != 0;
}
