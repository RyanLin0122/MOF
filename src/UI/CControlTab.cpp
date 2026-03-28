#include "UI/CControlTab.h"
#include "UI/CControlBase.h"
#include "global.h"

CControlTab::CControlTab()
{
    // 反編譯：CControlButtonBase::CControlButtonBase(this)
    // 由 C++ 繼承自動呼叫

    // 對齊反編譯：四個圖組的預設值
    // this[167]=5, this[168]=0, word this[169*2]=0xFFFF ...
    // 已在欄位初始化完成

    // CreateChildren + Init
    CreateChildren();
    Init();

    // 對齊反編譯：this[15] = 1（CControlBase 的某旗標）
    m_bPassKeyInputToParent = true;

    // 對齊反編譯：strcpy((char*)this + 208, "J0007")
    // offset 208 = CControlButtonBase 內的 m_szSoundName（音效名稱）
    strcpy(m_szSoundName, "J0007");
}

CControlTab::~CControlTab()
{
    // 對齊反編譯：
    // CControlText::~CControlText(this + 224) → m_Text 解構
    // CControlImage::~CControlImage(this) → 基底解構
    // C++ 自動處理
}

void CControlTab::CreateChildren()
{
    // 反編譯中由 CControlButtonBase 處理
    CControlButtonBase::CreateChildren();
}

void CControlTab::Init()
{
    // 對齊反編譯：CControlButtonBase::Init(this); m_bFocused = 1
    CControlButtonBase::Init();
    m_bFocused = 1;
}

void CControlTab::SetImage(unsigned int a2, uint16_t a3, uint16_t a4, uint16_t a5, uint16_t a6)
{
    // 對齊反編譯
    m_imgFocusedBlock = a3;
    m_imgFocusedId = a2;
    m_imgUnfocusedId = a2;
    m_imgUnfocusedBlock = a4;

    if (a5 != 0xFFFF)
    {
        m_imgHoverId = a2;
        m_imgHoverBlock = a5;
    }
    if (a6 != 0xFFFF)
    {
        m_imgNoneActiveId = a2;
        m_imgNoneActiveBlock = a6;
    }

    // 對齊反編譯：CControlImage::SetImageID(this, group, id, block)
    CControlImage::SetImageID(m_imgFocusedGroup, m_imgFocusedId, m_imgFocusedBlock);

    if (m_bTextTabType == 1)
        m_Text.SetPos(0, 0);
    else
        m_Text.SetTextPosToParentCenter();
}

void CControlTab::SetTextColor(uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    // 對齊反編譯
    m_colUnfocused = a3;
    m_colHover = a4;
    m_colFocused = a2;
    m_colNoneActive = a5;
    m_Text.m_TextColor = a2; // this[93]
}

void CControlTab::SetTabTextType()
{
    // 對齊反編譯
    m_bTextTabType = 1;
    CControlImage::SetAlpha(0);
    SetTextColor(0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFC99273);
    SetChildMoveByClick(1, 1);
}

uint8_t CControlTab::GetTabType()
{
    return m_bTextTabType;
}

void CControlTab::SetTabAlpha(int a2)
{
    CControlImage::SetShadeMode(0);
    CControlImage::SetAlpha(a2);
}

int* CControlTab::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 對齊反編譯：先檢查 IsActive
    if (!IsActive())
        return nullptr;

    switch (msg)
    {
    case 0: // Click
        PlaySoundClick();
        m_Text.m_TextColor = m_colFocused; // this[93] = this[179]
        dword_AFD34C = 0;
        break;

    case 3: // MouseDown
        if (m_bTextTabType == 1)
            NoneActive();
        dword_AFD34C = 1;
        break;

    case 4: // 恢復常態
        if (m_bFocused)
            m_Text.m_TextColor = m_colFocused;
        else
            m_Text.m_TextColor = m_colUnfocused;

        // 對齊反編譯：如果當前 block 匹配 hover block，切回非焦點圖
        // （hover 只對非焦點 tab 生效，所以恢復時應回到 unfocused 圖）
        if (static_cast<uint16_t>(m_usBlockID) == m_imgHoverBlock)
        {
            CControlImage::SetImageID(m_imgUnfocusedGroup, m_imgUnfocusedId, m_imgUnfocusedBlock);
        }
        break;

    case 7: // Hover
        if (!m_bFocused)
        {
            m_Text.m_TextColor = m_colHover;
            if (m_imgHoverBlock != 0xFFFF)
                CControlImage::SetImageID(m_imgHoverGroup, m_imgHoverId, m_imgHoverBlock);
        }
        break;

    default:
        break;
    }

    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

