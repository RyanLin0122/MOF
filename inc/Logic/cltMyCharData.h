#pragma once
#include <cstdint>
#include "System/cltEquipmentSystem.h"

class cltItemList;

class cltMyCharData {
public:
    static int SetMyAccount(cltMyCharData* self, int account);
    static unsigned int GetMyAccount(cltMyCharData* self);

    static void Initialize(cltMyCharData* self,
                           unsigned short charKind, unsigned char classKind,
                           long long a3,
                           int hp, int maxHp,
                           int a6, int a7, int a8, int a9, int a10,
                           int teamKind,
                           cltItemList* pItemList,
                           int a13, int a14, int a15,
                           stEquipItemInfo* pEquip1, stEquipItemInfo* pEquip2,
                           int a18, int a19, int a20, int a21, int a22, int a23,
                           int a24, int a25, int a26, int a27, int a28, int a29,
                           unsigned char nation, unsigned char sex, unsigned char hair,
                           int mapKind,
                           int a34, int a35, int a36, int a37, int a38, int a39,
                           int a40, int a41, int a42, int a43, int a44, int a45,
                           int a46, int a47, int a48, int a49, int a50, int a51,
                           int a52, int a53, int a54, int a55, int a56, int a57,
                           int a58, int a59, int a60, int a61);

    static void SetMapID(cltMyCharData* self, unsigned short mapKind);

    static void SetMyCharName(cltMyCharData* self, const char* name);

    // mofclient.c 0x518610：此函式在原始程式碼實際上只是 `return this;`
    // 原因是 cltMyCharData 的前幾個 byte 即為以 NUL 結尾的角色名稱字串，
    // 呼叫端會把回傳值直接餵給 _wsprintfA 的 "%s"。這裡提供相同語意的 shim，
    // 讓 minigame 等呼叫端能對齊 GT 的呼叫寫法。
    static cltMyCharData* GetMyCharName(cltMyCharData* self);

    // mofclient.c 0x519150：
    //   void cltMyCharData::IncLessonPt_Sword(this, a2)
    //   { cltLessonSystem::IncLessonPt_Sword((char*)this + 6568, a2); }
    // cltMyCharData 內嵌了 cltLessonSystem 於 offset 6568；這裡提供同語意
    // 的 shim，使小遊戲加分流程能對齊 GT（透過 MyCharData → LessonSystem）。
    static void IncLessonPt_Sword(cltMyCharData* self, unsigned int value);

    // mofclient.c 0x5190A0：根據 NPC 類型決定目前小遊戲類型（1=劍/2=弓/3=魔/4=驅魔）。
    uint16_t GetMiniGameKind();

    // mofclient.c 0x5198F0：自動攻擊狀態 setter，原本寫入 *((DWORD*)this+30)。
    static void SetAutoAttack(cltMyCharData* self, int active);
    // 補：取得自動攻擊狀態（僅 ClientCharacterManager 內部需要）。
    static int  GetAutoAttack(cltMyCharData* self);

    // mofclient.c 0x5196E0 / 0x5196F0：禁言狀態 setter / getter，
    // 原本寫入/讀取 *((DWORD*)this+26)。
    static void SetCanNotChatting(cltMyCharData* self, unsigned char active);
    static int  GetCanNotChatting(cltMyCharData* self);

    // mofclient.c 0x519590：刪角色時取消進行中交易。
    //   cltTradeSystem::Free((cltMyCharData *)((char *)this + 7152));
    //   v1 = (CUITradeUser *)CUIManager::GetUIWindow(g_UIMgr, 16);
    //   CUITradeUser::CompleteTradeCanceled(v1, 1u);
    static void DelCharCancelTrade(cltMyCharData* self);

    // mofclient.c 0x519230：地面物品拾取請求。
    //   完整流程：判定 m_bRequestPickUp(+15) → 取角色座標 → 找最近 FieldItem
    //   → 檢查可放入背包 → 送 CMoFNetwork::PickUpItem → 清旗標。
    static void RequestPickUpItem(cltMyCharData* self);
};

extern cltMyCharData g_clMyCharData;
