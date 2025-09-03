#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlImage.h"

// 參照 0041ADC0..0041AEF0
class CControlBoxLessonSchedule : public CControlBoxBase
{
public:
    CControlBoxLessonSchedule();
    virtual ~CControlBoxLessonSchedule();

    // 建立子控制項
    virtual void CreateChildren();
    // 顯示時要同步處理子控制項顯示狀態
    virtual void ShowChildren();

    // 設定課程是否成功：a2 != 0 -> 成功(17)，否則失敗(18)
    void SetLessonSucc(int a2);

private:
    // 對應 offset +312 的圖片
    CControlImage m_statusImg;
};
