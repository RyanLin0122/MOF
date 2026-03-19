#pragma once

#include "Object/CBaseObject.h"

class GameImage;

class cltAniObject : public CBaseObject {
public:
    cltAniObject();
    virtual ~cltAniObject();

    virtual void FrameProcess(float fElapsedTime) {}
    virtual void Process();
    virtual void Draw();

    void InitInfo(unsigned int dwResourceID, unsigned short wMaxFrames,
                  int nPosX, int nPosY, char byAlphaVal,
                  unsigned char byVisible, unsigned char byFlag);

private:
    GameImage*     m_pGameImage;      // DWORD +788
    unsigned short m_wCurrentFrame;   // current animation frame
    float          m_fAniFrame;       // animation frame counter (float)
    unsigned int   m_dwResourceID;    // resource ID
    unsigned short m_wMaxFrames;      // max frame count
    char           m_byAlphaVal;      // alpha value for rendering
    unsigned char  m_byFlag;          // additional flag (rotation etc.)
};
