#include "UI/CControlBoxBuyPetFeed.h"

// 對齊反編譯 00448B60
CControlBoxBuyPetFeed::CControlBoxBuyPetFeed()
    : CControlBase()
    , m_Name()
    , m_Icon()
    , m_Desc()
{
    // *((_DWORD *)this + 13) = 1 → m_bPassKeyInputToParent
    m_bPassKeyInputToParent = true;

    // m_Name: Create(this) + SetPos + m_isCentered + SetControlSetFont
    m_Name.Create(this);
    m_Name.SetPos(54, 103);
    m_Name.m_isCentered = 1;  // *((_DWORD *)this + 66) = 1
    m_Name.SetControlSetFont("BuyPetFeedName");

    // m_Icon: Create(this) + SetPos + SetScale(2.0f, 2.0f)
    m_Icon.Create(this);
    m_Icon.SetPos(25, 22);
    m_Icon.SetScale(2.0f, 2.0f);  // 0x40000000 = 2.0f

    // m_Desc: Create(this) + SetPos + m_isCentered
    m_Desc.Create(this);
    m_Desc.SetPos(54, 121);
    m_Desc.m_isCentered = 1;  // *((_DWORD *)this + 222) = 1
}

// 對齊反編譯 00448C80
CControlBoxBuyPetFeed::~CControlBoxBuyPetFeed()
{
    // 子物件自動逆序解構
}
