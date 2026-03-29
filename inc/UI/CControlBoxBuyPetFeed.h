#pragma once

#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"

// 寵物飼料購買格
// 反編譯對照：
//   this+0:     CControlBase (120 bytes)
//   this+120:   CControlText m_Name
//   this+552:   CControlImage m_Icon
//   this+744:   CControlText m_Desc
// 總大小：0x498 (1176) bytes
class CControlBoxBuyPetFeed : public CControlBase
{
public:
    CControlBoxBuyPetFeed();
    virtual ~CControlBoxBuyPetFeed();

    CControlText  m_Name;   // +120：名稱文字
    CControlImage m_Icon;   // +552：圖示
    CControlText  m_Desc;   // +744：描述文字
};
