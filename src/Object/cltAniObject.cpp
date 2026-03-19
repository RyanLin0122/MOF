#include "Object/cltAniObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

extern int dword_A73088;
extern int dword_A7308C;

cltAniObject::cltAniObject()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_wCurrentFrame(0)
    , m_fAniFrame(0.0f)
    , m_dwResourceID(0)
    , m_wMaxFrames(0)
    , m_byAlphaVal(0)
    , m_byFlag(0)
{
}

cltAniObject::~cltAniObject()
{
}

void cltAniObject::Process()
{
    // Advance animation frame
    m_fAniFrame += 0.2f;
    m_wCurrentFrame = (unsigned short)(int)m_fAniFrame;

    if (m_wCurrentFrame > m_wMaxFrames)
    {
        m_wCurrentFrame = 0;
        m_fAniFrame = 0.0f;
    }

    // Get game image from the image manager
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(8u, m_dwResourceID, 0, 1);

    // Set block (animation frame)
    m_pGameImage->SetBlockID(m_wCurrentFrame);

    // Set flags
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;

    // Set position (world to screen)
    float screenX = (float)(m_nPosX - dword_A73088);
    float screenY = (float)(m_nPosY - dword_A7308C);
    m_pGameImage->SetPosition(screenX, screenY);

    // Set flip from alpha value field
    m_pGameImage->m_bFlipX = (m_byAlphaVal != 0);

    // If flag is set, apply custom scale/rotation parameters
    if (m_byFlag)
    {
        m_pGameImage->m_fScaleX = -10.0f;
        m_pGameImage->m_fScaleY = 0.0f;
        m_pGameImage->m_baseVertices[3].x = 60.0f;
        m_pGameImage->m_baseVertices[3].y = -10.0f;
        m_pGameImage->m_baseVertices[2].x = 63.0f;
        m_pGameImage->m_baseVertices[2].y = 0.0f;
        m_pGameImage->m_bDrawPart1 = true;
    }

    // Set alpha to 255 and mark as processed
    m_pGameImage->SetAlpha(255);
    m_pGameImage->m_bFlag_450 = true;
    m_pGameImage->m_bVertexAnimation = false;

    m_pGameImage->Process();
}

void cltAniObject::Draw()
{
    if (m_pGameImage && m_pGameImage->GetGIDataPtr())
        m_pGameImage->Draw();
}

void cltAniObject::InitInfo(unsigned int dwResourceID, unsigned short wMaxFrames,
                            int nPosX, int nPosY, char byAlphaVal,
                            unsigned char byVisible, unsigned char byFlag)
{
    m_nPosX = nPosX;
    m_nPosY = nPosY;
    m_dwResourceID = dwResourceID;
    m_wMaxFrames = wMaxFrames;
    m_byAlphaVal = byAlphaVal;
    m_byFlag = byFlag;
    m_dwValid = byVisible;
    m_fAniFrame = 0.0f;
}
