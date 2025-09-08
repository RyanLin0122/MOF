#include "Info/cltMapInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

uint16_t cltMapInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;

    // 必須剛好 5 字元，例如 "D0123"、"I0007" 等
    if (std::strlen(a1) != 5)
        return 0;

    // 高位：以 (toupper(first) + 31) << 11 編碼
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;

    // 低位：後四碼數字（需 < 0x800）
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800)
        return static_cast<uint16_t>(hi | lo);

    return 0;
}
