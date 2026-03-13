#include "System/cltSexSystem.h"
#include <cctype>

cltSexSystem::cltSexSystem()
{
	mSex = 0;
}

void cltSexSystem::Initialize(uint8_t a2)
{
    mSex = a2;
}

BOOL cltSexSystem::IsMale()
{
    return (std::toupper(static_cast<unsigned char>(mSex)) == 77);
}

BOOL cltSexSystem::IsFemale()
{
    return (std::toupper(static_cast<unsigned char>(mSex)) == 70);
}

void cltSexSystem::SetSex_male()
{
    mSex = 77;
}

void cltSexSystem::SetSex_female()
{
    mSex = 70;
}

uint8_t cltSexSystem::GetSex()
{
    return mSex;
}