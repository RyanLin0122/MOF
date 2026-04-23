// cltChattingMgr implementation, restored 1:1 from mofclient.c.
// The original is decompiled C that manipulates one flat 80 KB struct via
// raw offset arithmetic; here we port the same behaviour on top of clean C++
// members.  Function order and branch structure mirror the ground truth so
// diffing against mofclient.c stays tractable.

#include "Logic/cltChattingMgr.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

#include "global.h"
#include "Text/DCTTextManager.h"
#include "Text/DCTIMMList.h"
#include "Font/MoFFont.h"
#include "Network/CMoFNetwork.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Logic/cltMyCharData.h"
#include "Logic/cltSystemMessage.h"
#include "System/cltMarriageSystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltClientPartySystem.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CUIManager.h"
#include "UI/CUIBasic.h"
#include "UI/CUICircle.h"
#include "UI/CUITradeUser.h"
#include "UI/CInterfaceDataCommunity.h"
#include "Other/DCTAbuseWordManager.h"
#include "Util/cltTimer.h"

// ---- Static members --------------------------------------------------------
cltMyCharData*          cltChattingMgr::m_pclMyChatData = nullptr;
ClientCharacterManager* cltChattingMgr::m_pClientCharMgr = nullptr;
DirectInputManager*     cltChattingMgr::m_pInputManager = nullptr;
DCTIMMList*             cltChattingMgr::m_pDCTIMMList = nullptr;
CMoFNetwork*            cltChattingMgr::m_pNetwork = nullptr;
int                     cltChattingMgr::s_WritedCount = 0;

// Local helper to unify the decompiler's nine-argument CreateTimer calls.
// (The first five args always are interval, releaseKey, a4=0, a5=1, and then
// four null callbacks plus a null user-data pointer.)
static unsigned int CreateRateLimitTimer(void* releaseKey) {
    return g_clTimerManager.CreateTimer(
        5000u, reinterpret_cast<std::uintptr_t>(releaseKey), 0, 1,
        nullptr, nullptr, nullptr, nullptr, nullptr);
}

//----- (004F6880) -----------------------------------------------------------
cltChattingMgr::cltChattingMgr() {
    // Match ground truth constructor initialisation order.  Most members are
    // already zero-initialised by their class-default-member-initialisers;
    // the ones that start non-zero are assigned explicitly below.
    m_bWritedPicked = 0;
    m_nWritedMax = 0;
    m_nWritedCurrent = 0;
    m_nIMMTextHeight = 12;
    m_nIMEIndex = 0;
    m_ChatState = 0;
    m_nChatBufferCount = 0;
    std::memset(m_ChatBuffer, 0, sizeof(m_ChatBuffer));
    m_bChatEnabled = 1;            // ground truth: *((_DWORD*)this + 6702) = 1
    m_bChatPostReady = 0;
    m_cChatFilter = 1;
    m_dwLastTime = timeGetTime();
    std::memset(m_szLastWhisperName, 0, sizeof(m_szLastWhisperName));
    m_nCursorBlink = 0;
    std::memset(m_TradeFilterBuffer, 0, sizeof(m_TradeFilterBuffer));
    std::memset(m_szTradePartnerA, 0, sizeof(m_szTradePartnerA));
    std::memset(m_szTradePartnerB, 0, sizeof(m_szTradePartnerB));
    m_bTradeFilterMode = 0;
    m_nTradeFilterCount = 0;
    m_nIMELength = 40;
    std::memset(m_szLastChatSent, 0, sizeof(m_szLastChatSent));
    m_dwTimerHandle1 = 0;
    m_dwTimerHandle2 = 0;
    m_wChatMsgCount = 0;
    // m_EmoticonSystem constructs itself.
}

//----- (004F6970) -----------------------------------------------------------
cltChattingMgr::~cltChattingMgr() {}

//----- (004F6980) -----------------------------------------------------------
void cltChattingMgr::Initialize(DCTIMMList* pIMMList, DirectInputManager* pInputMgr,
                                CMoFNetwork* pNetwork, ClientCharacterManager* pCharMgr,
                                cltMyCharData* pMyCharData) {
    m_pInputManager  = pInputMgr;
    m_pNetwork       = pNetwork;
    m_pDCTIMMList    = pIMMList;
    m_pClientCharMgr = pCharMgr;
    m_pclMyChatData  = pMyCharData;

    RECT rc{0, 0, 0, 0};
    int imeIdx = pIMMList ? pIMMList->GetUsableIMEIndex() : 0;
    m_nIMEIndex = imeIdx;
    unsigned short textSize = static_cast<unsigned short>(m_nIMELength);
    if (m_pDCTIMMList) {
        // mofclient.c: SetIMMInfo(list, idx, 0, size, 0, &rect, 1, 0, 17)
        m_pDCTIMMList->SetIMMInfo(imeIdx, 0, textSize, 0, &rc, 1, 0, 17);
    }

    std::memset(m_szWritedHistory, 0, sizeof(m_szWritedHistory));
    std::memset(m_szCmdPrefix, 0, sizeof(m_szCmdPrefix));

    // Localised channel prefixes.
    struct { int idx; int textId; bool trailingSpace; } kPrefixes[] = {
        { PREFIX_PARTY,   3661, true  },
        { PREFIX_WHISPER, 3076, true  },
        { PREFIX_CIRCLE,  3382, true  },
        { PREFIX_INVITE,  4602, true  },
        { PREFIX_BAN,     4603, true  },
        { PREFIX_USER,    3478, true  },
        { PREFIX_HELP,    3623, false },
        { PREFIX_CJOIN,   3719, true  },
        { PREFIX_CKICK,   3752, true  },
        { PREFIX_COUT,    3720, false },
        { PREFIX_POUT,    3074, false },
        { PREFIX_REPLY,   3934, true  },
        { PREFIX_PARTNER, 8355, false },
    };
    for (auto& p : kPrefixes) {
        const char* src = g_DCTTextManager.GetText(p.textId);
        if (src) {
            std::strncpy(m_szCmdPrefix[p.idx], src, sizeof(m_szCmdPrefix[p.idx]) - 1);
            m_szCmdPrefix[p.idx][sizeof(m_szCmdPrefix[p.idx]) - 1] = '\0';
        }
        if (p.trailingSpace) {
            size_t len = std::strlen(m_szCmdPrefix[p.idx]);
            if (len + 1 < sizeof(m_szCmdPrefix[p.idx])) {
                m_szCmdPrefix[p.idx][len]     = ' ';
                m_szCmdPrefix[p.idx][len + 1] = '\0';
            }
        }
    }
}

//----- (004F6E50) -----------------------------------------------------------
void cltChattingMgr::DrawBlockBox() {
    if (dword_6E17B0) {
        g_Block.PrepareDrawing();
        g_Block.Draw();
    }
}

