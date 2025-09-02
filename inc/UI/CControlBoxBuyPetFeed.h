#pragma once
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"

/**
 * 買寵糧 UI 盒：左側圖示 + 上方名稱 + 下方描述/數值
 * 反編譯對照：
 * - 成員：+120 CControlText，+552 CControlImage，+744 CControlText
 * - 版面：Name(54,103)，Icon(25,22) 且縮放 2.0，Desc(54,121)
 * - Name 使用 ControlSet 字型 "BuyPetFeedName"
 */
class CControlBoxBuyPetFeed : public CControlBase
{
public:
    CControlBoxBuyPetFeed();
    virtual ~CControlBoxBuyPetFeed();

    // 子件公開存取（如需外部改字、換圖）
    CControlText& NameText() { return m_Name; }
    CControlImage& Icon() { return m_Icon; }
    CControlText& DescText() { return m_Desc; }

private:
    CControlText  m_Name;  // +120
    CControlImage m_Icon;  // +552
    CControlText  m_Desc;  // +744
};
