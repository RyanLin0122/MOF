#include "Object/cltAniObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

extern int dword_A73088;
extern int dword_A7308C;

cltAniObject::cltAniObject()
    : CBaseObject()
{
    // Ground truth constructor only sets vftable, does not initialize derived fields
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

    // Write flip flag value to GameImage+392 (ground truth writes raw byte as DWORD)
    m_pGameImage->m_bFlipX = (unsigned char)m_byFlipFlag;

    // If transform flag is set, apply custom vertex parameters
    if (m_byTransformFlag)
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
    // Ground truth does NOT null-check m_pGameImage, only checks GameImage+8 (m_pGIData)
    if (m_pGameImage->GetGIDataPtr())
        m_pGameImage->Draw();
}

void cltAniObject::InitInfo(unsigned int dwResourceID, unsigned short wMaxFrames,
                            int nPosX, int nPosY, char byFlipFlag,
                            unsigned char byVisible, unsigned char byTransformFlag)
{
    m_nPosX = nPosX;
    m_nPosY = nPosY;
    m_dwResourceID = dwResourceID;
    m_wMaxFrames = wMaxFrames;
    m_byFlipFlag = byFlipFlag;
    m_byTransformFlag = byTransformFlag;
    m_dwValid = byVisible;
    m_fAniFrame = 0.0f;
}
