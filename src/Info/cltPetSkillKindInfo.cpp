#include "Info/cltPetSkillKindInfo.h"
#include <cstring>  // strlen
#include <cstdlib>  // atoi
#include <cctype>   // toupper

uint16_t cltPetSkillKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 6) return 0;

    int c = std::toupper(static_cast<unsigned char>(a1[0]));
    uint16_t hi = 0;

    // 'A' -> 置 0x8000；'P' -> 不置；其他 -> 無效
    if (c == 'A') {
        hi = 0x8000;
    }
    else if (c == 'P') {
        hi = 0;
    }
    else {
        return 0;
    }

    // 後五碼十進位，需 < 0x8000
    unsigned int num = static_cast<unsigned int>(std::atoi(a1 + 1));
    if (num < 0x8000u) {
        return static_cast<uint16_t>(hi | num);
    }
    return 0;
}
