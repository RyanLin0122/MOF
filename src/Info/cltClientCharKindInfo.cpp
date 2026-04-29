#include "Info/cltClientCharKindInfo.h"
#include "global.h"

#include <cstring>
#include <cstdio>
#include <windows.h>

// ---------------------------------------------------------------------------
// cltClientCharKindInfo — reconstructed from mofclient.c
// sub_00401350 .. sub_004015B0
// ---------------------------------------------------------------------------

// (0x00401350)
cltClientCharKindInfo::cltClientCharKindInfo()
    : cltCharKindInfo()
{
    // mofclient.c:
    //   cltCharKindInfo::cltCharKindInfo(this);             // parent ctor
    //   *(_DWORD *)this = &cltClientCharKindInfo::vftable;  // set derived vtable
    //   memset((char *)this + 262152, 0, 0x3FFFCu);          // zero ani table
    // 0x3FFFC / sizeof(void*) == 0xFFFF slots.
    m_ppMonsterAniInfoTable = new cltMonsterAniInfo*[0xFFFF];
    std::memset(m_ppMonsterAniInfoTable, 0,
                sizeof(cltMonsterAniInfo*) * 0xFFFF);
}

// (0x00401380) '`scalar deleting destructor'
cltClientCharKindInfo::~cltClientCharKindInfo()
{
    // mofclient.c:
    //   *(_DWORD *)this = &vftable;  Free(this);
    cltClientCharKindInfo::Free();
    if (m_ppMonsterAniInfoTable) {
        delete[] m_ppMonsterAniInfoTable;
        m_ppMonsterAniInfoTable = nullptr;
    }
    // Parent destructor runs automatically and frees the char-kind table.
}

// (0x004013F0)
int cltClientCharKindInfo::Initialize(char* String2)
{
    // mofclient.c:
    //   if ( cltCharKindInfo::Initialize(this, String2) )
    //     return 1;
    //   wsprintfA(Text, "Character kind info file was a failure to initialize[ %s ]", String2);
    //   MessageBoxA(0, Text, "", 0);
    //   return 0;
    if (cltCharKindInfo::Initialize(String2))
        return 1;

    CHAR Text[1024];
    wsprintfA(Text, "Character kind info file was a failure to initialize[ %s ]", String2);
    MessageBoxA(nullptr, Text, "", 0);
    return 0;
}

// (0x00401450)
void cltClientCharKindInfo::Free()
{
    // mofclient.c:
    //   walk 0xFFFF slots at (char*)this + 262152
    //   for each non-null cltMonsterAniInfo*:
    //       ~cltMonsterAniInfo(*v3);
    //       operator delete(v4);
    //       *v3 = 0;
    //   cltCharKindInfo::Free(this);
    for (int i = 0; i < 0xFFFF; ++i) {
        cltMonsterAniInfo* entry = m_ppMonsterAniInfoTable[i];
        if (entry) {
            delete entry;                    // dtor + operator delete
            m_ppMonsterAniInfoTable[i] = nullptr;
        }
    }
    cltCharKindInfo::Free();
}

// (0x00401490)
cltMonsterAniInfo* cltClientCharKindInfo::GetMonsterAniInfo(unsigned short a2)
{
    // mofclient.c:
    //   if ( *((_DWORD *)this + a2 + 65538) )
    //       return (cltMonsterAniInfo *)*((_DWORD *)this + a2 + 65538);
    //   result = cltCharKindInfo::GetCharKindInfo(this, a2);
    //   if ( result )
    //   {
    //       v4 = (char *)result + 72;
    //       if ( stricmp(v4, "NONE") )
    //       {
    //           v5 = new cltMonsterAniInfo(0x1F64);
    //           *((_DWORD *)this + a2 + 65538) = v5;
    //           if ( !cltMonsterAniInfo::Initialize(v5, v4) )
    //           {
    //               wsprintfA(Text, "Monster animation file was a failure to initialize. [ %s ]", v4);
    //               MessageBoxA(0, Text, "", 0);
    //               return 0;
    //           }
    //           return *((_DWORD *)this + a2 + 65538);
    //       }
    //   }
    //   return result;
    if (m_ppMonsterAniInfoTable[a2])
        return m_ppMonsterAniInfoTable[a2];

    void* rawResult = cltCharKindInfo::GetCharKindInfo(a2);
    if (!rawResult)
        return nullptr;

    // char kind info offset 72 holds the monster-animation file name.
    char* aniFileName = reinterpret_cast<char*>(rawResult) + 72;
    if (_stricmp(aniFileName, "NONE") == 0)
        return nullptr;

    cltMonsterAniInfo* ani = new cltMonsterAniInfo();
    m_ppMonsterAniInfoTable[a2] = ani;

    if (!ani->Initialize(aniFileName)) {
        CHAR Text[1024];
        wsprintfA(Text, "Monster animation file was a failure to initialize. [ %s ]", aniFileName);
        MessageBoxA(nullptr, Text, "", 0);
        return nullptr;
    }

    return m_ppMonsterAniInfoTable[a2];
}

// (0x00401590)
unsigned char cltClientCharKindInfo::GetCharLevel(unsigned short a2)
{
    // mofclient.c:
    //   return *((_BYTE *)GetCharKindInfo(this, a2) + 146);
    return *(reinterpret_cast<unsigned char*>(cltCharKindInfo::GetCharKindInfo(a2)) + 146);
}

// (0x004015B0)
stCharKindInfo* cltClientCharKindInfo::IsFieldItemBox(unsigned short a2)
{
    // mofclient.c:
    //   result = GetCharKindInfo(this, a2);
    //   if ( result )
    //     result = *((_DWORD *)result + 59);  // offset 236 = fieldItemBoxKind (DWORD)
    //   return result;
    // 原始 binary 將 DWORD 直接當作 stCharKindInfo* 回傳；呼叫端只做 non-null
    // 判定。x64 上指標寬度 = 8 bytes，但 stCharKindInfo +236 仍是 32-bit DWORD，
    // 因此這裡只讀 32 bits，再封裝為指標形式以保留呼叫面。
    stCharKindInfo* info =
        static_cast<stCharKindInfo*>(cltCharKindInfo::GetCharKindInfo(a2));
    if (!info) return nullptr;
    return reinterpret_cast<stCharKindInfo*>(
        static_cast<uintptr_t>(info->fieldItemBoxKind));
}
