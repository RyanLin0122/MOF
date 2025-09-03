#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "UI/CControlNumberImage.h"
#include "UI/CControlAlphaBox.h"

class CControlBoxSkill : public CControlBoxBase
{
public:
    CControlBoxSkill();
    virtual ~CControlBoxSkill();

    void CreateChildren(); // 004180F0

private:
    // 對應位移：
    // +312  技能圖示
    // +504  文字（名稱）
    // +936  小圖（底標/圖示）
    // +1128 數字圖片（等級/數值）
    // +5100 半透明覆蓋
    // +5308 邊框/前景圖
    CControlImage       m_icon;         // +312
    CControlText        m_title;        // +504
    CControlImage       m_subIcon;      // +936
    CControlNumberImage m_numberImg;    // +1128
    CControlAlphaBox    m_overlay;      // +5100
    CControlImage       m_frame;        // +5308
};