//----- (004F6E70) -----------------------------------------------------------
void cltChattingMgr::DeleteBlockBox() {
    g_Block.ClearData();
}

//----- (004F6E80) -----------------------------------------------------------
void cltChattingMgr::SetBlockShow(int show) {
    if (show == 1) g_Block.Show();
    else           g_Block.Hide();
}

//----- (004F6EA0) -----------------------------------------------------------
void cltChattingMgr::SetBlockBox(int x, int y, std::uint16_t w, std::uint16_t h_, std::uint8_t alpha) {
    g_Block.SetAttr(x, y, w, h_, 1.0f, 0.0f, 0.0f, 0.0f);
    g_Block.SetAlpha(alpha);
    g_Block.Create(nullptr);
}

//----- (004F6F00) -----------------------------------------------------------
void cltChattingMgr::SetChatWritedString(char* text) {
    if (!text || !std::strlen(text)) return;

    if (m_bWritedPicked == 1 && std::strcmp(text, m_szWritedHistory[m_nWritedCurrent]) == 0) {
        // Same as the currently-picked history entry — rewind the up/down
        // cursor to the real "most recent" position.
        m_bWritedPicked = 0;
        m_nWritedCurrent = m_nWritedMax;
        return;
    }

    // Drop the oldest entry when the ring is full, shifting entries 1..N into
    // slots 0..N-1.
    if (s_WritedCount >= 5) {
        s_WritedCount = 4;
        for (int i = 0; i < s_WritedCount; ++i) {
            std::memset(m_szWritedHistory[i], 0, sizeof(m_szWritedHistory[i]));
            std::strncpy(m_szWritedHistory[i], m_szWritedHistory[i + 1],
                         sizeof(m_szWritedHistory[i]) - 1);
            m_szWritedHistory[i][sizeof(m_szWritedHistory[i]) - 1] = '\0';
        }
    }

    m_bWritedPicked = 0;
    m_nWritedMax     = s_WritedCount;
    m_nWritedCurrent = s_WritedCount;
    std::memset(m_szWritedHistory[s_WritedCount], 0, sizeof(m_szWritedHistory[s_WritedCount]));
    std::strncpy(m_szWritedHistory[s_WritedCount], text,
                 sizeof(m_szWritedHistory[s_WritedCount]) - 1);
    m_szWritedHistory[s_WritedCount][sizeof(m_szWritedHistory[s_WritedCount]) - 1] = '\0';
    ++s_WritedCount;
}

//----- (004F7060) -----------------------------------------------------------
int cltChattingMgr::GetChatWritedString(int direction, char* out) {
    if (!out) return 0;
    if (!std::strlen(m_szWritedHistory[m_nWritedCurrent])) return 0;

    const int nation = static_cast<std::uint8_t>(g_MoFFont.GetNationCode());
    if (nation == 3 || nation == 5) {
        // Certain locales always pull the most recent non-empty entry.
        int idx = 4;
        while (!std::strlen(m_szWritedHistory[idx])) {
            if (--idx <= -1) return 0;
        }
        std::strcpy(out, m_szWritedHistory[idx]);
        return 1;
    }

    if (m_bWritedPicked == 1) {
        if (direction) {
            if (--m_nWritedCurrent < 0) m_nWritedCurrent = 0;
        } else {
            if (++m_nWritedCurrent > m_nWritedMax) m_nWritedCurrent = m_nWritedMax;
        }
    }
    m_bWritedPicked = 1;
    std::strcpy(out, m_szWritedHistory[m_nWritedCurrent]);
    return 1;
}

//----- (004F71A0) -----------------------------------------------------------
void cltChattingMgr::Poll() {
    // Ground truth outer gate (byte-for-byte):
    //   if ( *((_DWORD *)this + 6702)               -- m_bChatEnabled
    //     && g_dwMainGameState == 10
    //     && !*((_DWORD *)m_pclMyChatData + 19) )   -- cltMyCharData + 76
    //   { ... }
    // cltMyCharData is opaque so the +76 DWORD is read via raw offset.
    auto isChatBlocked = [](const cltMyCharData* data) -> bool {
        if (!data) return true;
        return *reinterpret_cast<const std::uint32_t*>(
                   reinterpret_cast<const char*>(data) + 76) != 0;
    };

    if (!m_bChatEnabled || g_dwMainGameState != 10 ||
        isChatBlocked(m_pclMyChatData)) {
        return;
    }

    // Process pending Enter-press only when one is queued; SendInputChat below
    // still runs every frame so the cursor keeps blinking.
    if (m_bChatPostReady) {
        if (m_ChatState) {
            if (m_pDCTIMMList && m_pDCTIMMList->IsActive(m_nIMEIndex) == 1) {
                char rawText[1026]{};
                m_pDCTIMMList->SetAutoDelete(m_nIMEIndex, 1);
                m_pDCTIMMList->GetIMMText(m_nIMEIndex, rawText, m_nIMELength);
                m_pDCTIMMList->SetAutoDelete(m_nIMEIndex, 0);
                SetChatState(0);

                char normalized[1026]{};
                std::strcpy(normalized, rawText);

                if (DispatchChatOrderCheck(normalized) && rawText[0] == '/') {
                    // Slash-command path: run every token except the target name
                    // through the abuse filter, then reassemble.
                    char tokens[1026]{};
                    char piece[1026]{};
                    char work[1026]{};
                    std::strcpy(work, rawText);

                    char* tok = std::strtok(work, " ");
                    int idx = 0;
                    while (tok && idx < 5) {
                        std::memset(piece, 0, sizeof(piece));
                        if (idx == 1) {
                            std::strcpy(piece, tok);   // target name — keep raw
                        } else {
                            g_DCTAbuseWordManager.ChangeString(tok, piece);
                        }
                        std::strcat(piece, " ");
                        std::strcat(tokens, piece);
                        const char* delim = (idx++ >= 2) ? "" : " ";
                        tok = std::strtok(nullptr, delim);
                    }
                    std::strcpy(normalized, tokens);
                    std::strcpy(rawText, normalized);
                } else {
                    g_DCTAbuseWordManager.ChangeString(rawText, normalized);
                    std::strcpy(rawText, normalized);
                }

                SetChatWritedString(rawText);
                if (IsValidChat(rawText)) {
                    bool slash = (rawText[0] == '/');
                    if (!slash) {
                        // m_bChatPostReady acts as the channel filter here: if the
                        // user pressed Enter in /P, /C or /R mode, rewrite the
                        // message in place to the corresponding slash command.
                        SetAutoChatMode(rawText, m_bChatPostReady);
                        slash = (rawText[0] == '/');
                    }

                    if (slash) {
                        if (!DispatchChatOrder(rawText + 1)) {
                            char* err = g_DCTTextManager.GetText(58064);
                            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, err, 0, 0, 0);
                            if (m_pNetwork) m_pNetwork->SysCommand(rawText);
                        }
                    } else {
                        // mofclient.c: when the basic UI's "remember last chat"
                        // flag at offset 449284 is 1, snapshot the outgoing text
                        // into this+79944 (m_szLastChatSent).  CUIBasic is not
                        // fully restored, so reach the flag via raw offset.
                        CUIBase* pUIWindow = g_UIMgr ? g_UIMgr->GetUIWindow(0) : nullptr;
                        if (pUIWindow &&
                            *reinterpret_cast<const std::uint32_t*>(
                                reinterpret_cast<const char*>(pUIWindow) + 449284) == 1) {
                            std::strncpy(m_szLastChatSent, rawText,
                                         sizeof(m_szLastChatSent) - 1);
                            m_szLastChatSent[sizeof(m_szLastChatSent) - 1] = '\0';
                        }
                        SendChattingMsg(rawText);
                    }
                }
            } else if (m_pDCTIMMList) {
                m_pDCTIMMList->SetActive(m_nIMEIndex, 1, g_hWnd);
            }
        } else {
            SetChatState(1);
        }
        m_bChatPostReady = 0;
    }

    // SendInputChat runs every poll while the gate is open (drives cursor blink).
    SendInputChat();
}

