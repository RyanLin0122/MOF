#include "Logic/cltMyCharData.h"
#include "global.h"

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
