#include "MiniGame/PowerBar.h"

#include <cmath>
#include <cstdlib>

#include "global.h"
#include "Sound/GameSound.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

PowerBar::PowerBar()
    : m_degree(0)
    , m_maxRange(200.0f)                  // 1127743488
    , m_decaySpeed(0.0f)
    , m_curOffset(0.0f)
    , m_running(1)
    , m_pBar(nullptr)
    , m_pCursor(nullptr)
    , m_highlightUp(0)
    , m_pHighlight(nullptr)
    , m_highlightAlpha(0)
    , m_highlightAccum(0.0f)
    , m_highlightThreshold(0.0083333333f) // 1/120
    , m_baseX(0.0f)
    , m_baseY(0.0f)
    , m_pointerAccum(0.0f)
    , m_pointerThreshold(0.03333333f)     // 1/30
{
}

PowerBar::~PowerBar() = default;

void PowerBar::InitPowerBar(int degree, float x, float y)
{
    m_maxRange = 200.0f;
    m_decaySpeed = 0.0f;
    m_curOffset = 0.0f;

    int r = std::rand() % 3;
    m_degree = degree;

    // mofclient.c：decaySpeed = maxRange * rate，rate 依 degree × r 決定。
    //   degree 0: 0.0222, 0.0185, 0.01666...
    //   degree 1: 0.0333, 0.0277, 0.0238
    //   degree 2: 0.0333, 0.0277, 0.0238
    float rate = 0.0f;
    if (degree == 0)
    {
        if (r == 0) rate = 0.022222223f;
        else if (r == 1) rate = 0.018518519f;
        else if (r == 2) rate = 0.016666668f;
    }
    else if (degree == 1)
    {
        if (r == 0) rate = 0.033333335f;
        else if (r == 1) rate = 0.027777778f;
        else if (r == 2) rate = 0.023809524f;
    }
    else if (degree == 2)
    {
        if (r == 0) rate = 0.033333335f;
        else if (r == 1) rate = 0.027777778f;
        else if (r == 2) rate = 0.023809524f;
    }
    m_decaySpeed = m_maxRange * rate;

    m_pBar = nullptr;
    m_pCursor = nullptr;
    m_highlightUp = 0;
    m_pHighlight = nullptr;
    m_highlightAlpha = 0;
    m_baseX = x;
    m_baseY = y;
    m_running = 1;
}

int PowerBar::GetPower()
{
    g_GameSoundManager.PlaySoundA((char*)"M0018", 0, 0);
    double v = std::fabs(static_cast<double>(m_curOffset));
    m_running = 0;
    if (v > 182.0) return 10;
    if (v > 179.0) return 9;
    if (v > 175.0) return 8;
    if (v > 170.0) return 7;
    if (v > 164.0) return 6;
    if (v > 157.0) return 5;
    if (v > 149.0) return 4;
    if (v > 140.0) return 3;
    if (v > 130.0) return 2;
    return 1;
}

void PowerBar::Process(float dt)
{
    cltImageManager* pMgr = cltImageManager::GetInstance();

    // 底圖（bar）
    GameImage* pBar = pMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pBar = pBar;
    if (pBar)
    {
        pBar->SetBlockID(2);
        pBar->m_bFlag_446 = true;
        pBar->m_fPosX = m_baseX;
        pBar->m_bFlag_447 = true;
        pBar->m_fPosY = m_baseY;
        pBar->Process();
    }

    // 指針（cursor）
    GameImage* pCursor = pMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pCursor = pCursor;
    if (pCursor)
    {
        pCursor->SetBlockID(4);
        pCursor->m_bFlag_446 = true;
        pCursor->m_fPosX = m_baseX + 22.0f;
        pCursor->m_bFlag_447 = true;
        pCursor->m_fPosY = m_baseY + 205.0f;

        if (m_running)
        {
            // 透過 pointer FrameSkip 衰減 curOffset
            m_pointerAccum += dt;
            int frames = 0;
            if (m_pointerAccum >= m_pointerThreshold)
            {
                frames = static_cast<int>(m_pointerAccum / m_pointerThreshold);
                m_pointerAccum -= frames * m_pointerThreshold;
            }
            m_curOffset -= frames * m_decaySpeed;
            if (-m_maxRange >= m_curOffset)
                m_curOffset = -m_maxRange;

            // highlight
            GameImage* pHL = pMgr->GetGameImage(9u, 0xB00073Au, 0, 1);
            m_pHighlight = pHL;
            if (pHL)
            {
                pHL->SetBlockID(2);
                pHL->m_bFlag_446 = true;
                pHL->m_fPosX = m_baseX + 17.0f;
                pHL->m_bFlag_447 = true;
                pHL->m_fPosY = m_baseY + 19.0f;

                m_highlightAccum += dt;
                int hframes = 0;
                if (m_highlightAccum >= m_highlightThreshold)
                {
                    hframes = static_cast<int>(m_highlightAccum / m_highlightThreshold);
                    m_highlightAccum -= hframes * m_highlightThreshold;
                }

                int alpha;
                if (m_highlightUp)
                    alpha = m_highlightAlpha + hframes * 5;
                else
                    alpha = m_highlightAlpha - hframes * 5;

                m_highlightAlpha = alpha;
                if (alpha > 255)
                {
                    m_highlightAlpha = 255;
                    m_highlightUp = 0;
                }
                else if (alpha < 0)
                {
                    m_highlightAlpha = 0;
                    m_highlightUp = 1;
                }

                pHL->SetAlpha(static_cast<unsigned int>(m_highlightAlpha));
                pHL->m_bFlag_450 = true;
                pHL->m_bVertexAnimation = false;
                pHL->Process();
            }
        }

        // 指針的 scaleY 直接設為 curOffset（負值會把圖壓縮/往上延伸）
        pCursor->m_fScaleY = m_curOffset;
        pCursor->m_bFlag_449 = true;
        pCursor->m_bVertexAnimation = false;
        pCursor->Process();
    }

    // mofclient.c：若 running 中且 curOffset 已觸底，將 degree 欄位歸 0（原始：
    // *((_DWORD *)this + 3) = 0 —— 這條分支其實沒有影響主要邏輯，但保留。）
    if (m_running)
    {
        if (-m_maxRange >= m_curOffset)
            m_curOffset = 0.0f;
    }
}

void PowerBar::Render()
{
    if (m_pBar)
        m_pBar->Draw();
    if (m_pCursor)
        m_pCursor->Draw();
    if (m_running && m_pHighlight)
        m_pHighlight->Draw();
}