//----- (004F75F0) -----------------------------------------------------------
void cltChattingMgr::SetIMELength(int length) {
    if (m_nIMELength != length) {
        m_nIMELength = length;
        g_IMMList.SetTextSize(m_nIMEIndex, length, 0);
    }
}

//----- (004F7620) -----------------------------------------------------------
void cltChattingMgr::SendInputChat() {
    CUIBasic* pUI = reinterpret_cast<CUIBasic*>(g_UIMgr ? g_UIMgr->GetUserInterface(0, 0) : nullptr);

    if (!m_ChatState) {
        if (pUI) pUI->SetInputChat(0, 0, nullptr, 0, nullptr);
        return;
    }

    char source[1024]{};
    char destination[1024]{};
    DWORD now = timeGetTime();

    if (m_pDCTIMMList) {
        m_pDCTIMMList->GetIMMText(m_nIMEIndex, source, 1024);
    }
    size_t srcLen = std::strlen(source);
    int editPos = m_pDCTIMMList ? m_pDCTIMMList->GetEditPosition(m_nIMEIndex) : 0;

    if (static_cast<int>(srcLen) == editPos) {
        std::strcpy(destination, source);
    } else if (static_cast<int>(srcLen) > editPos) {
        std::strncpy(destination, source, editPos);
    }

    if (now - m_dwLastTime > 0x12Cu) {
        m_dwLastTime = now;
        m_nCursorBlink = (m_nCursorBlink != 1);
    }

    if (pUI) pUI->SetInputChat(0, 0, source, m_nCursorBlink, destination);

    const int nation = static_cast<std::uint8_t>(g_MoFFont.GetNationCode());
    if (nation != 2 && nation != 3) return;

    g_Block.Hide();
    if (!m_pDCTIMMList) return;
    if (!m_pDCTIMMList->GetBegineComposition() && !m_pDCTIMMList->GetCandidate()) return;

    int underlineStart = 0;
    int underlineEnd   = 0;
    m_pDCTIMMList->GetUnderLine(m_nIMEIndex, &underlineStart, &underlineEnd);
    m_pDCTIMMList->GetEditPosition(m_nIMEIndex);
    underlineEnd   *= 2;
    underlineStart *= 2;

    char composition[256]{};
    const char* ctx = m_pDCTIMMList->GetCompositionString(m_nIMEIndex);
    if (ctx) wsprintfA(composition, "%s", ctx);

    // Strip the composition tail from the source copy to get the "stable" prefix.
    char preComp[256]{};
    size_t preLen = 0;
    if (srcLen >= std::strlen(composition)) preLen = srcLen - std::strlen(composition);
    if (preLen > sizeof(preComp) - 1) preLen = sizeof(preComp) - 1;
    std::memcpy(preComp, source, preLen);
    preComp[preLen] = '\0';

    // Slice out the selected portion of the composition (between underlines).
    char selected[256]{};
    int selLen = underlineEnd - underlineStart;
    if (selLen < 0) selLen = 0;
    if (selLen > static_cast<int>(sizeof(selected) - 1)) selLen = sizeof(selected) - 1;
    if (underlineStart >= 0 && underlineStart + selLen <= static_cast<int>(sizeof(composition))) {
        std::memcpy(selected, composition + underlineStart, selLen);
    }
    selected[selLen] = '\0';

    // Rebuild the "live" string: stable prefix + cursor-relative slice.
    int cursorOff = 2 * m_pDCTIMMList->GetComCursorPos(m_nIMEIndex);
    char live[512]{};
    wsprintfA(live, "%s", preComp);
    size_t liveLen = std::strlen(live);
    if (cursorOff < 0) cursorOff = 0;
    if (liveLen + cursorOff < sizeof(live)) {
        std::memcpy(live + liveLen, composition, cursorOff);
        live[liveLen + cursorOff] = '\0';
    }

    if (pUI) pUI->SetInputChat(0, 0, source, m_nCursorBlink, live);

    if (std::strcmp(selected, "") == 0) {
        g_Block.Hide();
        return;
    }

    // Measure the pieces and place the highlight block.
    char preCompLead[256]{};
    int leadLen = underlineStart;
    if (leadLen < 0) leadLen = 0;
    if (leadLen > static_cast<int>(sizeof(preCompLead) - 1)) leadLen = sizeof(preCompLead) - 1;
    std::memcpy(preCompLead, composition, leadLen);
    preCompLead[leadLen] = '\0';

    HDC dc = hdc;
    HGDIOBJ prev = SelectObject(dc, h);

    int preW = 0, preH = 0;
    int leadW = 0, leadH = 0;
    int selW = 0, selH = 0;
    g_MoFFont.GetTextLength(&preW, &preH, "ChatBallonText", preComp);
    g_MoFFont.GetTextLength(&leadW, &leadH, "ChatBallonText", preCompLead);
    g_MoFFont.GetTextLength(&selW, &selH, "ChatBallonText", selected);

    SetBlockBox(preW + leadW + 26, 535, static_cast<std::uint16_t>(selW), 2u, 0xFFu);
    g_Block.Show();
    SelectObject(dc, prev);
}

