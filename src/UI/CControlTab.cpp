#include "UI/CControlTab.h"
#include "UI/CControlBase.h"

CControlTab::CControlTab()
{
    // 對齊反編譯預設：四個圖組的 group=5、id=0、block=0xFFFF
    m_imgFocused = StateImg{};
    m_imgUnfocused = StateImg{};
    m_imgHover = StateImg{};
    m_imgNoneActive = StateImg{};

    // 基底字控制
    CControlButtonBase::CreateChildren();

    // 對齊：Init()
    Init();
}

CControlTab::~CControlTab() = default;

void CControlTab::Init()
{
    // 反編譯：CControlButtonBase::Init(this); 這裡基底未提供 Init，直接維持預設
    m_bFocused = true;
}

void CControlTab::SetTabTextType()
{
    m_bTextTabType = true;
    CControlImage::SetAlpha(0);
    // 對齊反編譯的預設顏色
    SetTextColor(0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFC99273);
    // 反編譯：SetChildMoveByClick(this,1,1) → 對應基底按壓位移方案
    EnablePressShift(true);
    SetPressShift(1, 1);
}

void CControlTab::SetImage(unsigned int giid,
    uint16_t blockFocused,
    uint16_t blockUnfocused,
    uint16_t blockHover,
    uint16_t blockNoneActive)
{
    m_imgFocused.group = 5;     m_imgFocused.id = giid;     m_imgFocused.block = blockFocused;
    m_imgUnfocused.group = 5;   m_imgUnfocused.id = giid;   m_imgUnfocused.block = blockUnfocused;
    if (blockHover != 0xFFFF) { m_imgHover.group = 5; m_imgHover.id = giid; m_imgHover.block = blockHover; }
    if (blockNoneActive != 0xFFFF) { m_imgNoneActive.group = 5; m_imgNoneActive.id = giid; m_imgNoneActive.block = blockNoneActive; }

    // 初始套用「焦點」圖（對齊反編譯 SetImage() 末端行為）
    CControlImage::SetImageID(m_imgFocused.group, m_imgFocused.id, m_imgFocused.block);

    // 文字位置：TextTab 則依反編譯固定 (0,0)；非 TextTab 置中到父控件
    if (m_bTextTabType)
        m_Text.SetPos(0, 0);
    else
        m_Text.SetTextPosToParentCenter();
}

void CControlTab::SetTextColor(uint32_t colFocused,
    uint32_t colUnfocused,
    uint32_t colHover,
    uint32_t colNoneActive)
{
    m_colFocused = colFocused;
    m_colUnfocused = colUnfocused;
    m_colHover = colHover;
    m_colNoneActive = colNoneActive;
    m_curTextColor = colFocused;     // 對齊：this[93] = a2
    m_Text.SetTextColor(m_curTextColor);
}

void CControlTab::SetTabAlpha(int a)
{
    CControlImage::SetShadeMode(0);
    CControlImage::SetAlpha(a);
}

void CControlTab::SetFocus(bool focus)
{
    if (m_bFocused == focus) return;
    m_bFocused = focus;

    if (m_bFocused)
    {
        CControlImage::SetImageID(m_imgFocused.group, m_imgFocused.id, m_imgFocused.block);
        Active();
        ApplyTextColor(m_colFocused);
    }
    else
    {
        CControlImage::SetImageID(m_imgUnfocused.group, m_imgUnfocused.id, m_imgUnfocused.block);
        NoneActive(); // 對齊反編譯 vtbl+108（離開焦點時呼叫）
        ApplyTextColor(m_colUnfocused);
    }
}

void CControlTab::Active()
{
    CControlBase::Active();
    ApplyTextColor(m_bFocused ? m_colFocused : m_colUnfocused);

    // 若目前 block 與「未啟用」相同，改回依焦點狀態圖
    if (m_imgNoneActive.block != 0xFFFF && m_imgNoneActive.block == CControlBase::GetHeight() /*不易取現值；此處無條件套用*/)
    {
        // 由於難以無侵入地取到目前 block，改成無條件套用
        CControlImage::SetImageID(m_bFocused ? m_imgFocused.group : m_imgUnfocused.group,
            m_bFocused ? m_imgFocused.id : m_imgUnfocused.id,
            m_bFocused ? m_imgFocused.block : m_imgUnfocused.block);
    }
}

void CControlTab::NoneActive()
{
    CControlBase::NoneActive();
    ApplyTextColor(m_colNoneActive);
    if (m_imgNoneActive.block != 0xFFFF)
        CControlImage::SetImageID(m_imgNoneActive.group, m_imgNoneActive.id, m_imgNoneActive.block);
}

int* CControlTab::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    switch (msg)
    {
    case 0: // Click / MouseUp
        PlaySoundClick();
        ApplyTextColor(m_colFocused); // 對齊：this[372] = this[716] 行為近似（以焦點色回填）
        break;
    case 3: // MouseDown
        if (m_bTextTabType)
            NoneActive();
        break;
    case 4: // 回復常態（含滑出）
        ApplyTextColor(m_bFocused ? m_colFocused : m_colUnfocused);
        // 若目前是未啟用圖塊，換成焦點/非焦點圖
        if (m_bFocused)
            ApplyStateImage(m_imgFocused);
        else
            ApplyStateImage(m_imgUnfocused);
        break;
    case 7: // Hover
        if (!m_bFocused)
            ApplyStateImage(m_imgHover);
        break;
    default:
        break;
    }

    return CControlImage::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

void CControlTab::SetText(const char* text)
{
    CControlButtonBase::SetText(text);
    if (m_bTextTabType)
    {
        int w = 0, h = 0;
        m_Text.GetTextPixelSize(&w, &h);
        SetSize(static_cast<uint16_t>(w), static_cast<uint16_t>(h));
    }
}

void CControlTab::SetText(int stringId)
{
    CControlButtonBase::SetText(stringId);
    if (m_bTextTabType)
    {
        int w = 0, h = 0;
        m_Text.GetTextPixelSize(&w, &h);
        SetSize(static_cast<uint16_t>(w), static_cast<uint16_t>(h));
    }
}

void CControlTab::ApplyStateImage(const StateImg& st)
{
    if (st.block != 0xFFFF)
        CControlImage::SetImageID(st.group, st.id, st.block);
}

void CControlTab::ApplyTextColor(uint32_t c)
{
    m_curTextColor = c;
    m_Text.SetTextColor(c);
}