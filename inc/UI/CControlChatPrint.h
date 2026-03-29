#pragma once

#include "UI/CControlBase.h"
#include "UI/CControlText.h"

class CControlChatPrint : public CControlBase
{
public:
    CControlChatPrint();
    virtual ~CControlChatPrint();

    void Create(CControlBase* pParent);
    void SetInputChat(int a2, int a3, char* a4, int a5, char* lpString);
    void SetAbsPos(int a2, int a3);
    virtual void Draw() override;

private:
    CControlText m_TextCursor;    // 對齊反編譯 offset +128（遊標「_」）
    CControlText m_TextName;      // 對齊反編譯 offset +560（名稱標示）
};
