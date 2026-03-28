#include <cstdio>
#include <cstring>
#include "UI/CControlBoxAbility.h"

// ====================================================================
// 建構/解構
// ====================================================================
CControlBoxAbility::CControlBoxAbility()
    : CControlBase()
    , m_Name()
    , m_Extra()
    , m_Edit()
    , m_BtnA()
    , m_BtnB()
    , m_Right()
{
    // 反編譯：建構完子物件後立即呼叫 CreateChildren
    CreateChildren();
}

CControlBoxAbility::~CControlBoxAbility()
{
    // 成員自動釋放，順序符合反編譯析構
}

// ====================================================================
// CreateChildren（對齊 004204C0）
// ====================================================================
void CControlBoxAbility::CreateChildren()
{
    // +120 文字（名稱）
    m_Name.Create(this);
    m_Name.SetY(4);
    // 反編譯：*((_DWORD *)this + 66) = 1 → m_Name alignment = 1（置中）
    m_Name.SetAlignment(1);

    // +552 文字（額外/點數）
    m_Extra.Create(this);
    m_Extra.SetPos(53, 4);
    // 反編譯：*((_DWORD *)this + 174) = 1 → m_Extra alignment = 1
    m_Extra.SetAlignment(1);

    // +984 輸入框
    m_Edit.Create(this);
    m_Edit.SetX(108);
    m_Edit.SetEBoxSize(25, 0, 1);
    m_Edit.SetWritable(0, 0, 0, 1);

    // +5336 按鈕 A（放在輸入框右側 +3）
    m_BtnA.Create(this);
    int editY = m_Edit.GetY();
    uint16_t editW = m_Edit.GetWidth();
    int editX = m_Edit.GetX();
    m_BtnA.SetPos(editX + editW + 3, editY);
    // 反編譯：SetImage(536870949, 79, 80, 81)（3 態：normal/hover/pressed）
    m_BtnA.SetImage(536870949u, 79, 536870949u, 80, 536870949u, 81);

    // 反編譯：strcpy((char *)this + 5544, "J0002") → 設定按鈕音效 ID
    m_BtnA.SetSoundName("J0002");

    // +6060 按鈕 B（位於 A 的正下方，間隔 +1）
    m_BtnB.Create(this);
    uint16_t btnAHeight = m_BtnA.GetHeight();
    m_BtnB.SetPos(m_BtnA.GetX(), m_BtnA.GetY() + btnAHeight + 1);
    // 反編譯：SetImage(536870949, 83, 84, 85, 86)（4 態）
    m_BtnB.SetImage(536870949u, 83, 84, 85, 86);

    // 反編譯：strcpy((char *)this + 6268, "J0002")
    m_BtnB.SetSoundName("J0002");

    // +6784 右側文字
    m_Right.Create(this);
    m_Right.SetControlSetFont("AbilityText");
    // 反編譯：*((_DWORD *)this + 1733) = -13677194
    m_Right.SetTextColor(static_cast<uint32_t>(-13677194));
    m_Right.SetPos(171, 4);

    // 反編譯：*((_WORD *)this + 16) = 193; *((_WORD *)this + 17) = EditBox height
    uint16_t editH = m_Edit.GetHeight();
    SetSize(193, editH);
}

// ====================================================================
// Create（對齊 00420690）
// ====================================================================
void CControlBoxAbility::Create(int x, int y, CControlBase* pParent, int kind, int nameTextId)
{
    CControlBase::Create(x, y, pParent);

    // 反編譯：(*(vtbl+64))(this, a5) → SetArrayIndex(kind)
    SetArrayIndex(kind);

    // ToolTip：a5 + 3235，其餘參數與反編譯一致
    SetToolTipData(static_cast<int16_t>(kind + 3235), 0, 0, 0, static_cast<char>(255), static_cast<int16_t>(0xFFFF), 0);

    // 名稱文字：以文字 ID 設定
    m_Name.SetText(nameTextId);
}

// ====================================================================
// GetEditBoxText（對齊 004206F0）
// 反編譯：return CControlText::GetText((char*)this + 4040)
// 4040 = 984（EditBox 偏移）+ EditBox 內部 m_Text 成員偏移
// ====================================================================
char* CControlBoxAbility::GetEditBoxText()
{
    // 對齊反編譯：直接存取內部 CControlText::GetText（ground truth 以偏移存取）
    return (char*)m_Edit.m_Text.GetText();
}

// ====================================================================
// SetPoint（對齊 00420700）
// ====================================================================
void CControlBoxAbility::SetPoint(uint16_t a2, uint16_t a3, uint16_t a4)
{
    // 反編譯：CControlText::SetTextItoa((char*)this + 4040, a2) → EditBox 內文字
    m_Edit.SetTextItoa(static_cast<int>(a2));

    // 反編譯：CControlText::SetTextItoa((char*)this + 552, a4) → Extra 文字
    m_Extra.SetTextItoa(static_cast<int>(a4));

    // 反編譯：_sprintf(Buffer, "%d", a3) 然後 SetText
    char buf[256];
    std::sprintf(buf, "%d", static_cast<int>(a3));
    m_Right.SetText(buf);
}
