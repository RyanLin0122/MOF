#include <cstdio>
#include "UI/CControlBoxAbility.h"

CControlBoxAbility::CControlBoxAbility()
    : CControlBase()
    , m_Name()
    , m_Extra()
    , m_Edit()
    , m_BtnA()
    , m_BtnB()
    , m_Right()
{
    // vtable 由編譯器設定
}

CControlBoxAbility::~CControlBoxAbility()
{
    // 成員自動釋放，順序符合反編譯析構
}

void CControlBoxAbility::CreateChildren()
{
    // +120 文字
    m_Name.Create(this);
    m_Name.SetY(4);
    // *((DWORD*)this + 66) = 1; 旗標語意不明，保持預設即可

    // +552 文字
    m_Extra.Create(this);
    m_Extra.SetPos(53, 4);
    // *((DWORD*)this + 174) = 1; 同上，保持預設

    // +984 輸入框
    m_Edit.Create(this);
    m_Edit.SetX(108);
    // SetEBoxSize(25, 0, 1) => 寬 25，單行對齊參數 1（符合反編譯）
    m_Edit.SetEBoxSize(25, 0, 1);
    // SetWritable(0,0,0,1) => 不可輸入（唯讀），但仍可顯示/聚焦處理（符合反編譯）
    m_Edit.SetWritable(0, 0, 0, 1);

    // +5336 按鈕 A（放在輸入框右側 +3）
    m_BtnA.Create(this);
    const int editY = m_Edit.GetY();
    const uint16_t editW = m_Edit.GetWidth();
    const int editX = m_Edit.GetX();
    m_BtnA.SetPos(editX + editW + 3, editY);
    // vtbl+112：SetImage(group, 79, 80, 81)（3 態）
    // 以「同一 imageId + 不同 block」版本表達：imageId = 536870949
    m_BtnA.SetImage(536870949u, 79, 536870949u, 80, 536870949u, 81);

    // 反編譯：strcpy((char*)this + 5544, "J0002")（位於按鈕 A 內部字串）
    // 若你的 CControlButtonBase 提供設定點擊音/資源鍵，可在此呼叫。否則略過不影響外觀。

    // +6060 按鈕 B（位於 A 的正下方，間隔 +1）
    m_BtnB.Create(this);
    const uint16_t btnAHeight = m_BtnA.GetHeight();
    m_BtnB.SetPos(m_BtnA.GetX(), m_BtnA.GetY() + btnAHeight + 1);
    // vtbl+112：SetImage(group, 83, 84, 85, 86)（4 態）
    m_BtnB.SetImage(536870949u, 83, 84, 85, 86);

    // 反編譯：strcpy((char*)this + 6268, "J0002")
    // 同上，若有 API 可設定音效鍵，這裡一併設定；無則可略。

    // +6784 右側文字
    m_Right.Create(this);
    m_Right.SetControlSetFont("AbilityText");
    m_Right.SetTextColor(static_cast<uint32_t>(-13677194)); // 對齊反編譯的顏色
    m_Right.SetPos(171, 4);

    // 設定本控制大小：寬 193，高 = EditBox 高
    const uint16_t editH = m_Edit.GetHeight();
    SetSize(193, editH);
}

void CControlBoxAbility::Create(int x, int y, CControlBase* pParent, int kind, int nameTextId)
{
    CControlBase::Create(x, y, pParent);

    // 反編譯：(*this->vtbl+64)(this, a5) —— 此處把 kind 設為控制類型/識別
    // 若有正式 API，請替換為該 API；預設以 m_nClassId 對齊。
    m_nClassId = kind;

    // ToolTip：a5 + 3235，其餘參數與反編譯一致
    // SetToolTipData(int16 a2, int a3, int a4, int a5, char a6, int16 a7, int a8)
    SetToolTipData(static_cast<int16_t>(kind + 3235), 0, 0, 0, 255, static_cast<int16_t>(0xFFFF), 0);

    // 名稱文字：以文字 ID 設定（對應 CControlText::SetText(int) 多載）
    m_Name.SetText(nameTextId);
}

char* CControlBoxAbility::GetEditBoxText()
{
    // 等價於：return CControlText::GetText((char*)this + 4040);
    // 這裡透過 EditBox 的內部文字控制項取得字串
    return m_Edit.GetText(); // 假設 CControlEditBox 已提供 GetText()；若沒有，可新增存取器映射到其內部 CControlText
}

void CControlBoxAbility::SetPoint(uint16_t a2, uint16_t a3, uint16_t a4)
{
    // EditBox 顯示 a2
    m_Edit.SetTextItoa(static_cast<int>(a2));

    // +552 文字顯示 a4
    m_Extra.SetTextItoa(static_cast<int>(a4));

    // +6784 右側文字顯示 a3（格式化）
    char buf[256] = {};
    std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(a3));
    m_Right.SetText(buf);
}
