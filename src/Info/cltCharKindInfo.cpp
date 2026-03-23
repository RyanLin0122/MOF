#include "Info/cltCharKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

uint16_t cltCharKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;

    if (std::strlen(a1) != 5)
        return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800)
        return static_cast<uint16_t>(hi | lo);

    return 0;
}

void* cltCharKindInfo::GetCharKindInfo(uint16_t)
{
    // 目前專案未建 CharKind 資料表，先回傳 nullptr 以滿足連結。
    return nullptr;
}

uint16_t cltCharKindInfo::GetRealCharID(uint16_t charKind)
{
    // stub：資料表未建立，直接回傳輸入值。
    return charKind;
}

stCharKindInfo* cltCharKindInfo::GetMonsterNameByKind(unsigned short /*kind*/)
{
    // Stub: real implementation looks up the monster name/info record.
    return nullptr;
}

int cltCharKindInfo::GetCharKindInfoByDropItemKind(uint16_t /*dropItemKindCode*/, stCharKindInfo** /*outChars*/)
{
    // Stub: real implementation searches the char kind table for entries
    // whose drop item kind code matches dropItemKindCode and writes them
    // into outChars, returning the count found.
    return 0;
}

int cltCharKindInfo::GetMonsterCharKinds(int /*a2*/, int /*a3*/, int /*a4*/, int /*a5*/, uint16_t* /*a6*/)
{
    // Stub: the reconstructed client does not have the original monster kind
    // table wired up yet, so report "no matching monsters".
    return 0;
}

int cltCharKindInfo::IsMonsterChar(uint16_t /*kindCode*/)
{
    // Stub: without the original char kind table, default to "not a monster".
    return 0;
}

void* cltCharKindInfo::GetBossInfoByKind(uint16_t /*kindCode*/)
{
    // Stub: no boss metadata is available in the reconstructed client yet.
    return nullptr;
}

// Global instance definition
cltCharKindInfo g_clCharKindInfo;
