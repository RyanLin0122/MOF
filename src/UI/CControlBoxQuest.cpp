#include "UI/CControlBoxQuest.h"
#include <stdio.h>
#include "global.h"

CControlBoxQuest::CControlBoxQuest()
    : CControlBoxBase()
    , m_title()
    , m_fitLevel()
    , m_completeText()
{
    // vftable 由編譯器處理；此處流程比照反編譯：
    CControlBoxBase::Init();
    CreateChildren();
}

CControlBoxQuest::~CControlBoxQuest()
{
    // 釋放順序比照反編譯輸出
    m_completeText.~CControlText(); // +1176
    m_fitLevel.~CControlText();     // +744
    m_title.~CControlText();        // +312
    GetBackground()->~CControlImage(); // +120 (來自 BoxBase)
    CControlBase::~CControlBase();
}

void CControlBoxQuest::CreateChildren()
{
    // 先建立 BoxBase 預設背景
    CControlBoxBase::CreateChildren();

    // 背景圖：SetImageID(m_bg, 3, 0xC000116, 0)
    GetBackground()->SetImageID(3u, 0xC000116u, 0);

    // 標題文字 (+312)
    m_title.Create(this);
    m_title.SetPos(45, 4);

    // 適配等級文字 (+744)
    m_fitLevel.Create(this);
    m_fitLevel.SetPos(183, 18);
    // 反編譯：*((_DWORD*)this + 222) = 2; 可能是對齊/樣式旗標。
    // 若你的 CControlText 有對齊 API，可改成：
    // m_fitLevel.SetAlign(CControlText::AlignRight);
    // 這裡留空以免與你現有實作衝突。

    // 完成提示文字 (+1176)
    m_completeText.Create(this);
    m_completeText.SetPos(45, 18);
    m_completeText.SetTextColor(-65536); // 反編譯：*((_DWORD*)this + 331) = -65536
    m_completeText.SetText(3262);
}

void CControlBoxQuest::SetQuestKindImage(unsigned short kind, int keepShade)
{
    // 0041AB00：顯示背景、必要時關閉陰影、再依 quest 類型切圖
    CControlImage* bg = GetBackground();
    bg->Show();

    if (!keepShade)
        bg->SetShadeMode(0);

    switch (kind)
    {
    case 0u: // 任務(?) 特別圖樣
        bg->SetImageID(3u, 0x180001E8u, 0x1Eu);
        break;
    case 1u:
        bg->SetImageID(3u, 0xC000116u, 0x2Bu);
        break;
    case 2u:
        bg->SetImageID(3u, 0xC000116u, 0x2Au);
        break;
    case 3u:
    case 6u:
        bg->SetImageID(3u, 0xC000116u, 0x28u);
        break;
    default:
        bg->SetImageID(3u, 0xC000116u, 0x29u);
        break;
    }
}

void CControlBoxQuest::SetTitle(int textId)
{
    m_title.SetText(textId);
}

void CControlBoxQuest::SetComplete(int on)
{
    if (on)
        m_completeText.Show();
    else
        m_completeText.Hide();
}

void CControlBoxQuest::SetFitLevel(unsigned char level)
{
    if (level)
    {
        m_fitLevel.SetText(3145, level);
    }
    else
    {
        m_fitLevel.SetText("");
    }
}
