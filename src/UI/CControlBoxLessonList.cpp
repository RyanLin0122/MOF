#include "UI/CControlBoxLessonList.h"

// 顏色常數：反編譯碼直接寫入 -7590900，推測為 ARGB 的某種深色
static const int kLessonTextColor = -7590900;

CControlBoxLessonList::CControlBoxLessonList()
    : CControlBoxBase()
    , m_titleText()
    , m_subText()
{
    // vftable
    // *(_DWORD *)this = &CControlBoxLessonList::`vftable'; 由 C++ 自動完成

    // 先做 BoxBase 的一般初始化（對應 CControlBoxBase::Init）
    CControlBoxBase::Init();

    // 建立/配置子控制項（等價於反編譯碼裡的 CreateChildren + 個別 Create/SetPos）
    CreateChildren();
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
    // 先讓 BoxBase 建好它自己的背景（等價反編譯：CControlBoxBase::CreateChildren）
    CControlBoxBase::CreateChildren();

    // 建立第一個文字物件（偏移 +312）
    // 反編譯碼中是呼叫該 CControlText 的 vtbl[12] 並帶入 parent=this
    m_titleText.Create(this);
    m_titleText.SetPos(43, 3);
    m_titleText.SetTextColor(kLessonTextColor);

    // 建立第二個文字物件（偏移 +744）
    m_subText.Create(this);
    m_subText.SetPos(43, 17);
    m_subText.SetTextColor(kLessonTextColor);

    // 反編譯碼在 ctor 末端並無額外動作（不需設字型就忠實維持原狀）。
}
