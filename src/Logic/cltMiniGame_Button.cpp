#include "Logic/cltMiniGame_Button.h"

#include <io.h>
#include <windows.h>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/ImageResource.h"
#include "Image/ImageResourceListDataMgr.h"
#include "Logic/DirectInputManager.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"

// ---------------------------------------------------------------------------
// cltMiniGame_Button — mofclient.c 0x5BE950
// ---------------------------------------------------------------------------
cltMiniGame_Button::cltMiniGame_Button()
{
    m_normalImageType = 0;
    m_normalResID = 0;
    m_normalBlockID = 0;
    m_overImageType = 0;
    m_overResID = 0;
    m_overBlockID = 0;
    m_downImageType = 0;
    m_downResID = 0;
    m_downBlockID = 0;
    m_disabledImageType = 0;
    m_disabledResID = 0;
    m_disabledBlockID = 0;
    m_nState = 0;
}

cltMiniGame_Button::~cltMiniGame_Button() = default;

// ---------------------------------------------------------------------------
// CreateBtn — mofclient.c 0x5BE990
// ---------------------------------------------------------------------------
void cltMiniGame_Button::CreateBtn(int x, int y,
                                   unsigned int imageType,
                                   unsigned int resNormal,   uint16_t blockNormal,
                                   unsigned int resOver,     uint16_t blockOver,
                                   unsigned int resDown,     uint16_t blockDown,
                                   unsigned int resDisabled, uint16_t blockDisabled,
                                   void (*callback)(unsigned int),
                                   unsigned int userData,
                                   int initialActive)
{
    CHAR Text[256];

    m_normalImageType = 0; m_normalResID = 0; m_normalBlockID = 0;
    m_overImageType = 0;   m_overResID = 0;   m_overBlockID = 0;
    m_downImageType = 0;   m_downResID = 0;   m_downBlockID = 0;
    m_disabledImageType = 0; m_disabledResID = 0; m_disabledBlockID = 0;

    GameImage* pImg = cltImageManager::GetInstance()->GetGameImage(
        imageType, resNormal, 0, 1);
    m_pImage = pImg;

    int w = 0;
    if (pImg->m_pGIData)
    {
        if (blockNormal < pImg->m_pGIData->m_Resource.m_animationFrameCount)
        {
            w = pImg->m_pGIData->m_Resource.m_pAnimationFrames[blockNormal].width;
        }
        else
        {
            if (_access("MofData/Local.dat", 0) != -1)
            {
                wsprintfA(Text, "%s:%i",
                          pImg->m_pGIData->m_szFileName,
                          blockNormal);
                MessageBoxA(nullptr, Text, "Block Error", 0);
            }
        }
    }

    int h = 0;
    if (pImg->m_pGIData)
    {
        if (blockNormal < pImg->m_pGIData->m_Resource.m_animationFrameCount)
        {
            h = pImg->m_pGIData->m_Resource.m_pAnimationFrames[blockNormal].height;
        }
        else
        {
            if (_access("MofData/Local.dat", 0) != -1)
            {
                wsprintfA(Text, "%s:%i",
                          pImg->m_pGIData->m_szFileName,
                          blockNormal);
                MessageBoxA(nullptr, Text, "Block Error", 0);
            }
        }
    }

    m_width  = w;
    m_height = h;
    m_left   = x;
    m_top    = y;
    m_right  = x + w;
    m_bottom = y + h;

    m_normalResID    = resNormal;
    m_overResID      = resOver;
    m_overBlockID    = blockOver;
    m_downResID      = resDown;
    m_downBlockID    = blockDown;
    m_disabledResID  = resDisabled;
    m_disabledBlockID = blockDisabled;

    m_normalImageType   = imageType;
    m_normalBlockID     = blockNormal;
    m_overImageType     = imageType;
    m_downImageType     = imageType;
    m_disabledImageType = imageType;

    m_pCallback = callback;
    m_userData  = userData;
    m_nActive   = initialActive;
    m_nState    = 0;
}

// ---------------------------------------------------------------------------
// GetImageType — mofclient.c 0x5BEBA0
// ---------------------------------------------------------------------------
unsigned int cltMiniGame_Button::GetImageType()
{
    switch (m_nState) {
        case 0:    return m_normalImageType;
        case 1:
        case 4:    return m_overImageType;
        case 3:    return m_downImageType;
        default:   return 0;
    }
}

