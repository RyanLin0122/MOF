#pragma once
#include <cstdint>

class cltItemList;

// 12-byte equip item info struct (matches ground truth layout).
struct stEquipItemInfo {
    std::uint16_t itemKind = 0;
    std::uint16_t pad      = 0;
    std::uint32_t value0   = 0;
    std::uint32_t value1   = 0;
};

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
};

extern cltMyCharData g_clMyCharData;
