#include "UI/CControlBoxBuyPetFeed.h"

CControlBoxBuyPetFeed::CControlBoxBuyPetFeed()
    : CControlBase()
    , m_Name()
    , m_Icon()
    , m_Desc()
{
    // 反編譯：*((DWORD*)this + 13) = 1
    m_bIsVisible = true;

    // 將子控制掛到本控制底下（等價於反編譯中的 vtbl+12 呼叫）
    m_Name.Create(this);
    // Name 位置與字型
    m_Name.SetPos(54, 103);
    // 反編譯：*((DWORD*)this + 66) = 1（位於 m_Name 區段內的旗標）
    // 行為上以設定控制字型集來對齊
    m_Name.SetControlSetFont("BuyPetFeedName");

    m_Icon.Create(this);
    m_Icon.SetPos(25, 22);
    // 反編譯：*((DWORD*)this + 147) = 0x40000000, *((DWORD*)this + 148) = 0x40000000
    // => 2.0f 縮放
    m_Icon.SetScale(2.0f, 2.0f);

    m_Desc.Create(this);
    m_Desc.SetPos(54, 121);
    // 反編譯：*((DWORD*)this + 222) = 1（位於 m_Desc 區段內的旗標）
    // 具體語意未在片段中呈現；此處保持預設可見/啟用行為即可
}

CControlBoxBuyPetFeed::~CControlBoxBuyPetFeed()
{
    // 依序解構子物件即可（與反編譯析構順序一致）
    // m_Desc, m_Icon, m_Name 將自動釋放，最後再回到 CControlBase
}
