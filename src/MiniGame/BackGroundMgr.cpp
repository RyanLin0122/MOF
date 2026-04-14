#include "MiniGame/BackGroundMgr.h"

#include <cstring>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

namespace {
// mofclient.c 的原始實作以 DWORD 位元寫入 float 欄位，下面兩個輔助函式
// 維持完全相同的 32-bit 位元模式。
float BitsToFloat(unsigned int bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}
}

BackGroundMgr::BackGroundMgr()
    : m_x(0.0f)
    , m_y(BitsToFloat(1112014848u))       // 50.0f
    , m_type(0)
    , m_dwResID(486539280u)               // 0x1D000090（預設背景資源）
    , m_pImage(nullptr)
{
}

void BackGroundMgr::InitBackGroundMgr(int type, float x, float y)
{
    m_x = x;
    m_type = type;
    m_y = y;

    if (type == 0)
        m_dwResID = 486539280u;  // 0x1D000090
    else if (type == 1)
        m_dwResID = 486539279u;  // 0x1D00008F
    else if (type == 2)
        m_dwResID = 486539281u;  // 0x1D000091
}

char BackGroundMgr::Image_UP(float dy)
{
    float ny = m_y - dy;
    m_y = ny;
    if (ny >= -140.0f)
        return 0;
    m_y = BitsToFloat(static_cast<unsigned int>(-1022623744));
    return 1;
}

char BackGroundMgr::Image_DOWN(float dy)
{
    float ny = m_y + dy;
    m_y = ny;
    if (ny <= 50.0f)
        return 0;
    m_y = BitsToFloat(1112014848u); // 50.0f
    return 1;
}

void BackGroundMgr::Process(float /*dt*/)
{
    GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(9u, m_dwResID, 0, 1);
    m_pImage = pImage;
    if (pImage)
    {
        pImage->m_fPosX = m_x;
        pImage->SetBlockID(0);
        pImage->m_bFlag_447 = true;
        pImage->m_fPosY = m_y;
        pImage->m_bFlag_446 = true;
        pImage->m_bVertexAnimation = false;
    }
}

void BackGroundMgr::Render()
{
    if (m_pImage)
        m_pImage->Draw();
}
