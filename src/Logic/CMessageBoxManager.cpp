#include "Logic/CMessageBoxManager.h"

void* g_pMsgBoxMgr = nullptr;

int CMessageBoxManager::AddOK(void* mgr, int textId, int a3, int a4, int a5, int a6) {
    (void)mgr;
    (void)textId;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    return 0;
}

int CMessageBoxManager::AddOK(void* mgr, const char* szText, int a3, int a4, int a5, int a6) {
    (void)mgr;
    (void)szText;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    return 0;
}
