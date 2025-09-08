#include "Info/cltQuestKindInfo.h"
#include <cctype>
#include <cstdlib>
#include <cstring>

std::uint16_t cltQuestKindInfo::TranslateKindCode(const char* a1)
{
    if (!a1) return 0;                       // 防呆（反編譯未顯示，但不影響行為）
    if (std::strlen(a1) != 5) return 0;      // 長度必須為 5

    // 與反編譯一致的位元組合：
    // v2 = (_toupper(*a1) + 31) << 11
    // v3 = _atoi(a1 + 1); if (v3 < 0x800) return v2 | v3; else return 0;
    int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    unsigned int v3 = static_cast<unsigned int>(std::atoi(a1 + 1));
    if (v3 < 0x800u)
        return static_cast<std::uint16_t>(v2 | v3);
    return 0;
}
