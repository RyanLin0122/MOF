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
    int GetGameState();

    // CMessageBoxManager 使用的排序管理
    void AddOrder(CUIMessageBoxBase* pBox);
    void DelOrder(CUIBase* pBase);
    void DeleteFocusWindow(CUIBase* pBase);
};

extern CUIManager* g_UIMgr;
