#include "MiniGame/Sword2.h"

#include <cstdio>

#include "global.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Logic/DirectInputManager.h"

Sword2::Sword2()
    : m_state(0)
    , m_round(0)
    , m_remainSec(10)
    , m_power(0)
    , m_pad4(0)
    , m_totalScore(0)
    , m_degree(0)
    , m_info()
    , m_powerBar()
    , m_accBar()
    , m_board()
    , m_bgMgr()
    , m_blackTop()
    , m_blackBottom()
    , m_timer()
    , m_gameOver(0)
    , m_roundFinished(0)
    , m_screenX((g_Game_System_Info.ScreenWidth - 800) / 2)
    , m_screenY((g_Game_System_Info.ScreenHeight - 600) / 2)
    , m_endAccum(0.0f)
    , m_endThreshold(0.01666666667f)    // 1/60
    , m_endPhase(0)
    , m_endY(0.0f)
    , m_endSubState(0)
    , m_pBgImage(nullptr)
    , m_pShake1(nullptr)
    , m_pShake2(nullptr)
    , m_shakeY(5.0f)
    , m_endGraphStep(0)
    , m_endGraphTimer(0.0f)
    , m_endGameOverTimer(0.0f)
{
    // 依原始：建構子內初始化 BackGroundMgr 與兩條黑色邊條。
    m_bgMgr.InitBackGroundMgr(0, 0.0f, -140.0f);

    float topY = static_cast<float>(m_screenY);
    m_blackTop.CreateBlackBG(static_cast<float>(m_screenX), topY, 800.0f, 50.0f);

    float botY = static_cast<float>(m_screenY) + 550.0f;
    m_blackBottom.CreateBlackBG(static_cast<float>(m_screenX), botY, 800.0f, 50.0f);
}

Sword2::~Sword2() = default;

void Sword2::InitSword2(int degree)
{
    m_degree     = degree;
    m_round      = 0;
    m_totalScore = 0;
    m_info.InitInfoMgr();
    InitRound();
    // 原始在 InitSword2 末端再覆寫一次結算 FrameSkip threshold = 1/30
    m_endThreshold = 0.0333333333f;
}

void Sword2::InitRound()
{
    m_screenX   = (g_Game_System_Info.ScreenWidth  - 800) / 2;
    m_screenY   = (g_Game_System_Info.ScreenHeight - 600) / 2;
    m_state     = 2;
    m_remainSec = 10;
    m_power     = 0;

    float x = static_cast<float>(m_screenX);
    float y = static_cast<float>(m_screenY);

    m_board.InitBoardMgr(x + 400.0f, y + 415.0f);
    m_powerBar.InitPowerBar(m_degree, x + 370.0f, y + 255.0f);
    m_accBar  .InitAccuracyBar(m_degree, x + 400.0f, y + 220.0f);

    m_endPhase = 0;
    m_endY     = 0.0f;

    m_timer.InitTimer(1);
    m_remainSec = 10;
    m_info.SetTime(10);

    m_gameOver         = 0;
    m_roundFinished    = 0;
    m_endGameOverTimer = 0.0f;
    m_endGraphTimer    = 0.0f;
    m_endGraphStep     = 0;
    m_pShake1          = nullptr;
    m_pShake2          = nullptr;
    m_shakeY           = 5.0f;
}

