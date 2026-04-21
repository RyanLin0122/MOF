#pragma once

// Minimal stub for the in-game main-menu UI window.  mofclient.c calls
// CUIManager::GetUIWindow(g_UIMgr, 1) and casts the result to CUIMenu*; the
// full window is not yet restored, so the lookup always returns nullptr and
// CShortKey::AdjustClientKey skips the branch.  This declaration only exists
// so the branch compiles with the right types.

#include "UI/CUIBase.h"

class CUIMenu : public CUIBase {
public:
    CUIMenu() = default;
    ~CUIMenu() override = default;

    // Ground-truth integration points — see CShortKey::AdjustClientKey.
    int*  GetMenuKeyIndex();                       // returns pointer to 6 ints
    void  SetMenuShortKeyName(char names[6][256]); // store 6 × 256-char labels

private:
    int  m_nMenuKeyIndex[6]{};
    char m_strMenuName[6][256]{};
};
