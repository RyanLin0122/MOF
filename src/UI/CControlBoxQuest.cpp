#include "UI/CControlBoxQuest.h"
#include <cstdio>
#include "global.h"

//--------------------------------------------------
// ctor (0041A960)
//--------------------------------------------------
CControlBoxQuest::CControlBoxQuest()
    : CControlBoxBase()
    , m_title()
    , m_fitLevel()
    , m_completeText()
{
    CControlBoxBase::Init();
    CreateChildren();
}

//--------------------------------------------------
// dtor (0041A9E0)
//--------------------------------------------------
CControlBoxQuest::~CControlBoxQuest()
{
    // 成員自動解構
}

//--------------------------------------------------
// CreateChildren (0041AA60)
//--------------------------------------------------
void CControlBoxQuest::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // 背景圖設定
    GetBackground()->SetImageID(3u, 0xC000116u, 0);

    // 標題文字 (+312)
    m_title.Create(this);
    m_title.SetPos(45, 4);

    // 適配等級文字 (+744)
    m_fitLevel.Create(this);
    m_fitLevel.SetPos(183, 18);

    // 完成提示文字 (+1176)
    m_completeText.Create(this);
    m_completeText.SetPos(45, 18);
    m_completeText.SetTextColor(0xFFFF0000);  // -65536
    m_completeText.SetText(3262);
}

//--------------------------------------------------
// SetQuestKindImage (0041AB00)
//--------------------------------------------------
void CControlBoxQuest::SetQuestKindImage(unsigned short kind, int keepShade)
{
    CControlImage* bg = GetBackground();
    bg->Show();

    if (!keepShade)
        bg->SetShadeMode(0);

    switch (kind)
    {
    case 0u:
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

//--------------------------------------------------
// SetTitle (0041ABC0)
//--------------------------------------------------
void CControlBoxQuest::SetTitle(int textId)
{
    m_title.SetText(textId);
}

//--------------------------------------------------
// SetComplete (0041ABE0)
//--------------------------------------------------
void CControlBoxQuest::SetComplete(int on)
{
    if (on)
        m_completeText.Show();
    else
        m_completeText.Hide();
}

//--------------------------------------------------
// SetFitLevel (0041AC10)
//--------------------------------------------------
void CControlBoxQuest::SetFitLevel(unsigned char level)
{
    if (level)
    {
        const char* text = g_DCTTextManager.GetText(3145);
        char buffer[256];
        sprintf(buffer, "%s %d", text, level);
        m_fitLevel.SetText(buffer);
    }
    else
    {
        m_fitLevel.SetText("");
    }
}
