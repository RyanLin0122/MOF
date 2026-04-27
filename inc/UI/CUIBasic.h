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

    // mofclient.c 0x4B6E70 / 0x4B6F60 / 0x4B6FE0：分別是
    //   - InitWeddingMap：地圖切換時重置婚禮 UI 標記
    //   - ResurrectDiedChar：玩家復活時關閉死亡 UI 視窗
    //   - SetChatMoveTextBonusEvent：debug 鍵 D 切換 fInfoFlag 時顯示的浮動文字
    // 完整 UI 邏輯尚未還原；提供 stub 維持 ClientCharacterManager / CharKey
    // InputProcess 呼叫面相容。
    void InitWeddingMap();
    void ResurrectDiedChar();
    void SetChatMoveTextBonusEvent(const char* text);

    // --- CShortKey::AdjustClientKey integration points ---
    // mofclient.c writes 12 / 2 / 10 × 256-char label strings into the window
    // whenever the short-key bindings change.  The full widget is not yet
    // restored, so these trivially store into internal buffers (and the buffers
    // are available for future widget work).
    void  SetQSLShortKeyName(char names[12][256]);
    void  SetQSLPageShortKeyName(char names[2][256]);
    void  SetMenuShortKeyName(char names[10][256]);
    int*  GetMenuKeyIndex();     // returns pointer to 10 ints (menu-button keys)

private:
    int  m_nMenuKeyIndex[10]{};
    char m_strQSLName[12][256]{};
    char m_strQSLPageName[2][256]{};
    char m_strMenuName[10][256]{};
};
