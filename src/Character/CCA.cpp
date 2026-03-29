#include "Character/CCA.h"
#include <cstring>

// CCA stub implementations
// The real CCA is a 292-byte opaque object from the original binary.
// These stubs satisfy the linker until the class is fully reconstructed.

CCA::CCA()
{
    memset(m_data, 0, sizeof(m_data));
}

CCA::~CCA()
{
}

void CCA::LoadCA(const char* path, CEffectBase** a2, CEffectBase** a3)
{
}

void CCA::Play(int motion, bool loop)
{
}

void CCA::Process()
{
}

void CCA::Draw()
{
}

void CCA::InitItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned int a5)
{
}

void CCA::ResetItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned char a5)
{
}

void CCA::SetItemID(unsigned short itemId, unsigned char sex, int a4, int a5, int a6, unsigned char a7)
{
}

void CCA::BegineEmoticon(int a2)
{
}

void CCA::EndEmoticon(unsigned short a2, unsigned char a3)
{
}

// ExGetIllustCharSexCode stub
// Real implementation is in the original binary.
unsigned char ExGetIllustCharSexCode(char charKind)
{
    return 0;
}
