#include "MiniGame/cltMiniGame_DrawNum.h"

#include <io.h>
#include <cstdlib>
#include <cstring>
#include <windows.h>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/ImageResource.h"
#include "Image/ImageResourceListDataMgr.h"

cltMiniGame_DrawNum::cltMiniGame_DrawNum()
    : m_active(0)
    , m_imageType(0)
    , m_dwResourceID(0)
    , m_blockBase(0)
    , m_alignMode(0)
    , m_pad17(0)
    , m_digitCount(0)
    , m_pImages{}
{
}

cltMiniGame_DrawNum::~cltMiniGame_DrawNum() = default;

void cltMiniGame_DrawNum::InitDrawNum(unsigned int imageType,
                                      unsigned int dwResourceID,
                                      uint16_t     blockBase,
                                      uint8_t      alignMode)
{
    m_imageType    = imageType;
    m_dwResourceID = dwResourceID;
    m_blockBase    = blockBase;
    m_alignMode    = alignMode;
    m_active       = 1;
}

void cltMiniGame_DrawNum::SetActive(int active)
{
    m_active = active;
}

// ---------------------------------------------------------------------------
// PrepareDrawing — mofclient.c 0x5BEE40
// ---------------------------------------------------------------------------
void cltMiniGame_DrawNum::PrepareDrawing(int x, int y, unsigned int value, int alpha)
{
    if (!m_active)
        return;

    CHAR Text[256];
    CHAR Text2[256];
    char Buffer[20];

    _itoa(value, Buffer, 10);
    int len = static_cast<int>(std::strlen(Buffer));
    m_digitCount = static_cast<uint16_t>(len);

    // First loop: right to left, accumulate total width and store GameImage pointers.
    int totalWidth = 0;
    for (int i = len - 1; i >= 0; --i)
    {
        uint16_t blockId = static_cast<uint16_t>(m_blockBase + (Buffer[i] - '0'));
        GameImage* pImg = cltImageManager::GetInstance()->GetGameImage(
            m_imageType, m_dwResourceID, 0, 1);
        m_pImages[i] = pImg;

        int w = 0;
        auto pGIData = pImg->m_pGIData;
        if (pGIData)
        {
            if (pGIData->m_Resource.m_animationFrameCount > blockId)
            {
                w = pGIData->m_Resource.m_pAnimationFrames[blockId].width;
            }
            else
            {
                if (_access("MofData/Local.dat", 0) != -1)
                {
                    wsprintfA(Text, "%s:%i", pImg->m_pGIData->m_szFileName, blockId);
                    MessageBoxA(nullptr, Text, "Block Error", 0);
                }
            }
        }
        totalWidth += w;
    }

    // Calculate start X based on alignMode.
    int curX = 0;
    switch (m_alignMode)
    {
        case 0: curX = x - totalWidth; break;
        case 1: curX = x;              break;
        default:                        break;
    }

    // Second loop: left to right, set image properties.
    for (int i = 0; i < m_digitCount; ++i)
    {
        GameImage* pImg = m_pImages[i];
        uint16_t blockId = static_cast<uint16_t>(m_blockBase + (Buffer[i] - '0'));

        uint16_t blockWidth = 0;
        auto pGIData = pImg->m_pGIData;
        if (!pGIData)
        {
            // width stays 0
        }
        else if (pGIData->m_Resource.m_animationFrameCount <= blockId)
        {
            if (_access("MofData/Local.dat", 0) != -1)
            {
                wsprintfA(Text, "%s:%i", pImg->m_pGIData->m_szFileName, blockId);
                MessageBoxA(nullptr, Text, "Block Error", 0);
            }
        }
        else
        {
            blockWidth = static_cast<uint16_t>(
                pGIData->m_Resource.m_pAnimationFrames[blockId].width);
        }

        // GT redundant Block Error check (same condition, different buffer)
        auto pGIData2 = pImg->m_pGIData;
        if (pGIData2
            && pGIData2->m_Resource.m_animationFrameCount <= blockId
            && _access("MofData/Local.dat", 0) != -1)
        {
            wsprintfA(Text2, "%s:%i", pImg->m_pGIData->m_szFileName, blockId);
            MessageBoxA(nullptr, Text2, "Block Error", 0);
        }

        pImg->SetAlpha(static_cast<unsigned int>(alpha));
        pImg->m_bFlag_450 = true;
        pImg->m_bVertexAnimation = false;
        pImg->m_fPosX = static_cast<float>(curX);
        pImg->m_fPosY = static_cast<float>(y);
        pImg->SetBlockID(blockId);
        pImg->m_bFlag_447 = true;
        pImg->m_bFlag_446 = true;
        pImg->m_bVertexAnimation = false;

        curX += blockWidth;
    }
}

void cltMiniGame_DrawNum::Draw()
{
    if (!m_active)
        return;
    for (int i = 0; i < m_digitCount; ++i)
    {
        if (m_pImages[i])
            m_pImages[i]->Draw();
    }
}
