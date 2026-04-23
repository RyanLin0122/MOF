#pragma once

// cltChattingMgr — restored from mofclient.c
//
// Responsible for the entire in-game chat pipeline:
//   * Hosts the IME input line (DCTIMMList) for the chat edit box, including
//     the highlight block that shows the IME selection range.
//   * Maintains a rolling 100-entry display buffer (filtered by channel),
//     a 5-entry "recent lines" history navigable with Up/Down, and a
//     100-entry trade-window chat buffer.
//   * Parses slash-prefixed commands (/P /W /invite /ban /user /? /CI /CB
//     /CE /PE /C /R /honey plus their localised Chinese/Korean aliases) and
//     dispatches them through CInterfaceDataCommunity or CMoFNetwork.
//   * Rate-limits outgoing chatting via two 5-second cltTimerManager timers
//     (five messages / five seconds) and routes the final text through
//     cltMoFC_EmoticonSystem so embedded emoticon words resolve to item IDs.
//
// The original class is accessed through a handful of static pointers set up
// by Initialize(); they are preserved as static members so that call sites
// reaching in through cltChattingMgr::m_pNetwork etc. keep working.

#include <cstdint>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "System/cltMoFC_EmoticonSystem.h"

class DCTIMMList;
class DirectInputManager;
class CMoFNetwork;
class ClientCharacterManager;
class cltMyCharData;

class cltChattingMgr {
public:
    cltChattingMgr();
    ~cltChattingMgr();

    void Initialize(DCTIMMList* pIMMList, DirectInputManager* pInputMgr,
                    CMoFNetwork* pNetwork, ClientCharacterManager* pCharMgr,
                    cltMyCharData* pMyCharData);

    void DrawBlockBox();
    void DeleteBlockBox();
    void SetBlockShow(int show);
    void SetBlockBox(int x, int y, std::uint16_t w, std::uint16_t h, std::uint8_t alpha);

    void SetChatWritedString(char* text);
    int  GetChatWritedString(int direction, char* out);

    void Poll();
    void SetIMELength(int length);
    void SendInputChat();
    void SetChatBlock(int baseX, int baseY);

    int  IsValidChat(char* text);
    void SetChatState(int state);
    int  GetChatState();

    void SetSysCommand(char* text);
    void SetAutoChatMode(char* text, std::uint8_t filter);

    // Overload 1 (mofclient.c 0x4F7E20): target identified by account id.
    void SetChatBuffer(std::uint16_t channel, unsigned int accountId,
                       char* message, std::uint16_t emoticonId, char* extra);
    // Overload 2 (mofclient.c 0x4F8070): target identified by character name.
    void SetChatBuffer(std::uint16_t channel, char* name, char* message,
                       std::uint16_t emoticonId);

    void SetChatFilter(std::uint8_t filter);
    void SendChatting();

    void StartTradeFilterChat(char* nameA, char* nameB);
    void EndTradeFilterChat();
    void SetChatBufferByTradeFilter(char* name, char* message);
    void SetTradeFilterChatBufferIndex();

    void InitChatMgr();

    void CompleteWhisper(std::uint8_t code, char* targetName, char* message);

    int  DispatchChatOrder(char* text);
    void DispatchChatOrder_PartyChat(char* text);
    void DispatchChatOrder_JoinParty(char* text);
    void DispatchChatOrder_KickoutParty(char* text);
    void DispatchChatOrder_Whisper(char* text);
    void DispatchChatOrder_ResWhisper(char* text);
    void DispatchChatOrder_PartnerRecall(char* text);
    void DispatchChatOrder_JoinCircle(char* text);
    void DispatchChatOrder_KickoutCircle(char* text);
    void DispatchChatOrder_OutCircle(char* text);
    void DispatchChatOrder_OutParty(char* text);
    void DispatchChatOrder_CircleChat(char* text);
    void DispatchChatOrder_Help(char* text);
    void DispatchChatOrder_DetailCharInfo(char* text);

    void SendWhisper(char* targetName, char* message);

    void FindEmoticonWord(unsigned int accountId, char* message, std::uint16_t emoticonId);
    void FindEmoticonWord(char* name, char* message, std::uint16_t emoticonId);

    void SendChattingMsg(char* message);

    BOOL DispatchChatOrderCheck(char* text);
    char IsValidCharName(char* text);

    void SendQuickChatting(std::uint8_t kind, char* text);
    void SetBanChat();

