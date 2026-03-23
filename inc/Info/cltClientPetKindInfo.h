#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <windows.h>
#include "Info/cltPetKindInfo.h"
#include "Info/cltPetAniInfo.h"

class cltClientPetKindInfo {
public:
    cltClientPetKindInfo();
    virtual ~cltClientPetKindInfo();

    void Free();

    // Returns cached or newly loaded pet animation info for the given pet kind.
    cltPetAniInfo* GetPetAniInfo(uint16_t petKind);

    // Fills in resource ID and block frame for the pet UI image.
    void GetPetUIImage(uint16_t petKind, unsigned int* outResId, uint16_t* outFrame);

private:
    cltPetKindInfo m_petKindInfo;                 // +4 (after vtable)
    cltPetAniInfo* m_aniInfoTable[0xFFFF] = {};   // +20
};