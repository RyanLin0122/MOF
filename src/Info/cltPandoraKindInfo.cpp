#include "Info/cltPandoraKindInfo.h"

std::uint16_t cltPandoraKindInfo::TranslateKindCode(char* a1)
{
    if (!a1 || std::strlen(a1) != 5)
        return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);

    if (lo < 0x800)
        return static_cast<std::uint16_t>(hi | lo);

    return 0;
}
