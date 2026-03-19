#pragma once

#include "Object/CBaseObject.h"

class CEffectBase;
class CEffect_Portal;

class CPortalObject : public CBaseObject {
public:
    CPortalObject();
    virtual ~CPortalObject();

    void CreatePortal(unsigned short wPortalType, int nPosX, int nPosY);
    virtual void FrameProcess(float fElapsedTime);
    virtual void Process();
    virtual void Draw();

private:
    CEffect_Portal* m_pEffect;   // DWORD +788
};
