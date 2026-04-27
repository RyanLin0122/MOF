#pragma once

// Minimal stub for the trade-user UI window referenced by cltChattingMgr.
// mofclient.c calls CUIManager::GetUserInterface(g_UIMgr, 16, 0) and casts the
// result to CUITradeUser*.  The underlying window is not yet restored, so the
// lookup returns nullptr and the chat manager's null checks short-circuit the
// call.  This declaration gives the chat manager a real type to compile
// against.

#include "UI/CUIBase.h"

class CUITradeUser : public CUIBase {
public:
    CUITradeUser();
    ~CUITradeUser() override;

    void SetChat(char* buffer);

    // mofclient.c 0x004D40A0：交易取消完成時的視窗收尾。
    //   - 若拖曳 icon 來源是交易視窗 (kind 10)，呼叫 CMoveIcon::Terminate
    //   - a2 != 0 時呼叫 cltMyCharData::CancelTrade、彈訊息盒 3011、關閉視窗
    // 真實 widget 內部行為尚未還原；目前 stub 只做 cltMyCharData 的取消通知，
    // 視窗動作放在 widget 完整還原後再補。
    void CompleteTradeCanceled(unsigned char a2);
};
