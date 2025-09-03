#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

class CControlBoxSpecialty : public CControlBoxBase
{
public:
    CControlBoxSpecialty();                  // 00418220
    virtual ~CControlBoxSpecialty();         // 004182D0

    void CreateChildren();                   // 00418360

private:
    // 依位移對應：
    // +312  圖示
    // +504  文字(左上：標題)
    // +936  文字(右側：值/描述)
    // +1368 邊框/底圖
    CControlImage m_icon;      // +312
    CControlText  m_title;     // +504
    CControlText  m_rightTxt;  // +936
    CControlImage m_frame;     // +1368
};
