#include "UI/CControlBoxCashShopStrgItem.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

//-------------------------------------------------------------
// ctor / dtor
//-------------------------------------------------------------
CControlBoxCashShopStrgItem::CControlBoxCashShopStrgItem()
    : CControlBoxBase()
    , m_countBox()
    , m_icon()
{
    // vftable 由編譯器處理
    CreateChildren();
    CControlBoxBase::Init();

    // *((_DWORD*)this + 78) = 0; *((_DWORD*)this + 79) = 0;  -> 對應 m_cashItemID = 0
    m_cashItemID = 0;
}

CControlBoxCashShopStrgItem::~CControlBoxCashShopStrgItem()
{
    // 正常 C++ 析構順序會自動呼叫子物件解構，
    // 與 IDA 顯示的手動解構順序行為等價。
}

//-------------------------------------------------------------
// 建立子控制項（對應 00417B20）
//-------------------------------------------------------------
void CControlBoxCashShopStrgItem::CreateChildren()
{
    // 先建立 BoxBase（背景圖等）
    CControlBoxBase::CreateChildren();

    // 背景圖 X 往右 1（對應 SetX((char*)this+120, 1)）
    if (auto* bg = GetBackground())
        bg->SetX(1);

    // 數字框掛在背景圖底下（對應 *((this+80)->Create)(this+320, this+120)）
    m_countBox.Create(GetBackground());

    // 自身尺寸設為背景圖尺寸（對應 CControlBase::SetSize(this, *((_DWORD*)this + 38)) 的等價處理）
    if (auto* bg2 = GetBackground())
        SetSize(bg2->GetWidth(), bg2->GetHeight());

    // 物品 Icon 掛在本控制項底下
    m_icon.Create(this);
    m_icon.SetImage(0x20000015u, 2u); // 對應 SetImage(..., 0x20000015, 2)

    // *((_DWORD *)this + 284) = 0; 
    // 原始碼直接寫入子物件內部位移（推測是不影響行為的旗標/暫存值）。
    // 這裡不做未定義的內部欄位存取；如需相同行為，可在 CControlImage 內提供對應 setter。
}

//-------------------------------------------------------------
// 隱藏（對應 00417B90）
//-------------------------------------------------------------
void CControlBoxCashShopStrgItem::HideChildren()
{
    CControlBoxBase::HideChildren();
    m_icon.Hide();
}

//-------------------------------------------------------------
// 設定 / 取得 現金道具 ID（對應 00417BB0 / 00417BF0）
//-------------------------------------------------------------
void CControlBoxCashShopStrgItem::SetCashItemID(uint64_t id, bool showIcon)
{
    m_cashItemID = id;
    if (showIcon) m_icon.Show();
    else          m_icon.Hide();
}

uint64_t CControlBoxCashShopStrgItem::GetCashItemID() const
{
    return m_cashItemID;
}
