#pragma once
#include <cstdint>
#include <cstddef>

#pragma pack(push, 1)
// Layout aligned with mofclient.c: each record is 0x118 bytes (280).
// Reconstructed client previously had two guessed fields; they are kept
// for source compatibility, padding fills the rest so sizeof matches.
struct stCharKindInfo {
    char _unknown_prefix[8];      // 0..7 (kindCode, nameText, ...)
    int moneyRule;                // 8   (TODO: verify real offset)
    int levelOrRankBase;          // 12  (TODO: verify real offset)
    char _reserved[0x118 - 16];   // 16..0x117
};
#pragma pack(pop)
static_assert(sizeof(stCharKindInfo) == 0x118, "stCharKindInfo size must be 0x118 bytes");

class cltCharKindInfo {
public:
    cltCharKindInfo();
    virtual ~cltCharKindInfo();

    // cltCharKindInfo::Initialize / Free are virtual in the binary
    // (vftable slots 1 and 2). cltClientCharKindInfo overrides Initialize.
    virtual int Initialize(char* String2);
    virtual void Free();

    // 5 碼字串 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetCharKindInfo(uint16_t kindCode);
    uint16_t GetRealCharID(uint16_t charKind);

    // Returns monster name/info block for the given kind code.
    stCharKindInfo* GetMonsterNameByKind(unsigned short kind);

    // Returns all char kind infos that reference the given drop item kind code.
    // outChars must point to an array of at least 65535 stCharKindInfo* elements.
    // Returns the number of entries written.
    int GetCharKindInfoByDropItemKind(uint16_t dropItemKindCode, stCharKindInfo** outChars);

    // Retrieves monster char kinds within level range [minLv, maxLv] matching nation type.
    int GetMonsterCharKinds(int a2, int a3, int a4, int a5, uint16_t* a6);

    // Checks whether a given kind code represents a monster character.
    int IsMonsterChar(uint16_t kindCode);

    // mofclient.c 292838: bit 0 of DWORD+26 (offset 104) in stCharKindInfo.
    int IsPlayerChar(uint16_t kindCode);

    // mofclient.c: auxiliary kind lookups used by Order*/monster death paths.
    // Implemented as minimal stubs until the stCharKindInfo layout is fully
    // mapped.  Return 0 / nullptr when the record is missing.
    int  GetDieDelayAniByKind(uint16_t kindCode);
    char* GetDeadSound(uint16_t kindCode);

    // Returns boss info for the given kind, or nullptr if not a boss.
    void* GetBossInfoByKind(uint16_t kindCode);

protected:
    // Storage layout logically equivalent to mofclient.c:
    //   - offset this+4:         65535 stCharKindInfo* slots
    //   - offset this+0x40000:   monster-name buffer pointer
    //   - offset this+0x40004:   monster-name count
    // In this reconstruction the 65535-slot array is heap-allocated
    // to avoid bloating the BSS of every translation unit.
    stCharKindInfo** m_ppCharKindTable;   // array of 65535 pointers
    void* m_pMonsterNameBuffer;
    int m_nMonsterNameCount;
};

extern cltCharKindInfo g_clCharKindInfo;