//----- (004F7B20) -----------------------------------------------------------
void cltChattingMgr::SetChatBlock(int baseX, int baseY) {
    int selLen   = m_pDCTIMMList ? static_cast<int>(m_pDCTIMMList->GetSelectedBlockLen(m_nIMEIndex)) : 0;
    int blockPos = m_pDCTIMMList ? m_pDCTIMMList->GetBlockStartPos(m_nIMEIndex) : 0;

    if (selLen <= 0) {
        SetBlockShow(0);
        return;
    }
    if (m_pDCTIMMList) m_pDCTIMMList->SetHWND(m_nIMEIndex, g_hWnd);
    SetBlockShow(1);
    SetBlockBox(baseX + 6 * blockPos, baseY,
                static_cast<std::uint16_t>(6 * selLen), 0xCu, 0x50u);
}

//----- (004F7BB0) -----------------------------------------------------------
int cltChattingMgr::IsValidChat(char* text) {
    if (!text) return 0;
    size_t len = std::strlen(text);
    if (len == 0) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (text[i] != ' ') return 1;
    }
    return 0;
}

//----- (004F7BF0) -----------------------------------------------------------
void cltChattingMgr::SetChatState(int state) {
    // Ground truth gate (mofclient.c 0x4F7BF0):
    //   if ( ClientCharacterManager::CanNotChatting(m_pClientCharMgr) ) {
    //       v3 = DCTTextManager::GetText(58217);
    //       cltSystemMessage::SetSystemMessage(..., v3, 0, 0, 0);
    //       return;
    //   }
    // CanNotChatting is a thin wrapper that returns
    // *((_DWORD*)g_clMyCharData + 26)  (i.e. byte offset 104).  cltMyCharData
    // is opaque in this restoration, so read the flag via raw offset.
    const std::uint32_t banFlag = *reinterpret_cast<const std::uint32_t*>(
        reinterpret_cast<const char*>(&g_clMyCharData) + 104);
    if (banFlag) {
        const char* msg = g_DCTTextManager.GetText(58217);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
        return;
    }

    m_ChatState = state;
    if (!m_pDCTIMMList) return;

    const int idx = m_nIMEIndex;
    if (state) {
        m_pDCTIMMList->ChangeLanguage(idx, 1, g_hWnd);
        m_pDCTIMMList->SetActive(idx, 1, g_hWnd);
        char empty[1] = {0};
        m_pDCTIMMList->SetIMMText(idx, empty);
    } else {
        m_pDCTIMMList->SetActive(idx, 0, g_hWnd);
        char empty[1] = {0};
        m_pDCTIMMList->SetIMMText(idx, empty);
        m_pDCTIMMList->ChangeLanguage(idx, 0, g_hWnd);
    }
}

//----- (004F7CE0) -----------------------------------------------------------
int cltChattingMgr::GetChatState() { return m_ChatState; }

//----- (004F7CF0) -----------------------------------------------------------
void cltChattingMgr::SetSysCommand(char* text) {
    SetChatState(1);
    if (m_pDCTIMMList) {
        m_pDCTIMMList->SetActive(m_nIMEIndex, 1, g_hWnd);
        m_pDCTIMMList->SetIMMText(m_nIMEIndex, text);
    }
}

//----- (004F7D30) -----------------------------------------------------------
void cltChattingMgr::SetAutoChatMode(char* text, std::uint8_t filter) {
    if (!text) return;
    char buffer[1024]{};
    const char* prefix = nullptr;
    switch (filter) {
        case 1: return;               // "all chat" — leave message as-is
        case 2: prefix = "/P "; break;
        case 5: prefix = "/C "; break;
        case 7: prefix = "/R "; break;
        default: break;
    }
    if (prefix) std::strcpy(buffer, prefix);
    std::strncat(buffer, text, sizeof(buffer) - std::strlen(buffer) - 1);
    std::strcpy(text, buffer);
}

// ---------------------------------------------------------------------------
// Chat buffer inserts (two overloads, both feed into SendChatting).
// ---------------------------------------------------------------------------

void cltChattingMgr::ShiftChatBufferDown(ChatEntry* arr, int count) {
    // mofclient.c moves entries 0..count-1 → 1..count before writing a fresh
    // one at slot 0 (newest-at-top layout).
    for (int i = count; i > 0; --i) {
        std::memmove(&arr[i], &arr[i - 1], sizeof(ChatEntry));
    }
}

//----- (004F7E20) -----------------------------------------------------------
void cltChattingMgr::SetChatBuffer(std::uint16_t channel, unsigned int accountId,
                                   char* message, std::uint16_t emoticonId, char* extra) {
    char nameField[128] = "";
    nameField[0] = '\0';
    std::uint8_t filterCode = 1;
    std::uint32_t color = 1;

    if (m_nChatBufferCount == 100) m_nChatBufferCount = 99;
    ShiftChatBufferDown(m_ChatBuffer, m_nChatBufferCount);

    switch (channel) {
        case 1u: {
            // mofclient.c: strcpy(&v18, ClientCharacterManager::GetCharName(m_pClientCharMgr, a3));
            const char* name = m_pClientCharMgr ? m_pClientCharMgr->GetCharName(accountId) : "";
            std::strncpy(nameField, name ? name : "", sizeof(nameField) - 1);
            nameField[sizeof(nameField) - 1] = '\0';
            filterCode = 1;
            color = static_cast<std::uint32_t>(-1);
            break;
        }
        case 2u: {
            const char* partyLabel = g_DCTTextManager.GetText(3661);
            auto* member = g_clPartySystem.GetPartyMemberInstance(accountId);
            const char* memberName = member ? member->szName : "";
            wsprintfA(nameField, "[ %s : %s ]",
                      partyLabel ? partyLabel : "", memberName);
            color = static_cast<std::uint32_t>(-16711681);
            filterCode = 2;
            break;
        }
        case 4u: {
            const char* whisperLabel = g_DCTTextManager.GetText(3076);
            wsprintfA(nameField, "[ %s  : %s ]",
                      whisperLabel ? whisperLabel : "", extra ? extra : "");
            color = static_cast<std::uint32_t>(-16711936);
            filterCode = 4;
            break;
        }
        case 5u: {
            const char* circleLabel = g_DCTTextManager.GetText(3382);
            wsprintfA(nameField, "[ %s : %s ]",
                      circleLabel ? circleLabel : "", extra ? extra : "");
            color = static_cast<std::uint32_t>(-16711936);
            filterCode = 5;
            break;
        }
        case 6u:
            std::strncpy(nameField, extra ? extra : "", sizeof(nameField) - 1);
            color = static_cast<std::uint32_t>(-19456);
            break;
        default:
            color = 1;
            break;
    }

    ChatEntry& slot = m_ChatBuffer[0];
    std::memset(slot.text, 0, sizeof(slot.text));
    wsprintfA(slot.text, "%s : %s", nameField, message ? message : "");
    slot.color  = color;
    slot.filter = filterCode;
    ++m_nChatBufferCount;

    SendChatting();
    FindEmoticonWord(accountId, message, emoticonId);
}

