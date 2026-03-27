#include "UI/CControlBoxMySAItem.h"
#include "Info/cltItemKindInfo.h"
#include "Text/DCTTextManager.h"
#include "global.h"
#include <cstdio>

// 外部函式（定義於 CControlText.cpp）
extern void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int flagUnused);

//----------------------------------------------------------------
// ctor / dtor  (對齊 004196B0)
//----------------------------------------------------------------
CControlBoxMySAItem::CControlBoxMySAItem()
    : CControlBoxBase()
    , m_NumberBox()
    , m_NameText()
    , m_QtyText()
    , m_PriceText()
{
    CreateChildren();
    Init();
}

CControlBoxMySAItem::~CControlBoxMySAItem()
{
    // 成員自動解構，順序與反編譯一致：
    // m_PriceText(+1940) -> m_QtyText(+1508) -> m_NameText(+1076)
    // -> CControlNumberBox 內部(+644,+436,+316)
    // -> m_Background(+120) -> CControlBase
}

//----------------------------------------------------------------
// CreateChildren  (對齊 004197F0)
//----------------------------------------------------------------
void CControlBoxMySAItem::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // *((_DWORD *)this + 16) = 0  →  m_State16 = 0
    m_State16 = 0;

    // m_NumberBox(+316)：Create(parent=m_Background at +120)
    m_NumberBox.Create(GetBackground());

    // m_NameText(+1076)：Create(parent=this)，定位 (42, 2)
    m_NameText.Create(this);
    m_NameText.SetPos(42, 2);

    // m_QtyText(+1508)：Create(parent=this)
    m_QtyText.Create(this);

    // m_PriceText(+1940)：Create(parent=this)，定位 (42, 19)
    m_PriceText.Create(this);
    m_PriceText.SetPos(42, 19);
}

//----------------------------------------------------------------
// Init  (對齊反編譯碼 — 行為等同 CControlBoxBase::Init)
//----------------------------------------------------------------
void CControlBoxMySAItem::Init()
{
    CControlBoxBase::Init();
}

//----------------------------------------------------------------
// SetBoxData  (對齊 00419870)
//
// 參數解析（根據反編譯 _WORD* this 的偏移與呼叫端語境）：
//   a2       = slotIndex（寫入 +312）
//   a3       = unused / reserved
//   a4       = (itemKind in lo16) | (qty in hi16)
//   Value    = price
//   Value_4  = unused / reserved
//----------------------------------------------------------------
void CControlBoxMySAItem::SetBoxData(int16_t a2, int a3, int a4, unsigned int Value, int Value_4)
{
    // 反編譯：if ( (_WORD)a4 && HIWORD(a4) && Value )
    unsigned short itemKind = static_cast<unsigned short>(a4 & 0xFFFF);
    unsigned short qty      = static_cast<unsigned short>((a4 >> 16) & 0xFFFF);

    if (itemKind && qty && Value)
    {
        // this[156] = a2  →  m_nSlotIndex = a2
        m_nSlotIndex = a2;

        stItemKindInfo* pInfo = g_clItemKindInfo.GetItemKindInfo(itemKind);
        if (pInfo)
        {
            // 設定背景圖為物品圖示
            GetBackground()->SetImageID(3u, pInfo->m_dwIconResID, pInfo->m_wIconBlockID);

            // 設定物品名稱
            m_NameText.SetText(static_cast<int>(pInfo->m_wTextCode));

            // 設定數量
            m_NumberBox.SetNumber(qty);

            // 格式化價格文字
            char moneyBuf[256];
            Numeric2MoneyByComma(Value, moneyBuf, 256, 0);

            const char* fmtText = g_DCTTextManager.GetParsedText(3921, 0, nullptr);
            char buffer[256];
            sprintf(buffer, fmtText, moneyBuf);
            m_PriceText.SetText(buffer);
        }
    }
}

//----------------------------------------------------------------
// ShowChildren  (對齊 00419970)
//----------------------------------------------------------------
void CControlBoxMySAItem::ShowChildren()
{
    CControlBoxBase::ShowChildren();
    // *((_DWORD *)this + 16) = 0
    m_State16 = 0;
}
