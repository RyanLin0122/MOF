#pragma once

#include "Object/CBaseObject.h"

class CEffect_MapEffect;
class ClientCharacter;

class CEffectObject : public CBaseObject {
public:
    CEffectObject();
    virtual ~CEffectObject();

    int InitEffectObject(unsigned short wEffectKindID, unsigned short wPlacementType,
                         unsigned short wObjectKind, ClientCharacter* pCharacter,
                         int nPosX, int nPosY);
    virtual void FrameProcess(float fElapsedTime);
    virtual void Process();
    virtual void Draw();

private:
    CEffect_MapEffect* m_pEffect;   // DWORD +788
};
