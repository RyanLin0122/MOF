#include "MiniGame/AccuracyBar.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "global.h"
#include "Sound/GameSound.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

AccuracyBar::AccuracyBar()
    : m_degree(0)
    , m_speed(0.0f)
    , m_center(432.0f)              // 1138032640
    , m_half(0.0f)
    , m_curPos(0.0f)
    , m_dir(0)
    , m_running(0)
    , m_pBar(nullptr)
    , m_pCursor(nullptr)
    , m_pHighlight(nullptr)
    , m_highlightUp(0)
    , m_highlightAlpha(0)
    , m_highlightAccum(0.0f)
    , m_highlightThreshold(0.0083333333f)  // 1/120 (1007192201)
    , m_baseX(0.0f)
    , m_baseY(0.0f)
    , m_pointerAccum(0.0f)
    , m_pointerThreshold(0.03333333f)      // 1/30 (1023969417)
    , m_hitLocked(0)
    , m_endTimer(0.0f)
{
}

AccuracyBar::~AccuracyBar() = default;

void AccuracyBar::InitAccuracyBar(int degree, float x, float y)
{
    m_degree = degree;
    int r = std::rand() % 3;
    m_center = 432.0f;
    m_speed  = 0.0f;

    // mofclient.c：依難度 × 隨機索引決定 m_speed
    //  1097020211 = 15.00f
    //  1094538581 = 12.30f
    //  1092765989 = 10.95f
    //
    // degree=0: r=0→15, r=1→12.30, r=2→(no set)
    // degree=1: r=0→15, r=1→12.30, r=2→10.95
    // degree=2: r=0→15, r=1→12.30, r=2→10.95
    auto f = [](unsigned int bits) -> float {
        float f; std::memcpy(&f, &bits, sizeof(f)); return f;
    };
    switch (degree)
    {
        case 0:
            if (r == 0) m_speed = f(1097020211u);
            else if (r == 1) m_speed = f(1094538581u);
            break;
        case 1:
            if (r == 0) m_speed = f(1097020211u);
            else if (r == 1) m_speed = f(1094538581u);
            else if (r == 2) m_speed = f(1092765989u);
            break;
        case 2:
            if (r == 0) m_speed = f(1097020211u);
            else if (r == 1) m_speed = f(1094538581u);
            else if (r == 2) m_speed = f(1092765989u);
            break;
    }

    m_dir = 0;
    m_curPos = x;
    m_half = x;              // mofclient.c：*((float*)this + 3) = x;
    m_running = 0;
    m_pBar = nullptr;
    m_pCursor = nullptr;
    m_highlightUp = 0;
    m_pHighlight = nullptr;
    m_highlightAlpha = 0;
    m_baseX = x;
    m_baseY = y;
    m_hitLocked = 0;
    m_endTimer = 0.0f;
}

void AccuracyBar::Run()
{
    m_running = 1;
}

int AccuracyBar::GetAccuracy()
{
    g_GameSoundManager.PlaySoundA((char*)"M0018", 0, 0);
    double diff = std::fabs(static_cast<double>(m_curPos)) - static_cast<double>(m_half);
    m_running = 0;
    m_hitLocked = 1;
    diff = std::fabs(diff);
    if (diff < 5.0)  return 0;
    if (diff < 10.0) return 1;
    if (diff < 15.0) return 2;
    if (diff < 20.0) return 3;
    if (diff < 25.0) return 4;
    if (diff < 30.0) return 5;
    if (diff < 35.0) return 6;
    if (diff < 40.0) return 7;
    if (diff < 45.0) return 8;
    return 9;
}

bool AccuracyBar::Process(float dt)
{
    cltImageManager* pMgr = cltImageManager::GetInstance();

    // 底圖（bar）
    GameImage* pBar = pMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pBar = pBar;
    if (pBar)
    {
        pBar->SetBlockID(0);
        pBar->m_bFlag_446 = true;
        pBar->m_fPosX = m_baseX;
        pBar->m_bFlag_447 = true;
        pBar->m_fPosY = m_baseY;
        pBar->Process();
    }

    // 指針（cursor）
    GameImage* pCur = pMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pCursor = pCur;
    if (pCur)
    {
        pCur->SetBlockID(1);
        pCur->m_bFlag_446 = true;

        if (m_running)
        {
            if (m_dir)
            {
                // 向 + 方向擺動（透過 FrameSkip 累積 dt）
                m_pointerAccum += dt;
                int frames = 0;
                if (m_pointerAccum >= m_pointerThreshold)
                {
                    frames = static_cast<int>(m_pointerAccum / m_pointerThreshold);
                    m_pointerAccum -= frames * m_pointerThreshold;
                }
                m_curPos += frames * m_speed;
                float upper = m_center * 0.5f + m_half;
                if (upper <= m_curPos)
                {
                    m_dir = 0;
                    m_curPos = upper;
                }
            }
            else
            {
                // 向 - 方向擺動（注意 mofclient 這條分支沒有 FrameSkip）
                m_curPos -= m_speed;
                float lower = m_half - m_center * 0.5f;
                if (lower >= m_curPos)
                {
                    m_dir = 1;
                    m_curPos = lower;
                }
            }

            // highlight alpha
            GameImage* pHL = pMgr->GetGameImage(9u, 0xB00073Au, 0, 1);
            m_pHighlight = pHL;
            if (pHL)
            {
                pHL->SetBlockID(1);
                pHL->m_bFlag_446 = true;
                pHL->m_fPosX = m_baseX - 215.0f;
                pHL->m_bFlag_447 = true;
                pHL->m_fPosY = m_baseY - 2.0f;

                m_highlightAccum += dt;
                int frames = 0;
                if (m_highlightAccum >= m_highlightThreshold)
                {
                    frames = static_cast<int>(m_highlightAccum / m_highlightThreshold);
                    m_highlightAccum -= frames * m_highlightThreshold;
                }

                int alpha;
                if (m_highlightUp)
                    alpha = m_highlightAlpha + frames * 5;
                else
                    alpha = m_highlightAlpha - frames * 5;

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

        pCur->m_fPosX = m_curPos;
        pCur->m_bFlag_447 = true;
        pCur->m_fPosY = m_baseY - 3.0f;
        pCur->Process();
    }

    if (m_hitLocked)
    {
        m_endTimer += dt;
        if (m_endTimer > 1.0f)
            return true;
    }
    return false;
}

void AccuracyBar::Render()
{
    if (m_running && m_pCursor && m_pHighlight)
        m_pHighlight->Draw();
    if (m_pBar)
        m_pBar->Draw();
    if (m_pCursor)
        m_pCursor->Draw();
}
