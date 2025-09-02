#include "UI/CControlBoxBase.h"
#include "UI/stToolTipData.h"   // CControlBase 也有帶入；此處明確引用其 Init()

//------------------------------------------------------------------------------
// 建構 / 解構
//------------------------------------------------------------------------------
CControlBoxBase::CControlBoxBase()
    : CControlBase()
    , m_Background()
{
    // vtable 指派由編譯器完成
}

CControlBoxBase::~CControlBoxBase()
{
    // 成員依序自動解構：m_Background -> CControlBase
}

//------------------------------------------------------------------------------
// Init：(*this+13)=1, (*this+16)=0, ToolTip.Init()
//------------------------------------------------------------------------------
void CControlBoxBase::Init()
{
    // 反編譯：*((DWORD*)this + 13) = 1
    m_bIsVisible = true;

    // 反編譯：*((DWORD*)this + 16) = 0
    m_State16 = 0;

    // 反編譯：stToolTipData::Init((char*)this + 72)
    m_ToolTip.Init();
}

//------------------------------------------------------------------------------
// CreateChildren：背景圖 Create(this) + SetShadeMode(0)
//------------------------------------------------------------------------------
void CControlBoxBase::CreateChildren()
{
    m_Background.Create(this);
    // 反編譯：CControlImage::SetShadeMode(v1, 0)
    m_Background.SetShadeMode(0);
}

//------------------------------------------------------------------------------
// ShowChildren：呼叫基底後把 (*this+16) 設為 1
//------------------------------------------------------------------------------
void CControlBoxBase::ShowChildren()
{
    CControlBase::ShowChildren();
    m_State16 = 1;
}

//------------------------------------------------------------------------------
// HideChildren：復位欄位 → 基底 HideChildren → vtbl+84（OnHidden）
//------------------------------------------------------------------------------
void CControlBoxBase::HideChildren()
{
    // 反編譯對照：
    // *((WORD*)this + 124) = -1;
    // *((WORD*)this + 76)  = 0;
    // *((WORD*)this + 77)  = 0;
    // *((DWORD*)this + 69) = 0;
    m_Word124 = 0xFFFF;
    m_Word76 = 0;
    m_Word77 = 0;
    m_Dword69 = 0;

    CControlBase::HideChildren();

    // 對應 vtbl+84
    OnHidden();
}

//------------------------------------------------------------------------------
// IsInItem：回傳 (*this+42)!=0
//------------------------------------------------------------------------------
BOOL CControlBoxBase::IsInItem()
{
    return m_InItemFlag != 0;
}

//------------------------------------------------------------------------------
// SetAlpha：委派給背景圖
//------------------------------------------------------------------------------
void CControlBoxBase::SetAlpha(int a)
{
    m_Background.SetAlpha(a);
}
