#include "UI/CControlBoxCreateChar.h"
#include "Info/cltBasicAppearKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "global.h"

//-------------------------------------------------------------
// 靜態資料
//-------------------------------------------------------------

// 每種 type 的最大選項數量（index 0~6）
// 反編譯中以 word_6C6B12 為起點的 uint16_t 陣列讀取
// type 0=名稱(不用), 1=性別(2), 2=髮型(3), 3=臉型(3), 4~6=裝備外觀
const uint16_t CControlBoxCreateChar::s_maxAppearCount[7] = {
    0, 2, 3, 3, 4, 4, 4
};

// 男性角色裝備外觀代碼表（type 4, 5, 6 各 MAX_APPEAR_SLOT 個）
const char* CControlBoxCreateChar::s_maleAppearKindCode[3][MAX_APPEAR_SLOT] = {
    { "J0501", "J0502", "J0503", "J0504" },   // type 4
    { "J0601", "J0602", "J0603", "J0604" },   // type 5
    { "J0701", "J0702", "J0703", "J0704" },   // type 6
};

// 女性角色裝備外觀代碼表
const char* CControlBoxCreateChar::s_femaleAppearKindCode[3][MAX_APPEAR_SLOT] = {
    { "J0503", "J0504", "J0505", "J0506" },   // type 4
    { "J0603", "J0604", "J0605", "J0606" },   // type 5
    { "J0703", "J0704", "J0705", "J0706" },   // type 6
};

//-------------------------------------------------------------
// ctor / dtor
//-------------------------------------------------------------
CControlBoxCreateChar::CControlBoxCreateChar()
    : CControlBase()
    , m_label()
    , m_editBox()
    , m_btnUp()
    , m_btnDown()
{
    CreateChildren();
    m_value    = 0;
    m_itemKind = 0;
}

CControlBoxCreateChar::~CControlBoxCreateChar()
{
    // 成員自動解構，順序與反編譯一致
}

//-------------------------------------------------------------
// CreateChildren（對齊反編譯 ctor 內呼叫）
//-------------------------------------------------------------
void CControlBoxCreateChar::CreateChildren()
{
    // m_label 掛在本控制項下
    m_label.Create(this);

    // m_editBox 掛在本控制項下
    m_editBox.Create(this);

    // m_btnUp：Create + SetImage(536870949=0x20000025, blocks 79/80/81/82)
    m_btnUp.Create(this);
    m_btnUp.SetImage(536870949u, 79, 80, 81, 82);

    // m_btnDown：Create + SetImage(536870949, blocks 83/84/85/86)
    m_btnDown.Create(this);
    m_btnDown.SetImage(536870949u, 83, 84, 85, 86);
}

//-------------------------------------------------------------
// Create（對齊反編譯 0041FEF0）
//-------------------------------------------------------------
void CControlBoxCreateChar::Create(int x, int y, CControlBase* pParent,
                                    uint8_t type, int labelTextId)
{
    CControlBase::Create(x, y, pParent);

    // 設定 arrayIndex = type（反編譯：vtbl+64 即 SetArrayIndex）
    SetArrayIndex(type);

    // 標籤文字
    m_label.SetText(labelTextId);

    if (type == 0)
    {
        // ---- 名稱輸入模式 ----
        m_label.SetY(4);
        m_editBox.SetX(26);
        m_editBox.SetEBoxSize(131, 0, 0);
        m_editBox.SetWritable(1, 0x0C, 0, 1);

        if (g_MoFFont.IsAnsiCharSet())
            m_editBox.SetAlphaDigit(1);

        // 自身尺寸 = editBox 尺寸 + label 佔用
        uint16_t ebW = m_editBox.GetWidth();
        uint16_t ebH = m_editBox.GetHeight();
        SetSize(ebW + 26, ebH);
    }
    else if (type < 7)
    {
        // ---- 外觀選擇模式（type 1~6）----
        m_label.SetY(4);
        m_editBox.SetX(26);
        m_editBox.SetEBoxSize(88, 0, 0);
        m_editBox.SetWritable(0, 0, 0, 1);

        // btnUp 擺在 editBox 右邊 + 3px
        int ebY  = m_editBox.GetY();
        int ebW  = m_editBox.GetWidth();
        int ebX  = m_editBox.GetX();
        m_btnUp.SetPos(ebX + ebW + 3, ebY);

        // btnDown 擺在 btnUp 正下方 + 1px
        int buH = m_btnUp.GetHeight();
        int buX = m_btnUp.GetX();
        int buY = m_btnUp.GetY();
        m_btnDown.SetPos(buX, buY + buH + 1);

        // 自身尺寸
        uint16_t ebHeight = m_editBox.GetHeight();
        uint16_t btnW     = m_btnUp.GetWidth();
        uint16_t editW    = m_editBox.GetWidth();
        SetSize(editW + btnW + 26, ebHeight);

        // 初始化外觀資料
        SetAppearData(0, 0);
    }
    else
    {
        // ---- 能力值顯示模式（type >= 7）----
        m_label.SetX(14);

        // 反編譯：*((_DWORD*)this + 68) = 1
        // 此寫入對應 m_label 內部某個欄位（推測為 ShadowColor 或 FontWeight），
        // 因重建標頭與原始二進位記憶體佈局可能不完全吻合，以 SetShadowColor(1) 模擬。
        m_label.SetShadowColor(1);

        m_editBox.SetPos(0, 23);
        m_editBox.SetEBoxSize(24, 0, 1);
        m_editBox.SetWritable(0, 0, 0, 1);

        // btnUp 擺在 editBox 右邊 + 3px
        int ebY2 = m_editBox.GetY();
        int ebW2 = m_editBox.GetWidth();
        int ebX2 = m_editBox.GetX();
        m_btnUp.SetPos(ebX2 + ebW2 + 3, ebY2);

        // btnDown 擺在 btnUp 正下方 + 1px
        int buH2 = m_btnUp.GetHeight();
        int buY2 = m_btnUp.GetY();
        int buX2 = m_btnUp.GetX();
        m_btnDown.SetPos(buX2, buY2 + buH2 + 1);

        // 自身尺寸
        uint16_t editW2 = m_editBox.GetWidth();
        uint16_t btnW2  = m_btnUp.GetWidth();
        uint16_t ebH2   = m_editBox.GetHeight();
        SetSize(editW2 + btnW2, ebH2 + 23);

        // ToolTip（type + 3228 = 對應能力文字 ID）
        SetToolTipData(static_cast<int16_t>(type + 3228), 0, 0, 0, 255, 0xFFFF, 0);

        // 初始值 4
        SetAbilityData(4);
    }
}

