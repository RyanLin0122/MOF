#include "UI/CControlButton.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

// ==========================
// 建構 / 解構
// ==========================
CControlButton::CControlButton()
    : CControlButtonBase()
{
    // 對齊反編譯：預設各 state 的 group=5、id=0、block=0xFFFF
    m_imgNormal = StateImg{};
    m_imgHover = StateImg{};
    m_imgPressed = StateImg{};
    m_imgDisabled = StateImg{};

    // 反編譯：*((DWORD*)this + 48) = 1;（此處不特別用到，僅維持按鈕可互動）
    // 基底子控制
    CControlButtonBase::CreateChildren();
    // 若基底有 Init()，此處呼叫（對齊反編譯）
    // CControlButtonBase::Init();

    // 預設四態文字色（對齊反編譯）
    m_colNormal = 0xFFFFFFFF;
    m_colHover = 0xFFEA5400; // -1376512
    m_colPressed = 0xFFEA5400; // -1376512
    m_colDisabled = 0xFFC98E33; // -3566989

    m_curTextColor = m_colNormal;
}

CControlButton::~CControlButton()
{
    // 正常 C++ 會自動解構 m_Text 與基底 CControlImage / CControlBase
    // 反編譯顯示在此手動呼叫 ~CControlText 與 ~CControlImage，實作上不需要手動做。
}

// ==========================
// 建立
// ==========================
void CControlButton::Create(CControlBase* pParent)
{
    // 反編譯：*((DWORD*)this + 93) = -1, *((DWORD*)this + 92)=1, *((BYTE*)this + 704)=1
    m_curTextColor = 0xFFFFFFFF;
    CControlImage::Create(pParent);
}

void CControlButton::Create(int x, int y, CControlBase* pParent)
{
    m_curTextColor = 0xFFFFFFFF;
    CControlImage::Create(x, y, pParent);
}

// ==========================
// 顯示/啟用/停用
// ==========================
void CControlButton::Show()
{
    CControlImage::Show();
    // 反編譯：若可顯示（vtbl+76 為 true），套用 normal 外觀與文字色
    ApplyStateImage(m_imgNormal);
    ApplyTextColor(m_colNormal);
}

void CControlButton::NoneActive()
{
    CControlImage::NoneActive();
    ApplyStateImage(m_imgDisabled);
    ApplyTextColor(m_colDisabled);
}

void CControlButton::Active()
{
    CControlImage::Active();
    ApplyStateImage(m_imgNormal);
    ApplyTextColor(m_colNormal);
}

// ==========================
// 設圖多載（對齊反編譯）
// ==========================
void CControlButton::SetImage(unsigned int imageIdAll,
    uint16_t blockNormal,
    uint16_t blockHover,
    uint16_t blockPressed,
    uint16_t blockDisabled)
{
    // 反編譯 5 參數版本：把同一 imageId 套給四態，僅 block 不同
    SetImage(imageIdAll, blockNormal,
        imageIdAll, blockHover,
        imageIdAll, blockPressed,
        imageIdAll, blockDisabled);
}

void CControlButton::SetImage(unsigned int imageIdNormal, uint16_t blockNormal)
{
    // 僅設定「常態」圖塊；group 使用預設（反編譯：+164 初值 5）
    m_imgNormal.id = imageIdNormal;
    m_imgNormal.block = blockNormal;

    // 立刻套到背景圖，並把文字置中（反編譯行為）
    CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
    m_Text.SetTextPosToParentCenter();
}

void CControlButton::SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
    unsigned int imageIdHover, uint16_t blockHover,
    unsigned int imageIdPressed, uint16_t blockPressed)
{
    SetImage(imageIdNormal, blockNormal);
    m_imgHover.id = imageIdHover;
    m_imgHover.block = blockHover;
    m_imgPressed.id = imageIdPressed;
    m_imgPressed.block = blockPressed;
}

void CControlButton::SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
    unsigned int imageIdHover, uint16_t blockHover,
    unsigned int imageIdPressed, uint16_t blockPressed,
    unsigned int imageIdDisabled, uint16_t blockDisabled)
{
    SetImage(imageIdNormal, blockNormal,
        imageIdHover, blockHover,
        imageIdPressed, blockPressed);
    m_imgDisabled.id = imageIdDisabled;
    m_imgDisabled.block = blockDisabled;
}

// ==========================
// 文字顏色
// ==========================
void CControlButton::SetButtonTextColor(uint32_t colNormal,
    uint32_t colHover,
    uint32_t colPressed,
    uint32_t colDisabled)
{
    m_colNormal = colNormal;
    m_colHover = colHover;
    m_colPressed = colPressed;
    m_colDisabled = colDisabled;
}

// ==========================
// 事件處理（對齊反編譯 switch-case）
// ==========================
int* CControlButton::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 清除「剛點擊」旗標（對照 this[49]=0）；本還原不需要該旗標即可達成外觀行為
    // 反編譯先檢查可互動（vtbl+76）；這裡假設可互動即走分支
    switch (msg)
    {
    case 0: // Click / MouseUp on button
        // 若有定義 pressed 狀態圖塊則套用
        ApplyStateImage(m_imgPressed);
        // 點擊回呼（反編譯 vtbl+104）+ 播放音效
        PlaySoundClick();
        ApplyTextColor(m_colPressed);
        break;

    case 3: // MouseDown
        ApplyStateImage(m_imgPressed);
        // 反編譯：呼叫 vtbl+108（多半是「按下中」行為），之後把目前底圖還原成 normal
        // 本還原僅維持「按下中」外觀，後續放開或滑出時會回復
        ApplyTextColor(m_colNormal); // 對齊反編譯將當前色設回 normal
        break;

    case 4: // Back to normal (e.g., MouseOut / Cancel press)
        ApplyStateImage(m_imgNormal);
        ApplyTextColor(m_colNormal);
        break;

    case 7: // Hover
        // 標記事件來源為 Hover（反編譯 this[49]=1）
        ApplyStateImage(m_imgHover);
        ApplyTextColor(m_colHover);
        break;

    default:
        break;
    }

    // 照反編譯最後仍會把事件往基底傳（帶原參數）
    return CControlImage::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

// ==========================
// 私有：套用狀態圖 / 文字色
// ==========================
void CControlButton::ApplyStateImage(const StateImg& st)
{
    if (st.block != 0xFFFF)
        CControlImage::SetImageID(st.group, st.id, st.block);
}

void CControlButton::ApplyTextColor(uint32_t c)
{
    m_curTextColor = c;
    m_Text.SetTextColor(c);
}