    // Static subsystem pointers (mirrors the ground truth statics).
    static cltMyCharData*          m_pclMyChatData;
    static ClientCharacterManager* m_pClientCharMgr;
    static DirectInputManager*     m_pInputManager;
    static DCTIMMList*             m_pDCTIMMList;
    static CMoFNetwork*            m_pNetwork;

private:
    // --- Channel-prefix strings (localised) ----------------------------------
    // mofclient.c fills this table from DCTTextManager text IDs.  Indices match
    // the order the ground truth Initialize() writes them in, and the strings
    // (except 6, 9, 10, 12) are appended with a trailing space.
    enum : int {
        PREFIX_PARTY       = 0,   // "Party "  — text 3661
        PREFIX_WHISPER     = 1,   // "Whisper" — text 3076 (local alias of "W ")
        PREFIX_CIRCLE      = 2,   // "Circle " — text 3382
        PREFIX_INVITE      = 3,   // text 4602 + " "  (alias of "invite ")
        PREFIX_BAN         = 4,   // text 4603 + " "  (alias of "ban ")
        PREFIX_USER        = 5,   // text 3478 + " "  (alias of "user ")
        PREFIX_HELP        = 6,   // text 3623        (alias of "?")
        PREFIX_CJOIN       = 7,   // text 3719 + " "  (alias of "CI ")
        PREFIX_CKICK       = 8,   // text 3752 + " "  (alias of "CB ")
        PREFIX_COUT        = 9,   // text 3720        (alias of "CE")
        PREFIX_POUT        = 10,  // text 3074        (alias of "PE")
        PREFIX_REPLY       = 11,  // text 3934 + " "  (alias of "R ")
        PREFIX_PARTNER     = 12,  // text 8355        (alias of "honey")
        PREFIX_COUNT       = 13
    };
    char m_szCmdPrefix[PREFIX_COUNT][20]{};

    // --- Display buffer entry (264 bytes, 100 entries) -----------------------
    struct ChatEntry {
        std::uint8_t  filter;        // channel byte (1 all / 2 party / 4 whisp / 5 circle …)
        std::uint8_t  pad0[3];
        std::uint32_t color;         // packed ARGB
        char          text[256];     // "<name> : <message>"
    };

    static void ShiftChatBufferDown(ChatEntry* arr, int count);

    int          m_ChatState = 0;                    // 0 closed / 1 open
    ChatEntry    m_ChatBuffer[100]{};                // newest-at-top display buffer
    int          m_nChatBufferCount = 0;
    char         m_szChatDisplayBuffer[25600]{};     // rendered "color:text\n..." stream

    char         m_szLastWhisperName[256]{};

    // 5-slot history of strings the user has typed, navigated with Up/Down.
    char         m_szWritedHistory[5][100]{};
    int          m_nWritedMax = 0;
    int          m_nWritedCurrent = 0;
    int          m_bWritedPicked = 0;

    // Rate-limit timers (see SendChattingMsg).
    unsigned int m_dwTimerHandle1 = 0;
    unsigned int m_dwTimerHandle2 = 0;
    std::uint16_t m_wChatMsgCount = 0;

    // Module-enabled flag. Ground truth: *((_DWORD*)this + 6702) (byte 26808),
    // initialised to 1 in the ctor and used by Poll as the outer gate. The
    // ground truth never writes it after construction, so it is effectively a
    // constant 1, but it is still modelled here to preserve Poll's semantics.
    int          m_bChatEnabled = 1;

    // IME state.
    std::uint8_t m_bChatPostReady = 0;   // Enter pressed this frame
    std::uint8_t m_cChatFilter = 1;      // currently selected channel (1 = all)
    DWORD        m_dwLastTime = 0;
    int          m_nCursorBlink = 0;
    int          m_nIMMTextHeight = 12;
    int          m_nIMEIndex = 0;
    int          m_nIMELength = 40;

    // Trade-window chat mirror.
    int          m_bTradeFilterMode = 0;
    int          m_nTradeFilterCount = 0;
    ChatEntry    m_TradeFilterBuffer[100]{};
    char         m_szTradePartnerA[32]{};
    char         m_szTradePartnerB[32]{};

    // Embedded emoticon-system (original had it at offset 79676).
    cltMoFC_EmoticonSystem m_EmoticonSystem{};

    // Shout-Event answer buffer (ground-truth offset this+79944).  While a
    // GM-driven Shout Event is active (CUIBasic +449284 == 1), Poll snapshots
    // every outgoing chat line here as the candidate answer — see the
    // TODO(raw-offset #3) block in cltChattingMgr.cpp.  The identifier is
    // retained as m_szLastChatSent because the symbol name is not yet known
    // from the decompilation; rename to m_szShoutEventAnswer once the
    // containing CUIBasic flag is identified and the Shout-Event path is
    // fully restored.
    char         m_szLastChatSent[256]{};

    // Function-static counter from mofclient.c
    // (`_Cnt__1__SetChatWritedString_cltChattingMgr__QAEXPAD_Z_4HA`).  Tracks
    // how many slots of m_szWritedHistory are populated (capped at 5).  Kept
    // as a static member so GetChatWritedString can observe it via the static
    // machinery just like the decompiled code.
    static int   s_WritedCount;
};

extern cltChattingMgr g_clChattingMgr;
