#include "MiniGame/cltMiniGame_DrawNum.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

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

void cltMiniGame_DrawNum::PrepareDrawing(int x, int y, unsigned int value, int alpha)
{
    if (!m_active)
        return;

    // 1) 將數字轉成字串（十進位）。
    char buf[20];
    std::snprintf(buf, sizeof(buf), "%u", value);
    int len = static_cast<int>(std::strlen(buf));
    if (len <= 0)
        return;
    if (len > kMaxDigits)
        len = kMaxDigits;

    m_digitCount = static_cast<uint16_t>(len);

    // 2) 第一輪迴圈：累計總寬度並填入 m_pImages[i]。
    //    mofclient.c 的第一輪是由右向左（i = len-1 → 0）取 GameImage 和寬度。
    int totalWidth = 0;
    for (int i = len - 1; i >= 0; --i)
    {
        uint16_t blockId = static_cast<uint16_t>(m_blockBase + (buf[i] - '0'));
        GameImage* pImg = cltImageManager::GetInstance()->GetGameImage(
            m_imageType, m_dwResourceID, 0, 1);
        m_pImages[i] = pImg;

        if (pImg && pImg->m_pGIData
            && blockId < pImg->m_pGIData->m_Resource.m_animationFrameCount)
        {
            AnimationFrameData* pFrame =
                &pImg->m_pGIData->m_Resource.m_pAnimationFrames[blockId];
            totalWidth += pFrame->width;
        }
    }

    // 3) 計算起始 X 位置（依 m_alignMode）。
    //    mofclient.c：0 -> 右對齊（cursor = x - totalWidth）
    //                 1 -> 左對齊（cursor = x）
    //                 其他 -> cursor 保持 0（等同未初始化）
    int curX;
    switch (m_alignMode)
    {
        case 0: curX = x - totalWidth; break;
        case 1: curX = x;              break;
        default: curX = 0;             break;
    }

    // 4) 第二輪迴圈：由左向右依序填入各數字的 GameImage 屬性。
    for (int i = 0; i < len; ++i)
    {
        GameImage* pImg = m_pImages[i];
        if (!pImg)
            continue;

        uint16_t blockId = static_cast<uint16_t>(m_blockBase + (buf[i] - '0'));
        int blockWidth = 0;
        if (pImg->m_pGIData
            && blockId < pImg->m_pGIData->m_Resource.m_animationFrameCount)
        {
            AnimationFrameData* pFrame =
                &pImg->m_pGIData->m_Resource.m_pAnimationFrames[blockId];
            blockWidth = pFrame->width;
        }

        pImg->SetAlpha(static_cast<unsigned int>(alpha));
        pImg->m_bFlag_450 = true;
        pImg->m_bVertexAnimation = false;
        pImg->m_fPosX = static_cast<float>(curX);
        pImg->m_fPosY = static_cast<float>(y);
        pImg->SetBlockID(blockId);
        pImg->m_bFlag_446 = true;
        pImg->m_bFlag_447 = true;

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