//----- (004F8070) -----------------------------------------------------------
void cltChattingMgr::SetChatBuffer(std::uint16_t channel, char* name, char* message,
                                   std::uint16_t emoticonId) {
    char nameField[128] = "";
    std::uint8_t filterCode = 1;
    std::uint32_t color = 1;

    if (channel != 8) {
        // Scrub profanity from the message before it is stored or forwarded
        // to the trade-window mirror.
        char filtered[1024]{};
        if (message) {
            g_DCTAbuseWordManager.ChangeString(message, filtered);
            std::strcpy(message, filtered);
        }
        SetChatBufferByTradeFilter(name, message);
    }

    if (m_nChatBufferCount == 100) m_nChatBufferCount = 99;
    ShiftChatBufferDown(m_ChatBuffer, m_nChatBufferCount);

    switch (channel) {
        case 1u:
            std::strncpy(nameField, name ? name : "", sizeof(nameField) - 1);
            color = static_cast<std::uint32_t>(-1);
            filterCode = 1;
            break;
        case 2u: {
            const char* partyLabel = g_DCTTextManager.GetText(3661);
            wsprintfA(nameField, "[ %s : %s ]",
                      partyLabel ? partyLabel : "", name ? name : "");
            filterCode = 2;
            color = static_cast<std::uint32_t>(-16711681);
            break;
        }
        case 3u: {
            const char* whisperLabel = g_DCTTextManager.GetText(3076);
            wsprintfA(nameField, "[ %s > %s ]",
                      whisperLabel ? whisperLabel : "", name ? name : "");
            filterCode = 4;
            std::strncpy(m_szLastWhisperName, name ? name : "",
                         sizeof(m_szLastWhisperName) - 1);
            color = static_cast<std::uint32_t>(-16711936);
            break;
        }
        case 4u: {
            const char* whisperLabel = g_DCTTextManager.GetText(3076);
            wsprintfA(nameField, "[ %s < %s ]",
                      whisperLabel ? whisperLabel : "", name ? name : "");
            std::strncpy(m_szLastWhisperName, name ? name : "",
                         sizeof(m_szLastWhisperName) - 1);
            filterCode = 4;
            color = static_cast<std::uint32_t>(-16711936);
            CUIBasic* pUI = reinterpret_cast<CUIBasic*>(
                g_UIMgr ? g_UIMgr->GetUserInterface(0, 0) : nullptr);
            if (pUI) pUI->ReceivedWhisper(name);
            break;
        }
        case 5u: {
            const char* circleLabel = g_DCTTextManager.GetText(3382);
            wsprintfA(nameField, "[ %s : %s ]",
                      circleLabel ? circleLabel : "", name ? name : "");
            filterCode = 5;
            color = static_cast<std::uint32_t>(-196864);
            break;
        }
        case 6u:
            std::strncpy(nameField, name ? name : "", sizeof(nameField) - 1);
            color = static_cast<std::uint32_t>(-19456);
            break;
        case 8u:
            filterCode = 8;
            color = static_cast<std::uint32_t>(-256);
            break;
        default:
            color = 1;
            break;
    }

    // PK / war opponent bubble — override colour to bright blue.
    // mofclient.c:
    //   v16 = GetCharByName(&g_ClientCharMgr, String2);
    //   if ( v16 && *((_DWORD *)v16 + 2885) )
    //       v10 = -16776961;
    // ClientCharacter does not expose the offset-11540 flag through a named
    // member, so read it via raw offset like the other call sites do.
    if (name) {
        ClientCharacter* target = g_ClientCharMgr.GetCharByName(name);
        if (target && *reinterpret_cast<const std::uint32_t*>(
                          reinterpret_cast<const char*>(target) + 11540)) {
            color = static_cast<std::uint32_t>(-16776961);
        }
    }

    ChatEntry& slot = m_ChatBuffer[0];
    std::memset(slot.text, 0, sizeof(slot.text));
    if (filterCode == 8) {
        wsprintfA(slot.text, "%s", message ? message : "");
    } else {
        wsprintfA(slot.text, "%s : %s", nameField, message ? message : "");
    }
    slot.filter = filterCode;
    slot.color  = color;
    ++m_nChatBufferCount;

    SendChatting();

    if (channel == 3) {
        char* myName = m_pClientCharMgr ? m_pClientCharMgr->GetMyCharName() : nullptr;
        FindEmoticonWord(myName, message, emoticonId);
    } else {
        FindEmoticonWord(name, message, emoticonId);
    }
}

//----- (004F8430) -----------------------------------------------------------
void cltChattingMgr::SetChatFilter(std::uint8_t filter) {
    if (m_cChatFilter != filter) {
        m_cChatFilter = filter;
        SendChatting();
    }
}

//----- (004F8450) -----------------------------------------------------------
void cltChattingMgr::SendChatting() {
    std::memset(m_szChatDisplayBuffer, 0, sizeof(m_szChatDisplayBuffer));
    int writeOffset = 0;
    int idx = m_nChatBufferCount - 1;

    while (idx >= 0) {
        const ChatEntry& entry = m_ChatBuffer[idx];
        std::uint8_t filter = m_cChatFilter;
        if (filter == 1 || entry.filter == filter) {
            const char* dst = m_szChatDisplayBuffer + writeOffset;
            size_t written;
            if (idx <= 0) {
                std::sprintf(m_szChatDisplayBuffer + writeOffset, "%u:%s",
                             entry.color, entry.text);
                written = std::strlen(dst);
                writeOffset += static_cast<int>(written) + 1;
            } else {
                std::sprintf(m_szChatDisplayBuffer + writeOffset, "%u:%s\n",
                             entry.color, entry.text);
                written = std::strlen(dst);
                // Overlap-write the next entry on top of the null terminator
                // so the buffer ends up as a single multi-line "\n"-separated
                // string (mofclient.c intentional behaviour).
                writeOffset += static_cast<int>(written);
            }
        }
        --idx;
    }

    CUIBasic* pUI = reinterpret_cast<CUIBasic*>(
        g_UIMgr ? g_UIMgr->GetUserInterface(0, 0) : nullptr);
    if (pUI) pUI->SetChat(m_szChatDisplayBuffer);
}

//----- (004F8530) -----------------------------------------------------------
void cltChattingMgr::StartTradeFilterChat(char* nameA, char* nameB) {
    m_bTradeFilterMode = 1;
    std::strncpy(m_szTradePartnerA, nameA ? nameA : "", sizeof(m_szTradePartnerA) - 1);
    m_szTradePartnerA[sizeof(m_szTradePartnerA) - 1] = '\0';
    std::strncpy(m_szTradePartnerB, nameB ? nameB : "", sizeof(m_szTradePartnerB) - 1);
    m_szTradePartnerB[sizeof(m_szTradePartnerB) - 1] = '\0';
    std::memset(m_TradeFilterBuffer, 0, sizeof(m_TradeFilterBuffer));
    m_nTradeFilterCount = 0;
}

