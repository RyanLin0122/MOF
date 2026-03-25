#pragma once
#include <cstdint>
#include "Logic/cltMiniGame_Button.h"

class GameImage;

class cltHelpMessage {
public:
    cltHelpMessage();
    virtual ~cltHelpMessage();

    void Initialize(int helpKind);
    void SetDisable();
    void SetActive();
    void IsShow(int helpState);
    void Free();
    int  CloseHelpBox();
    void Poll();
    void PrepareDrawing();
    void Draw();

    // 靜態回呼
    static void OnTimer_TimeOutView(unsigned int timerID, cltHelpMessage* pThis);
    static void OnBtn_Exit(cltHelpMessage* pThis);

private:
    int              m_bActive = 0;         // DWORD[1]
    int              m_bDisabling = 0;      // DWORD[2]
    int              m_bCanShow = 1;        // DWORD[3]  — 建構時設為 1
    unsigned int     m_dwResourceID = 0;    // DWORD[4]
    uint16_t         m_wBlockID = 0;        // WORD[10]  (byte 20)
    uint16_t         _pad1 = 0;
    int              m_nAlpha = 0;          // DWORD[6]
    GameImage*       m_pBgImage = nullptr;  // DWORD[7]
    GameImage*       m_pCloseImage = nullptr;// DWORD[8]
    unsigned int     m_hTimer = 0;          // DWORD[9]
    cltMiniGame_Button m_btnExit;           // offset 40
    int              m_bBtnCreated = 1;     // DWORD[33] — 建構時設為 1
};

// 全域輔助函式（mofclient.c OnHSPopupMessage）
void OnHSPopupMessage(int helpKind);
