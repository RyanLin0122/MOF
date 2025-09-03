#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlText.h"

// 0041AC70..0041AD40
class CControlBoxLessonList : public CControlBoxBase
{
public:
    CControlBoxLessonList();
    virtual ~CControlBoxLessonList();

    // 建立本控制項的子控制項
    virtual void CreateChildren();

    //（可視需求擴充對外介面，例如設定文字內容等）

private:
    // 對應偏移 +312 的 CControlText
    CControlText m_titleText;    // 左上(43,3)，文字色 -7590900

    // 對應偏移 +744 的 CControlText
    CControlText m_subText;      // 左上(43,17)，文字色 -7590900
};