//----- (004F85B0) -----------------------------------------------------------
void cltChattingMgr::EndTradeFilterChat() {
    m_bTradeFilterMode = 0;
}

//----- (004F85C0) -----------------------------------------------------------
void cltChattingMgr::SetChatBufferByTradeFilter(char* name, char* message) {
    if (!m_bTradeFilterMode) return;
    if (!name) return;
    if (std::strcmp(name, m_szTradePartnerA) != 0 &&
        std::strcmp(name, m_szTradePartnerB) != 0) {
        return;
    }

    if (m_nTradeFilterCount == 100) m_nTradeFilterCount = 99;
    ShiftChatBufferDown(m_TradeFilterBuffer, m_nTradeFilterCount);

    ChatEntry& slot = m_TradeFilterBuffer[0];
    std::memset(slot.text, 0, sizeof(slot.text));
    wsprintfA(slot.text, "%s : %s", name, message ? message : "");
    ++m_nTradeFilterCount;

    std::unique_ptr<char[]> out(new char[25600]());
    int offset = 0;
    for (int i = m_nTradeFilterCount - 1; i >= 0; --i) {
        const ChatEntry& entry = m_TradeFilterBuffer[i];
        const char* fmt = (i > 0) ? "%s\n" : "%s";
        std::sprintf(out.get() + offset, fmt, entry.text);
        offset += static_cast<int>(std::strlen(entry.text) + 1);
    }

    CUITradeUser* pTrade = reinterpret_cast<CUITradeUser*>(
        g_UIMgr ? g_UIMgr->GetUserInterface(16, 0) : nullptr);
    if (pTrade) pTrade->SetChat(out.get());
}

//----- (004F8770) -----------------------------------------------------------
void cltChattingMgr::SetTradeFilterChatBufferIndex() {
    const char* parsed = g_DCTTextManager.GetParsedText(4341, 0, nullptr);
    ChatEntry& slot = m_TradeFilterBuffer[0];
    std::memset(slot.text, 0, sizeof(slot.text));
    wsprintfA(slot.text, "%s", parsed ? parsed : "");
    ++m_nTradeFilterCount;

    CUITradeUser* pTrade = reinterpret_cast<CUITradeUser*>(
        g_UIMgr ? g_UIMgr->GetUserInterface(16, 0) : nullptr);
    if (pTrade) pTrade->SetChat(slot.text);
}

//----- (004F87D0) -----------------------------------------------------------
void cltChattingMgr::InitChatMgr() {
    m_ChatState = 0;
    m_cChatFilter = 1;
    m_nChatBufferCount = 0;
    std::memset(m_ChatBuffer, 0, sizeof(m_ChatBuffer));
    std::memset(m_szChatDisplayBuffer, 0, sizeof(m_szChatDisplayBuffer));

    CUIBasic* pUI = reinterpret_cast<CUIBasic*>(
        g_UIMgr ? g_UIMgr->GetUserInterface(0, 0) : nullptr);
    if (pUI) pUI->SetChat(nullptr);

    std::memset(m_TradeFilterBuffer, 0, sizeof(m_TradeFilterBuffer));
    std::memset(m_szTradePartnerA, 0, sizeof(m_szTradePartnerA));
    std::memset(m_szTradePartnerB, 0, sizeof(m_szTradePartnerB));
    m_bTradeFilterMode = 0;
    m_nTradeFilterCount = 0;
}

//----- (004F8860) -----------------------------------------------------------
void cltChattingMgr::CompleteWhisper(std::uint8_t code, char* targetName, char* message) {
    char buffer[256]{};
    switch (code) {
        case 0u: {
            const char* fmt = g_DCTTextManager.GetText(3289);
            const char* name = targetName ? targetName : "NULL";
            std::sprintf(buffer, fmt ? fmt : "%s", name);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buffer, 0, 0, 0);
            break;
        }
        case 1u:
            SetChatBuffer(3u, targetName, message, 0);
            g_clQuestSystem.CompleteFunctionQuest(12);
            break;
        case 2u:
            if (targetName) {
                const char* fmt = g_DCTTextManager.GetText(3410);
                std::sprintf(buffer, fmt ? fmt : "%s", targetName);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buffer, 0, 0, 0);
            }
            break;
        case 3u:
            if (targetName) {
                const char* fmt = g_DCTTextManager.GetText(58055);
                std::sprintf(buffer, fmt ? fmt : "%s", targetName);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buffer, 0, 0, 0);
            }
            break;
        default:
            return;
    }
}

// Helper: case-insensitive prefix match returning new position after the match.
static inline bool StartsWithI(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    size_t plen = std::strlen(prefix);
    if (plen == 0) return false;
    return _strnicmp(s, prefix, plen) == 0;
}
static inline bool StartsWith(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    size_t plen = std::strlen(prefix);
    if (plen == 0) return false;
    return std::strncmp(s, prefix, plen) == 0;
}

