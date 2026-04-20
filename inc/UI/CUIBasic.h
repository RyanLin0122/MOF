#pragma once

// Minimal stub for the main in-game basic UI window referenced by
// cltChattingMgr.  mofclient.c calls CUIManager::GetUserInterface(g_UIMgr, 0, 0)
// and casts the result to CUIBasic*.  The full CUIBasic window is not yet
// restored, so every lookup returns nullptr and the chat manager's null checks
// short-circuit the call.  This declaration gives the chat manager a real type
// to compile against.

#include "UI/CUIBase.h"

class CUIBasic : public CUIBase {
public:
    CUIBasic();
    ~CUIBasic() override;

    void SetChat(char* buffer);
    void SetInputChat(int a2, int a3, char* source, int cursorBlink, char* composed);
    void ReceivedWhisper(char* name);
};
