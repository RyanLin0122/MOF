#include "UI/CControlBoxSAReceiveItem.h"
#include "global.h"
#include "Info/cltItemKindInfo.h"

//--------------------------------------------------
// ctor (004199A0)
//--------------------------------------------------
CControlBoxSAReceiveItem::CControlBoxSAReceiveItem()
    : CControlBoxBase()
    , m_countBox()
{
    CreateChildren();
    Init();
}

//--------------------------------------------------
// dtor (00419A20)
//--------------------------------------------------
CControlBoxSAReceiveItem::~CControlBoxSAReceiveItem()
{
    // 成員自動解構
}

//--------------------------------------------------
// CreateChildren (00419AB0)
//--------------------------------------------------
void CControlBoxSAReceiveItem::CreateChildren()
{
    CControlBoxBase::CreateChildren();
    m_countBox.Create(this);
}

//--------------------------------------------------
// Init (00419AD0)
//--------------------------------------------------
void CControlBoxSAReceiveItem::Init()
{
    CControlBoxBase::Init();
    m_itemIndex = 0;
    m_itemKind  = 0;
}

//--------------------------------------------------
// SetBoxData (00419AF0)
//--------------------------------------------------
void CControlBoxSAReceiveItem::SetBoxData(unsigned int itemIdx,
                                           unsigned short itemKind,
                                           unsigned short quantity)
{
    if (itemKind && quantity)
    {
        m_itemIndex = itemIdx;
        m_itemKind  = itemKind;

        stItemKindInfo* pInfo = g_clItemKindInfo.GetItemKindInfo(itemKind);
        if (pInfo)
        {
            GetBackground()->SetImageID(3u, pInfo->m_dwIconResID, pInfo->m_wIconBlockID);
            m_countBox.SetNumber(quantity);
        }
    }
}

//--------------------------------------------------
// GetItemIndex (00419B50)
//--------------------------------------------------
unsigned int CControlBoxSAReceiveItem::GetItemIndex() const
{
    return m_itemIndex;
}