//----- (004F8970) -----------------------------------------------------------
int cltChattingMgr::DispatchChatOrder(char* text) {
    if (!text) return 0;
    ClientCharacter* me = m_pClientCharMgr ? m_pClientCharMgr->GetMyCharacterPtr() : nullptr;
    if (!me) return 0;

    // Ground truth: skip the IsValidCharName check when the GM / special-auth
    // flag at ClientCharacter + 11540 (= *((_DWORD*)me + 2885)) is non-zero.
    // ClientCharacter doesn't expose the field so we read it via raw offset.
    const std::uint32_t gmBypass =
        *reinterpret_cast<const std::uint32_t*>(
            reinterpret_cast<const char*>(me) + 11540);
    if (!gmBypass && !IsValidCharName(text)) {
        const char* err = g_DCTTextManager.GetText(4745);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, err, 0, 0, 0);
        return 1;
    }

    auto try_prefix = [&](const char* english, int localIdx, void (cltChattingMgr::*handler)(char*)) -> bool {
        if (StartsWithI(text, english)) {
            (this->*handler)(text + std::strlen(english));
            return true;
        }
        if (StartsWith(text, m_szCmdPrefix[localIdx])) {
            (this->*handler)(text + std::strlen(m_szCmdPrefix[localIdx]));
            return true;
        }
        return false;
    };

    if (try_prefix("P ",      PREFIX_PARTY,   &cltChattingMgr::DispatchChatOrder_PartyChat))    return 1;
    if (try_prefix("W ",      PREFIX_WHISPER, &cltChattingMgr::DispatchChatOrder_Whisper))      return 1;
    if (try_prefix("invite ", PREFIX_INVITE,  &cltChattingMgr::DispatchChatOrder_JoinParty))    return 1;
    if (try_prefix("ban ",    PREFIX_BAN,     &cltChattingMgr::DispatchChatOrder_KickoutParty)) return 1;
    if (try_prefix("user ",   PREFIX_USER,    &cltChattingMgr::DispatchChatOrder_DetailCharInfo)) return 1;
    if (try_prefix("?",       PREFIX_HELP,    &cltChattingMgr::DispatchChatOrder_Help))         return 1;
    if (try_prefix("CI ",     PREFIX_CJOIN,   &cltChattingMgr::DispatchChatOrder_JoinCircle))   return 1;
    if (try_prefix("CB ",     PREFIX_CKICK,   &cltChattingMgr::DispatchChatOrder_KickoutCircle)) return 1;
    if (try_prefix("CE",      PREFIX_COUT,    &cltChattingMgr::DispatchChatOrder_OutCircle))    return 1;
    if (try_prefix("PE",      PREFIX_POUT,    &cltChattingMgr::DispatchChatOrder_OutParty))     return 1;
    if (try_prefix("C ",      PREFIX_CIRCLE,  &cltChattingMgr::DispatchChatOrder_CircleChat))   return 1;
    if (try_prefix("R ",      PREFIX_REPLY,   &cltChattingMgr::DispatchChatOrder_ResWhisper))   return 1;
    if (try_prefix("honey",   PREFIX_PARTNER, &cltChattingMgr::DispatchChatOrder_PartnerRecall)) return 1;
    return 0;
}

//----- (004F9080) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_PartyChat(char* text) {
    if (g_clPartySystem.IsCreated()) {
        if (IsValidChat(text) && m_pNetwork) {
            m_pNetwork->PartyChatting(text);
        }
    } else {
        const char* msg = g_DCTTextManager.GetText(58058);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
    }
}

//----- (004F90E0) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_JoinParty(char* text) {
    // Ground truth: strtok mutates the input directly.  Callers pass a local
    // stack buffer, so the mutation is self-contained.
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        g_pInterfaceDataCommunity->ChatOrderPartyInvite(name);
    }
}

//----- (004F9150) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_KickoutParty(char* text) {
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        g_pInterfaceDataCommunity->ChatOrderPartyKickOut(name);
    }
}

//----- (004F91C0) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_Whisper(char* text) {
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        // Remainder of `text` starts right after the space we just replaced
        // (ground truth: &text[strlen(tok) + 1]).
        SendWhisper(name, &text[std::strlen(tok) + 1]);
    }
}

//----- (004F9250) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_ResWhisper(char* text) {
    if (std::strlen(m_szLastWhisperName) > 0) {
        SendWhisper(m_szLastWhisperName, text);
    } else {
        const char* msg = g_DCTTextManager.GetText(58056);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
    }
}

//----- (004F92A0) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_PartnerRecall(char* /*text*/) {
    cltMarriageSystem* marriage = g_clMarriageSystem;
    if (!marriage || !marriage->IsMarried()) {
        const char* msg = g_DCTTextManager.GetText(8270);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
        return;
    }
    if (marriage->CanRecallSpouse()) {
        const char* msg = g_DCTTextManager.GetText(8226);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
        return;
    }
    char* spouse = marriage->GetSpouseName();
    if (g_ClientCharMgr.GetCharByName(spouse)) {
        const char* msg = g_DCTTextManager.GetText(8344);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
        return;
    }
    g_Network.RequestSpouseRecall();
}

//----- (004F9320) -----------------------------------------------------------
void cltChattingMgr::SendWhisper(char* targetName, char* message) {
    if (!targetName || !message) return;
    const char* myName = m_pClientCharMgr ? m_pClientCharMgr->GetMyCharName() : "";
    if (myName && _stricmp(myName, targetName) == 0) {
        const char* msg = g_DCTTextManager.GetText(58060);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
        return;
    }
    if (IsValidChat(message) && std::strlen(targetName) <= 13u && m_pNetwork) {
        m_pNetwork->Whisper(targetName, message);
    }
}

//----- (004F93A0) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_JoinCircle(char* text) {
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        g_pInterfaceDataCommunity->ChatOrderCircleInvite(name);
    }
}

//----- (004F9410) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_KickoutCircle(char* text) {
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        g_pInterfaceDataCommunity->ChatOrderCircleKickOut(name);
    }
}

//----- (004F9480) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_OutCircle(char* /*text*/) {
    if (g_pInterfaceDataCommunity) g_pInterfaceDataCommunity->ChatOrderCircleOut();
}

//----- (004F9490) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_OutParty(char* /*text*/) {
    if (g_pInterfaceDataCommunity) g_pInterfaceDataCommunity->ChatOrderPartyOut();
}

//----- (004F94A0) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_CircleChat(char* text) {
    CUICircle* pCircle = reinterpret_cast<CUICircle*>(
        g_UIMgr ? g_UIMgr->GetUIWindow(36) : nullptr);
    const char* circleName = pCircle ? pCircle->GetCircleName() : "";
    if (circleName && std::strlen(circleName) > 0) {
        if (IsValidChat(text) && m_pNetwork) m_pNetwork->CircleChat(text);
    } else {
        const char* msg = g_DCTTextManager.GetText(58057);
        cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
    }
}

//----- (004F9520) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_Help(char* /*text*/) {
    // Twelve localised cmd/description pairs followed by the "/honey" pair.
    static const struct { int cmdId; int descId; } kHelpEntries[] = {
        { 4616, 4604 }, { 4617, 4605 }, { 4618, 4606 }, { 4619, 4607 },
        { 4620, 4608 }, { 4621, 4609 }, { 4622, 4610 }, { 4623, 4611 },
        { 4624, 4612 }, { 4625, 4613 }, { 4626, 4614 }, { 4627, 4615 },
        { 8363, 8362 },
    };
    for (auto& e : kHelpEntries) {
        char* cmd  = g_DCTTextManager.GetText(e.cmdId);
        char* desc = g_DCTTextManager.GetText(e.descId);
        SetChatBuffer(6u, desc, cmd, 0);
    }
}

//----- (004F9760) -----------------------------------------------------------
void cltChattingMgr::DispatchChatOrder_DetailCharInfo(char* text) {
    char delim[2] = " ";
    char* tok = std::strtok(text, delim);
    if (tok) {
        char name[128];
        std::strcpy(name, tok);
        g_pInterfaceDataCommunity->OpenUserInfo(name);
    }
}

//----- (004F97D0) -----------------------------------------------------------
void cltChattingMgr::FindEmoticonWord(unsigned int accountId, char* message, std::uint16_t emoticonId) {
    // Ground truth forwards unconditionally; no null checks.
    m_EmoticonSystem.FindEmoticonWord(accountId, message, emoticonId);
}

