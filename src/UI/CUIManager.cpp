#include "UI/CUIManager.h"
#include "UI/CUIBase.h"
#include "UI/CMessageBoxManager.h"

int CUIManager::IsCharActionKey(CUIManager* /*pMgr*/, int /*keyId*/) {
    return 0;
}

CUIBase* CUIManager::GetUIWindow(int /*id*/) { return nullptr; }
int CUIManager::IsOpenUserInterface(int /*id*/) { return 0; }
int CUIManager::GetGameState() { return 0; }
void CUIManager::AddOrder(CUIMessageBoxBase* /*pBox*/) {}
void CUIManager::DelOrder(CUIBase* /*pBase*/) {}
void CUIManager::DeleteFocusWindow(CUIBase* /*pBase*/) {}
