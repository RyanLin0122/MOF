#include "Info/cltCharKindInfo.h"
#include "global.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ---------------------------------------------------------------------------
// Constructor / destructor (aligned with mofclient.c)
// ---------------------------------------------------------------------------
cltCharKindInfo::cltCharKindInfo()
{
    // Allocate and zero the 65536-slot pointer table (uint16 range).
    // mofclient.c memsets 0xFFFF slots; we allocate one extra so any
    // call with index 0xFFFF stays within bounds.
    m_ppCharKindTable = new stCharKindInfo*[0x10000];
    std::memset(m_ppCharKindTable, 0, sizeof(stCharKindInfo*) * 0x10000);
    m_pMonsterNameBuffer = nullptr;
    m_nMonsterNameCount = 0;

    // Mirror mofclient.c: g_pcltCharKindInfo = this;
    g_pcltCharKindInfo = this;
}

cltCharKindInfo::~cltCharKindInfo()
{
    cltCharKindInfo::Free();
    if (m_ppCharKindTable) {
        delete[] m_ppCharKindTable;
        m_ppCharKindTable = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Initialize / Free
// ---------------------------------------------------------------------------
// NOTE: The original cltCharKindInfo::Initialize is a ~400-line text parser
// that reads charkindinfo.txt and InitMonsterAinFrame. That parser is out of
// scope for the ClientCharKindInfo/ClientPetKindInfo/ClientPortalInfo task.
// We keep the table empty and return success so the derived client classes
// link and run; actual data population will be added when the parser is
// ported.
int cltCharKindInfo::Initialize(char* /*String2*/)
{
    return 1;
}

void cltCharKindInfo::Free()
{
    // mofclient.c walks the 65535 pointer slots and operator deletes each
    // non-null entry.
    if (m_ppCharKindTable) {
        for (int i = 0; i < 0xFFFF; ++i) {
            if (m_ppCharKindTable[i]) {
                ::operator delete(m_ppCharKindTable[i]);
                m_ppCharKindTable[i] = nullptr;
            }
        }
    }
    if (m_pMonsterNameBuffer) {
        ::operator delete(m_pMonsterNameBuffer);
        m_pMonsterNameBuffer = nullptr;
    }
    m_nMonsterNameCount = 0;
    g_pcltCharKindInfo = nullptr;
}

// ---------------------------------------------------------------------------
// Static: TranslateKindCode
// ---------------------------------------------------------------------------
uint16_t cltCharKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800)
        return static_cast<uint16_t>(hi | lo);

    return 0;
}

// ---------------------------------------------------------------------------
// Lookup helpers (align with mofclient.c semantics)
// ---------------------------------------------------------------------------
void* cltCharKindInfo::GetCharKindInfo(uint16_t a2)
{
    // mofclient.c: return *((_DWORD *)this + a2 + 1);
    if (!m_ppCharKindTable) return nullptr;
    return m_ppCharKindTable[a2];
}

uint16_t cltCharKindInfo::GetRealCharID(uint16_t charKind)
{
    // Placeholder: real binary resolves alt/transform forms.
    return charKind;
}

stCharKindInfo* cltCharKindInfo::GetMonsterNameByKind(unsigned short /*kind*/)
{
    // Placeholder.
    return nullptr;
}

int cltCharKindInfo::GetCharKindInfoByDropItemKind(uint16_t /*dropItemKindCode*/, stCharKindInfo** /*outChars*/)
{
    // Placeholder.
    return 0;
}

int cltCharKindInfo::GetMonsterCharKinds(int /*a2*/, int /*a3*/, int /*a4*/, int /*a5*/, uint16_t* /*a6*/)
{
    // Placeholder.
    return 0;
}

int cltCharKindInfo::IsMonsterChar(uint16_t kindCode)
{
    // mofclient.c 292849: return bit 1 of (DWORD+26)/(byte offset 104) of
    // the record, or 0 when the kind is unknown.
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    const unsigned int flags = *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(info) + 104);
    return static_cast<int>((flags >> 1) & 1u);
}

int cltCharKindInfo::IsPlayerChar(uint16_t kindCode)
{
    // mofclient.c 292838: return bit 0 of (DWORD+26).
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    const unsigned int flags = *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(info) + 104);
    return static_cast<int>(flags & 1u);
}

// mofclient.c 0x00565830：分身(Clone)旗標 = stCharKindInfo +209 byte。
unsigned char cltCharKindInfo::GetIsClone(uint16_t kindCode)
{
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    return *(reinterpret_cast<unsigned char*>(info) + 209);
}

int cltCharKindInfo::GetDieDelayAniByKind(uint16_t kindCode)
{
    // mofclient.c: reads a byte at a fixed offset in the char-kind record.
    // Until the record is fully mapped we return 0 (no delay) when unknown.
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    // The decomp indexes DWORD+50 (offset 200); gate on a plausible range.
    return static_cast<int>(
        *reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(info) + 200));
}

char* cltCharKindInfo::GetDeadSound(uint16_t kindCode)
{
    // mofclient.c: returns char* to a sound-id string embedded in the record.
    // Returning an empty static string here keeps GameSound::PlaySoundA safe
    // until the exact string-field offset is restored.
    (void)kindCode;
    static char s_empty[1] = { '\0' };
    return s_empty;
}

void* cltCharKindInfo::GetBossInfoByKind(uint16_t /*kindCode*/)
{
    // Placeholder.
    return nullptr;
}

// Global instance definition
cltCharKindInfo g_clCharKindInfo;
