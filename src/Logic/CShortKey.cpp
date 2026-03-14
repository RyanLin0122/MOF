#include "Logic/CShortKey.h"

void* g_pShortKeyList = nullptr;

int CShortKey::SetAllDefaultKey(void* keyList) {
    (void)keyList;
    return 0;
}

int CShortKey::SaveKeySetting(void* keyList) {
    (void)keyList;
    return 0;
}
