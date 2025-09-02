#include <cstdio>
#include "UI/CControlBoxBuySkill.h"
#include "global.h" // 提供 g_DCTTextManager / DCTTextManager

CControlBoxBuySkill::CControlBoxBuySkill()
    : CControlBase()
    , m_Title()
    , m_Icon()
    , m_NeedPT()
    , m_Left()
    , m_Right()
    , m_BottomDecor()
{
    // 對齊反編譯：vftable 指派由編譯器處理；建立子件並配置
    m_bIsVisible = true; // *((DWORD*)this + 13) = 1
    CreateChildren();
}

CControlBoxBuySkill::~CControlBoxBuySkill()
{
    // 子物件自動解構；順序符合反編譯析構
}

void CControlBoxBuySkill::CreateChildren()
{
    // 標題文字 (+120)
    m_Title.Create(this);
    m_Title.SetPos(59, 20);
    // *((DWORD*)this + 66) = 1（旗標語意不明，維持預設顯示即可）

    // 技能圖示 (+552)
    m_Icon.Create(this);
    m_Icon.SetPos(44, 43);

    // 需求點數文字 (+744)
    m_NeedPT.Create(this);
    m_NeedPT.SetControlSetFont("BuySkillNeetPT");
    m_NeedPT.SetPos(59, 80);
    m_NeedPT.SetText(3601);                 // 文字 ID
    // *((DWORD*)this + 222) = 1（旗標）
    m_NeedPT.SetTextColor(static_cast<uint32_t>(-6944247)); // *((DWORD*)this + 223)

    // 4 行 左/右欄（y: 99,113,127,141）
    int y = 99;
    for (int i = 0; i < kBuySkillRowCount; ++i)
    {
        // 左欄標籤 (+1176 起)
        m_Left[i].Create(this);
        m_Left[i].SetPos(30, y);

        const char* key = g_DCTTextManager.GetText(3061 + i);
        char buf[256] = {};
        std::snprintf(buf, sizeof(buf), "%s :", key);
        m_Left[i].SetText(buf);

        // 右欄數值 (+2904 起)
        m_Right[i].Create(this);
        m_Right[i].SetPos(66, y);

        y += 14;
    }

    // 底部裝飾影像 (+4632)  imageId=570425350, block=8
    m_BottomDecor.Create(this);
    m_BottomDecor.SetImage(570425350u, 8);
}

void CControlBoxBuySkill::Show()
{
    CControlBase::Show();
    // 反編譯：在 Show 內呼叫底部裝飾的 vtbl+44（常見為 Hide/關閉顯示）
    m_BottomDecor.Hide();
}
