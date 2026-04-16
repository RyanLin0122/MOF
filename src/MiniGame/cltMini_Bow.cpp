// cltMini_Bow — 弓箭手小遊戲，對齊 mofclient.c 原始實作。
// 每個函式都對齊 mofclient.c 的呼叫順序。

#include "MiniGame/cltMini_Bow.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Sound/GameSound.h"
#include "Logic/DirectInputManager.h"
#include "Logic/cltSystemMessage.h"
#include "System/cltLessonSystem.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Util/cltTimer.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_MiniGame_Archer_String.h"

extern unsigned char g_cGameBowState;

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2900 — 建構子
// ---------------------------------------------------------------------------
cltMini_Bow::cltMini_Bow()
    : m_bgResID(0), m_totalScore(0), m_difficultyBaseScore(0),
      m_winMark(0), m_currentRoundScore(0), m_finalScore(0), m_displayScore(0),
      m_bonusMultiplier(0), m_scoreCap(0),
      m_arrowShotCount(0), m_arrowLoaded(0), m_arrowShooting(0),
      m_arrowBlockID(0), m_targetMoveSpeed(0), m_totalPoint(0),
      m_firstTimeEnd(0), m_shootSpeed(0), m_targetX(0),
      m_arrowX(0), m_arrowY(0), m_startAreaX(0), m_startAreaY(0),
      m_hitTargetY(0), m_initArrowX(0), m_initArrowY(0),
      m_shootCounter(0), m_curArrowSlot(0), m_curTargetSlot(0),
      m_slotRanking(0), m_slotSelectDeg(0), m_slotHelp(0), m_slotShowPoint(0),
      m_slotWin(0), m_slotLose(0),
      m_slotIdx_11(0), m_slotIdx_12(0), m_slotIdx_13(0),
      m_slotIdx_14(0), m_slotIdx_15(0), m_slotIdx_16(0),
      m_timerHandle(0), m_pollFrame(0), m_prevState(100),
      m_drawAlphaBox(0), m_showTime2(0), m_difficulty(0),
      m_finalReady(0), m_serverAck(0), m_serverResult(0), m_serverValid(0),
      m_startTick(0), m_serverTimeMs(0), m_exitTick(0),
      m_pBgImage(nullptr)
{
    memset(m_slots, 0, sizeof(m_slots));
    memset(m_arrowScores, 0, sizeof(m_arrowScores));
    memset(m_arrowNumXPos, 0, sizeof(m_arrowNumXPos));

    // 對齊 mofclient.c：建構子讀取尚未初始化的 DWORD[1253]/[1254]，
    // InitMiniGameImage 後才設為 screenX+400 / screenY+367。
    m_initArrowX = 0;
    m_initArrowY = 0;
    m_arrowX = 0;
    m_arrowY = 0;

    g_cGameBowState = 0;
    m_prevState      = 100;
    m_showTime2      = 0;
    m_showTime       = 0;
    m_timerHandle    = 0;
    m_pollFrame      = 0;

    InitMiniGameImage();
    InitBtnFocus();
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2A80 — 解構子
// ---------------------------------------------------------------------------
cltMini_Bow::~cltMini_Bow()
{
}

// ---------------------------------------------------------------------------
// Button callbacks
// ---------------------------------------------------------------------------
void cltMini_Bow::OnBtn_Start(cltMini_Bow* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow::OnBtn_Ranking(cltMini_Bow* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow::OnBtn_Exit()
{
    g_cGameBowState = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow::OnBtn_RankingPre(cltMini_Bow* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow::OnBtn_RankingNext(cltMini_Bow* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow::OnBtn_Help(cltMini_Bow* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow::OnBtn_ShowPoint(cltMini_Bow* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow::OnBtn_ExitPopUp(cltMini_Bow* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow::OnBtn_DegreeEasy(cltMini_Bow* self)
{
    self->SetGameDegree(1);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Bow::OnBtn_DegreeNormal(cltMini_Bow* self)
{
    self->SetGameDegree(2);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Bow::OnBtn_DegreeHard(cltMini_Bow* self)
{
    self->SetGameDegree(4);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ---------------------------------------------------------------------------
// Timer callbacks
// ---------------------------------------------------------------------------
void cltMini_Bow::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Bow::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Bow* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
    m_pGameSoundMgr->PlaySoundA((char*)"M0002", 0, 0);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2B90 — Init_Ranking
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_Ranking()
{
    m_slots[m_slotRanking].active = 1;
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[3].SetActive(1); // RankingPre  (button idx 3 = offset +1124)
    m_buttons[4].SetActive(1); // RankingNext (button idx 4 = offset +1216)
    m_buttons[5].SetActive(1); // ExitPopUp   (button idx 5 = offset +1308)
    m_slots[m_slotWin].active  = 0;
    m_slots[m_slotLose].active = 0;
    m_drawNumFinal.SetActive(0);
    m_myRankingText[0] = 0;
    m_curRankPage = 0;
    memset(m_ranking, 0, sizeof(m_ranking));
    RequestRanking(0x14u, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameBowState = 7;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2C70 — Ranking
// ---------------------------------------------------------------------------
void cltMini_Bow::Ranking()
{
    if (m_pInputMgr->IsKeyDown(1)
        || m_pInputMgr->IsJoyButtonPush(1))
    {
        Init_Wait();
        m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
    }
    m_buttons[3].SetBtnState(0);
    if (m_curRankPage)
    {
        if (m_pInputMgr->IsKeyDown(16)
            || m_pInputMgr->IsJoyButtonPush(0))
        {
            Init_PreRanking();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
    }
    else
    {
        m_buttons[3].SetBtnState(4);
    }
    m_buttons[4].SetBtnState(0);
    if (m_curRankPage <= 0x1C && m_rankCount == 10)
    {
        if (m_pInputMgr->IsKeyDown(17)
            || m_pInputMgr->IsJoyButtonPush(0))
        {
            Init_NextRanking();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
    }
    else
    {
        m_buttons[4].SetBtnState(4);
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2D80 / 005B2DA0
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_PreRanking()
{
    if (m_curRankPage)
    {
        --m_curRankPage;
        RequestRanking(0x14u, m_curRankPage);
    }
}

void cltMini_Bow::Init_NextRanking()
{
    if (m_curRankPage < 0x1D)
    {
        ++m_curRankPage;
        RequestRanking(0x14u, m_curRankPage);
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3120 — Init_Wait
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_Wait()
{
    memset(m_arrowScores, 0, sizeof(m_arrowScores));
    m_serverTimeMs = 0;
    m_difficulty   = 0; // 原始碼：*((_BYTE*)this + 588) = 0 — 此處對應 Init_Wait 清除

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime2);
    m_buttons[1].SetActive(1);  // Ranking
    m_buttons[2].SetActive(1);  // Exit
    m_buttons[6].SetActive(1);  // Help
    m_buttons[7].SetActive(1);  // ShowPoint

    m_slots[m_slotWin].active       = 0;
    m_slots[m_slotLose].active      = 0;
    m_slots[m_slotSelectDeg].active = 0;
    m_slots[m_slotHelp].active      = 0;
    m_slots[m_slotShowPoint].active = 0;
    m_slots[m_slotRanking].active   = 0;

    m_drawNumFinal.SetActive(0);
    ResetArrow();
    m_slots[m_curArrowSlot].active  = 0;
    g_cGameBowState = 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3280 — SetGameDegree
// ---------------------------------------------------------------------------
void cltMini_Bow::SetGameDegree(std::uint8_t degree)
{
    m_difficulty        = degree;
    m_currentRoundScore = 30;
    m_bonusMultiplier   = 0.0f;

    switch (degree)
    {
    case 1: // Easy
        m_winMark             = 50;
        m_difficultyBaseScore = 100;
        m_scoreCap            = 90;
        m_bonusMultiplier     = 0.4f;
        m_bgResID             = 0x20000117u;
        break;
    case 2: // Normal
        m_winMark             = 100;
        m_difficultyBaseScore = 240;
        m_scoreCap            = 180;
        m_bonusMultiplier     = 1.0f;
        m_bgResID             = 0x20000119u;
        break;
    case 4: // Hard
        m_winMark             = 200;
        m_difficultyBaseScore = 320;
        m_scoreCap            = 360;
        m_bonusMultiplier     = 1.7f;
        m_bgResID             = 0x20000118u;
        break;
    }

    m_drawNumReady.SetActive(1);

    // 箭初始位置
    m_arrowX = static_cast<std::uint16_t>(m_screenX) + 401;
    m_arrowY = m_initArrowY;

    m_slots[m_curArrowSlot].active = 1;
    m_slots[m_slotWin].active      = 0;
    m_slots[m_slotLose].active     = 0;
    m_drawNumFinal.SetActive(0);

    m_arrowShotCount = 0;
    m_totalPoint     = 0;
    m_firstTimeEnd   = 1;
    m_totalScore     = 1;
    m_arrowLoaded    = 0;
    m_shootSpeed     = 100;

    InitMiniGameTime(0, 3u);
    m_timerHandle = 0;
    unsigned int readyTime = GetReadyTime();
    m_timerHandle = g_clTimerManager.CreateTimer(
        1000 * readyTime,
        reinterpret_cast<unsigned int>(this),
        1000u, 1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_DecreaseReadyTime),
        nullptr);

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[m_slotSelectDeg].active = 0;
    g_cGameBowState = 4;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3440 — Ready
// ---------------------------------------------------------------------------
void cltMini_Bow::Ready()
{
    m_slots[m_curArrowSlot].x = m_arrowX;
    m_slots[m_curArrowSlot].y = m_arrowY;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3480 — StartGame
// ---------------------------------------------------------------------------
void cltMini_Bow::StartGame()
{
    m_drawNumReady.SetActive(0);
    m_startAreaX   = static_cast<std::uint16_t>(m_initArrowX); // WORD[2506] = low16(DWORD[1253])
    m_startAreaY   = static_cast<std::uint16_t>(m_initArrowY); // WORD[2508] = low16(DWORD[1254])
    m_arrowBlockID = 14;
    m_curTargetSlot = 0;

    unsigned int t = timeGetTime();
    srand(t);
    CreateTarget();

    m_arrowShooting = 0;
    m_pInputMgr->IsKeyUp(57);
    m_startTick   = timeGetTime();
    m_serverAck   = 0;
    m_serverResult = 0;
    m_serverValid  = 0;
    m_focusEnabled = 0;
    g_cGameBowState = 5;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3520 — Gamming
// ---------------------------------------------------------------------------
void cltMini_Bow::Gamming()
{
    bool spacePressed = m_pInputMgr->IsKeyPressed(57);
    bool joyPush      = m_pInputMgr->IsJoyButtonPush(0);
    bool joyDown      = m_pInputMgr->IsJoyButtonDown(0);

    if ((spacePressed && !m_arrowLoaded)
        || (joyPush && !m_arrowLoaded)
        || (joyDown && !m_arrowLoaded))
    {
        LoadArrow();
        m_arrowLoaded = 0;
    }
    else
    {
        bool spaceUp = m_pInputMgr->IsKeyUp(57);
        if (spaceUp && !m_arrowLoaded && m_arrowShotCount < 10)
        {
            // 開始射箭
            m_arrowLoaded    = 1;
            m_arrowY         = m_initArrowY;
            m_shootCounter   = 0;
            m_pGameSoundMgr->PlaySoundA((char*)"M0005", 0, 0);
        }
        else
        {
            bool joyUp = m_pInputMgr->IsJoyButtonUp(0);
            if (joyUp)
            {
                if (m_arrowLoaded)
                {
                    ShootArrow();
                    goto after_shoot;
                }
                if (m_arrowShotCount < 10)
                {
                    m_arrowLoaded    = 1;
                    m_arrowY         = m_initArrowY;
                    m_shootCounter   = 0;
                    m_pGameSoundMgr->PlaySoundA((char*)"M0005", 0, 0);
                }
            }
        }
    }

    if (m_arrowLoaded)
        ShootArrow();

after_shoot:
    MoveTarget();
    m_slots[m_curTargetSlot].x    = m_targetX;
    m_slots[m_curArrowSlot].x     = m_arrowX;
    m_slots[m_curArrowSlot].y     = m_arrowY;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3670 — EndGame
// ---------------------------------------------------------------------------
void cltMini_Bow::EndGame()
{
    if (m_firstTimeEnd)
    {
        if (!cltValidMiniGameScore::IsValidScore(0x14u, m_totalPoint))
        {
            const char* txt = m_pDCTTextManager->GetText(58092);
            char buf[256];
            wsprintfA(buf, "client : %s : %i", txt, static_cast<int>(m_totalPoint));
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        m_totalScore = 0;
        m_finalScore = m_currentRoundScore;

        if (m_totalPoint < static_cast<std::uint16_t>(m_difficultyBaseScore))
        {
            // 失敗 — 原始碼 DWORD[5*(BYTE[609]+115)] = DWORD[5*m_slotLose+575] = m_slots[m_slotLose].active
            m_slots[m_slotLose].active = 1;
        }
        else
        {
            // 成功
            m_totalScore = 1;
            m_finalScore = m_winMark;
            m_slots[m_slotWin].active = 1;
        }

        m_drawNumFinal.SetActive(1);
        m_serverTimeMs = timeGetTime() - m_startTick;

        if (m_totalScore)
        {
            unsigned int extra = static_cast<unsigned int>(
                static_cast<double>(
                    static_cast<unsigned int>(m_totalPoint) - m_difficultyBaseScore)
                * m_bonusMultiplier)
                + m_finalScore;
            m_finalScore = extra;
            if (static_cast<unsigned int>(m_scoreCap) < extra)
                m_finalScore = m_scoreCap;
        }

        unsigned int baseScore = m_finalScore;
        int mult = GetMultipleNum();
        m_displayScore = baseScore * mult;
        SendScore(m_stage, m_totalPoint, m_serverTimeMs, m_difficulty, baseScore);
        m_firstTimeEnd = 0;
    }

    if (m_serverAck)
    {
        if (m_serverValid)
        {
            int result = m_serverResult;
            m_showTime2    = 4;
            m_serverValid  = 0;
            m_focusEnabled = 1;

            if (!result)
            {
                const char* txt = m_pDCTTextManager->GetText(58092);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, (char*)txt, 0, 0, 0);
                Init_Wait();
                return;
            }

            g_clLessonSystem.TraningLessonFinished(
                m_dword522, m_difficulty, m_totalScore,
                reinterpret_cast<unsigned int*>(&m_displayScore));

            if (!g_clLessonSystem.CanTraningLesson(0x14u))
                m_showTime2 = 0;

            m_buttons[12].SetActive(1); // ExitPopUp (button at offset +1952)
            InitBtnFocus();
        }

        if ((m_pInputMgr->IsKeyDown(1) && m_focusEnabled)
            || (m_pInputMgr->IsJoyButtonPush(1) && m_focusEnabled))
        {
            Init_Wait();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
        else if (timeGetTime() - m_exitTick > 5000)
        {
            Init_Wait();
        }
    }
    else
    {
        m_exitTick = timeGetTime();
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3910 — ExitGame
// ---------------------------------------------------------------------------
int cltMini_Bow::ExitGame()
{
    if (m_timerHandle)
    {
        g_clTimerManager.ReleaseTimer(m_timerHandle);
        m_timerHandle = 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3980 — MoveLargeArrow
// ---------------------------------------------------------------------------
void cltMini_Bow::MoveLargeArrow(std::uint8_t direction)
{
    if (direction == 1)
    {
        if (m_arrowX > static_cast<std::uint16_t>(m_screenX + 150))
            m_arrowX -= 2;
    }
    else if (direction == 2)
    {
        if (m_arrowX < static_cast<std::uint16_t>(m_screenY + 650))
            m_arrowX += 2;
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B39F0 — LoadArrow
// ---------------------------------------------------------------------------
void cltMini_Bow::LoadArrow()
{
    if (m_arrowY <= static_cast<std::uint16_t>(m_screenY + 400))
        m_arrowY = static_cast<std::uint16_t>(m_screenY) + 467;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3A20 — ShootArrow
// ---------------------------------------------------------------------------
void cltMini_Bow::ShootArrow()
{
    if (m_arrowShooting)
    {
        m_arrowX -= m_targetMoveSpeed;
    }
    else
    {
        ++m_shootCounter;
        if ((m_shootCounter & 1) == 0 || m_shootCounter == 1)
        {
            if (m_curArrowSlot < 10)
            {
                m_slots[m_curArrowSlot].active = 0;
                ++m_curArrowSlot;
                m_slots[m_curArrowSlot].active = 1;
            }
        }

        std::uint16_t curY = m_arrowY;
        int hitY = m_hitTargetY;
        if (curY > hitY)
        {
            std::uint16_t step = static_cast<std::uint16_t>(m_shootSpeed);
            m_arrowY = curY - step;
            int half = m_shootSpeed / 2;
            if (half <= 1) half = 1;
            m_shootSpeed = half;
        }

        if (m_arrowY <= static_cast<std::uint16_t>(hitY))
        {
            m_arrowY = static_cast<std::uint16_t>(m_hitTargetY); // WORD[2504] = low16(DWORD[1252])
            CheckPoint();
        }
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3B10 — ResetArrow
// ---------------------------------------------------------------------------
void cltMini_Bow::ResetArrow()
{
    m_arrowLoaded   = 0;
    m_arrowShooting = 0;

    // 清除 slots 2..6 (5 個箭 trail slots)
    for (int i = 0; i < 5; ++i)
        m_slots[2 + i].active = 0;

    m_arrowBlockID  = 14;
    m_curArrowSlot  = 2;
    m_slots[2].active = 1;

    m_arrowX     = m_initArrowX;
    m_arrowY     = m_initArrowY;
    m_shootSpeed = 100;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3B80 — CheckPoint
// ---------------------------------------------------------------------------
int cltMini_Bow::CheckPoint()
{
    int diff = abs(static_cast<int>(m_arrowX) - m_targetX);

    // 建立箭命中特效
    auto* pEffect = new CEffect_MiniGame_Archer_String();

    if (static_cast<std::uint16_t>(diff) >= 0x31u)
    {
        // 脫靶
        m_arrowScores[m_arrowShotCount] = 0;
        m_slots[m_curArrowSlot].active  = 0;
        float fx = static_cast<float>(m_screenX) + 400.0f;
        float fy = static_cast<float>(m_screenY) + 140.0f;
        pEffect->SetEffect(4, fx, fy);
        m_pGameSoundMgr->PlaySoundA((char*)"M0007", 0, 0);
    }
    else
    {
        // 命中
        char grade = 3;
        m_slots[m_curArrowSlot].y       = m_hitTargetY;
        m_slots[m_curTargetSlot].active  = 0;

        // 對齊 mofclient.c：DWORD[5*curTargetSlot+583] 寫入下一個 slot 的 x
        ++m_curTargetSlot;
        m_slots[m_curTargetSlot].x       = m_targetX;
        m_slots[m_curTargetSlot].active  = 1;
        m_slots[m_curTargetSlot].y       = m_hitTargetY;

        // 計分
        if (static_cast<std::uint16_t>(diff) == 0)
            m_arrowScores[m_arrowShotCount] = 80;
        else
            m_arrowScores[m_arrowShotCount] =
                static_cast<std::uint8_t>(50 - static_cast<int>(
                    static_cast<double>(static_cast<std::uint16_t>(diff)) * 1.041666666666667));

        std::uint8_t score = m_arrowScores[m_arrowShotCount];
        if (score > 0x31u)
            grade = 0;
        else if (score > 0x27u)
            grade = 1;
        else if (score > 0x1Du)
            grade = 2;

        m_arrowX -= m_targetMoveSpeed;
        float fx = static_cast<float>(m_screenX) + 400.0f;
        float fy = static_cast<float>(m_screenY) + 140.0f;
        pEffect->SetEffect(grade, fx, fy);
        m_pGameSoundMgr->PlaySoundA((char*)"M0006", 0, 0);
    }

    g_EffectManager_MiniGame.BulletAdd(pEffect);

    m_totalPoint += m_arrowScores[m_arrowShotCount];
    m_arrowShooting = 1;
    ++m_arrowShotCount;
    return 50 - diff;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3DF0 — MoveTarget
// ---------------------------------------------------------------------------
void cltMini_Bow::MoveTarget()
{
    m_targetX -= m_targetMoveSpeed;

    if (m_targetX < m_screenX - 120)
    {
        if (!m_arrowShooting)
            ++m_arrowShotCount;

        if (m_arrowShotCount == 10)
        {
            g_cGameBowState = 6;
        }
        else
        {
            ResetArrow();
            CreateTarget();
        }
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3E50 — CreateTarget
// ---------------------------------------------------------------------------
int cltMini_Bow::CreateTarget()
{
    static const char speeds[] = { 10, 8, 5 };

    int r = rand();
    m_curTargetSlot = 0;
    m_targetX       = r % 10 + m_screenX + 900;
    m_slots[0].active = 1;

    int r2 = rand();
    m_targetMoveSpeed = speeds[r2 % 3];
    return 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3EC0 — Init_SelectDegree
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[9].SetActive(1);   // DegreeEasy   (+1676)
    m_buttons[10].SetActive(1);  // DegreeNormal  (+1768)
    m_buttons[11].SetActive(1);  // DegreeHard    (+1860)
    m_slots[m_slotSelectDeg].active = 1;
    g_cGameBowState = 1;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3F30 — Init_Help
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);  // ExitPopUp (+1584)
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);
    m_slots[m_slotHelp].active = 1;
    g_cGameBowState = 2;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B3FB0 — Init_ShowPoint
// ---------------------------------------------------------------------------
void cltMini_Bow::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);
    m_slots[m_slotShowPoint].active = 1;
    g_cGameBowState = 3;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B2E60 — ShowPointText
// ---------------------------------------------------------------------------
void cltMini_Bow::ShowPointText()
{
    int baseX = m_uiPos[10];
    int baseY = m_uiPos[11];

    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3341);
    g_MoFFont.SetTextLineA(baseX + 70, baseY + 50,
                          0xFFFFFFFF, title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(baseX + 135, baseY + 127,
                          0xFFFFFFFF, "100",            1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 135, baseY + 157,
                          0xFFFFFFFF, "240",            1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 135, baseY + 187,
                          0xFFFFFFFF, "320",            1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 265, baseY + 127,
                          0xFFFFFFFF, " 50 ~  90",      1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 265, baseY + 157,
                          0xFFFFFFFF, "100 ~ 180",      1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 265, baseY + 187,
                          0xFFFFFFFF, "200 ~ 360",      1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 405, baseY + 127,
                          0xFFFFFFFF, "30",             1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 405, baseY + 157,
                          0xFFFFFFFF, "30",             1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 405, baseY + 187,
                          0xFFFFFFFF, "30",             1, -1, -1);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B4030 — Poll
// ---------------------------------------------------------------------------
int cltMini_Bow::Poll()
{
    m_drawAlphaBox = 0;
    ++m_pollFrame;

    if ((g_cGameBowState == 4 || g_cGameBowState == 5 || g_cGameBowState == 6)
        && static_cast<int>(m_pollFrame) < SETTING_FRAME)
    {
        return 0;
    }

    m_pollFrame = 0;

    switch (g_cGameBowState)
    {
    case 1: case 2: case 3:
        if (m_pInputMgr->IsKeyDown(1)
            || m_pInputMgr->IsJoyButtonPush(1))
        {
            Init_Wait();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
        m_drawAlphaBox = 1;
        break;
    case 4:
        Ready();
        break;
    case 5:
        Gamming();
        break;
    case 6:
        EndGame();
        break;
    case 7:
        Ranking();
        m_drawAlphaBox = 1;
        break;
    case 100:
        ExitGame();
        return 1;
    default:
        break;
    }

    // 按鈕 Poll
    if (!m_focusLocked)
    {
        for (int i = 0; i < kButtonCount; ++i)
        {
            if (m_buttons[i].Poll())
            {
                for (int j = 0; j < kButtonCount; ++j)
                {
                    if (m_buttonOrder[j] == i)
                        m_focusIdx = j;
                }
            }
        }
    }

    if (m_prevState != g_cGameBowState)
    {
        m_prevState = g_cGameBowState;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }
    MoveBtnFocus(g_cGameBowState);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B4220 — PrepareDrawing
// ---------------------------------------------------------------------------
void cltMini_Bow::PrepareDrawing()
{
    // 背景圖
    m_pBgImage = m_pclImageMgr->GetGameImage(9u, m_bgResID, 0, 1);
    if (m_pBgImage)
    {
        m_pBgImage->SetBlockID(0);
        m_pBgImage->m_bFlag_447 = true;
        m_pBgImage->m_bFlag_446 = true;
        m_pBgImage->m_bVertexAnimation = false;
        m_pBgImage->m_fPosX = static_cast<float>(m_screenX);
        m_pBgImage->m_fPosY = static_cast<float>(m_screenY);
    }

    // 箭得分數字
    if (m_arrowShotCount)
    {
        for (int i = 0; i < m_arrowShotCount; ++i)
        {
            std::uint16_t xp = m_arrowNumXPos[i];
            if (m_arrowScores[i] < 10)
                xp -= 8;
            m_arrowScoreNums[i].PrepareDrawing(xp, m_screenY + 59,
                                               m_arrowScores[i], 255);
        }
    }

    // Slot 圖像
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (m_slots[i].active)
        {
            GameImage* pImg = m_pclImageMgr->GetGameImage(
                9u, m_slots[i].resID, 0, 1);
            m_slots[i].pImage = pImg;
            if (pImg)
            {
                pImg->SetBlockID(m_slots[i].blockID);
                pImg->m_bFlag_447 = true;
                pImg->m_bFlag_446 = true;
                pImg->m_bVertexAnimation = false;
                pImg->m_fPosX = static_cast<float>(m_slots[i].x);
                pImg->m_fPosY = static_cast<float>(m_slots[i].y);
            }
        }
    }

    // 半透明遮罩
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();

    // 按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // DrawNum
    unsigned int readyT = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220, readyT, 255);
    m_drawNumPoint.PrepareDrawing(m_screenX + 480, m_screenY + 59,
                                  m_totalPoint, 255);
    m_drawNumFinal.PrepareDrawing(m_uiPos[8], m_uiPos[9], m_displayScore, 255);

    // AlphaBox
    m_topBlackBox.PrepareDrawing();
    m_midBlackBox.PrepareDrawing();

    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();

    if (g_Game_System_Info.ScreenWidth > 800)
    {
        m_sideBoxL.PrepareDrawing();
        m_sideBoxR.PrepareDrawing();
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B4480 — Draw
// ---------------------------------------------------------------------------
void cltMini_Bow::Draw()
{
    if (m_pBgImage)
        m_pBgImage->Draw();

    m_midBlackBox.Draw();

    // 遊戲 slot（0 .. m_slotHelp-1）
    for (int i = 0; i < m_slotHelp; ++i)
    {
        if (m_slots[i].active && m_slots[i].pImage)
            m_slots[i].pImage->Draw();
    }

    // 箭得分數字
    for (int i = 0; i < m_arrowShotCount; ++i)
        m_arrowScoreNums[i].Draw();

    m_topBlackBox.Draw();
    m_drawNumReady.Draw();
    m_drawNumPoint.Draw();
    m_drawNumFinal.Draw();

    // Help / ShowPoint popup slot
    if (m_slots[m_slotHelp].active && m_slots[m_slotHelp].pImage)
        m_slots[m_slotHelp].pImage->Draw();
    if (m_slots[m_slotShowPoint].active && m_slots[m_slotShowPoint].pImage)
        m_slots[m_slotShowPoint].pImage->Draw();

    // 按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    // 排名
    if (g_cGameBowState == 7)
    {
        if (m_rankDrawCounter)
            goto after_ranking;
        DrawRanking(m_uiPos[0], m_uiPos[1], 0);
    }

    if (g_cGameBowState == 3)
        ShowPointText();

after_ranking:
    if (g_Game_System_Info.ScreenWidth > 800)
    {
        m_sideBoxL.Draw();
        m_sideBoxR.Draw();
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B4610 — InitMiniGameImage
// ---------------------------------------------------------------------------
void cltMini_Bow::InitMiniGameImage()
{
    // 計算關鍵座標（對齊 mofclient.c DWORD[1252..1254]）
    m_hitTargetY  = m_screenY + 157;
    m_initArrowX  = m_screenX + 400;
    m_initArrowY  = m_screenY + 367;

    // --- Slot 索引設定 ---
    m_curArrowSlot  = 2;
    m_slotIdx_11    = 11;
    m_slotIdx_12    = 12;
    m_slotIdx_13    = 13;
    m_slotIdx_14    = 14;
    m_slotIdx_15    = 15;
    m_slotIdx_16    = 16;
    m_slotWin       = 17;
    m_slotLose      = 18;
    m_slotRanking   = 19;
    m_slotSelectDeg = 20;
    m_slotHelp      = 21;
    m_slotShowPoint = 22;

    // --- 清除所有 slot ---
    memset(m_slots, 0, sizeof(m_slots));

    // --- 填入 slot 資料 ---
    // Slot 0: 靶（左）
    m_slots[0].resID   = 0x10000022u;
    m_slots[0].blockID = 9;
    m_slots[0].x       = m_screenX + 200;
    m_slots[0].y       = m_hitTargetY;

    // Slot 1: 靶（右）
    m_slots[1].resID   = 0x10000022u;
    m_slots[1].blockID = 10;
    m_slots[1].x       = m_initArrowX;
    m_slots[1].y       = m_hitTargetY;

    // Slots 2-10: 箭（block 8, 0-7）
    std::uint16_t arrowXInit = static_cast<std::uint16_t>(m_arrowX);
    std::uint16_t arrowYInit = static_cast<std::uint16_t>(m_arrowY);
    m_slots[2].resID   = 0x10000022u;
    m_slots[2].blockID = 8;
    m_slots[2].x       = arrowXInit;
    m_slots[2].y       = arrowYInit;
    for (int i = 3; i <= 10; ++i)
    {
        m_slots[i].resID   = 0x10000022u;
        m_slots[i].blockID = static_cast<std::uint16_t>(i - 3);
        m_slots[i].x       = arrowXInit;
        m_slots[i].y       = arrowYInit;
    }

    // Slots 11-12: 裝飾 (0x22000015)
    m_slots[11].resID   = 0x22000015u;
    m_slots[11].blockID = 4;
    m_slots[11].x       = m_screenX + 686;
    m_slots[11].y       = m_screenY + 435;
    m_slots[12].resID   = 0x22000015u;
    m_slots[12].blockID = 5;
    m_slots[12].x       = m_screenX + 686;
    m_slots[12].y       = m_screenY + 435;

    // Slots 13-16: 裝飾 (0x22000007)
    m_slots[13].resID   = 0x22000007u;
    m_slots[13].blockID = 1;
    m_slots[13].x       = m_screenX + 686;
    m_slots[13].y       = m_screenY + 440;
    m_slots[14].resID   = 0x22000007u;
    m_slots[14].blockID = 5;
    m_slots[14].x       = m_screenX + 686;
    m_slots[14].y       = m_screenY + 440;
    m_slots[15].resID   = 0x22000007u;
    m_slots[15].blockID = 3;
    m_slots[15].x       = m_screenX + 742;
    m_slots[15].y       = m_screenY + 440;
    m_slots[16].resID   = 0x22000007u;
    m_slots[16].blockID = 7;
    m_slots[16].x       = m_screenX + 742;
    m_slots[16].y       = m_screenY + 440;

    // Slot 17: Win popup (0x20000002)
    m_slots[17].resID   = 0x20000002u;
    m_slots[17].blockID = 0;
    m_slots[17].x       = m_uiPos[4];
    m_slots[17].y       = m_uiPos[5];

    // Slot 18: Lose popup
    m_slots[18].resID   = 0x20000002u;
    m_slots[18].blockID = 1;
    m_slots[18].x       = m_uiPos[4];
    m_slots[18].y       = m_uiPos[5];

    // Slot 19: Ranking BG (0x2200000A)
    m_slots[19].resID   = 0x2200000Au;
    m_slots[19].blockID = 12;
    m_slots[19].x       = m_uiPos[0];
    m_slots[19].y       = m_uiPos[1];

    // Slot 20: Select degree (0x1000001B)
    m_slots[20].resID   = 0x1000001Bu;
    m_slots[20].blockID = 20;
    m_slots[20].x       = m_uiPos[8];
    m_slots[20].y       = m_uiPos[9];

    // Slot 21: Help (0x10000019)
    m_slots[21].resID   = 0x10000019u;
    m_slots[21].blockID = 0;
    m_slots[21].x       = m_uiPos[6];
    m_slots[21].y       = m_uiPos[7];

    // Slot 22: ShowPoint (0x10000020)
    m_slots[22].resID   = 0x10000020u;
    m_slots[22].blockID = 0;
    m_slots[22].x       = m_uiPos[10];
    m_slots[22].y       = m_uiPos[11];

    // --- 建立 13 個按鈕（對齊 mofclient.c） ---
    // Button 0: Start
    m_buttons[0].CreateBtn(
        m_screenX + 37, m_screenY + 472, 9u,
        0x2200000Au, 0, 0x2200000Au, 3u, 0x2200000Au, 6u,
        0x20000014u, 9u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_Start),
        reinterpret_cast<unsigned int>(this), 1);

    // Button 1: Ranking
    m_buttons[1].CreateBtn(
        m_screenX + 183, m_screenY + 472, 9u,
        0x2200000Au, 1u, 0x2200000Au, 4u, 0x2200000Au, 7u,
        0x20000014u, 0xAu,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_Ranking),
        reinterpret_cast<unsigned int>(this), 1);

    // Button 2: Exit
    m_buttons[2].CreateBtn(
        m_screenX + 621, m_screenY + 472, 9u,
        0x2200000Au, 2u, 0x2200000Au, 5u, 0x2200000Au, 8u,
        0x20000014u, 0xBu,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_Exit),
        reinterpret_cast<unsigned int>(this), 1);

    // Button 3: RankingPre
    m_buttons[3].CreateBtn(
        m_uiPos[0] + 17, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0xDu, 0x2200000Au, 0xFu, 0x2200000Au, 0x11u,
        0x2200000Au, 0x13u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_RankingPre),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 4: RankingNext
    m_buttons[4].CreateBtn(
        m_uiPos[0] + 62, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0xEu, 0x2200000Au, 0x10u, 0x2200000Au, 0x12u,
        0x2200000Au, 0x14u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_RankingNext),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 5: Ranking ExitPopUp
    m_buttons[5].CreateBtn(
        m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 6: Help
    m_buttons[6].CreateBtn(
        m_screenX + 329, m_screenY + 472, 9u,
        0x1000009Bu, 0xCu, 0x1000009Bu, 0xEu, 0x1000009Bu, 0x10u,
        0x1000009Bu, 0x12u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_Help),
        reinterpret_cast<unsigned int>(this), 1);

    // Button 7: ShowPoint
    m_buttons[7].CreateBtn(
        m_screenX + 475, m_screenY + 472, 9u,
        0x1000009Bu, 0xDu, 0x1000009Bu, 0xFu, 0x1000009Bu, 0x11u,
        0x1000009Bu, 0x13u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_ShowPoint),
        reinterpret_cast<unsigned int>(this), 1);

    // Button 8: Generic ExitPopUp (Help/ShowPoint 共用)
    m_buttons[8].CreateBtn(
        m_screenX + 566, m_screenY + 513, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 9: DegreeEasy
    m_buttons[9].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 48, 9u,
        0x1000009Bu, 0, 0x1000009Bu, 3u, 0x1000009Bu, 6u,
        0x1000009Bu, 9u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_DegreeEasy),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 10: DegreeNormal
    m_buttons[10].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 102, 9u,
        0x1000009Bu, 1u, 0x1000009Bu, 4u, 0x1000009Bu, 7u,
        0x1000009Bu, 0xAu,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_DegreeNormal),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 11: DegreeHard
    m_buttons[11].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 156, 9u,
        0x1000009Bu, 2u, 0x1000009Bu, 5u, 0x1000009Bu, 8u,
        0x1000009Bu, 0xBu,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_DegreeHard),
        reinterpret_cast<unsigned int>(this), 0);

    // Button 12: EndGame ExitPopUp
    m_buttons[12].CreateBtn(
        m_uiPos[4] + 215, m_uiPos[5] + 170, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    // --- 初始化 DrawNum ---
    for (int i = 0; i < kArrowCount; ++i)
    {
        m_arrowScoreNums[i].InitDrawNum(9u, 0x22000016u, 0, 0);
        m_arrowScoreNums[i].SetActive(1);
        m_arrowNumXPos[i] = static_cast<std::uint16_t>(m_screenX) + 40 * i + 51;
    }
    m_drawNumPoint.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumReady.SetActive(0);
    m_drawNumPoint.SetActive(1);
    m_drawNumFinal.SetActive(0);

    // --- 初始化 AlphaBox ---
    m_topBlackBox.Create(m_screenX, 0, 800, 40,
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    m_midBlackBox.Create(m_screenX, m_screenY + 500, 800, 100,
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    m_alphaBox.Create(m_screenX, 9, 800, 600,
                      0.0f, 0.0f, 0.0f, 0.5f, nullptr);

    m_drawAlphaBox = 0;
    m_bgResID = 0x20000117u;
    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_02.ogg");

    if (g_Game_System_Info.ScreenWidth > 800)
    {
        int sideW = (g_Game_System_Info.ScreenWidth - 800) / 2;
        m_sideBoxL.Create(0, 0,
            static_cast<unsigned short>(sideW),
            static_cast<unsigned short>(g_Game_System_Info.ScreenHeight),
            0.0f, 0.0f, 0.0f, 1.0f, nullptr);
        m_sideBoxR.Create(sideW + 800, 0,
            static_cast<unsigned short>(sideW),
            static_cast<unsigned short>(g_Game_System_Info.ScreenHeight),
            0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    }
}
