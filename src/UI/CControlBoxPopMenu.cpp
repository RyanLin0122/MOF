#include "UI/CControlBoxPopMenu.h"
#include "Sound/GameSound.h"
#include "global.h"

//----------------------------------------------------------------
// 輔助函式
//----------------------------------------------------------------
int CControlBoxPopMenu::FloatToByte(float v)
{
    if (v >= 1.0f) return 255;
    if (v > 0.0f)  return static_cast<int>(v * 255.0f + 0.5f);
    return 0;
}

DWORD CControlBoxPopMenu::PackARGB(float r, float g, float b, float a)
{
    int iR = FloatToByte(r);
    int iG = FloatToByte(g);
    int iB = FloatToByte(b);
    int iA = FloatToByte(a);
    // 反編譯：v11 | ((v10 | ((v9 | ((_DWORD)v12 << 8)) << 8)) << 8)
    // 其中 v9=R, v10=G, v11=B, v12=A  →  ARGB = (A<<24)|(R<<16)|(G<<8)|B
    return static_cast<DWORD>(iB | ((iG | ((iR | (iA << 8)) << 8)) << 8));
}

//----------------------------------------------------------------
// ctor / dtor  (對齊 0041A180)
//----------------------------------------------------------------
CControlBoxPopMenu::CControlBoxPopMenu()
    : CControlBase()
    , m_fColor2R(0.0f)
    , m_fColor2G(0.0f)
    , m_fColor2B(0.0f)
    , m_fColor2A(1.0f)
    , m_Text()
{
    // 反編譯初始化 +120~+132：Color1 (normal) 預設值
    m_fColor1R = 0.0f;
    m_fColor1G = 0.0f;
    m_fColor1B = 0.0f;
    m_fColor1A = 1.0f;

    m_nBtnDown = 0;

    // 反編譯初始化 +136~+148：Color2 (active) 預設值
    // 原始整數位元組轉 float：
    //   1052819650 → 0.371f, 1025548449 → 0.035f,
    //   1024495761 → 0.031f, 1065353216 → 1.0f
    union { int i; float f; } cvt;
    cvt.i = 1052819650; m_fColor2R = cvt.f;
    cvt.i = 1025548449; m_fColor2G = cvt.f;
    cvt.i = 1024495761; m_fColor2B = cvt.f;
    cvt.i = 1065353216; m_fColor2A = cvt.f;

    CreateChildren();
    Init();
}

CControlBoxPopMenu::~CControlBoxPopMenu()
{
    // m_Text -> CControlBase 自動解構
}

//----------------------------------------------------------------
// CreateChildren  (對齊 0041A240)
//----------------------------------------------------------------
void CControlBoxPopMenu::CreateChildren()
{
    // m_Text.Create(parent=this) — 對應 vtbl+12 呼叫
    m_Text.Create(this);

    // 反編譯：*((_DWORD *)this + 75) = 1
    // 對應 CControlText 內部某旗標，最接近語意的 public setter
    m_Text.SetWantSpaceFirstByte(1);
}

//----------------------------------------------------------------
// Init  (對齊 0041A260)
//----------------------------------------------------------------
void CControlBoxPopMenu::Init()
{
    // *((_DWORD *)this + 13) = 1  →  m_bIsVisible = true
    m_bIsVisible = true;
}

//----------------------------------------------------------------
// Create  (對齊 0041A270)
//----------------------------------------------------------------
void CControlBoxPopMenu::Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent)
{
    CControlBase::Create(x, y, w, h, pParent);
    m_Text.SetTextPosToParentCenter();
}

//----------------------------------------------------------------
// SetTextColorMouseInput  (對齊 0041A2B0)
//
// color2(a2-a5) → +136（Active/MouseOver）
// color1(a6-a9) → +120（Normal）
//----------------------------------------------------------------
int CControlBoxPopMenu::SetTextColorMouseInput(
    float color2R, float color2G, float color2B, float color2A,
    float color1R, float color1G, float color1B, float color1A)
{
    // 反編譯：+120~+132 = color1 (normal)
    m_fColor1R = color1R;
    m_fColor1G = color1G;
    m_fColor1B = color1B;
    m_fColor1A = color1A;

    // 反編譯：+136~+148 = color2 (mouseOver)
    m_fColor2R = color2R;
    m_fColor2G = color2G;
    m_fColor2B = color2B;
    m_fColor2A = color2A;

    // 以 color2 計算 ARGB 並寫入 m_Text 的文字色（對應 GT offset 304 = m_Text + 148）
    DWORD color = PackARGB(color2R, color2G, color2B, color2A);
    m_Text.SetTextColor(color);
    return static_cast<int>(color);
}

//----------------------------------------------------------------
// NoneActive  (對齊 0041A430)
//----------------------------------------------------------------
void CControlBoxPopMenu::NoneActive()
{
    CControlBase::NoneActive();
    // 反編譯：*((_DWORD *)this + 76) = -3502787 → offset 304 = m_Text.m_TextColor
    m_Text.SetTextColor(static_cast<DWORD>(-3502787));
}

//----------------------------------------------------------------
// Active  (對齊 0041A450)
//----------------------------------------------------------------
void CControlBoxPopMenu::Active()
{
    CControlBase::Active();
    // 以 Color2 計算 ARGB → 寫入 m_Text 文字色
    m_Text.SetTextColor(PackARGB(m_fColor2R, m_fColor2G, m_fColor2B, m_fColor2A));
}

//----------------------------------------------------------------
// Show  (對齊 0041A5A0)
//----------------------------------------------------------------
void CControlBoxPopMenu::Show()
{
    CControlBase::Show();
    // 反編譯：vtbl+68 → NoneActive()
    NoneActive();
}

//----------------------------------------------------------------
// ControlKeyInputProcess  (對齊 0041A5C0)
//----------------------------------------------------------------
int* CControlBoxPopMenu::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    if (!m_pParent)
        return nullptr;

    switch (msg)
    {
    case 0: // ButtonDown
        ButtonPosDown();
        g_GameSoundManager.PlaySoundA((char*)"J0003", 0, 0);
        dword_AFD34C = 0;
        break;

    case 3: // ButtonUp
        ButtonPosUp();
        dword_AFD34C = 1;
        break;

    case 4: // MouseLeave（恢復 Color2 / mouseOver 色）
        ButtonPosUp();
        m_Text.SetTextColor(PackARGB(m_fColor2R, m_fColor2G, m_fColor2B, m_fColor2A));
        break;

    case 7: // MouseEnter（切換為 Color1 / normal 色）
        m_Text.SetTextColor(PackARGB(m_fColor1R, m_fColor1G, m_fColor1B, m_fColor1A));
        break;

    default:
        break;
    }

    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

//----------------------------------------------------------------
// SetText  (對齊 0041A8C0)
//----------------------------------------------------------------
void CControlBoxPopMenu::SetText(int stringId)
{
    m_Text.SetText(stringId);
}

//----------------------------------------------------------------
// ButtonPosDown  (對齊 0041A8E0)
//----------------------------------------------------------------
void CControlBoxPopMenu::ButtonPosDown()
{
    if (!m_nBtnDown)
    {
        m_Text.MoveWindow(1, 1);
        m_nBtnDown = 1;
    }
}

//----------------------------------------------------------------
// ButtonPosUp  (對齊 0041A910)
//----------------------------------------------------------------
void CControlBoxPopMenu::ButtonPosUp()
{
    if (m_nBtnDown)
    {
        m_Text.MoveWindow(-1, -1);
        m_nBtnDown = 0;
    }
}
