#pragma once

// Minimal stub for the trade-user UI window referenced by cltChattingMgr.
// mofclient.c calls CUIManager::GetUserInterface(g_UIMgr, 16, 0) and casts the
// result to CUITradeUser*.  The underlying window is not yet restored, so the
// lookup returns nullptr and the chat manager's null checks short-circuit the
// call.  This declaration gives the chat manager a real type to compile
// against.

#include "UI/CUIBase.h"

class CUITradeUser : public CUIBase {
public:
    CUITradeUser();
    ~CUITradeUser() override;

    void SetChat(char* buffer);
};
