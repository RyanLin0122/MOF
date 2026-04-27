#include "UI/CUITradeUser.h"

CUITradeUser::CUITradeUser() = default;
CUITradeUser::~CUITradeUser() = default;

void CUITradeUser::SetChat(char* /*buffer*/) {}

// mofclient.c 0x004D40A0：交易取消完成。原始實作牽涉 CMoveIcon /
// CMessageBoxManager / vftable[4]（關窗呼叫），這些 widget 主流程尚未
// 完整還原；本 stub 保持空動作，供 cltMyCharData::DelCharCancelTrade
// 呼叫面相容。a2 = 1 表示「真的取消（伺服器拒絕等）」，a2 = 0 表示
// 「自行 commit 完成不需訊息盒」。
void CUITradeUser::CompleteTradeCanceled(unsigned char /*a2*/) {}
