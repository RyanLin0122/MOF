#include "Info/cltClientPetKindInfo.h"

cltClientPetKindInfo::cltClientPetKindInfo()
{
    std::memset(m_aniInfoTable, 0, sizeof(m_aniInfoTable));
}

cltClientPetKindInfo::~cltClientPetKindInfo()
{
    Free();
}

void cltClientPetKindInfo::Free()
{
    for (int i = 0; i < 0xFFFF; ++i)
    {
        if (m_aniInfoTable[i])
        {
            delete m_aniInfoTable[i];
            m_aniInfoTable[i] = nullptr;
        }
    }
}

cltPetAniInfo* cltClientPetKindInfo::GetPetAniInfo(uint16_t petKind)
{
    cltPetAniInfo*& cached = m_aniInfoTable[petKind];
    if (cached)
        return cached;

    strPetKindInfo* kindInfo = m_petKindInfo.GetPetKindInfo(petKind);
    if (!kindInfo)
        return nullptr;

    char* aniFile = kindInfo->AnimationInfoFileGi;
    if (_stricmp(aniFile, "NONE") == 0)
        return nullptr;

    cltPetAniInfo* ani = new cltPetAniInfo();
    cached = ani;

    if (!ani->Initialize(aniFile))
    {
        char text[1024];
        wsprintfA(text, "Pet animation file was a failure to initialize. [ %s ]", aniFile);
        MessageBoxA(nullptr, text, "", 0);
        return nullptr;
    }

    return cached;
}

void cltClientPetKindInfo::GetPetUIImage(uint16_t petKind, unsigned int* outResId, uint16_t* outFrame)
{
    strPetKindInfo* info = m_petKindInfo.GetPetKindInfo(petKind);
    // outResId = offset 148 (dwPetStopResource), outFrame = offset 152 (wPetStopBlockId)
    *outResId = info->dwPetStopResource;
    *outFrame = info->wPetStopBlockId;
}