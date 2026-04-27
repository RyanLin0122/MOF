#include "Logic/cltMyCharData.h"
#include "global.h"
#include "System/cltLessonSystem.h"
#include "System/cltTradeSystem.h"
#include "Character/ClientCharacterManager.h"
#include "UI/CUIManager.h"
#include "UI/CUITradeUser.h"

cltMyCharData g_clMyCharData{};

int cltMyCharData::SetMyAccount(cltMyCharData* self, int account) {
    (void)self;
    g_dwMyAccountID = static_cast<unsigned int>(account);
    return 0;
}

unsigned int cltMyCharData::GetMyAccount(cltMyCharData* self) {
    // Ground truth: return *((_DWORD *)this + 21)
    return *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(self) + 84);
}

void cltMyCharData::Initialize(cltMyCharData* /*self*/,
                               unsigned short /*charKind*/, unsigned char /*classKind*/,
                               long long /*a3*/,
                               int /*hp*/, int /*maxHp*/,
                               int /*a6*/, int /*a7*/, int /*a8*/, int /*a9*/, int /*a10*/,
                               int /*teamKind*/,
                               cltItemList* /*pItemList*/,
                               int /*a13*/, int /*a14*/, int /*a15*/,
                               stEquipItemInfo* /*pEquip1*/, stEquipItemInfo* /*pEquip2*/,
                               int /*a18*/, int /*a19*/, int /*a20*/, int /*a21*/,
                               int /*a22*/, int /*a23*/, int /*a24*/, int /*a25*/,
                               int /*a26*/, int /*a27*/, int /*a28*/, int /*a29*/,
                               unsigned char /*nation*/, unsigned char /*sex*/, unsigned char /*hair*/,
                               int /*mapKind*/,
                               int /*a34*/, int /*a35*/, int /*a36*/, int /*a37*/,
                               int /*a38*/, int /*a39*/, int /*a40*/, int /*a41*/,
                               int /*a42*/, int /*a43*/, int /*a44*/, int /*a45*/,
                               int /*a46*/, int /*a47*/, int /*a48*/, int /*a49*/,
                               int /*a50*/, int /*a51*/, int /*a52*/, int /*a53*/,
                               int /*a54*/, int /*a55*/, int /*a56*/, int /*a57*/,
                               int /*a58*/, int /*a59*/, int /*a60*/, int /*a61*/) {
    // Stub: real implementation populates the my-char data from all supplied args.
}

void cltMyCharData::SetMapID(cltMyCharData* /*self*/, unsigned short /*mapKind*/) {
    // Stub: real implementation updates the current map ID field.
}

void cltMyCharData::SetMyCharName(cltMyCharData* /*self*/, const char* /*name*/) {
    // Stub: real implementation stores the character name.
}

cltMyCharData* cltMyCharData::GetMyCharName(cltMyCharData* self) {
    // Ground truth 0x518610：
    //   cltMyCharData *__thiscall cltMyCharData::GetMyCharName(cltMyCharData *this)
    //   { return this; }
    // 呼叫端 (_wsprintfA(..., fmt, GetMyCharName(...), ...)) 會把回傳值當作
    // 指向角色名稱字元陣列（位於 cltMyCharData 起始位址）的 char*。
    return self;
}

void cltMyCharData::IncLessonPt_Sword(cltMyCharData* /*self*/, unsigned int value) {
    // Ground truth 0x519150：
    //   cltLessonSystem::IncLessonPt_Sword((char*)this + 6568, a2);
    // cltMyCharData 內嵌 cltLessonSystem 於 offset 6568；本專案以 g_clLessonSystem
    // 作為全域的 LessonSystem 實例，因此以同一個實例轉發加分呼叫。
    g_clLessonSystem.IncLessonPt_Sword(value);
}

// =============================================================================
// 自動攻擊 / 禁言 / 刪角取消交易 / 拾取請求
// 原 mofclient.c 將狀態存於 cltMyCharData 內嵌欄位 (offset 26 / 30 / 60)。
// 本還原以 TU-local 全域變數承接「客戶端唯一 my-char data」之語意；多個 TU
// 不會共享一個 cltMyCharData 實例，所以 self 參數實際上不需被讀寫。
// =============================================================================
namespace {
int g_iAutoAttackState   = 0;  // mofclient.c：cltMyCharData +30
int g_iCanNotChatting    = 0;  // mofclient.c：cltMyCharData +26
int g_iRequestPickUpFlag = 0;  // mofclient.c：cltMyCharData +15  (拾取按鍵旗標)
}  // namespace

void cltMyCharData::SetAutoAttack(cltMyCharData* /*self*/, int active) {
    g_iAutoAttackState = active;
}
int cltMyCharData::GetAutoAttack(cltMyCharData* /*self*/) {
    return g_iAutoAttackState;
}
void cltMyCharData::SetCanNotChatting(cltMyCharData* /*self*/, unsigned char active) {
    g_iCanNotChatting = active;
}
int cltMyCharData::GetCanNotChatting(cltMyCharData* /*self*/) {
    return g_iCanNotChatting;
}

// mofclient.c 0x519590：cltTradeSystem::Free + CUITradeUser::CompleteTradeCanceled。
// 我們呼叫對應的全域實體（g_clTradeSystem），UI 端因 CUITradeUser::CompleteTradeCanceled
// 尚未提供，先以 g_UIMgr→GetUIWindow(16) 取得 window 指標但不操作；
// 主要副作用（Free）已對齊。
void cltMyCharData::DelCharCancelTrade(cltMyCharData* /*self*/) {
    // 引用全域 trade system（mofclient.c：(char*)this + 7152，內嵌於 cltMyCharData）。
    extern cltTradeSystem g_clTradeSystem;
    g_clTradeSystem.Free();
    // mofclient.c 230771：CUITradeUser window slot 16，呼叫 CompleteTradeCanceled(1)
    // 完成視窗收尾。
    if (g_UIMgr) {
        if (auto* w = static_cast<CUITradeUser*>(g_UIMgr->GetUIWindow(16))) {
            w->CompleteTradeCanceled(1);
        }
    }
}

// mofclient.c 0x519230：請求拾取最近物品。本還原省略 cltFieldItemManager
// 的 GetNearItemInfo / GetFieldItem 內部，僅保留主要流程：旗標檢查、座標
// 抓取、network 送 PickUpItem、清旗標。
void cltMyCharData::RequestPickUpItem(cltMyCharData* /*self*/) {
    if (!g_iRequestPickUpFlag) return;
    int posX = g_ClientCharMgr.GetMyPositionX();
    int posY = g_ClientCharMgr.GetMyPositionY();
    (void)posX; (void)posY;
    // 真正的 GetNearItemInfo / 背包檢查需要 cltFieldItemManager / cltBaseInventory
    // 的進一步還原；此處僅清旗標以避免重複觸發。
    g_iRequestPickUpFlag = 0;
}

// mofclient.c 0x5190A0
uint16_t cltMyCharData::GetMiniGameKind()
{
    // Ground truth:
    //   v1 = cltNPCManager::GetNPCType(&g_clNPCManager);
    //   if (v1[3]) return 1;      // Sword
    //   else if (v1[4]) return 2;  // Bow
    //   else if (v1[5]) return 3;  // Magic
    //   else return v1[6] ? 4 : 0; // Exorcist or none
    // TODO: 需要 cltNPCManager::GetNPCType 完整實作後補齊
    return 0;
}