int Sword2::Process()
{
    float dt = static_cast<float>(m_timer.GetCurrentFrameTime());

    // --- 狀態機 ---
    if (m_state == 2)
    {
        if (m_powerBar.IsRunning()
            && (cltMoF_BaseMiniGame::m_pInputMgr->IsKeyDown(57)
                || cltMoF_BaseMiniGame::m_pInputMgr->IsJoyButtonPush(0)))
        {
            m_power = m_powerBar.GetPower();
            m_accBar.Run();
            m_state = 3;
        }
        m_powerBar.Process(dt);
        m_accBar  .Process(dt);
        m_board   .Process(dt);
    }
    else if (m_state == 3)
    {
        if (m_accBar.IsRunning()
            && (cltMoF_BaseMiniGame::m_pInputMgr->IsKeyDown(57)
                || cltMoF_BaseMiniGame::m_pInputMgr->IsJoyButtonPush(0)))
        {
            int newPower = m_power - m_accBar.GetAccuracy();
            m_power = newPower;
            if (newPower < 1)
                m_power = 1;
            m_gameOver = 1;
        }
        m_powerBar.Process(dt);
        if (m_accBar.Process(dt))
            m_state = 4;
        m_board.Process(dt);
    }
    else if (m_state == 4)
    {
        // 結算動畫 FrameSkip
        m_endAccum += dt;
        int frames = 0;
        if (m_endAccum >= m_endThreshold)
        {
            frames = static_cast<int>(m_endAccum / m_endThreshold);
            m_endAccum -= frames * m_endThreshold;
        }

        float v42 = 0.0f;
        if (m_endPhase == 0)
        {
            // Phase 0：讓畫面先往下平移 10 單位，等 BackGround 完成後進入 phase 1。
            m_board.SetPositionY(10.0f);
            if (m_bgMgr.Image_DOWN(10.0f))
                m_endPhase = 1;
        }
        else if (m_endPhase == 1)
        {
            // 輸出原始 debug 訊息
            char buf[256];
            std::snprintf(buf, sizeof(buf), "%i: %f\n", m_endGraphStep, m_endGraphTimer);
            OutputDebugStringA(buf);

            switch (m_endGraphStep)
            {
                case 0:
                {
                    float step = static_cast<float>(frames) * 10.0f;
                    v42 = step;
                    m_endGraphTimer += step;
                    if (m_endGraphTimer > 30.0f)
                        m_endGraphStep = 1;
                    break;
                }
                case 1:
                {
                    float step = static_cast<float>(frames) * 20.0f;
                    v42 = step;
                    m_endGraphTimer += step;
                    if (m_endGraphTimer > 90.0f)
                        m_endGraphStep = 2;
                    break;
                }
                case 2:
                {
                    float step = static_cast<float>(frames) * 30.0f;
                    v42 = step;
                    m_endGraphTimer += step;
                    if (m_endGraphTimer > 150.0f)
                        m_endGraphStep = 3;
                    break;
                }
                case 3:
                {
                    float step = static_cast<float>(frames) * 40.0f;
                    v42 = step;
                    m_endGraphTimer += step;
                    if (m_endGraphTimer > 600.0f)
                        m_endGraphStep = 4;
                    break;
                }
                default:
                    break;
            }
            m_board.SetPositionY(-v42);
            if (m_bgMgr.Image_UP(v42))
            {
                m_endPhase = 2;
                m_board.SetBreakBoard(m_power);
            }
            m_shakeY -= v42;

            // 兩支「震動火花」圖示
            cltImageManager* pMgr = cltImageManager::GetInstance();

            GameImage* pS1 = pMgr->GetGameImage(9u, 0xB00073Au, 0, 1);
            m_pShake1 = pS1;
            if (pS1)
            {
                pS1->SetBlockID(0);
                pS1->m_bFlag_446 = true;
                pS1->m_fPosX = static_cast<float>(m_screenX) + 135.0f;
                pS1->m_bFlag_447 = true;
                pS1->m_fPosY = static_cast<float>(m_screenY) + m_shakeY;
                pS1->Process();
            }

            GameImage* pS2 = pMgr->GetGameImage(9u, 0xB00073Au, 0, 1);
            m_pShake2 = pS2;
            if (pS2)
            {
                pS2->SetBlockID(0);
                pS2->m_bFlag_446 = true;
                pS2->m_fPosX = static_cast<float>(m_screenX) + 470.0f;
                pS2->m_bFlag_447 = true;
                pS2->m_fPosY = static_cast<float>(m_screenY) + m_shakeY;
                pS2->Process();
            }
        }

        if (m_board.Process(dt))
        {
            m_info.SetScore(m_round, m_power);
            m_totalScore += m_power;
            m_info.SetScore(5, m_totalScore);
            ++m_round;
            if (m_round == 5)
            {
                m_roundFinished = 1;
                m_pShake1 = nullptr;
                m_pShake2 = nullptr;
            }
            else
            {
                InitRound();
            }
        }
    }

    // --- 上方 TopHat 圖示（無論狀態皆繪製）---
    {
        cltImageManager* pMgr = cltImageManager::GetInstance();
        GameImage* pBg = pMgr->GetGameImage(9u, 0x20000039u, 0, 1);
        m_pBgImage = pBg;
        if (pBg)
        {
            pBg->SetBlockID(5);
            pBg->m_bFlag_446 = true;
            pBg->m_fPosX = static_cast<float>(m_screenX) + 400.0f;
            pBg->m_bFlag_447 = true;
            pBg->m_fPosY = static_cast<float>(m_screenY) + 50.0f;
            pBg->Process();
        }
    }

    // --- 剩餘秒數 ---
    if (!m_gameOver)
    {
        int tick = m_timer.GetCurrentSecondDelta();
        if (tick)
        {
            int newRemain = m_remainSec - tick;
            m_remainSec = newRemain;
            if (newRemain > 0)
            {
                m_info.SetTime(newRemain);
            }
            else
            {
                m_info.SetScore(m_round, 0);
                ++m_round;
                if (m_round == 5)
                {
                    m_roundFinished = 1;
                    m_gameOver      = 1;
                    m_state         = 4;
                    m_endPhase      = 2;
                }
                else
                {
                    InitRound();
                }
            }
        }
    }

    m_info.Process(dt);

    int result = 0;
    if (m_roundFinished)
    {
        m_endGameOverTimer += dt;
        if (m_endGameOverTimer > 1.2f)
            result = 1;
    }
    return result;
}

void Sword2::Render()
{
    if (m_state == 4)
    {
        if (m_pShake1) m_pShake1->Draw();
        if (m_pShake2) m_pShake2->Draw();
    }
    m_info.Render();
    m_board.Render();
    if (m_state == 2 || m_state == 3)
    {
        m_powerBar.Render();
        m_accBar.Render();
    }
    if (m_pBgImage && m_pBgImage->m_pGIData)
        m_pBgImage->Draw();
    m_blackTop.RenderBlackBG();
    m_blackBottom.RenderBlackBG();
}
