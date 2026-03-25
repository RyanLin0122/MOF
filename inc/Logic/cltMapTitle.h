#pragma once
#include <cstdint>
#include "UI/CControlChatBallon.h"

class GameImage;

class cltMapTitle {
public:
    cltMapTitle();
    virtual ~cltMapTitle();

    void Poll();
    void PrepareDrawing();
    void Draw();
    void SetMapTitle(uint16_t mapID);
    void SetBossMopID(uint16_t mopID);

private:
    int          m_bShowTitle = 0;       // DWORD[1]
    int          m_bShowBoss = 0;        // DWORD[2]
    int          m_nAlpha = 0;           // DWORD[3]
    uint16_t     m_mapID = 0;            // WORD[8]
    uint16_t     _pad1 = 0;
    unsigned int m_titleResID = 0;       // DWORD[5]
    uint16_t     m_titleBlockID = 0;     // WORD[12]
    uint16_t     _pad2 = 0;
    unsigned int m_bossResID = 0;        // DWORD[7]
    uint16_t     m_bossBlockID = 0;      // WORD[16]
    uint16_t     _pad3 = 0;
    int          m_nUnknown = 0;         // DWORD[9]
    GameImage*   m_pTitleImage = nullptr; // DWORD[10]
    GameImage*   m_pBossImage = nullptr;  // DWORD[11]
    CControlChatBallon m_chatBallon;     // offset 48
};
