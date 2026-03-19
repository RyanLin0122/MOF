#pragma once

#include "Object/CBaseObject.h"
#include "Object/stObjectInfo.h"

class GameImage;

class CStaticObject : public CBaseObject {
public:
    CStaticObject();
    CStaticObject(unsigned short wObjectID);
    virtual ~CStaticObject();

    virtual void FrameProcess(float fElapsedTime) {}
    virtual void Process();
    virtual void Draw();

    int GetSXtoD(const char* pStr);

private:
    GameImage*    m_pGameImage;    // DWORD +788
    stObjectInfo* m_pObjectInfo;   // DWORD +789
};
