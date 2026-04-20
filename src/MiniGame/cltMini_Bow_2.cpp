// cltMini_Bow_2 — 弓箭手小遊戲 2（躲飛矛），對齊 mofclient.c 原始實作。

#include "MiniGame/cltMini_Bow_2.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <windows.h>

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

extern unsigned char g_cGameBow_2State;

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5010 — 建構子
// ---------------------------------------------------------------------------
cltMini_Bow_2::cltMini_Bow_2()
{
    // GT: 子物件建構後立即呼叫 InitMiniGameImage（內部會 memset slots）
    InitMiniGameImage();

    // GT: BYTE[49] = 1
    m_showTime = 1;

    // GT: DWORD[3934..3940] = 0 — 清除 7 個 timer handles
    m_timerLiveTime     = 0;
    m_timerNormalSpear  = 0;
    m_timerSnipeSpear   = 0;
    m_timerOctetSpear   = 0;
    m_timer2VolleySpear = 0;
    m_timerRainSpear    = 0;
    m_timerHorizonSpear = 0;

    // GT: DWORD[3949] = 0
    m_pollFrame = 0;

    // GT: BYTE[2061] = 100, BYTE[568] = 0
    m_prevState = 100;
    m_showTime2 = 0;

    InitBtnFocus();
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5190 — 解構子
// ---------------------------------------------------------------------------
cltMini_Bow_2::~cltMini_Bow_2()
{
}

// ---------------------------------------------------------------------------
// Button callbacks
// ---------------------------------------------------------------------------
void cltMini_Bow_2::OnBtn_Start(cltMini_Bow_2* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow_2::OnBtn_Ranking(cltMini_Bow_2* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow_2::OnBtn_Exit()
{
    g_cGameBow_2State = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow_2::OnBtn_RankingPre(cltMini_Bow_2* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow_2::OnBtn_RankingNext(cltMini_Bow_2* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow_2::OnBtn_Help(cltMini_Bow_2* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow_2::OnBtn_ShowPoint(cltMini_Bow_2* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Bow_2::OnBtn_ExitPopUp(cltMini_Bow_2* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Bow_2::OnBtn_DegreeEasy(cltMini_Bow_2* self)
{
    self->SetGameDegree(1);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Bow_2::OnBtn_DegreeNormal(cltMini_Bow_2* self)
{
    self->SetGameDegree(2);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Bow_2::OnBtn_DegreeHard(cltMini_Bow_2* self)
{
    self->SetGameDegree(4);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ---------------------------------------------------------------------------
// Timer callbacks
// ---------------------------------------------------------------------------
void cltMini_Bow_2::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Bow_2::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Bow_2* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
}

void cltMini_Bow_2::OnTimer_IncreaseLiveTime(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->IncreaseLiveTime();
}

void cltMini_Bow_2::OnTimer_CreateNormalSpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->CreateNormalSpear();
}

void cltMini_Bow_2::OnTimer_CreateSnipeSpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->CreateSnipeSpear();
}

void cltMini_Bow_2::OnTimer_CreateOctetSpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->CreateOctetSpear();
}

void cltMini_Bow_2::OnTimer_Create2VolleySpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->Create2VolleySpear();
}

void cltMini_Bow_2::OnTimer_CreateRainSpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->CreateRainSpear();
}

void cltMini_Bow_2::OnTimer_CreateHorizonSpear(unsigned int /*id*/, cltMini_Bow_2* self)
{
    self->CreateHorizonSpear();
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B52A0 — Init_SelectDegree
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[9].SetActive(1);   // DegreeEasy
    m_buttons[10].SetActive(1);  // DegreeNormal
    m_buttons[11].SetActive(1);  // DegreeHard
    m_slots[m_slotSelectDeg].active = 1;
    g_cGameBow_2State = 1;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5330 — Init_Ranking
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_Ranking()
{
    m_slots[m_slotRanking].active = 1;
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[3].SetActive(1); // RankingPre
    m_buttons[4].SetActive(1); // RankingNext
    m_buttons[5].SetActive(1); // ExitPopUp
    m_slots[m_slotWin].active  = 0;
    m_slots[m_slotLose].active = 0;
    m_myRankingText[0] = 0;
    m_curRankPage = 0;
    memset(m_ranking, 0, sizeof(m_ranking));
    RequestRanking(0x15u, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameBow_2State = 7;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5430 / 005B5470
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_PreRanking()
{
    if (m_curRankPage)
    {
        --m_curRankPage;
        RequestRanking(0x15u, m_curRankPage);
    }
}

void cltMini_Bow_2::Init_NextRanking()
{
    if (m_curRankPage < 0x1D)
    {
        ++m_curRankPage;
        RequestRanking(0x15u, m_curRankPage);
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B54B0 — Init_Help
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);
    m_slots[m_slotHelp].active = 1;
    g_cGameBow_2State = 2;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5550 — Init_ShowPoint
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);
    m_slots[m_slotShowPoint].active = 1;
    g_cGameBow_2State = 3;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5650 — SetGameDegree
// ---------------------------------------------------------------------------
void cltMini_Bow_2::SetGameDegree(std::uint8_t degree)
{
    m_difficulty        = degree;
    m_currentRoundScore = 30;
    m_bonusMultiplier   = 0.0f;

    switch (degree)
    {
    case 1: // Easy
        m_winMark             = 50;
        m_difficultyBaseScore = 7000;
        m_scoreCap            = 90;
        m_bonusMultiplier     = 4.0f;
        m_bgResID             = 0x20000110u;
        break;
    case 2: // Normal
        m_winMark             = 100;
        m_difficultyBaseScore = 15000;
        m_scoreCap            = 180;
        m_bonusMultiplier     = 8.0f;
        m_bgResID             = 0x20000112u;
        break;
    case 4: // Hard
        m_winMark             = 200;
        m_difficultyBaseScore = 25000;
        m_scoreCap            = 360;
        m_bonusMultiplier     = 16.0f;
        m_bgResID             = 0x20000111u;
        break;
    }

    m_char.Initalize(210.0f, degree, m_screenX, m_screenY);

    m_slots[m_slotWin].active      = 0;
    m_slots[m_slotLose].active     = 0;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[m_slotSelectDeg].active = 0;

    InitMiniGameTime(m_difficultyBaseScore, 3u);
    m_dword149 = 0;
    unsigned int readyTime = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000 * readyTime,
        reinterpret_cast<std::uintptr_t>(this),
        0x3E8u, 1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_DecreaseReadyTime),
        nullptr);

    m_drawNumReady.SetActive(1);
    m_firstTimeEnd = 1;
    m_totalScore   = 1;
    m_liveTime     = 0;
    g_cGameBow_2State = 4;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B57F0..005B5880 — LiveTime
// ---------------------------------------------------------------------------
void cltMini_Bow_2::IncreaseLiveTime()
{
    m_liveTimeChanged = 1;
    m_liveTime += 105;
}

unsigned int cltMini_Bow_2::GetLiveTime()
{
    return m_liveTime;
}

// ---------------------------------------------------------------------------
// mofclient.c — Ready（state 4，等待倒數結束）
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Ready()
{
    // 空實作：倒數由 timer callback 驅動
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5BE0 — StartGame
// ---------------------------------------------------------------------------
void cltMini_Bow_2::StartGame()
{
    m_drawNumReady.SetActive(0);
    m_char.SetActive(1);
    memset(m_spears, 0, sizeof(m_spears));

    m_timerLiveTime = 0;
    m_timerLiveTime = g_clTimerManager.CreateTimer(
        0xFFFFFFFF,
        reinterpret_cast<std::uintptr_t>(this),
        0x64u, 1,
        nullptr, nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_IncreaseLiveTime),
        nullptr);

    m_timerNormalSpear = 0;
    m_timerNormalSpear = g_clTimerManager.CreateTimer(
        0xFFFFFFFF,
        reinterpret_cast<std::uintptr_t>(this),
        0x190u, 1,
        nullptr, nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_CreateNormalSpear),
        nullptr);

    m_drawNumLiveTime.SetActive(1);
    m_drawNumTarget.SetActive(1);
    m_liveTimeChanged   = 0;
    m_alphaWhiteValue   = 0;
    m_alphaRedValue     = 0;
    m_whiteFlash.SetAlpha(0);
    m_redFlash.SetAlpha(static_cast<unsigned char>(m_alphaRedValue));
    m_hitPhase          = 0;
    m_isHit             = 0;
    m_startTick         = timeGetTime();
    m_serverAck         = 0;
    m_serverResult      = 0;
    m_serverValid       = 0;
    m_focusEnabled      = 0;
    g_cGameBow_2State   = 5;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B5CF0 — Gamming
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Gamming()
{
    float posX = static_cast<float>(m_char.GetPosX());
    float posY = static_cast<float>(m_char.GetPosY());

    CheckCollision();

    if (m_isHit)
    {
        // 命中動畫：白閃/紅閃交替
        switch (m_hitPhase)
        {
        case 0:
            m_alphaWhiteValue += 10;
            if (m_alphaWhiteValue > 175)
            {
                m_alphaWhiteValue = 175;
                m_hitPhase = 1;
            }
            break;
        case 1:
            m_alphaWhiteValue -= 10;
            m_alphaRedValue   += 10;
            if (m_alphaRedValue > 175)
            {
                m_alphaRedValue = 175;
                m_hitPhase = 2;
            }
            break;
        case 2:
            m_alphaWhiteValue += 10;
            m_alphaRedValue   -= 10;
            if (m_alphaRedValue < 0)
            {
                m_alphaRedValue = 0;
                m_hitPhase = 3;
            }
            break;
        case 3:
            m_alphaWhiteValue -= 10;
            if (m_alphaWhiteValue < 0)
            {
                m_alphaWhiteValue = 0;
                g_cGameBow_2State = 6;
                goto update_alpha;
            }
            break;
        default:
            break;
        }

    update_alpha:
        m_whiteFlash.SetAlpha(static_cast<unsigned char>(m_alphaWhiteValue));
        m_redFlash.SetAlpha(static_cast<unsigned char>(m_alphaRedValue));
        return;
    }

    // 方向鍵操作
    // 上
    if (m_pInputMgr->IsKeyPressed(200)
        || m_pInputMgr->IsJoyStickPush(1, 1)
        || m_pInputMgr->IsJoyStickDown(1, 1))
    {
        posY -= 7.0f;
        std::uint8_t dir;
        if (m_pInputMgr->IsKeyPressed(203)
            || m_pInputMgr->IsJoyStickPush(0, 1)
            || m_pInputMgr->IsJoyStickDown(0, 1))
            dir = 1;
        else if (m_pInputMgr->IsKeyPressed(205)
            || m_pInputMgr->IsJoyStickPush(0, 2)
            || m_pInputMgr->IsJoyStickDown(0, 2))
            dir = 7;
        else
            dir = 0;
        m_char.SetDirection(dir);
        m_pGameSoundMgr->PlaySoundA((char*)"M0020", 0, 0);
    }

    // 下
    if (m_pInputMgr->IsKeyPressed(208)
        || m_pInputMgr->IsJoyStickPush(1, 2)
        || m_pInputMgr->IsJoyStickDown(1, 2))
    {
        posY += 7.0f;
        std::uint8_t dir;
        if (m_pInputMgr->IsKeyPressed(203)
            || m_pInputMgr->IsJoyStickPush(0, 1)
            || m_pInputMgr->IsJoyStickDown(0, 1))
            dir = 3;
        else if (m_pInputMgr->IsKeyPressed(205)
            || m_pInputMgr->IsJoyStickPush(0, 2)
            || m_pInputMgr->IsJoyStickDown(0, 2))
            dir = 5;
        else
            dir = 4;
        m_char.SetDirection(dir);
        m_pGameSoundMgr->PlaySoundA((char*)"M0020", 0, 0);
    }

    // 左
    if (m_pInputMgr->IsKeyPressed(203)
        || m_pInputMgr->IsJoyStickPush(0, 1)
        || m_pInputMgr->IsJoyStickDown(0, 1))
    {
        posX -= 7.0f;
        std::uint8_t dir;
        if (m_pInputMgr->IsKeyPressed(200)
            || m_pInputMgr->IsJoyStickPush(1, 1)
            || m_pInputMgr->IsJoyStickDown(1, 1))
            dir = 1;
        else if (m_pInputMgr->IsKeyPressed(208)
            || m_pInputMgr->IsJoyStickPush(1, 2)
            || m_pInputMgr->IsJoyStickDown(1, 2))
            dir = 3;
        else
            dir = 2;
        m_char.SetDirection(dir);
        m_pGameSoundMgr->PlaySoundA((char*)"M0020", 0, 0);
    }

    // 右
    if (m_pInputMgr->IsKeyPressed(205)
        || m_pInputMgr->IsJoyStickPush(0, 2)
        || m_pInputMgr->IsJoyStickDown(0, 2))
    {
        posX += 7.0f;
        std::uint8_t dir;
        if (m_pInputMgr->IsKeyPressed(200)
            || m_pInputMgr->IsJoyStickPush(1, 1)
            || m_pInputMgr->IsJoyStickDown(1, 1))
            dir = 7;
        else if (m_pInputMgr->IsKeyPressed(208)
            || m_pInputMgr->IsJoyStickPush(1, 2)
            || m_pInputMgr->IsJoyStickDown(1, 2))
            dir = 5;
        else
            dir = 6;
        m_char.SetDirection(dir);
        m_pGameSoundMgr->PlaySoundA((char*)"M0020", 0, 0);
    }

    CreateSpear();
    m_char.SetPos(posX, posY);
    m_char.Poll();
    PollSpear();
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B61F0 — CheckCollision
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CheckCollision()
{
    if (m_isHit)
        return;

    LONG charX = static_cast<LONG>(m_char.m_curPosX) - 8;
    LONG charY = static_cast<LONG>(m_char.m_curPosY) - 7;

    for (int i = 0; i < kSpearCount; ++i)
    {
        if (!m_spears[i].m_active)
            continue;

        cltBow2_Spear& spear = m_spears[i].m_spear;
        RECT rc;
        rc.left   = static_cast<LONG>(spear.m_posX) - 5;
        rc.right  = rc.left + 10;
        rc.top    = static_cast<LONG>(spear.m_posY) - 5;
        rc.bottom = rc.top + 10;

        POINT pt1 = { charX, charY };
        POINT pt2 = { charX, charY + 14 };
        POINT pt3 = { charX + 16, charY };
        POINT pt4 = { charX + 16, charY + 14 };

        if (PtInRect(&rc, pt1) || PtInRect(&rc, pt2)
            || PtInRect(&rc, pt3) || PtInRect(&rc, pt4))
        {
            m_isHit = 1;
            m_char.SetDirection(8);
            m_pGameSoundMgr->PlaySoundA((char*)"M0021", 0, 0);

            if (m_timerLiveTime)
            {
                g_clTimerManager.ReleaseTimer(m_timerLiveTime);
                m_timerLiveTime = 0;
            }
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6330 — ReleaseSpearTimer
// ---------------------------------------------------------------------------
void cltMini_Bow_2::ReleaseSpearTimer()
{
    if (m_timerNormalSpear)
        g_clTimerManager.ReleaseTimer(m_timerNormalSpear);
    if (m_timerSnipeSpear)
        g_clTimerManager.ReleaseTimer(m_timerSnipeSpear);
    if (m_timerOctetSpear)
        g_clTimerManager.ReleaseTimer(m_timerOctetSpear);
    if (m_timer2VolleySpear)
        g_clTimerManager.ReleaseTimer(m_timer2VolleySpear);
    if (m_timerRainSpear)
        g_clTimerManager.ReleaseTimer(m_timerRainSpear);
    if (m_timerHorizonSpear)
        g_clTimerManager.ReleaseTimer(m_timerHorizonSpear);

    m_timerNormalSpear  = 0;
    m_timerSnipeSpear   = 0;
    m_timerOctetSpear   = 0;
    m_timer2VolleySpear = 0;
    m_timerRainSpear    = 0;
    m_timerHorizonSpear = 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B63E0 — AddSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::AddSpear(std::uint8_t posIndex, std::uint8_t direction,
                              float moveType, float speed)
{
    for (int i = 0; i < kSpearCount; ++i)
    {
        if (!m_spears[i].m_active)
        {
            m_spears[i].m_active = 1;
            m_spears[i].m_spear.Create(posIndex, direction, moveType, speed);
            m_pGameSoundMgr->PlaySoundA((char*)"M0019", 0, 0);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6450 — CreateSpear（根據 liveTime 決定飛矛模式）
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateSpear()
{
    if (!m_liveTimeChanged)
        return;

    unsigned int lt = GetLiveTime();

    if (lt > 0x2710 && lt < 0x2779)
    {
        // 10000..10105 → Snipe 模式
        CreateSnipeSpear();
        m_timerSnipeSpear = 0;
        m_timerSnipeSpear = g_clTimerManager.CreateTimer(
            0xFFFFFFFF,
            reinterpret_cast<std::uintptr_t>(this),
            0xBB8u, 1,
            nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(OnTimer_CreateSnipeSpear),
            nullptr);
        m_liveTimeChanged = 0;
    }
    else if (lt > 0x3A98 && lt < 0x3B01)
    {
        // 15000..15105 → Octet 模式
        CreateOctetSpear();
        m_timerOctetSpear = 0;
        m_timerOctetSpear = g_clTimerManager.CreateTimer(
            0xFFFFFFFF,
            reinterpret_cast<std::uintptr_t>(this),
            0x1B58u, 1,
            nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(OnTimer_CreateOctetSpear),
            nullptr);
        m_liveTimeChanged = 0;
    }
    else if (lt > 0x7530 && lt < 0x7599)
    {
        // 30000..30105 → 2Volley 模式
        Create2VolleySpear();
        m_timer2VolleySpear = 0;
        m_timer2VolleySpear = g_clTimerManager.CreateTimer(
            0xFFFFFFFF,
            reinterpret_cast<std::uintptr_t>(this),
            0x12Cu, 1,
            nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(OnTimer_Create2VolleySpear),
            nullptr);
        m_liveTimeChanged = 0;
    }
    else if (lt > 0x9C40 && lt < 0x9CA9)
    {
        // 40000..40105 → Rain 模式
        CreateRainSpear();
        m_timerRainSpear = 0;
        m_timerRainSpear = g_clTimerManager.CreateTimer(
            0xFFFFFFFF,
            reinterpret_cast<std::uintptr_t>(this),
            0xC8u, 1,
            nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(OnTimer_CreateRainSpear),
            nullptr);
        m_liveTimeChanged = 0;
    }
    else if (lt > 0xC350 && lt < 0xC3B9)
    {
        // 50000..50105 → Horizon 模式
        CreateHorizonSpear();
        m_timerHorizonSpear = 0;
        m_timerHorizonSpear = g_clTimerManager.CreateTimer(
            0xFFFFFFFF,
            reinterpret_cast<std::uintptr_t>(this),
            0xC8u, 1,
            nullptr, nullptr, nullptr,
            reinterpret_cast<cltTimer::TimerCallback>(OnTimer_CreateHorizonSpear),
            nullptr);
    }

    m_liveTimeChanged = 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6610 — CreateNormalSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateNormalSpear()
{
    std::uint8_t dir = (timeGetTime() & 3) + 1;
    DWORD pos = timeGetTime();
    AddSpear(pos % 0x11, dir, 103.0f, 2.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6650 — CreateSnipeSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateSnipeSpear()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    std::uint8_t dir1 = rand() % 4 + 1;
    int pos1 = rand();
    AddSpear(pos1 % 17, dir1, 103.0f, 4.0f);

    std::uint8_t dir2 = (timeGetTime() & 3) + 1;
    DWORD pos2 = timeGetTime();
    AddSpear(pos2 % 0x11, dir2, 103.0f, 2.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B66D0 — CreateOctetSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateOctetSpear()
{
    AddSpear(0,    1, 100.0f, 2.0f);
    AddSpear(0,    3, 100.0f, 2.0f);
    AddSpear(0x10, 2, 101.0f, 2.0f);
    AddSpear(0x10, 4, 101.0f, 2.0f);
    AddSpear(8,    1, 102.0f, 2.0f);
    AddSpear(8,    2, 102.0f, 2.0f);
    AddSpear(8,    3, 102.0f, 2.0f);
    AddSpear(8,    4, 102.0f, 2.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6780 — Create2VolleySpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Create2VolleySpear()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    std::uint8_t pos = rand() % 17;
    std::uint8_t dir = rand() & 2;
    AddSpear(pos, dir, 103.0f, 3.0f);
    AddSpear(pos, dir + 2, 103.0f, 3.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6800 — CreateRainSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateRainSpear()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    std::uint8_t dir = rand() % 4 + 1;
    int pos = rand();
    AddSpear(pos % 17, dir, 100.0f, 2.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6850 — CreateHorizonSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::CreateHorizonSpear()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    std::uint8_t dir = rand() % 4 + 1;
    int pos = rand();
    AddSpear(pos % 17, dir, 101.0f, 2.0f);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B68A0 — PollSpear
// ---------------------------------------------------------------------------
void cltMini_Bow_2::PollSpear()
{
    for (int i = 0; i < kSpearCount; ++i)
    {
        m_spears[i].m_spear.Poll();
        if (!m_spears[i].m_spear.GetActive())
            m_spears[i].m_active = 0;
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B68D0 — EndStage
// ---------------------------------------------------------------------------
void cltMini_Bow_2::EndStage()
{
    // 清除所有飛矛
    for (int i = 0; i < kSpearCount; ++i)
        m_spears[i].m_active = 0;
    ReleaseSpearTimer();
    m_char.SetActive(0);

    if (m_firstTimeEnd)
    {
        if (!cltValidMiniGameScore::IsValidScore(0x15u, m_liveTime))
        {
            const char* txt = m_pDCTTextManager->GetText(58092);
            char buf[256];
            wsprintfA(buf, "client : %s : %i", txt, static_cast<int>(m_liveTime));
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        m_totalScore = 0;
        m_finalScore = m_currentRoundScore;

        if (m_liveTime < static_cast<unsigned int>(m_difficultyBaseScore))
        {
            // 失敗
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

        if (m_totalScore)
        {
            unsigned int extra = static_cast<unsigned int>(
                static_cast<double>((m_liveTime - m_difficultyBaseScore) / 1000u)
                * m_bonusMultiplier)
                + m_finalScore;
            m_finalScore = extra;
            if (static_cast<unsigned int>(m_scoreCap) < extra)
                m_finalScore = m_scoreCap;
        }

        unsigned int baseScore = m_finalScore;
        int mult = GetMultipleNum();
        m_displayScore = baseScore * mult;
        SendScore(m_stage, m_liveTime, m_liveTime, m_difficulty, baseScore);
        m_firstTimeEnd = 0;
    }

    if (m_serverAck)
    {
        if (m_serverValid)
        {
            int result = m_serverResult;
            m_showTime2   = 4;
            m_serverValid = 0;
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

            if (!g_clLessonSystem.CanTraningLesson(0x15u))
                m_showTime2 = 0;

            m_buttons[12].SetActive(1);
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
// mofclient.c @ 005B5AB0 — Init_Wait
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Init_Wait()
{
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
    m_char.SetActive(0);
    m_liveTime = 0;
    g_cGameBow_2State = 0;
    memset(m_spears, 0, sizeof(m_spears));
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6B70 — Ranking
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Ranking()
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
// mofclient.c @ 005B6C80 — ShowPointText
// ---------------------------------------------------------------------------
void cltMini_Bow_2::ShowPointText()
{
    int baseX = m_uiPos[10];
    int baseY = m_uiPos[11];

    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3341);
    g_MoFFont.SetTextLineA(baseX + 70, baseY + 50,
                          0xFFFFFFFF, title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");

    // 秒數格式：N + "秒"
    const char* secText = m_pDCTTextManager->GetText(3224);
    char buf[32];

    wsprintfA(buf, "%s%s", "7", secText);
    g_MoFFont.SetTextLineA(baseX + 143, baseY + 127,
                          0xFFFFFFFF, buf, 1, -1, -1);

    wsprintfA(buf, "%s%s", "15", secText);
    g_MoFFont.SetTextLineA(baseX + 143, baseY + 157,
                          0xFFFFFFFF, buf, 1, -1, -1);

    wsprintfA(buf, "%s%s", "25", secText);
    g_MoFFont.SetTextLineA(baseX + 143, baseY + 187,
                          0xFFFFFFFF, buf, 1, -1, -1);

    g_MoFFont.SetTextLineA(baseX + 265, baseY + 127,
                          0xFFFFFFFF, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 265, baseY + 157,
                          0xFFFFFFFF, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 265, baseY + 187,
                          0xFFFFFFFF, "200 ~ 360", 1, -1, -1);

    g_MoFFont.SetTextLineA(baseX + 405, baseY + 127,
                          0xFFFFFFFF, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 405, baseY + 157,
                          0xFFFFFFFF, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(baseX + 405, baseY + 187,
                          0xFFFFFFFF, "30", 1, -1, -1);
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6F40 — ExitGame
// ---------------------------------------------------------------------------
int cltMini_Bow_2::ExitGame()
{
    return 1;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B58C0 — Poll
// ---------------------------------------------------------------------------
int cltMini_Bow_2::Poll()
{
    m_drawAlphaBox = 0;
    ++m_pollFrame;

    if ((g_cGameBow_2State == 4 || g_cGameBow_2State == 5 || g_cGameBow_2State == 6)
        && static_cast<int>(m_pollFrame) < SETTING_FRAME)
    {
        return 0;
    }

    m_pollFrame = 0;

    switch (g_cGameBow_2State)
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
        EndStage();
        break;
    case 7:
        Ranking();
        m_drawAlphaBox = 1;
        break;
    case 100:
        return ExitGame();
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

    if (m_prevState != g_cGameBow_2State)
    {
        m_prevState = g_cGameBow_2State;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }
    MoveBtnFocus(g_cGameBow_2State);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B6F50 — PrepareDrawing
// ---------------------------------------------------------------------------
void cltMini_Bow_2::PrepareDrawing()
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

    // Slot 圖像
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (m_slots[i].active)
        {
            GameImage* pImg = m_pclImageMgr->GetGameImage(
                9u, m_slots[i].resID, 0, 1);
            m_slotImages[i] = pImg;
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

    // 按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // DrawNum: 倒數
    unsigned int readyT = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220, readyT, 255);

    // DrawNum: 存活時間
    m_drawNumLiveTime.PrepareDrawing(m_screenX + 720, m_screenY + 67,
                                     m_liveTime / 100u, 255);

    // 生命條圖示
    if (m_drawNumLiveTime.m_active)
    {
        GameImage* pImg = m_pclImageMgr->GetGameImage(9u, 0x20000079u, 0, 1);
        m_pLifeBarImage = pImg;
        if (pImg)
        {
            pImg->SetBlockID(0);
            pImg->m_bFlag_447 = true;
            pImg->m_bFlag_446 = true;
            pImg->m_bVertexAnimation = false;
            pImg->m_fPosX = static_cast<float>(m_screenX + 700);
            pImg->m_fPosY = static_cast<float>(m_screenY + 83);
        }
    }

    // DrawNum: 目標分數
    m_drawNumTarget.PrepareDrawing(m_screenX + 74, m_screenY + 107,
                                   m_difficultyBaseScore / 1000u, 255);

    // DrawNum: 結算分數
    m_drawNumFinal.PrepareDrawing(m_uiPos[4], m_uiPos[5],
                                  m_displayScore, 255);

    // 角色
    m_char.PrepareDrawing();

    // 飛矛
    for (int i = 0; i < kSpearCount; ++i)
    {
        if (m_spears[i].m_active)
            m_spears[i].m_spear.PrepareDrawing();
    }

    // AlphaBox（遊戲進行中顯示）
    if (g_cGameBow_2State == 5)
    {
        m_whiteFlash.PrepareDrawing();
        m_redFlash.PrepareDrawing();
    }

    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B71E0 — Draw
// ---------------------------------------------------------------------------
void cltMini_Bow_2::Draw()
{
    if (m_pBgImage)
        m_pBgImage->Draw();

    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    // Slot 圖像
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (m_slots[i].active && m_slotImages[i])
            m_slotImages[i]->Draw();
    }

    // 按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    // 排名
    if (g_cGameBow_2State == 7 && !m_rankDrawCounter)
        DrawRanking(m_uiPos[0], m_uiPos[1], 1);

    // 角色
    m_char.Draw();

    // ShowPointText
    if (g_cGameBow_2State == 3)
        ShowPointText();

    // 飛矛
    for (int i = 0; i < kSpearCount; ++i)
    {
        if (m_spears[i].m_active)
            m_spears[i].m_spear.Draw();
    }

    // DrawNum
    m_drawNumReady.Draw();
    m_drawNumLiveTime.Draw();
    m_drawNumTarget.Draw();
    m_drawNumFinal.Draw();

    // 生命條圖示
    if (m_drawNumLiveTime.m_active && m_pLifeBarImage)
        m_pLifeBarImage->Draw();

    // AlphaBox（遊戲進行中）
    if (g_cGameBow_2State == 5)
    {
        m_whiteFlash.Draw();
        m_redFlash.Draw();
    }
}

// ---------------------------------------------------------------------------
// mofclient.c @ 005B7330 — InitMiniGameImage
// ---------------------------------------------------------------------------
void cltMini_Bow_2::InitMiniGameImage()
{
    // --- Slot 索引設定 ---
    m_slotRanking   = 0;
    m_slotWin       = 1;
    m_slotLose      = 2;
    m_slotSelectDeg = 3;
    m_slotHelp      = 4;
    m_slotShowPoint = 5;

    // --- 清除所有 slot ---
    memset(m_slots, 0, sizeof(m_slots));
    memset(m_slotImages, 0, sizeof(m_slotImages));

    // --- 填入 slot 資料 ---
    // Slot 0: Ranking BG (0x2200000A)
    m_slots[0].resID   = 0x2200000Au;
    m_slots[0].blockID = 12;
    m_slots[0].x       = m_uiPos[0];
    m_slots[0].y       = m_uiPos[1];

    // Slot 1: Win popup (0x20000002)
    m_slots[1].resID   = 0x20000002u;
    m_slots[1].blockID = 0;
    m_slots[1].x       = m_uiPos[2];
    m_slots[1].y       = m_uiPos[3];

    // Slot 2: Lose popup (0x20000002)
    m_slots[2].resID   = 0x20000002u;
    m_slots[2].blockID = 1;
    m_slots[2].x       = m_uiPos[2];
    m_slots[2].y       = m_uiPos[3];

    // Slot 3: Select degree (0x1000009B)
    m_slots[3].resID   = 0x1000009Bu;
    m_slots[3].blockID = 20;
    m_slots[3].x       = m_uiPos[8];
    m_slots[3].y       = m_uiPos[9];

    // Slot 4: Help (0x2000004E)
    m_slots[4].resID   = 0x2000004Eu;
    m_slots[4].blockID = 0;
    m_slots[4].x       = m_uiPos[6];
    m_slots[4].y       = m_uiPos[7];

    // Slot 5: ShowPoint (0x100000A0)
    m_slots[5].resID   = 0x100000A0u;
    m_slots[5].blockID = 0;
    m_slots[5].x       = m_uiPos[10];
    m_slots[5].y       = m_uiPos[11];

    // --- 建立 13 個按鈕（對齊 mofclient.c） ---
    // Button 0: Start
    m_buttons[0].CreateBtn(
        m_screenX + 37, m_screenY + 472, 9u,
        0x2200000Au, 0, 0x2200000Au, 3u, 0x2200000Au, 6u,
        0x20000014u, 9u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Start),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 1: Ranking
    m_buttons[1].CreateBtn(
        m_screenX + 183, m_screenY + 472, 9u,
        0x2200000Au, 1u, 0x2200000Au, 4u, 0x2200000Au, 7u,
        0x20000014u, 0xAu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Ranking),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 2: Exit
    m_buttons[2].CreateBtn(
        m_screenX + 621, m_screenY + 472, 9u,
        0x2200000Au, 2u, 0x2200000Au, 5u, 0x2200000Au, 8u,
        0x20000014u, 0xBu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Exit),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 3: RankingPre
    m_buttons[3].CreateBtn(
        m_uiPos[0] + 17, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0xDu, 0x2200000Au, 0xFu, 0x2200000Au, 0x11u,
        0x2200000Au, 0x13u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_RankingPre),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 4: RankingNext
    m_buttons[4].CreateBtn(
        m_uiPos[0] + 62, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0xEu, 0x2200000Au, 0x10u, 0x2200000Au, 0x12u,
        0x2200000Au, 0x14u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_RankingNext),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 5: Ranking ExitPopUp
    m_buttons[5].CreateBtn(
        m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 6: Help
    m_buttons[6].CreateBtn(
        m_screenX + 329, m_screenY + 472, 9u,
        0x1000009Bu, 0xCu, 0x1000009Bu, 0xEu, 0x1000009Bu, 0x10u,
        0x1000009Bu, 0x12u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Help),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 7: ShowPoint
    m_buttons[7].CreateBtn(
        m_screenX + 475, m_screenY + 472, 9u,
        0x1000009Bu, 0xDu, 0x1000009Bu, 0xFu, 0x1000009Bu, 0x11u,
        0x1000009Bu, 0x13u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ShowPoint),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 8: Generic ExitPopUp (Help/ShowPoint 共用)
    m_buttons[8].CreateBtn(
        m_screenX + 566, m_screenY + 513, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 9: DegreeEasy
    m_buttons[9].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 48, 9u,
        0x1000009Bu, 0, 0x1000009Bu, 3u, 0x1000009Bu, 6u,
        0x1000009Bu, 9u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeEasy),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 10: DegreeNormal
    m_buttons[10].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 102, 9u,
        0x1000009Bu, 1u, 0x1000009Bu, 4u, 0x1000009Bu, 7u,
        0x1000009Bu, 0xAu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeNormal),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 11: DegreeHard
    m_buttons[11].CreateBtn(
        m_uiPos[8] + 36, m_uiPos[9] + 156, 9u,
        0x1000009Bu, 2u, 0x1000009Bu, 5u, 0x1000009Bu, 8u,
        0x1000009Bu, 0xBu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeHard),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 12: EndStage ExitPopUp
    m_buttons[12].CreateBtn(
        m_uiPos[2] + 215, m_uiPos[3] + 170, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // --- 初始化 DrawNum ---
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumReady.SetActive(0);
    m_drawNumLiveTime.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumLiveTime.SetActive(0);
    m_drawNumTarget.InitDrawNum(9u, 0x22000007u, 0x11u, 1u);
    m_drawNumTarget.SetActive(0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumFinal.SetActive(0);

    // --- 初始化 AlphaBox ---
    m_whiteFlash.Create(0, 0,
        static_cast<unsigned short>(g_Game_System_Info.ScreenWidth),
        static_cast<unsigned short>(g_Game_System_Info.ScreenHeight),
        1.0f, 1.0f, 1.0f, 0.0f, nullptr);

    m_redFlash.Create(0, 0,
        static_cast<unsigned short>(g_Game_System_Info.ScreenWidth),
        static_cast<unsigned short>(g_Game_System_Info.ScreenHeight),
        1.0f, 0.0f, 0.0f, 0.0f, nullptr);

    m_alphaBox.Create(m_screenX, m_screenY + 9, 800, 600,
                      0.0f, 0.0f, 0.0f, 0.5f, nullptr);

    m_drawAlphaBox = 0;
    m_bgResID = 0x20000110u;
    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_02.ogg");

    Init_Wait();
}
