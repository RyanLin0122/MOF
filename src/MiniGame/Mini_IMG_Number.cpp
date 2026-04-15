#include "MiniGame/Mini_IMG_Number.h"

#include <cstdlib>
#include <cstring>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/ImageResource.h"
#include "Image/ImageResourceListDataMgr.h"

Mini_IMG_Number::Mini_IMG_Number()
    : m_pImages{}
    , m_digitBlock{}
    , m_digitCount(0)
    , m_pad51{}
    , m_dwResourceID(0)
    , m_blockBase(0)
    , m_digitCapacity(0)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_fHalfWidth(0.0f)
{
}

Mini_IMG_Number::~Mini_IMG_Number() = default;

void Mini_IMG_Number::InitMini_IMG_Number(unsigned int dwResourceID,
                                          uint16_t     blockBase,
                                          uint16_t     digitCapacity)
{
    m_dwResourceID  = dwResourceID;
    m_blockBase     = blockBase;
    m_digitCapacity = digitCapacity;
}

void Mini_IMG_Number::SetNumber(int value, float x, float y)
{
    int abs_v = std::abs(value);
    m_digitCount = 0;
    m_fHalfWidth = 0.0f;

    // digitBlock[0] 放最低位，依序向上。
    m_digitBlock[0] = static_cast<uint8_t>(m_blockBase + (abs_v % 10));
    m_digitCount = 1;

    if (abs_v >= 10)
    {
        int divisor = 10;
        do
        {
            int q = abs_v / divisor;
            divisor *= 10;
            m_digitBlock[m_digitCount] =
                static_cast<uint8_t>(m_blockBase + (q % 10));
            ++m_digitCount;
        }
        while (divisor <= abs_v);
    }

    m_fX = x;
    m_fY = y;

    // mofclient.c：取 GameImage 查詢每位數 block 寬度並累加為 halfWidth。
    // 對齊 ground truth：只檢查 GetGameImage 結果，pGIData 直接解參考（無 guard）。
    GameImage* pQuery = cltImageManager::GetInstance()->GetGameImage(9u, m_dwResourceID, 0, 1);
    if (pQuery)
    {
        for (int i = m_digitCount - 1; i >= 0; --i)
        {
            uint8_t b = m_digitBlock[i];
            m_fHalfWidth += static_cast<float>(
                pQuery->m_pGIData->m_Resource.m_pAnimationFrames[b].width);
        }
        m_fHalfWidth *= 0.5f;
    }
    else
    {
        // mofclient.c：若資源抓不到，直接把 digitCount 減一（無邊界防呆）。
        --m_digitCount;
    }
}

void Mini_IMG_Number::Process()
{
    int idx = static_cast<int>(m_digitCount) - 1;
    if (idx < 0)
        return;

    float curX = m_fX;
    float curY = m_fY;

    // 由高位到低位填寫每一位數對應的 GameImage。
    // 對齊 ground truth：只檢查 GetGameImage 結果，pGIData 直接解參考（無 guard）。
    for (; idx >= 0; --idx)
    {
        GameImage* pImg = cltImageManager::GetInstance()->GetGameImage(9u, m_dwResourceID, 0, 1);
        m_pImages[idx] = pImg;
        if (!pImg)
            continue;

        uint8_t b = m_digitBlock[idx];
        int blockWidth =
            pImg->m_pGIData->m_Resource.m_pAnimationFrames[b].width;

        pImg->m_fPosX = curX - m_fHalfWidth;
        pImg->m_fPosY = curY;
        pImg->SetBlockID(b);
        pImg->m_bFlag_446 = true;
        pImg->m_bFlag_447 = true;
        pImg->m_bVertexAnimation = false;

        curX += static_cast<float>(blockWidth);
    }
}

void Mini_IMG_Number::Render()
{
    if (!m_digitCount)
        return;

    // mofclient.c：只檢查 GameImage 內部的 m_pGIData 是否存在，
    // 不對 m_pImages[i] 指標本身做 null guard。
    for (int i = static_cast<int>(m_digitCount) - 1; i >= 0; --i)
    {
        GameImage* p = m_pImages[i];
        if (p->m_pGIData)
            p->Draw();
    }
}