//----- (004F97F0) -----------------------------------------------------------
void cltChattingMgr::FindEmoticonWord(char* name, char* message, std::uint16_t emoticonId) {
    // Ground truth forwards unconditionally; no null checks.
    m_EmoticonSystem.FindEmoticonWord(name, message, emoticonId);
}

//----- (004F9810) -----------------------------------------------------------
void cltChattingMgr::SendChattingMsg(char* message) {
    // Rate limit: five messages per five seconds.  Timer1 covers the opening
    // "normal" burst; once the user hits five messages we flip to Timer2 for
    // the spam cool-down.
    if (!m_wChatMsgCount && !m_dwTimerHandle1) {
        m_dwTimerHandle1 = CreateRateLimitTimer(this);
    }

    bool rateLimited = false;

    if (m_wChatMsgCount == 4) {
        if (m_dwTimerHandle1) {
            if (g_clTimerManager.GetTimer(m_dwTimerHandle1)) {
                // Burst timer still alive — switch to the spam timer.
                g_clTimerManager.ReleaseTimer(m_dwTimerHandle1);
                m_dwTimerHandle1 = 0;
                m_wChatMsgCount = 4;
                m_dwTimerHandle2 = CreateRateLimitTimer(this);
                rateLimited = true;
            } else {
                // Burst timer expired — reset counters and continue.
                m_wChatMsgCount = 0;
                m_dwTimerHandle1 = CreateRateLimitTimer(this);
            }
        } else if (m_dwTimerHandle2) {
            if (g_clTimerManager.GetTimer(m_dwTimerHandle2)) {
                rateLimited = true;
            } else {
                // Spam timer expired — reset and accept.
                m_wChatMsgCount = 0;
                m_dwTimerHandle1 = CreateRateLimitTimer(this);
            }
        }
    }

    if (rateLimited) {
        char* err = g_DCTTextManager.GetText(5314);
        SetChatBuffer(8u, (char*)"", err, 0);
        return;
    }

    ++m_wChatMsgCount;
    m_EmoticonSystem.SendChattingMsg(message);
}

//----- (004F9960) -----------------------------------------------------------
BOOL cltChattingMgr::DispatchChatOrderCheck(char* text) {
    if (!text || !std::strlen(text)) return 0;
    if (_strnicmp(text, "P ",      2) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_PARTY]))   return 1;
    if (_strnicmp(text, "W ",      2) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_WHISPER])) return 1;
    if (_strnicmp(text, "invite ", 7) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_INVITE]))  return 1;
    if (_strnicmp(text, "ban ",    4) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_BAN]))     return 1;
    if (_strnicmp(text, "user ",   5) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_USER]))    return 1;
    if (_strnicmp(text, "?",       1) == 0) return 1;
    if (StartsWith(text, m_szCmdPrefix[PREFIX_HELP]))    return 1;
    if (_strnicmp(text, "CI ",     3) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_CJOIN],
                  std::strlen(m_szCmdPrefix[PREFIX_CJOIN])) == 0) return 1;
    if (_strnicmp(text, "CB ",     3) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_CKICK],
                  std::strlen(m_szCmdPrefix[PREFIX_CKICK])) == 0) return 1;
    if (_strnicmp(text, "CE",      2) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_COUT],
                  std::strlen(m_szCmdPrefix[PREFIX_COUT])) == 0) return 1;
    if (_strnicmp(text, "PE",      2) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_POUT],
                  std::strlen(m_szCmdPrefix[PREFIX_POUT])) == 0) return 1;
    if (_strnicmp(text, "C ",      2) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_CIRCLE],
                  std::strlen(m_szCmdPrefix[PREFIX_CIRCLE])) == 0) return 1;
    if (_strnicmp(text, "R ",      2) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_REPLY],
                  std::strlen(m_szCmdPrefix[PREFIX_REPLY])) == 0) return 1;
    return 0;
}

//----- (004F9D80) -----------------------------------------------------------
char cltChattingMgr::IsValidCharName(char* text) {
    if (!text) return 0;
    // Commands that do not carry a character name always pass.
    if (_strnicmp(text, "P ",   2) == 0) return 1;
    if (_strnicmp(text, "C ",   2) == 0) return 1;
    if (_strnicmp(text, "R ",   2) == 0) return 1;
    if (_strnicmp(text, "WARP ",5) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_PARTY],
                  std::strlen(m_szCmdPrefix[PREFIX_PARTY])) == 0)  return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_CIRCLE],
                  std::strlen(m_szCmdPrefix[PREFIX_CIRCLE])) == 0) return 1;
    if (_strnicmp(text, m_szCmdPrefix[PREFIX_REPLY],
                  std::strlen(m_szCmdPrefix[PREFIX_REPLY])) == 0)  return 1;

    char tempBuf[512]{};
    std::strncpy(tempBuf, text, sizeof(tempBuf) - 1);
    tempBuf[sizeof(tempBuf) - 1] = '\0';
    size_t fullLen = std::strlen(text);
    bool isWhisperCmd =
        _strnicmp(text, "W ", 2) == 0 ||
        std::strncmp(text, m_szCmdPrefix[PREFIX_WHISPER],
                     std::strlen(m_szCmdPrefix[PREFIX_WHISPER])) == 0;

    if (!isWhisperCmd) {
        char* tok = std::strtok(tempBuf, " ");
        if (!tok) return 0;
        size_t targetLen = fullLen - (std::strlen(tok) + 1);
        return static_cast<char>(targetLen <= 13);
    }

    (void)std::strtok(tempBuf, " ");
    char* name = std::strtok(nullptr, " ");
    if (!name) return 0;
    return static_cast<char>(std::strlen(name) <= 13);
}

//----- (004F9FB0) -----------------------------------------------------------
void cltChattingMgr::SendQuickChatting(std::uint8_t kind, char* text) {
    switch (kind) {
        case 1u: SendChattingMsg(text); break;
        case 2u: DispatchChatOrder_PartyChat(text); break;
        case 4u: SendWhisper(m_szLastWhisperName, text); break;
        case 5u: DispatchChatOrder_CircleChat(text); break;
        default: return;
    }
}

//----- (004FA020) -----------------------------------------------------------
void cltChattingMgr::SetBanChat() {
    m_ChatState = 0;
    if (!m_pDCTIMMList) return;
    const int idx = m_nIMEIndex;
    m_pDCTIMMList->SetActive(idx, 0, g_hWnd);
    char empty[1] = {0};
    m_pDCTIMMList->SetIMMText(idx, empty);
    m_pDCTIMMList->ChangeLanguage(idx, 0, g_hWnd);
}
