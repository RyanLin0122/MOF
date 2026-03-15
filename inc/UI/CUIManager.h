#pragma once

class CUIManager {
public:
    CUIManager() = default;
    ~CUIManager() = default;

    // Returns non-zero if the character-action key with the given keyId is
    // currently pressed.
    static int IsCharActionKey(CUIManager* pMgr, int keyId);
};

extern CUIManager* g_UIMgr;