//-------------------------------------------------------------
// Show（對齊反編譯 00420180）
//-------------------------------------------------------------
void CControlBoxCreateChar::Show()
{
    CControlBase::Show();

    // 名稱輸入模式（arrayIndex == 0）：隱藏按鈕，設定 EditBox 焦點
    if (GetArrayIndex() == 0)
    {
        m_btnUp.Hide();
        m_btnDown.Hide();
        m_editBox.SetFocus(1);
    }
}

//-------------------------------------------------------------
// SetAppearData（對齊反編譯 004201C0）
// direction: 0=重設, 1=循環上, 2=循環下
// sex: 非零=男性
//-------------------------------------------------------------
void CControlBoxCreateChar::SetAppearData(uint8_t direction, uint8_t sex)
{
    int type = GetArrayIndex();

    // 循環選擇或重設
    if (direction == 1)
        CircleLoopUpper(&m_value, s_maxAppearCount[type]);
    else if (direction == 2)
        CircleLoopLower(&m_value, s_maxAppearCount[type]);
    else
        m_value = 0;

    switch (type)
    {
    case 1:
        // 性別
        if (m_value)
            m_editBox.SetText(3184);   // 女
        else
            m_editBox.SetText(3185);   // 男
        break;

    case 2:
    {
        // 髮型（反編譯不做 null 檢查，直接解參考）
        strBasicAppearKindInfo* hair = g_clBasicAppearKindInfo.GetHair(m_value);
        m_editBox.SetText(static_cast<int>(hair->code));
        break;
    }

    case 3:
    {
        // 臉型
        strBasicAppearKindInfo* face = g_clBasicAppearKindInfo.GetFace(m_value);
        m_editBox.SetText(static_cast<int>(face->code));
        break;
    }

    case 4:
    case 5:
    case 6:
    {
        // 裝備外觀
        const char* kindCode;
        int idx = type - 4;
        if (sex)
            kindCode = s_maleAppearKindCode[idx][m_value];
        else
            kindCode = s_femaleAppearKindCode[idx][m_value];

        uint16_t kind = cltItemKindInfo::TranslateKindCode(kindCode);
        m_itemKind = kind;

        stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(kind);
        m_editBox.SetText(static_cast<int>(info->m_wTextCode));
        break;
    }

    default:
        return;
    }
}

//-------------------------------------------------------------
// SetAbilityData（對齊反編譯 00420320）
//-------------------------------------------------------------
void CControlBoxCreateChar::SetAbilityData(int value)
{
    m_value = value;
    m_editBox.SetTextItoa(value);
}

//-------------------------------------------------------------
// GetEditBoxText（對齊反編譯 00420340）
//-------------------------------------------------------------
char* CControlBoxCreateChar::GetEditBoxText()
{
    // 對齊反編譯：直接存取內部 CControlText::GetText（ground truth 以偏移存取）
    return (char*)m_editBox.m_Text.GetText();
}
