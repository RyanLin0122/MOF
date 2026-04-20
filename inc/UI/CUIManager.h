#pragma once

class CUIBase;
class CUIMessageBoxBase;

class CUIManager {
public:
    CUIManager() = default;
    ~CUIManager() = default;

    // Returns non-zero if the character-action key with the given keyId is
    // currently pressed.
    static int IsCharActionKey(CUIManager* pMgr, int keyId);

    CUIBase* GetUIWindow(int id);
    // mofclient.c: CUIManager::GetUserInterface(g_UIMgr, id, subId) — used by
    // the chat manager to reach CUIBasic / CUITradeUser windows.  Subtype is
    // currently ignored; the placeholder always returns nullptr because the
    // actual UI windows are not yet restored.
    CUIBase* GetUserInterface(int id, int subId);
    int IsOpenUserInterface(int id);
    int GetGameState();

    // CMessageBoxManager 使用的排序管理
    void AddOrder(CUIMessageBoxBase* pBox);
    void DelOrder(CUIBase* pBase);
    void DeleteFocusWindow(CUIBase* pBase);
};

extern CUIManager* g_UIMgr;
