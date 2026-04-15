#include "MiniGame/Mini_Speek.h"

#include <cstring>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

namespace {
inline float bitsToFloat(unsigned int bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}
}

// =====================================================================
// Mini_Speek_Mgr
// =====================================================================
Mini_Speek_Mgr::Mini_Speek_Mgr()
    : m_pImage(nullptr)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_patientType(0)
    , m_bUsed(0)
{
}

Mini_Speek_Mgr::~Mini_Speek_Mgr() = default;

void Mini_Speek_Mgr::SetPosition(float x, float y, int patientType)
{
    m_fX           = x;
    m_pImage       = nullptr;
    m_fY           = y;
    m_patientType  = patientType;
    m_bUsed        = 0;
}

void Mini_Speek_Mgr::SetUsed(bool used)
{
    m_bUsed = used ? 1 : 0;
}

void Mini_Speek_Mgr::Process()
{
    if (m_bUsed)
    {
        cltImageManager* pMgr = cltImageManager::GetInstance();
        GameImage* pImg = pMgr->GetGameImage(9u, 0x2000003Au, 0, 1);
        std::uint16_t blockID = static_cast<std::uint16_t>(m_patientType);
        m_pImage = pImg;
        if (pImg)
        {
            pImg->m_wBlockID  = blockID;
            pImg->m_bFlag_446 = true;

            pImg->m_fPosX = m_fX;
            pImg->m_bFlag_447 = true;

            pImg->m_fPosY = m_fY;
            pImg->m_bFlag_447 = true;

            pImg->Process();
        }
    }
}

void Mini_Speek_Mgr::Render()
{
    if (m_bUsed)
    {
        GameImage* pImg = m_pImage;
        if (pImg)
            pImg->Draw();
    }
}

// =====================================================================
// Mini_Speek_Thanks
// =====================================================================
Mini_Speek_Thanks::Mini_Speek_Thanks()
    : m_pImage(nullptr)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_state(0)
    , m_alphaScale(bitsToFloat(1132396544u))   // 255.0
    , m_colorScale(bitsToFloat(1120403456u))   // 102.0
    , m_bUsed(0)
    , m_FrameSkip_vft(nullptr)
    , m_accum(0.0f)
    , m_threshold(bitsToFloat(1015580809u))    // ≈ 1/60
{
}

Mini_Speek_Thanks::~Mini_Speek_Thanks() = default;

void Mini_Speek_Thanks::InitPosition(float x, float y)
{
    m_pImage     = nullptr;
    m_fX         = x;
    m_fY         = y;
    m_state      = 0;
    m_alphaScale = bitsToFloat(1132396544u);   // 255.0
    m_colorScale = bitsToFloat(1120403456u);   // 102.0
    m_bUsed      = 1;
}

void Mini_Speek_Thanks::SetUsed(bool used)
{
    m_bUsed = used ? 1 : 0;
}

void Mini_Speek_Thanks::Process(float dt)
{
    if (!m_bUsed)
        return;

    float accum = dt + m_accum;
    bool below = accum < m_threshold;
    m_accum = accum;
    int frames = 0;
    if (!below)
    {
        long long n = static_cast<long long>(accum / m_threshold);
        if (n)
            m_accum = accum - static_cast<float>(static_cast<int>(n)) * m_threshold;
        frames = static_cast<int>(n);
    }

    int v15 = frames;
    if (m_state)
    {
        if (m_state == 1)
        {
            m_alphaScale -= static_cast<float>(v15) * 5.0f;
            m_fY         -= static_cast<float>(v15);
        }
    }
    else
    {
        float fa = m_alphaScale - (static_cast<float>(v15) + static_cast<float>(v15));
        m_alphaScale = fa;
        if (fa < 220.0f)
        {
            m_alphaScale = bitsToFloat(1130102784u);  // 229.5 / 230 等亮度峰
            m_state      = 1;
        }
    }
    if (m_alphaScale < 0.0f)
        m_bUsed = 0;

    cltImageManager* pMgr = cltImageManager::GetInstance();
    GameImage* pImg = pMgr->GetGameImage(9u, 0x20000032u, 0, 1);
    m_pImage = pImg;
    if (pImg)
    {
        // 取得 block 寬度（block 0 的 width 在 GIData 對應的 block 表）
        // mofclient.c：(double)*(int *)(*(_DWORD *)(*((_DWORD *)v7 + 2) + 32) + 540)
        //            = pGIData -> 第 10 個 block (52 bytes/筆) 的 width
        float halfWidth = 0.0f;
        if (pImg->m_pGIData)
        {
            // pGIData 指向 ImageResourceListData；其偏移 +32 處有 ImageResource* (m_pImageResource)
            // 該 ImageResource 偏移 +540 (=20+52*10) 的 int 即第 10 個 block 的寬度。
            // 為避免依賴 ImageResource 的內部 layout，這裡用 RECT 方式取 block 0 的寬度。
            RECT r{};
            std::uint16_t saved = pImg->m_wBlockID;
            pImg->m_wBlockID = 10;
            pImg->GetBlockRect(&r);
            pImg->m_wBlockID = saved;
            halfWidth = static_cast<float>(r.right - r.left);
        }

        pImg->m_wBlockID  = 10;
        pImg->m_bFlag_446 = true;

        pImg->m_bFlag_447 = true;
        pImg->m_fPosX     = m_fX - halfWidth * 0.5f;

        pImg->m_fPosY     = m_fY;
        pImg->m_bFlag_447 = true;

        pImg->m_dwAlpha   = static_cast<unsigned int>(static_cast<long long>(m_alphaScale));
        pImg->m_bFlag_450 = true;
        pImg->m_bVertexAnimation = false;

        pImg->m_dwColor   = static_cast<unsigned int>(static_cast<long long>(m_colorScale));
        pImg->m_bFlag_449 = true;
        pImg->m_bVertexAnimation = false;

        pImg->Process();
    }
}

void Mini_Speek_Thanks::Render()
{
    if (m_bUsed)
    {
        GameImage* pImg = m_pImage;
        if (pImg)
            pImg->Draw();
    }
}
