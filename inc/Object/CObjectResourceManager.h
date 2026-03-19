#pragma once

#include "Object/stObjectInfo.h"
#include "global.h"

class CObjectResourceManager {
public:
    CObjectResourceManager();
    virtual ~CObjectResourceManager();

    int Initialize(char* szFileName);
    void Free();
    stObjectInfo* GetObjectInfoByID(unsigned short wObjectID);

    static unsigned short TranslateKindCode(char* szCode);

private:
    stObjectInfo* m_pObjectInfos[0xFFFF]; // array of pointers indexed by object ID
    int m_nCount;                          // number of loaded objects
};
