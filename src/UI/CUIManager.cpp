#include "UI/CUIManager.h"
#include "UI/CUIBase.h"
#include "UI/CMessageBoxManager.h"
#include "Util/CLog.h"

#include <cstdio>

int CUIManager::IsCharActionKey(CUIManager* /*pMgr*/, int /*keyId*/) {
    return 0;
}

CUIBase* CUIManager::GetUIWindow(int /*id*/) { return nullptr; }
CUIBase* CUIManager::GetUserInterface(int /*id*/, int /*subId*/) { return nullptr; }
int CUIManager::IsOpenUserInterface(int /*id*/) { return 0; }
int CUIManager::GetGameState() { return 0; }
void CUIManager::AddOrder(CUIMessageBoxBase* /*pBox*/) {}
void CUIManager::DelOrder(CUIBase* /*pBase*/) {}
void CUIManager::DeleteFocusWindow(CUIBase* /*pBase*/) {}

//----- (00433CC0) --------------------------------------------------------
void CUIManager::OpenLogWindow()
{
    // mofclient.c 對應：
    //   v1 = 8;  // 預設 quiet
    //   v2 = fopen("MoFData/log.dat", "rt");
    //   if (v2) { v1 = 7; fclose(v2); }
    //   v3 = (CLog*)operator new(0x114u);
    //   if (v3) v4 = CLog::CLog(v3, v1, 300, 600, "Log.txt");
    //   else    v4 = 0;
    //   logger = v4;
    unsigned int flag = CLog::kQuiet;
    FILE* probe = std::fopen("MoFData/log.dat", "rt");
    if (probe) {
        flag = CLog::kStdout | CLog::kFile | CLog::kWindow;
        std::fclose(probe);
    }
    if (logger) return;  // 已建立過就不再重建（防呆，原 binary 不檢查）
    new CLog(flag, 300, 600, "Log.txt");
    // 注意：CLog::CLog 內會把 Singleton<CLog>::m_singleton（== logger 別名）
    // 設為 this，因此這裡不需要明確賦值。
}
