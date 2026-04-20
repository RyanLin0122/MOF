#pragma once

// Thin wrapper around the community/social UI actions that cltChattingMgr
// dispatches into (party invite/kick/leave, circle invite/kick/leave, open
// player info).  mofclient.c accesses a single global instance via
// g_pInterfaceDataCommunity; in the restored build we provide the same
// symbol.  Each method is a no-op stub — the community UI is not yet
// implemented, and the chat manager only needs the entry points to compile
// and to route command-line chat orders safely.

class CInterfaceDataCommunity {
public:
    CInterfaceDataCommunity() = default;
    ~CInterfaceDataCommunity() = default;

    void ChatOrderPartyInvite(char* name);
    void ChatOrderPartyKickOut(char* name);
    void ChatOrderPartyOut();
    void ChatOrderCircleInvite(char* name);
    void ChatOrderCircleKickOut(char* name);
    void ChatOrderCircleOut();
    void OpenUserInfo(char* name);
};

extern CInterfaceDataCommunity* g_pInterfaceDataCommunity;
