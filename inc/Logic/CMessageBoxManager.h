#pragma once

class CMessageBoxManager {
public:
    static int AddOK(void* mgr, int textId, int a3, int a4, int a5, int a6);
    static int AddOK(void* mgr, const char* szText, int a3, int a4, int a5, int a6);
};

extern void* g_pMsgBoxMgr;
