#include "System/cltSexSystem.h"
#include <cctype>   // std::toupper

// 00590880
cltSexSystem::cltSexSystem()
{
    // 反編譯僅回傳 this；為維持一致，不主動初始化 mSex。
}

// 005908A0
void cltSexSystem::Initialize(uint8_t a2)
{
    // *(_BYTE*)this = a2;
    mSex = a2;
}

// 005908B0
BOOL cltSexSystem::IsMale()
{
    // _toupper(*(unsigned __int8*)this) == 77 ('M')
    return (std::toupper(static_cast<unsigned char>(mSex)) == 77);
}

// 005908D0
BOOL cltSexSystem::IsFemale()
{
    // _toupper(*(unsigned __int8*)this) == 70 ('F')
    return (std::toupper(static_cast<unsigned char>(mSex)) == 70);
}

// 005908F0
void cltSexSystem::SetSex_male()
{
    // *(_BYTE*)this = 77 ('M')
    mSex = 77;
}

// 00590900
void cltSexSystem::SetSex_female()
{
    // *(_BYTE*)this = 70 ('F')
    mSex = 70;
}

// 00590910
uint8_t cltSexSystem::GetSex()
{
    // return *(_BYTE*)this
    return mSex;
}