// ---------------------------------------------------------------------------
// GetResourceID — mofclient.c 0x5BEBE0
// ---------------------------------------------------------------------------
unsigned int cltMiniGame_Button::GetResourceID()
{
    switch (m_nState) {
        case 0:    return m_normalResID;
        case 1:    return m_overResID;
        case 3:    return m_downResID;
        case 4:    return m_disabledResID;
        default:   return 0;
    }
}

// ---------------------------------------------------------------------------
// GetBlockID — mofclient.c 0x5BEC20
// ---------------------------------------------------------------------------
uint16_t cltMiniGame_Button::GetBlockID()
{
    switch (m_nState) {
        case 0:    return m_normalBlockID;
        case 1:    return m_overBlockID;
        case 3:    return m_downBlockID;
        case 4:    return m_disabledBlockID;
        default:   return 0;
    }
}

// ---------------------------------------------------------------------------
// SetActive — mofclient.c 0x5BEC60
// ---------------------------------------------------------------------------
void cltMiniGame_Button::SetActive(int active)
{
    m_nActive = active;
}

// ---------------------------------------------------------------------------
// SetBtnState — mofclient.c 0x5BEC70
// ---------------------------------------------------------------------------
void cltMiniGame_Button::SetBtnState(uint8_t state)
{
    m_nState = state;
}

// ---------------------------------------------------------------------------
// SetPosition — mofclient.c 0x5BEC80
// ---------------------------------------------------------------------------
void cltMiniGame_Button::SetPosition(int x, int y)
{
    m_left   = x;
    m_top    = y;
    m_right  = x + m_width;
    m_bottom = y + m_height;
}

// ---------------------------------------------------------------------------
// ButtonAction — mofclient.c 0x5BECB0
// ---------------------------------------------------------------------------
void cltMiniGame_Button::ButtonAction()
{
    m_pCallback(m_userData);
    m_nState = 1;
}

// ---------------------------------------------------------------------------
// Poll — mofclient.c 0x5BECD0
// ---------------------------------------------------------------------------
int cltMiniGame_Button::Poll()
{
    if (!m_nActive)
        return 0;
    if (m_nState == 4)
        return 0;

    m_nState = 0;

    DirectInputManager* pInput = cltMoF_BaseMiniGame::m_pInputMgr;
    int mx = pInput->GetMouse_X();
    int my = pInput->GetMouse_Y();

    RECT rc;
    rc.left   = m_left;
    rc.top    = m_top;
    rc.right  = m_right;
    rc.bottom = m_bottom;
    POINT pt;
    pt.x = mx;
    pt.y = my;

    if (!PtInRect(&rc, pt))
        return 0;

    m_nState = 1;

    if (pInput->IsLMButtonPressed())
    {
        m_nState = 3;
        return 1;
    }

    if (pInput->IsLMButtonUp())
    {
        m_pCallback(m_userData);
        m_nState = 1;
    }

    return 1;
}

// ---------------------------------------------------------------------------
// PrepareDrawing — mofclient.c 0x5BED50
// ---------------------------------------------------------------------------
void cltMiniGame_Button::PrepareDrawing()
{
    if (!m_nActive)
        return;

    unsigned int resID = GetResourceID();
    unsigned int imgType = GetImageType();
    m_pImage = cltImageManager::GetInstance()->GetGameImage(imgType, resID, 0, 1);

    uint16_t blockID = GetBlockID();
    m_pImage->m_fPosX = static_cast<float>(m_left);
    m_pImage->m_fPosY = static_cast<float>(m_top);
    m_pImage->SetBlockID(blockID);
    m_pImage->m_bFlag_447 = true;
    m_pImage->m_bFlag_446 = true;
    m_pImage->m_bVertexAnimation = false;
}

// ---------------------------------------------------------------------------
// Draw — mofclient.c 0x5BEDC0
// ---------------------------------------------------------------------------
void cltMiniGame_Button::Draw()
{
    if (m_nActive && m_pImage)
        m_pImage->Draw();
}
