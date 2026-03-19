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
                  int nPosX, int nPosY, char byFlipFlag,
                  unsigned char byVisible, unsigned char byTransformFlag);

private:
    GameImage*     m_pGameImage;       // offset 3152 (DWORD +788)
    unsigned short m_wCurrentFrame;    // offset 3156 (WORD +1578)
    unsigned short _padAniFrame;       // padding for float alignment
    float          m_fAniFrame;        // offset 3160 (float +790)
    char           _reserved[32];      // gap between m_fAniFrame and m_dwResourceID (bytes 3164-3195)
    unsigned int   m_dwResourceID;     // offset 3196 (DWORD +799)
    unsigned short m_wMaxFrames;       // offset 3200 (WORD +1600)
    char           m_byFlipFlag;       // offset 3202 - written to GameImage+392 (direction/flip)
    unsigned char  m_byTransformFlag;  // offset 3203 - controls vertex transform
};
