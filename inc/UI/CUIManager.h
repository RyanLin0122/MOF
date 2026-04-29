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

    // -----------------------------------------------------------------
    // mofclient.c 0x433CC0：建立 CLog 單例。
    //   - 預設 flag = 8 (CLog::kQuiet)：不寫任何輸出
    //   - 若 "MoFData/log.dat" 存在 → flag = 7 (stdout|file|window)
    //   - 對應 binary 用 `operator new(0x114u)` 之後 ctor 寫入 Singleton<CLog>::m_singleton
    //   被 mofclient.c 0x4307E0 CUIManager::CUIManager 開頭呼叫
    // -----------------------------------------------------------------
    void OpenLogWindow();
};

extern CUIManager* g_UIMgr;
