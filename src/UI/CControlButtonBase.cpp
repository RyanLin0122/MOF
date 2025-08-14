#include "UI/CControlButtonBase.h"

// ==========================
// 建構 / 解構
// ==========================
CControlButtonBase::CControlButtonBase()
{
    // 預設就顯示與可按，位移開啟（視覺習慣）
    m_bEnablePressShift = true;
    m_pressShiftX = 1;
    m_pressShiftY = 1;
}

CControlButtonBase::~CControlButtonBase()
{
    // m_Text 為成員，會自動解構；其父子鏈結由 CControlBase 維持
}

// ==========================
// 建立
// ==========================
void CControlButtonBase::Create(CControlBase* pParent)
{
    CControlImage::Create(pParent);
    CreateChildren();
}

void CControlButtonBase::Create(int x, int y, CControlBase* pParent)
{
    CControlImage::Create(x, y, pParent);
    CreateChildren();
}

void CControlButtonBase::Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent)
{
    CControlImage::Create(x, y, w, h, pParent);
    CreateChildren();
}

void CControlButtonBase::CreateChildren()
{
    // 文字作為子控制掛載（相對於按鈕左上角）
    // 預設放在(0,0)，通常會由 UI 版面或 SetCenterPos() 來置中
    if (m_Text.GetParent() != this)
    {
        m_Text.Create(this);
        // 常見預設：讓文字居中（你的 CControlText 若有置中 API，可在此設定）
        // 例如：m_Text.SetAlign(CControlText::AlignCenter);
        //      m_Text.SetCenterPos();  // 需要 CControlText 支援
    }
}

// ==========================
// 文字
// ==========================
void CControlButtonBase::SetText(const char* utf8Text)
{
    m_Text.SetText(utf8Text);
}

void CControlButtonBase::SetText(int stringId)
{
    m_Text.SetText(stringId);
}

// ==========================
// 按下位移效果
// ==========================
void CControlButtonBase::ButtonPosDown()
{
    if (m_bPressed) return;
    m_bPressed = true;

    if (m_bEnablePressShift)
    {
        // 位移所有子物件（包含文字等），營造壓下視覺
        SetChildPosMove(m_pressShiftX, m_pressShiftY);
    }

    PlaySoundClick();
}

void CControlButtonBase::ButtonPosUp()
{
    if (!m_bPressed) return;
    m_bPressed = false;

    if (m_bEnablePressShift)
    {
        // 位移還原
        SetChildPosMove(-m_pressShiftX, -m_pressShiftY);
    }
}

// ==========================
// 音效（可被覆寫）
// ==========================
void CControlButtonBase::PlaySoundClick()
{
    // 預設不做事；專案若有音效管理（例如 CSoundMgr::Play("BtnClick")），
    // 可在衍生類別覆寫，或在此直接呼叫。
    // 例：
    // CSoundMgr::Play("BtnClick");
}
