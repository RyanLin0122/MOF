#include "MiniGame/Mini_AniCtrl.h"

#include <cstring>
#include <new>

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

Mini_AniCtrl::Mini_AniCtrl()
    : m_pImage(nullptr)
    , m_pAniArr(nullptr)
    , m_aniCount(0)
    , m_pKeyArr(nullptr)
    , m_keyCount(0)
    , m_curKey(0)
    , m_curFrame(0)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_alpha(255)
    , m_FrameSkip_vft(nullptr)
    , m_accum(0.0f)
    , m_threshold(bitsToFloat(1015580809u))   // ~1/60
    , m_dword13(0)
{
}

Mini_AniCtrl::~Mini_AniCtrl()
{
    if (m_pAniArr)
    {
        operator delete(m_pAniArr);
        m_pAniArr = nullptr;
    }
    if (m_pKeyArr)
    {
        operator delete(m_pKeyArr);
        m_pKeyArr = nullptr;
    }
}

void Mini_AniCtrl::Create_Mini_AniCtrl(Mini_AniInfo* aniArr, int aniCount,
                                       Mini_KeyInfo* keyArr, int keyCount, int frameRate)
{
    const std::size_t aniBytes = static_cast<std::size_t>(8 * aniCount);
    Mini_AniInfo* pAni = static_cast<Mini_AniInfo*>(operator new(aniBytes));
    if (pAni)
    {
        std::memset(pAni, 0, aniBytes);
    }
    m_aniCount = aniCount;
    m_pAniArr  = pAni;
    if (pAni)
        std::memcpy(pAni, aniArr, aniBytes);

    const std::size_t keyBytes = static_cast<std::size_t>(8 * keyCount);
    Mini_KeyInfo* pKey = static_cast<Mini_KeyInfo*>(operator new(keyBytes));
    if (pKey)
    {
        std::memset(pKey, 0, keyBytes);
    }
    m_keyCount = keyCount;
    m_pKeyArr  = pKey;
    if (pKey)
        std::memcpy(pKey, keyArr, keyBytes);

    m_threshold = 1.0f / static_cast<float>(frameRate);
    m_curKey    = 0;
    m_curFrame  = 0;
    m_dword13   = 0;
    m_alpha     = 255;
}

void Mini_AniCtrl::Play(int keyIdx)
{
    if (m_curKey != keyIdx)
    {
        m_curKey   = keyIdx;
        m_curFrame = m_pKeyArr[keyIdx].startFrame;
    }
}

void Mini_AniCtrl::SetFrameRate(int frameRate)
{
    m_threshold = 1.0f / static_cast<float>(frameRate);
}

void Mini_AniCtrl::SetPosition(float x, float y)
{
    m_fX = x;
    m_fY = y;
}

void Mini_AniCtrl::SetAlpha(int alpha)
{
    m_alpha = alpha;
}

void Mini_AniCtrl::Process(float dt)
{
    // FrameSkip 累計：每 m_threshold 秒前進一格
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
    int newFrame = m_curFrame + frames;
    m_curFrame = newFrame;
    Mini_KeyInfo& key = m_pKeyArr[m_curKey];
    if (newFrame > key.lastFrame)
        m_curFrame = key.startFrame;

    cltImageManager* pMgr = cltImageManager::GetInstance();
    GameImage* pImg = pMgr->GetGameImage(9u, m_pAniArr[m_curFrame].resourceID, 0, 1);
    m_pImage = pImg;
    if (pImg)
    {
        std::uint16_t blockID = m_pAniArr[m_curFrame].blockID;
        pImg->m_bFlag_446 = true;
        pImg->m_wBlockID  = blockID;

        pImg->m_fPosX = m_fX;
        pImg->m_bFlag_447 = true;

        pImg->m_fPosY = m_fY;
        pImg->m_bFlag_447 = true;

        pImg->m_dwAlpha   = static_cast<unsigned int>(m_alpha);
        pImg->m_bFlag_450 = true;
        pImg->m_bVertexAnimation = false;

        pImg->Process();
    }
}

void Mini_AniCtrl::Render()
{
    GameImage* pImg = m_pImage;
    if (pImg)
    {
        if (pImg->m_pGIData)
            pImg->Draw();
    }
}
