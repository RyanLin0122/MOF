#pragma once
#include <cstdint>
#include "CControlBase.h"
#include "CControlText.h"
#include "CControlEditBox.h"
#include "CControlButton.h"

/**
 * 能力點設定區塊：
 * - Name(文字)         : +120
 * - Extra/Point(文字)  : +552
 * - EditBox(輸入框)    : +984
 * - ButtonA(按鈕)      : +5336（3 態 79/80/81）
 * - ButtonB(按鈕)      : +6060（4 態 83/84/85/86）
 * - RightText(文字)    : +6784（字型 "AbilityText"，顏色 -13677194）
 */
class CControlBoxAbility : public CControlBase
{
public:
    CControlBoxAbility();
    virtual ~CControlBoxAbility();

    // 反編譯：Create(this, x, y, parent, kind, nameTextId)
    void Create(int x, int y, CControlBase* pParent, int kind, int nameTextId);

    // 反編譯：CreateChildren()
    void CreateChildren();

    // 取輸入框的當前文字（等價於對 +4040 的 CControlText::GetText）
    char* GetEditBoxText();

    // 反編譯：SetPoint(a2, a3, a4)
    // a2 -> EditBox（itoa）
    // a4 -> +552 文字（itoa）
    // a3 -> +6784 文字（格式化字串）
    void SetPoint(uint16_t a2, uint16_t a3, uint16_t a4);

    // 子件取用（若外部要改圖、綁事件）
    CControlText& NameText() { return m_Name; }
    CControlText& ExtraText() { return m_Extra; }
    CControlEditBox& Edit() { return m_Edit; }
    CControlButton& ButtonA() { return m_BtnA; }
    CControlButton& ButtonB() { return m_BtnB; }
    CControlText& RightText() { return m_Right; }

private:
    CControlText     m_Name;   // +120
    CControlText     m_Extra;  // +552
    CControlEditBox  m_Edit;   // +984
    CControlButton   m_BtnA;   // +5336
    CControlButton   m_BtnB;   // +6060
    CControlText     m_Right;  // +6784
};
