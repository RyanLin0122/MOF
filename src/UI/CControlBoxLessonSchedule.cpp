#include "UI/CControlBoxLessonSchedule.h"

//----------------------------------------
// ctor / dtor
//----------------------------------------
CControlBoxLessonSchedule::CControlBoxLessonSchedule()
    : CControlBoxBase()
    , m_statusImg()
{
    // vftable 由編譯器設定
    // 反編譯順序：Base() -> m_statusImg() -> 設 vftable -> BoxBase::Init() -> CreateChildren()
    CControlBoxBase::Init();
    CreateChildren();
}

CControlBoxLessonSchedule::~CControlBoxLessonSchedule()
{
    // 讓成員與基底自動解構，行為等同反編譯碼
}

//----------------------------------------
// children
//----------------------------------------
void CControlBoxLessonSchedule::CreateChildren()
{
    // 先建立 BoxBase 自己的背景圖（+120）
    CControlBoxBase::CreateChildren();

    // 建立 status 圖（+312）
    m_statusImg.Create(this);

    // 反編譯：CControlImage::SetShadeMode(img, 0);
    m_statusImg.SetShadeMode(0);
}

void CControlBoxLessonSchedule::ShowChildren()
{
    CControlBoxBase::ShowChildren();

    // 反編譯在 ShowChildren 裡對 +312 呼叫 vtbl+44（在多數控件我們對應為 Hide）
    m_statusImg.Hide();
}

//----------------------------------------
// behaviors
//----------------------------------------
void CControlBoxLessonSchedule::SetLessonSucc(int a2)
{
    // 反編譯：若 a2 != 0 -> 使用 index 17，否則 18
    const unsigned int kResID = 0x20000053u;
    const unsigned short index = (a2 ? 17 : 18);

    m_statusImg.SetImage(kResID, index);

    // vtbl+40：顯示
    m_statusImg.Show();

    // 反編譯：CControlBase::SetCenterPos(v2); -> 將本圖置中於父容器
    m_statusImg.SetCenterPos();
}
