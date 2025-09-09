#include "Info/cltNPCInfo.h"
#include <cstring> // strlen
#include <cstdlib> // atoi
#include <cctype>  // toupper

uint16_t cltNPCInfo::TranslateKindCode(char* s) {
    // 與反編譯一致：
    // - 長度必須為 5
    // - 取 s[0]（轉大寫）後加 31，左移 11 位
    // - 後 4 碼以十進位解析，需 < 0x800
    // - 否則回傳 0
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;

    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) {
        return static_cast<uint16_t>(hi | num);
    }
    return 0;
}
