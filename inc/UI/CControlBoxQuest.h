#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlText.h"
#include "UI/CControlImage.h"
#include "Text/DCTIMM.h"   // DCTTextManager / g_DCTTextManager

class CControlBoxQuest : public CControlBoxBase
{
public:
    CControlBoxQuest();
    virtual ~CControlBoxQuest();

    void CreateChildren();

    // 0041AB00
    void SetQuestKindImage(unsigned short kind, int keepShade);

    // 0041ABC0
    void SetTitle(int textId);

    // 0041ABE0
    void SetComplete(int on);

    // 0041AC10
    void SetFitLevel(unsigned char level);

private:
    // 順序需符合位移：+312, +744, +1176
    CControlText m_title;        // +312
    CControlText m_fitLevel;     // +744
    CControlText m_completeText; // +1176
};
