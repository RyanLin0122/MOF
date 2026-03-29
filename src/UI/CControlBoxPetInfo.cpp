#include "UI/CControlBoxPetInfo.h"

// 對齊反編譯 00449B00
CControlBoxPetInfo::CControlBoxPetInfo()
    : CControlBase()
    , m_BgImage()
    , m_Name()
    , m_IconImage()
    , m_SelectMark()
{
    // *((_DWORD *)this + 13) = 1 → m_bPassKeyInputToParent
    m_bPassKeyInputToParent = true;

    // m_BgImage (+120): Create(this) + SetImage(0x22000026, 15) + SetPos(0, 0)
    m_BgImage.Create(this);
    m_BgImage.SetImage(570425382u, 15);  // 0x22000026, block 15 (未選取)
    m_BgImage.SetPos(0, 0);

    // m_Name (+312): Create(this) + SetPos(56, 106) + m_isCentered + SetControlSetFont
    m_Name.Create(this);
    m_Name.SetPos(56, 106);
    m_Name.m_isCentered = 1;  // *((_DWORD *)this + 114) = 1
    m_Name.SetControlSetFont("BuyPetFeedName");

    // m_IconImage (+744): Create(this) + SetX(56)
    m_IconImage.Create(this);
    m_IconImage.SetX(56);
    // *((_DWORD *)this + 214) = 1 → 對應 m_IconImage 內部 CControlBase 偏移 112
    // 為 stToolTipData 尾端或 m_nClassId 欄位，設 1 無明確語意影響

    // m_SelectMark (+936): Create(this) + SetImage(0x22000029, 17) + SetPos(90, 9)
    m_SelectMark.Create(this);
    m_SelectMark.SetImage(570425385u, 17);  // 0x22000029, block 17
    m_SelectMark.SetPos(90, 9);
}

// 對齊反編譯 004449D0
CControlBoxPetInfo::~CControlBoxPetInfo()
{
    // 子物件自動逆序解構
}

// 對齊反編譯 00449C00
void CControlBoxPetInfo::SetSelectImage()
{
    // 背景圖切到 block 16（選取狀態）
    m_BgImage.SetImage(570425382u, 16);  // 0x22000026, block 16
}

// 對齊反編譯 00449C20
void CControlBoxPetInfo::SetNoneSelectImage()
{
    // 背景圖切回 block 15（未選取狀態）
    m_BgImage.SetImage(570425382u, 15);  // 0x22000026, block 15
}
