#include "UI/CControlMyItem.h"

//------------------------------------------------------------------------------
// 建構/解構
//------------------------------------------------------------------------------
CControlMyItem::CControlMyItem()
    : CControlBoxBase()
    , m_Icon()
    , m_TextA()
    , m_TextB()
    , m_TextC()
{
    // vtable 指派由編譯器自動處理
    CreateChildren();   // 對應反編譯：CControlMyItem::CreateChildren(this)
    Init();             // 對應反編譯：CControlBoxBase::Init(this)
}

CControlMyItem::~CControlMyItem()
{
    // 反編譯中依序解構了 TextC/TextB/TextA/圖片(+312) 以及 (+120) 與 CControlBase。
    // 正常 C++ 下，成員會自動正確解構；基底（含其成員，如 +120 的圖片）由其解構函式負責。
}

//------------------------------------------------------------------------------
// 建立子控制
//------------------------------------------------------------------------------
void CControlMyItem::CreateChildren()
{
    // 先讓基底建立它自己的子控制（例如背景等）
    CControlBoxBase::CreateChildren();

    // 圖片（+312）：反編譯僅對其 SetPos(1,2)
    // 註：反編譯沒有看到對 m_Icon 的 Create(this) 呼叫；為保行為一致，只設定座標。
    m_Icon.SetPos(1, 2);

    // TextA（+504）
    m_TextA.Create(this);
    m_TextA.SetMultiLineSize(70, 12);
    m_TextA.SetPos(40, 2);

    // TextB（+936）
    m_TextB.Create(this);
    m_TextB.SetMultiLineSize(75, 12);
    m_TextB.SetPos(150, 2);

    // TextC（+1368）
    m_TextC.Create(this);
    m_TextC.SetMultiLineSize(110, 12);
    m_TextC.SetPos(204, 2);
}

//------------------------------------------------------------------------------
// 顯示子控制
//------------------------------------------------------------------------------
void CControlMyItem::ShowChildren()
{
    CControlBase::ShowChildren(); // 對應反編譯：CControlBase::ShowChildren(this)
    m_State16 = 0;                // 對應：*((_DWORD*)this + 16) = 0
}
