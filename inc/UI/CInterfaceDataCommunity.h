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

    // GT 0x4D9FB0 — clears the "wait call" flag (*(DWORD*)this = 0) and
    // re-syncs the friend / pop-up menus.  cltMyCharData::ReactiveCommunity
    // forwards to this method on g_pInterfaceDataCommunity; UI parts that
    // expect to be re-armed are not yet ported, so the body only resets the
    // wait flag for now (logical-equivalent for the gating used by
    // cltChattingMgr/community trade flows).
    void ReactiveCommunity();

    // GT m_iWaitCallFlag (DWORD at offset 0).  Set by WaitCallCommunity, read
    // by IsWaitCallCommunity, cleared by ReactiveCommunity.  Made public so
    // the few external callers in mofclient.c that touch the raw DWORD have
    // a named field to use.
    int m_iWaitCallFlag = 0;
};

extern CInterfaceDataCommunity* g_pInterfaceDataCommunity;
