#include "UI/CControlButton.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "global.h"

// 反編譯中 ControlKeyInputProcess 使用的靜態變數（座標去重）
static int s_nLastX = 0;
static int s_nLastY = 0;

// ==========================
// 建構
// 反編譯：CControlButton::CControlButton
//   CControlButtonBase::CControlButtonBase(this);
//   this[164]=5, this[165]=0, WORD[332]=-1  (normal)
//   this[167]=5, this[168]=0, WORD[338]=-1  (hover)
//   this[170]=5, this[171]=0, WORD[344]=-1  (pressed)
//   WORD[350]=-1                             (disabled block)
//   this[173]=5, this[174]=0                 (disabled group/id)
//   this[48] = 1;  // enable childMoveByClick
//   CreateChildren();
//   Init();
//   this[178] = -1376512;  // colHover
//   this[179] = -1376512;  // colPressed
//   this[177] = -1;        // colNormal
//   this[180] = -3566989;  // colDisabled
// ==========================
CControlButton::CControlButton()
    : CControlButtonBase()
{
    m_imgNormal = StateImg{};
    m_imgHover = StateImg{};
    m_imgPressed = StateImg{};
    m_imgDisabled = StateImg{};

    // 反編譯：this[48] = 1 (enable childMoveByClick)
    m_bChildMoveByClick = 1;

    CControlButtonBase::CreateChildren();
    CControlButtonBase::Init();

    m_colHover = 0xFFEB5400;    // -1376512
    m_colPressed = 0xFFEB5400;  // -1376512
    m_colNormal = 0xFFFFFFFF;   // -1
    m_colDisabled = 0xFFC98E33; // -3566989
}

CControlButton::~CControlButton()
{
    // 反編譯：
    //   CControlText::~CControlText((char*)this + 224);
    //   CControlImage::~CControlImage(this);
    // C++ 自動處理
}

// ==========================
// 建立
// 反編譯：
//   this[93] = -1;    // curTextColor
//   this[92] = 1;     // unknown
//   BYTE[704] = 1;    // unknown flag
//   CControlBase::Create(this, a2);
// ==========================
void CControlButton::Create(CControlBase* pParent)
{
    // 反編譯：this[93] = -1 → m_Text.m_TextColor = 白色
    m_Text.SetTextColor(0xFFFFFFFF);
    // 反編譯：this[92] = 1 → m_Text.m_isCentered = 1（置中）
    m_Text.SetAlignment(1);
    CControlBase::Create(pParent);
}

void CControlButton::Create(int x, int y, CControlBase* pParent)
{
    m_Text.SetTextColor(0xFFFFFFFF);
    m_Text.SetAlignment(1);
    CControlBase::Create(x, y, pParent);
}

// ==========================
// 顯示
// 反編譯：
//   CControlBase::Show(this);
//   if (IsActive()) {
//     this[30] = this[164];  // group
//     this[31] = this[165];  // id
//     WORD[64] = this[166];  // block (actually stored at DWORD 166 lower word)
//     this[93] = this[177];  // curTextColor = colNormal
//   }
// ==========================
void CControlButton::Show()
{
    CControlBase::Show();
    if (IsActive())
    {
        CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
        m_Text.SetTextColor(m_colNormal);
    }
}

// ==========================
// NoneActive
// 反編譯：
//   CControlBase::NoneActive(this);
//   if (WORD[350] != 0xFFFF) {
//     this[30] = this[173]; this[31] = this[174]; WORD[64] = this[175];
//   }
//   this[93] = this[180];
// ==========================
void CControlButton::NoneActive()
{
    CControlBase::NoneActive();
    if (m_imgDisabled.block != 0xFFFF)
    {
        CControlImage::SetImageID(m_imgDisabled.group, m_imgDisabled.id, m_imgDisabled.block);
    }
    m_Text.SetTextColor(m_colDisabled);
}

// ==========================
// Active
// 反編譯：
//   CControlBase::Active(this);
//   this[30] = this[164]; this[31] = this[165]; WORD[64] = this[166];
//   this[93] = this[177];
// ==========================
void CControlButton::Active()
{
    CControlBase::Active();
    CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
    m_Text.SetTextColor(m_colNormal);
}

// ==========================
// SetImage 多載
// ==========================

