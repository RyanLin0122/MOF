#include "UI/CInterfaceDataCommunity.h"

void CInterfaceDataCommunity::ChatOrderPartyInvite(char* /*name*/) {}
void CInterfaceDataCommunity::ChatOrderPartyKickOut(char* /*name*/) {}
void CInterfaceDataCommunity::ChatOrderPartyOut() {}
void CInterfaceDataCommunity::ChatOrderCircleInvite(char* /*name*/) {}
void CInterfaceDataCommunity::ChatOrderCircleKickOut(char* /*name*/) {}
void CInterfaceDataCommunity::ChatOrderCircleOut() {}
void CInterfaceDataCommunity::OpenUserInfo(char* /*name*/) {}

// GT 0x4D9FB0:
//   *(DWORD*)this = 0;
//   v1 = CUIManager::GetUIWindow(g_UIMgr, 30);  // CUIFriend
//   CUIFriend::SetInvenPartyButton(v1);
//   v2 = CUIManager::GetUserInterface(g_UIMgr, 3, 0);  // CUIPopMenu
//   if (v2) CUIPopMenu::SetupMenu(v2);
//
// CUIFriend / CUIPopMenu's relevant methods are not yet ported, so this
// restoration only resets the wait-call gate.  The friend/popup re-sync
// will plug in once those windows are restored.
void CInterfaceDataCommunity::ReactiveCommunity() {
    m_iWaitCallFlag = 0;
}
