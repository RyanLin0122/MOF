#include "Info/cltClientPetKindInfo.h"

#include <cstring>
#include <cstdio>
#include <windows.h>

// ---------------------------------------------------------------------------
// cltClientPetKindInfo — reconstructed from mofclient.c
// sub_004EDA20 .. sub_004EDBE0
// ---------------------------------------------------------------------------

// (0x004EDA20)
cltClientPetKindInfo::cltClientPetKindInfo()
{
    // mofclient.c:
    //   cltPetKindInfo::cltPetKindInfo((char *)this + 4);
    //   *(_DWORD *)this = &cltClientPetKindInfo::vftable;
    //   return this;
    // ground truth 未對 m_aniInfoTable 做任何初始化；g_clPetKindInfo 為
    // 全域物件，依賴 BSS 零值初始化。(組合成員 m_petKindInfo 會被編譯器
    // 預先建構)
}

// (0x004EDA60)
cltClientPetKindInfo::~cltClientPetKindInfo()
{
    // mofclient.c:
    //   *(_DWORD *)this = &vftable;
    //   cltClientPetKindInfo::Free(this);
    //   if ( this ) v2 = (char *)this + 4;
    //   else         v2 = 0;
    //   cltPetKindInfo::~cltPetKindInfo(v2);
    Free();
    // m_petKindInfo 的解構由編譯器自動處理（相當於反編譯最後呼叫
    // cltPetKindInfo::~cltPetKindInfo 於 this+4）。
}

// (0x004EDAC0)
void cltClientPetKindInfo::Free()
{
    // mofclient.c:
    //   v1 = (_DWORD *)((char *)this + 20);
    //   v2 = 0xFFFF;
    //   do {
    //       if ( *v1 ) {
    //           (**(void (**)(_DWORD, int))*v1)(*v1, 1);  // scalar deleting dtor
    //           *v1 = 0;
    //       }
    //       ++v1; --v2;
    //   } while ( v2 );
    for (int i = 0; i < 0xFFFF; ++i) {
        cltPetAniInfo* entry = m_aniInfoTable[i];
        if (entry) {
            delete entry;               // dtor + operator delete (相當於 scalar deleting dtor)
            m_aniInfoTable[i] = nullptr;
        }
    }
}

// (0x004EDAF0)
cltPetAniInfo* cltClientPetKindInfo::GetPetAniInfo(uint16_t a2)
{
    // mofclient.c:
    //   v2 = (char *)this + 4 * a2 + 20;
    //   if ( *(_DWORD *)v2 )
    //       return *(cltPetAniInfo **)v2;
    //   result = cltPetKindInfo::GetPetKindInfo((cltPetKindInfo *)((char *)this + 4), a2);
    //   if ( result ) {
    //       v4 = (char *)result + 8;  // AnimationInfoFileGi
    //       if ( stricmp(v4, "NONE") ) {
    //           v5 = new cltPetAniInfo();
    //           *(_DWORD *)v2 = v5;
    //           if ( !cltPetAniInfo::Initialize(v5, v4) ) {
    //               wsprintfA(...);
    //               MessageBoxA(...);
    //               return 0;
    //           }
    //           return *(cltPetAniInfo **)v2;
    //       }
    //   }
    //   return result;
    if (m_aniInfoTable[a2])
        return m_aniInfoTable[a2];

    strPetKindInfo* kindInfo = m_petKindInfo.GetPetKindInfo(a2);
    if (!kindInfo)
        return nullptr;

    char* aniFileName = kindInfo->AnimationInfoFileGi; // offset 8 in strPetKindInfo
    if (_stricmp(aniFileName, "NONE") == 0)
        return nullptr;

    cltPetAniInfo* ani = new cltPetAniInfo();
    m_aniInfoTable[a2] = ani;

    if (!ani->Initialize(aniFileName)) {
        CHAR Text[1024];
        wsprintfA(Text, "Pet animation file was a failure to initialize. [ %s ]", aniFileName);
        MessageBoxA(nullptr, Text, "", 0);
        return nullptr;
    }

    return m_aniInfoTable[a2];
}

// (0x004EDBE0)
void cltClientPetKindInfo::GetPetUIImage(uint16_t a2, unsigned int* a3, uint16_t* a4)
{
    // mofclient.c:
    //   v4 = cltPetKindInfo::GetPetKindInfo(this+4, a2);
    //   *a3 = *((_DWORD *)v4 + 37);  // offset 148 = dwPetStopResource
    //   *a4 = *((_WORD  *)v4 + 76);  // offset 152 = wPetStopBlockId
    strPetKindInfo* info = m_petKindInfo.GetPetKindInfo(a2);
    *a3 = info->dwPetStopResource;
    *a4 = info->wPetStopBlockId;
}
