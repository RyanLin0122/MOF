#include "UI/CControlBoxMaking.h"
#include "global.h"

//----------------------------------------------------------------
// ctor / dtor  (對齊 00417C30)
//----------------------------------------------------------------
CControlBoxMaking::CControlBoxMaking()
    : CControlBoxBase()
    , m_icon()
    , m_text()
{
    // 反編譯流程：先建子元件、再 Init
    CreateChildren();
    CControlBoxBase::Init();
}

CControlBoxMaking::~CControlBoxMaking()
{
    // 成員自動解構，順序：m_text(+504) -> m_icon(+312) -> 基底(m_Background+120, CControlBase)
}

//----------------------------------------------------------------
// CreateChildren  (對齊 00417D00)
//----------------------------------------------------------------
void CControlBoxMaking::CreateChildren()
{
    // 先讓 BoxBase 建立其背景圖(+120)
    CControlBoxBase::CreateChildren();

    // 反編譯碼：(*(vtbl+8))(m_icon, 1, 2, 0)
    // CControlImage::Create(int x, int y, CControlBase* parent) with null parent
    m_icon.Create(1, 2, static_cast<CControlBase*>(nullptr));

    // 建立文字並掛在本控制底下
    m_text.Create(this);
    m_text.SetPos(43, 6);

    // 反編譯碼：*((_DWORD*)this + 163) = -7590900
    // -7590900 = 0xFF8C0B8C (ARGB 字色)
    m_text.SetTextColor(static_cast<DWORD>(-7590900));
}

//----------------------------------------------------------------
// ShowChildren  (對齊 00417D50)
//----------------------------------------------------------------
void CControlBoxMaking::ShowChildren()
{
    // 反編譯碼是呼叫 CControlBase::ShowChildren（非 BoxBase 版本）
    CControlBase::ShowChildren();

    // *((_DWORD *)this + 16) = 0
    m_State16 = 0;
}
