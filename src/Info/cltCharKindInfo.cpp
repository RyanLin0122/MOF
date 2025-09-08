#include "Info/cltCharKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

uint16_t cltCharKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;

    if (std::strlen(a1) != 5)
        return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800)
        return static_cast<uint16_t>(hi | lo);

    return 0;
}
