#include "Other/DCTAbuseWordManager.h"

#include <cstring>

void DCTAbuseWordManager::ChangeString(const char* src, char* dst) {
    if (!dst) return;
    if (!src) { dst[0] = '\0'; return; }
    std::strcpy(dst, src);
}
