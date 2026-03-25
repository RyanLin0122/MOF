#include "Logic/cltMiniGame_Button.h"

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

cltMiniGame_Button::cltMiniGame_Button() = default;

cltMiniGame_Button::~cltMiniGame_Button()
{
    if (m_pImage)
    {
        cltImageManager::GetInstance()->ReleaseGameImage(m_pImage);
        m_pImage = nullptr;
    }
}

void cltMiniGame_Button::CreateBtn(int x, int y,
                                   unsigned int imageType,
                                   unsigned int resNormal,   uint16_t blockNormal,
                                   unsigned int resOver,     uint16_t blockOver,
                                   unsigned int resDown,     uint16_t blockDown,
                                   unsigned int resDisabled, uint16_t blockDisabled,
                                   void (*callback)(unsigned int),
                                   unsigned int userData,
                                   int /*reserved*/)
{
    m_x = x;
    m_y = y;
    m_imageType = imageType;
    m_resIDs[0] = resNormal;
    m_resIDs[1] = resOver;
    m_resIDs[2] = resDown;
    m_resIDs[3] = resDisabled;
    m_blockIDs[0] = blockNormal;
    m_blockIDs[1] = blockOver;
    m_blockIDs[2] = blockDown;
    m_blockIDs[3] = blockDisabled;
    m_pCallback = callback;
    m_userData = userData;
    m_nState = 0;
}

void cltMiniGame_Button::SetActive(int active)
{
    m_nActive = active;
}

void cltMiniGame_Button::SetBtnState(uint8_t state)
{
    m_nState = state;
}

void cltMiniGame_Button::SetPosition(int x, int y)
{
    m_x = x;
    m_y = y;
}

void cltMiniGame_Button::ButtonAction()
{
    if (m_pCallback)
        m_pCallback(m_userData);
}

int cltMiniGame_Button::Poll()
{
    return 0;
}

void cltMiniGame_Button::PrepareDrawing()
{
    if (!m_nActive)
        return;

    if (!m_pImage)
    {
        m_pImage = cltImageManager::GetInstance()->GetGameImage(
            m_imageType,
            GetResourceID(),
            0,
            0);
    }

    if (!m_pImage)
        return;

    m_pImage->SetPosition((float)m_x, (float)m_y);
    m_pImage->SetBlockID(GetBlockID());
    m_pImage->SetAlpha(255u);
    m_pImage->Process();
}

void cltMiniGame_Button::Draw()
{
    if (m_nActive && m_pImage)
        m_pImage->Draw();
}

unsigned int cltMiniGame_Button::GetImageType()
{
    return m_imageType;
}

unsigned int cltMiniGame_Button::GetResourceID()
{
    unsigned int index = (m_nState < 4) ? m_nState : 0;
    return m_resIDs[index];
}

uint16_t cltMiniGame_Button::GetBlockID()
{
    unsigned int index = (m_nState < 4) ? m_nState : 0;
    return m_blockIDs[index];
}
