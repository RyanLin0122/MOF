#include "UI/CControlBoxLessonList.h"

CControlBoxLessonList::CControlBoxLessonList()
    : CControlBoxBase()
    , m_titleText()
    , m_subText()
{
    // 反編譯順序：Init → CreateChildren → 再在 ctor 本體建立/配置兩個 CControlText
    CControlBoxBase::Init();
    CreateChildren();

    // 反編譯碼在 ctor 裡 CreateChildren 之後，明確對兩個 Text 執行 Create/SetPos/SetTextColor
    m_titleText.Create(this);
    m_titleText.SetPos(43, 3);
    m_titleText.SetTextColor(static_cast<DWORD>(-7590900));

    m_subText.Create(this);
    m_subText.SetPos(43, 17);
    m_subText.SetTextColor(static_cast<DWORD>(-7590900));
}

CControlBoxLessonList::~CControlBoxLessonList()
{
    // 反編譯碼的順序是：
    // CControlText::~( +744 )
    // CControlText::~( +312 )
    // CControlImage::~( +120 )  // 這是 CControlBoxBase 內含的背景圖
    // CControlBase::~( this )
    //
    // 在標準 C++ 下，基底類別 CControlBoxBase 的解構會正確處理它的成員，
    // 這裡只需讓成員依宣告順序自動解構即可（不重覆手動去解構 Base 的成員）。
}

void CControlBoxLessonList::CreateChildren()
{
    // GT 的 CreateChildren 無獨立函式本體（只有前置宣告），
    // 呼叫 CControlBoxBase::CreateChildren 建立背景圖即可。
    // 兩個 CControlText 的 Create/SetPos/SetTextColor 在 ctor 中完成。
    CControlBoxBase::CreateChildren();
}
