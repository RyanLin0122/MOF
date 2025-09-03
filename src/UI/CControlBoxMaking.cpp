#include "UI/CControlBoxMaking.h"
#include "global.h"

CControlBoxMaking::CControlBoxMaking()
    : CControlBoxBase()
    , m_icon()
    , m_text()
{

    // 依反編譯流程：先建子元件、再初始化共用 BoxBase 狀態
    CreateChildren();
    CControlBoxBase::Init();
}

CControlBoxMaking::~CControlBoxMaking()
{
    // 與反編譯碼一致的解構順序
    m_text.~CControlText();   // +504
    m_icon.~CControlImage();  // +312
    GetBackground()->~CControlImage(); // +120 (CControlBoxBase 擁有)
    CControlBase::~CControlBase();     // base
}

void CControlBoxMaking::CreateChildren()
{
    // 先讓 BoxBase 建立其預設的背景圖(+120)等
    CControlBoxBase::CreateChildren();

    // 反編譯碼：(*(vtbl+8))(m_icon, 1, 2, 0)
    // 依上下文最合理詮釋為定位到 (1,2)
    // 若日後確認 vtbl+8 的真正語意不同，再替換為正確 API。
    m_icon.Create(this);      // 等價於把 m_icon 註冊為子控制
    m_icon.SetPos(1, 2);

    // 建立文字並掛在本控制底下
    m_text.Create(this);
    m_text.SetPos(43, 6);

    // 設定字色：反編譯碼寫入 *((_DWORD*)this + 163) = -7590900
    // 這個值依先前類別慣例是 ARGB 或 RGB 顏色常數。
    // 使用文字控制的顏色 setter（若你的 CControlText 沒有這個函式，可改為你現有的 API）。
    m_text.SetTextColor(-7590900);
}

void CControlBoxMaking::ShowChildren()
{
    // 反編譯碼是呼叫 CControlBase::ShowChildren(this)（不是 BoxBase 版本）
    CControlBase::ShowChildren();

    // *((_DWORD *)this + 16) = 0; 具體語意未見，但多數 Box* 在 Show 時會重置某旗標
    // 若你的 CControlBase 有對應欄位與 setter，建議改用 setter。
    // 這裡不直接動底層欄位，避免與你現有結構衝突；如需完全一致，可在 CControlBase 暴露一個 ResetShowFlag()。
    // Example（若有）：this->ResetShowFlag();
}
