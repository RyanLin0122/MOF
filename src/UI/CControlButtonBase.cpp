#include "UI/CControlButtonBase.h"
#include "global.h"
#include "Sound/GameSound.h"
#include <cstring>

// ==========================
// 建構 / 解構
// 反編譯：CControlButtonBase::CControlButtonBase
//   CControlImage::CControlImage(this);
//   CControlText::CControlText((char*)this + 224);
//   this[50] = 0;  // pressed
//   this[48] = 0;  // childMoveByClick
//   this[49] = 0;  // mouseOver
//   this[51] = 2;  // shiftAmount
// ==========================
CControlButtonBase::CControlButtonBase()
    : CControlImage()
    , m_Text()
{
    m_bPressed = 0;
    m_bChildMoveByClick = 0;
    m_bMouseOver = 0;
    m_nShiftAmount = 2;
}

CControlButtonBase::~CControlButtonBase()
{
    // 反編譯：~CControlText((char*)this + 224); ~CControlImage(this);
    // C++ 自動按逆序解構成員
}

// ==========================
// CreateChildren
// 反編譯：呼叫 m_Text 的 virtual Create(this)
//   (*(void (__thiscall **)(char*, this))(*((_DWORD*)this + 56) + 12))((char*)this + 224, this);
// ==========================
void CControlButtonBase::CreateChildren()
{
    m_Text.Create(this);
}

// ==========================
// Init
// 反編譯：
//   (*(void (__thiscall **)(this))(*(_DWORD*)this + 68))(this);  // virtual Show()
//   strcpy((char*)this + 208, "J0003");
// ==========================
void CControlButtonBase::Init()
{
    Show();
    strcpy(m_szSoundName, "J0003");
}

// ==========================
// 文字
// ==========================
void CControlButtonBase::SetText(const char* a2)
{
    m_Text.SetText(a2);
}

void CControlButtonBase::SetText(int stringId)
{
    m_Text.SetText(stringId);
}

// ==========================
// PlaySoundClick
// 反編譯：GameSound::PlaySoundA(&g_GameSoundManager, (char*)this + 208, 0, 0);
// ==========================
void CControlButtonBase::PlaySoundClick()
{
    g_GameSoundManager.PlaySoundA(m_szSoundName, 0, 0);
}

// ==========================
// ButtonPosDown
// 反編譯：
//   if (this[48]) {            // childMoveByClick enabled
//     if (!this[50]) {         // not already pressed
//       SetChildPosMove(this[51], this[51]);
//       this[50] = 1;
//     }
//   }
// ==========================
void CControlButtonBase::ButtonPosDown()
{
    if (m_bChildMoveByClick)
    {
        if (!m_bPressed)
        {
            CControlBase::SetChildPosMove(m_nShiftAmount, m_nShiftAmount);
            m_bPressed = 1;
        }
    }
}

// ==========================
// ButtonPosUp
// 反編譯：
//   if (this[48]) {
//     if (this[50]) {
//       SetChildPosMove(-this[51], -this[51]);
//       this[50] = 0;
//     }
//   }
// ==========================
void CControlButtonBase::ButtonPosUp()
{
    if (m_bChildMoveByClick)
    {
        if (m_bPressed)
        {
            CControlBase::SetChildPosMove(-m_nShiftAmount, -m_nShiftAmount);
            m_bPressed = 0;
        }
    }
}

// ==========================
// SetChildMoveByClick
// 反編譯：
//   this[51] = a3;  // shiftAmount
//   this[48] = a2;  // enable
// ==========================
void CControlButtonBase::SetChildMoveByClick(int a2, int a3)
{
    m_nShiftAmount = a3;
    m_bChildMoveByClick = a2;
}

// ==========================
// IsMouseOver
// 反編譯：return this[49];
// ==========================
int CControlButtonBase::IsMouseOver()
{
    return m_bMouseOver;
}

// ==========================
// SetSoundName
// ground truth: strcpy((char*)this + 208, name)
// ==========================
void CControlButtonBase::SetSoundName(const char* name)
{
    strncpy(m_szSoundName, name, sizeof(m_szSoundName) - 1);
    m_szSoundName[sizeof(m_szSoundName) - 1] = '\0';
}
