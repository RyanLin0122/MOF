#pragma once

class CShortKey {
public:
    static int SetAllDefaultKey(void* keyList);
    static int SaveKeySetting(void* keyList);
};

extern void* g_pShortKeyList;
