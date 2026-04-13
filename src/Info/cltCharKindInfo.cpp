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

int cltCharKindInfo::IsMonsterChar(uint16_t /*kindCode*/)
{
    // Placeholder.
    return 0;
}

void* cltCharKindInfo::GetBossInfoByKind(uint16_t /*kindCode*/)
{
    // Placeholder.
    return nullptr;
}

// Global instance definition
cltCharKindInfo g_clCharKindInfo;