void CControlTab::SetFocus(int a2)
{
    // 對齊反編譯
    if (m_bFocused == a2)
        return;
    if (a2 && !IsActive())
        return;

    m_bFocused = a2;

    if (!IsActive())
        return;

    if (a2)
    {
        CControlImage::SetImageID(m_imgFocusedGroup, m_imgFocusedId, m_imgFocusedBlock);
        Active();
        m_Text.m_TextColor = m_colFocused;
    }
    else
    {
        CControlImage::SetImageID(m_imgUnfocusedGroup, m_imgUnfocusedId, m_imgUnfocusedBlock);
        NoneActive();
        m_Text.m_TextColor = m_colUnfocused;
    }
}

void CControlTab::Active()
{
    CControlBase::Active();
    if (m_bFocused)
        m_Text.m_TextColor = m_colFocused;
    else
        m_Text.m_TextColor = m_colUnfocused;

    // 對齊反編譯：若目前 block 是 noneActive block，改回依焦點狀態圖
    if (static_cast<uint16_t>(m_usBlockID) == m_imgNoneActiveBlock)
    {
        if (m_bFocused)
            CControlImage::SetImageID(m_imgFocusedGroup, m_imgFocusedId, m_imgFocusedBlock);
        else
            CControlImage::SetImageID(m_imgUnfocusedGroup, m_imgUnfocusedId, m_imgUnfocusedBlock);
    }
}

void CControlTab::NoneActive()
{
    CControlBase::NoneActive();
    m_Text.m_TextColor = m_colNoneActive;
    if (m_imgNoneActiveBlock != 0xFFFF)
        CControlImage::SetImageID(m_imgNoneActiveGroup, m_imgNoneActiveId, m_imgNoneActiveBlock);
}

void CControlTab::SetText(char* a2)
{
    // 對齊反編譯
    CControlButtonBase::SetText(a2);
    if (m_bTextTabType == 1)
    {
        // 對齊反編譯：ground truth 以 WORD 讀取 GetTextLength 回傳值
        DWORD v5[2], v6[2];
        m_Text.GetTextLength(v5);
        short v3 = *reinterpret_cast<short*>(reinterpret_cast<char*>(v5) + 4);  // height = WORD at +4 bytes
        m_Text.GetTextLength(v6);
        short v4 = *reinterpret_cast<short*>(v6);  // width = first WORD
        m_usHeight = static_cast<uint16_t>(v3);
        m_usWidth = static_cast<uint16_t>(v4);
    }
}

void CControlTab::SetText(int a2)
{
    // 對齊反編譯
    CControlButtonBase::SetText(a2);
    if (m_bTextTabType == 1)
    {
        // 對齊反編譯：ground truth 以 WORD 讀取 GetTextLength 回傳值
        DWORD v5[2], v6[2];
        m_Text.GetTextLength(v5);
        short v3 = *reinterpret_cast<short*>(reinterpret_cast<char*>(v5) + 4);  // height = WORD at +4 bytes
        m_Text.GetTextLength(v6);
        short v4 = *reinterpret_cast<short*>(v6);  // width = first WORD
        m_usHeight = static_cast<uint16_t>(v3);
        m_usWidth = static_cast<uint16_t>(v4);
    }
}
