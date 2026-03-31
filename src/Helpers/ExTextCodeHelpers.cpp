#include "global.h"
#include "Info/cltItemKindInfo.h"
#include "System/cltLevelSystem.h"

float SetRatePerThousand(int a1)
{
    return static_cast<float>(static_cast<double>(a1) * 0.1);
}

float SetRatePerTenThousand(int a1)
{
    return static_cast<float>(static_cast<double>(a1) * 0.0099999998);
}

uint8_t ExGetMyLevel()
{
    return g_clLevelSystem.GetLevel();
}

unsigned int ExGetTextCodeHuntItemType(unsigned int a1)
{
    switch (a1)
    {
    case 0x1000:    return 3770;
    case 0x2000:    return 3771;
    case 0x4000:    return 3772;
    case 0x8000:    return 3768;
    case 0x10000:   return 3769;
    case 0x20000:   return 3773;
    case 0x40000:   return 3774;
    case 0x80000:   return 3775;
    case 0x100000:  return 3776;
    case 0x200000:  return 3777;
    default:        return a1;
    }
}

unsigned int ExGetTextCodeFashionItemType(unsigned int a1)
{
    switch (a1)
    {
    case 0:      return 3311;
    case 1:      return 3778;
    case 2:      return 3779;
    case 4:      return 3780;
    case 8:      return 3781;
    case 0x10:   return 3782;
    case 0x20:   return 3783;
    case 0x40:   return 3784;
    case 0x80:   return 3785;
    case 0x100:  return 3786;
    case 0x400:  return 4914;
    case 0x800:  return 57036;
    case 0x1000: return 57037;
    default:     return a1;
    }
}

unsigned int ExGetTextCodeInstantItemType(unsigned int a1)
{
    switch (a1)
    {
    case 0: return 3787;
    case 1: return 3788;
    case 3: return 3789;
    default: return a1;
    }
}

int ExGetTextCodeHuntItemAttribute(stItemKindInfo* a1)
{
    if (!a1)
        return 0;

    switch (reinterpret_cast<uint32_t*>(a1)[55])
    {
    case 0: return 3797;
    case 1: return 3798;
    case 2: return 3799;
    case 3: return 3800;
    case 4: return 3801;
    case 5: return 3802;
    default: return 0;
    }
}
