#include "Logic/cltMyCharData.h"
#include "global.h"
#include "System/cltLessonSystem.h"

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