// 反編譯：5 參數版 → 呼叫 8 參數版，同一 imageId
void CControlButton::SetImage(unsigned int imageIdAll,
    uint16_t blockNormal,
    uint16_t blockHover,
    uint16_t blockPressed,
    uint16_t blockDisabled)
{
    SetImage(imageIdAll, blockNormal,
        imageIdAll, blockHover,
        imageIdAll, blockPressed,
        imageIdAll, blockDisabled);
}

// 反編譯：2 參數版
//   this[165] = a2;         // normal.id
//   WORD[332] = a3;         // normal.block
//   CControlImage::SetImageID(this, this[164], this[165], this[166]);
//   CControlText::SetTextPosToParentCenter((char*)this + 224);
void CControlButton::SetImage(unsigned int imageIdNormal, uint16_t blockNormal)
{
    m_imgNormal.id = imageIdNormal;
    m_imgNormal.block = blockNormal;
    CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
    m_Text.SetTextPosToParentCenter();
}

// 反編譯：6 參數版
//   SetImage(a2, a3);  // 先設 normal
//   this[168] = a4;    // hover.id
//   WORD[338] = a5;    // hover.block
//   this[171] = a6;    // pressed.id
//   WORD[344] = a7;    // pressed.block
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

// 反編譯：8 參數版
//   SetImage(a2, a3, a4, a5, a6, a7);
//   this[174] = a8;    // disabled.id
//   WORD[350] = a9;    // disabled.block
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
// 事件處理
// 反編譯完全對照 CControlButton::ControlKeyInputProcess
// ==========================
int* CControlButton::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    int* result;

    // 反編譯：座標去重（a6 != 1 且 msg 為 7 或 4 時）
    if (a6 != 1 && (msg == 7 || msg == 4))
    {
        if (s_nLastX == x && s_nLastY == y)
            return (int*)x;  // 反編譯：return a4 (即 x 指標語義)
        s_nLastX = x;
        s_nLastY = y;
    }

    // 反編譯：this[49] = 0; 清除 hover 旗標
    m_bMouseOver = 0;

    // 反編譯：result = IsActive(); if (!result) return result;
    result = (int*)(intptr_t)IsActive();
    if (!result)
        return result;

    switch (msg)
    {
    case 0: // Click
        // 反編譯：if (WORD[344] != 0xFFFF) 套用 pressed 圖
        if (m_imgPressed.block != 0xFFFF)
        {
            CControlImage::SetImageID(m_imgPressed.group, m_imgPressed.id, m_imgPressed.block);
        }
        // 反編譯：vtbl+104 = ButtonPosDown
        ButtonPosDown();
        CControlButtonBase::PlaySoundClick();
        // 反編譯：this[93] = this[179] (pressed text color)
        m_Text.SetTextColor(m_colPressed);
        dword_AFD34C = 0;
        break;

    case 3: // MouseDown
        // 反編譯：if (WORD[338] != 0xFFFF) 套用 hover 圖
        if (m_imgHover.block != 0xFFFF)
        {
            CControlImage::SetImageID(m_imgHover.group, m_imgHover.id, m_imgHover.block);
        }
        // 反編譯：vtbl+108 = ButtonPosUp
        ButtonPosUp();
        // 反編譯：恢復 normal 圖
        CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
        // 反編譯：this[93] = this[177] (normal text color)
        m_Text.SetTextColor(m_colNormal);
        dword_AFD34C = 1;
        break;

    case 4: // Recovery / MouseOut
        // 反編譯：套用 normal 圖
        CControlImage::SetImageID(m_imgNormal.group, m_imgNormal.id, m_imgNormal.block);
        // 反編譯：vtbl+108 = ButtonPosUp
        ButtonPosUp();
        // 反編譯：this[93] = this[177] (normal text color)
        m_Text.SetTextColor(m_colNormal);
        break;

    case 7: // Hover
        // 反編譯：this[49] = 1
        m_bMouseOver = 1;
        // 反編譯：if (WORD[338] != 0xFFFF) 套用 hover 圖
        if (m_imgHover.block != 0xFFFF)
        {
            CControlImage::SetImageID(m_imgHover.group, m_imgHover.id, m_imgHover.block);
        }
        // 反編譯：this[93] = this[178] (hover text color)
        m_Text.SetTextColor(m_colHover);
        break;

    default:
        break;
    }

    // 反編譯：最後呼叫 CControlBase::ControlKeyInputProcess
    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}
