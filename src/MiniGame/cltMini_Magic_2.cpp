#include "MiniGame/cltMini_Magic_2.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <windows.h>
#include <mmsystem.h>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/ImageResource.h"
#include "Image/ImageResourceListDataMgr.h"
#include "Logic/DirectInputManager.h"
#include "Sound/GameSound.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Util/cltTimer.h"
#include "MiniGame/cltValidMiniGameScore.h"
#include "Logic/cltSystemMessage.h"
#include "System/cltLessonSystem.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_MiniGame_Class_Sword.h"
#include "Effect/CEffect_MiniGame_MagicStick_Left.h"
#include "Effect/CEffect_MiniGame_MagicStick_Right.h"
#include "Effect/CEffect_MiniGame_Wizard_String.h"
#include "global.h"

extern unsigned char g_cGameMagic_2State;

// ---------------------------------------------------------------------------
// Constructor — mofclient.c 0x5BAAD0
// ---------------------------------------------------------------------------
cltMini_Magic_2::cltMini_Magic_2()
    : cltMoF_BaseMiniGame()
    , m_difficulty(0)
    , m_showTime2(0)
    , m_totalScore(0)
    , m_passScore(0)
    , m_winBaseScore(0)
    , m_roundFixedScore(0)
    , m_baseScore(0)
    , m_displayScore(0)
    , m_incrementFactor(0.0f)
    , m_maxScore(0)
    , m_gameScore(0)
    , m_drawAlphaBox(0)
    , m_focusEnabledMagic(0)
    , m_pBgImage(nullptr)
    , m_bgResID(536871187) // 0x20000013
    , m_lessonType(0)
    , m_timeLevelIndex(0)
    , m_timeCreateTable{}
    , m_lastCreateTick(0)
    , m_totalTargetCount(0)
    , m_currentTargetIndex(0)
    , m_targetData{}
    , m_screenDamageActive(0)
    , m_screenDamageCount(0)
    , m_uiSlotLeftBox(0)
    , m_uiSlotRightBox(0)
    , m_uiSlotRanking(0)
    , m_uiSlotPass(0)
    , m_uiSlotFail(0)
    , m_uiSlotDegreeSelect(0)
    , m_uiSlotHelp(0)
    , m_uiSlotShowPoint(0)
    , m_pollFrameCounter(0)
    , m_prevState(100)
    , m_slotImages{}
    , m_slots{}
{
    InitMiniGameImage();
    InitBtnFocus();
    m_showTime2 = 0;
    memset(m_targets, 0, sizeof(m_targets));
    m_pollFrameCounter = 0;
}

// ---------------------------------------------------------------------------
// Destructor — mofclient.c 0x5BAC20
// ---------------------------------------------------------------------------
cltMini_Magic_2::~cltMini_Magic_2() = default;

// ---------------------------------------------------------------------------
// OnBtn_Start — mofclient.c 0x5BAD20
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_Start(cltMini_Magic_2* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_SelectDegree — mofclient.c 0x5BAD40
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    // 啟用三個難度按鈕 (buttons 9, 10, 11 at offsets 1676, 1768, 1860)
    m_buttons[9].SetActive(1);
    m_buttons[10].SetActive(1);
    m_buttons[11].SetActive(1);

    m_slots[m_uiSlotDegreeSelect].active = 1;
    g_cGameMagic_2State = 1;
}

// ---------------------------------------------------------------------------
// OnBtn_Ranking — mofclient.c 0x5BADB0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_Ranking(cltMini_Magic_2* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_Ranking — mofclient.c 0x5BADD0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_Ranking()
{
    m_slots[m_uiSlotRanking].active = 1;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    // 啟用排名按鈕 (buttons 3, 4, 5 at offsets 1124, 1216, 1308)
    m_buttons[3].SetActive(1);
    m_buttons[4].SetActive(1);
    m_buttons[5].SetActive(1);

    m_slots[m_uiSlotPass].active = 0;
    m_slots[m_uiSlotFail].active = 0;
    m_myRankingText[0] = 0;
    m_curRankPage = 0;
    memset(m_ranking, 0, sizeof(m_ranking));
    RequestRanking(0x1F, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameMagic_2State = 7;
}

// ---------------------------------------------------------------------------
// OnBtn_Exit — mofclient.c 0x5BAEA0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_Exit()
{
    g_cGameMagic_2State = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

// ---------------------------------------------------------------------------
// OnBtn_RankingPre — mofclient.c 0x5BAEC0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_RankingPre(cltMini_Magic_2* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_PreRanking — mofclient.c 0x5BAEE0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_PreRanking()
{
    if (m_curRankPage > 0)
    {
        m_curRankPage--;
        RequestRanking(0x1F, m_curRankPage);
    }
}

// ---------------------------------------------------------------------------
// OnBtn_RankingNext — mofclient.c 0x5BAF00
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_RankingNext(cltMini_Magic_2* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_NextRanking — mofclient.c 0x5BAF20
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_NextRanking()
{
    if (m_curRankPage < 0x1D)
    {
        m_curRankPage++;
        RequestRanking(0x1F, m_curRankPage);
    }
}

// ---------------------------------------------------------------------------
// OnBtn_Help — mofclient.c 0x5BAF40
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_Help(cltMini_Magic_2* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_Help — mofclient.c 0x5BAF60
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    // ExitPopUp 按鈕 (button 8 at offset 1584)
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(
        m_uiPos[4] + 402,   // WORD[311] + 402
        m_uiPos[5] + 475);  // WORD[312] + 475

    m_slots[m_uiSlotHelp].active = 1;
    g_cGameMagic_2State = 2;
}

// ---------------------------------------------------------------------------
// OnBtn_ShowPoint — mofclient.c 0x5BAFE0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_ShowPoint(cltMini_Magic_2* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

// ---------------------------------------------------------------------------
// Init_ShowPoint — mofclient.c 0x5BB000
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(
        m_uiPos[8] + 380,   // WORD[315] + 380
        m_uiPos[9] + 216);  // WORD[316] + 216

    m_slots[m_uiSlotShowPoint].active = 1;
    g_cGameMagic_2State = 3;
}

// ---------------------------------------------------------------------------
// OnBtn_ExitPopUp — mofclient.c 0x5BB080
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_ExitPopUp(cltMini_Magic_2* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

// ---------------------------------------------------------------------------
// OnBtn_DegreeEasy — mofclient.c 0x5BB0A0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_DegreeEasy(cltMini_Magic_2* self)
{
    self->SetGameDegree(1);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ---------------------------------------------------------------------------
// OnBtn_DegreeNormal — mofclient.c 0x5BB0C0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_DegreeNormal(cltMini_Magic_2* self)
{
    self->SetGameDegree(2);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ---------------------------------------------------------------------------
// OnBtn_DegreeHard — mofclient.c 0x5BB0E0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnBtn_DegreeHard(cltMini_Magic_2* self)
{
    self->SetGameDegree(4);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ---------------------------------------------------------------------------
// SetGameDegree — mofclient.c 0x5BB100
// ---------------------------------------------------------------------------
void cltMini_Magic_2::SetGameDegree(std::uint8_t degree)
{
    uint8_t easyThreshold = 0;

    m_difficulty = degree;
    m_totalTargetCount = 0;
    m_currentTargetIndex = 0;
    m_roundFixedScore = 30;
    m_incrementFactor = 0.0f;

    switch (degree)
    {
        case 1: // Easy
            m_winBaseScore = 50;
            m_passScore = 20;
            m_totalTargetCount = 50;
            easyThreshold = 45;
            m_timeCreateTable[0] = 930;
            m_timeCreateTable[1] = 930;
            m_timeCreateTable[2] = 930;
            m_timeCreateTable[3] = 930;
            m_timeCreateTable[4] = 930;
            m_maxScore = 90;
            m_incrementFactor = 4.0f;
            m_bgResID = 536871187; // 0x20000013
            break;
        case 2: // Normal
            m_winBaseScore = 100;
            m_passScore = 55;
            m_totalTargetCount = 60;
            easyThreshold = 50;
            m_timeCreateTable[0] = 930;
            m_timeCreateTable[1] = 830;
            m_timeCreateTable[2] = 830;
            m_timeCreateTable[3] = 700;
            m_timeCreateTable[4] = 600;
            m_maxScore = 180;
            m_incrementFactor = 8.0f;
            m_bgResID = 536871189; // 0x20000015
            break;
        case 4: // Hard
            m_winBaseScore = 200;
            m_timeCreateTable[0] = 830;
            m_timeCreateTable[1] = 830;
            m_passScore = 85;
            m_totalTargetCount = 70;
            easyThreshold = 60;
            m_timeCreateTable[2] = 700;
            m_timeCreateTable[3] = 600;
            m_timeCreateTable[4] = 600;
            m_maxScore = 360;
            m_incrementFactor = 12.0f;
            m_bgResID = 536871188; // 0x20000014
            break;
    }

    m_slots[m_uiSlotPass].active = 0;
    m_slots[m_uiSlotFail].active = 0;
    m_slots[m_uiSlotLeftBox].active = 1;

    m_boxLeft.Initailize(
        static_cast<float>(m_screenX + 267),
        static_cast<float>(m_screenY + 224));
    m_boxRight.Initailize(
        static_cast<float>(m_screenX + 545),
        static_cast<float>(m_screenY + 224));

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_slots[m_uiSlotDegreeSelect].active = 0;

    InitMiniGameTime(0x32, 3);

    m_dword149 = 0;
    unsigned int readyTime = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000 * readyTime,
        reinterpret_cast<unsigned int>(this),
        1000,
        1, 0, 0,
        reinterpret_cast<void(__cdecl*)(unsigned int, unsigned int)>(OnTimer_TimeOutReadyTime),
        reinterpret_cast<void(__cdecl*)(unsigned int, unsigned int)>(OnTimer_DecreaseReadyTime),
        0);

    m_drawNumReady.SetActive(1);
    m_gameScore = 0;
    memset(m_targets, 0, sizeof(m_targets));
    m_gameActive = 1;
    m_totalScore = 1;
    g_cGameMagic_2State = 4;

    // 初始化 target 資料表 (kind/type/direction)
    int idx = 0;
    for (; idx < 40; ++idx)
    {
        m_targetData[idx].kind = 0;       // 小妖
        m_targetData[idx].type = idx % 4;
        m_targetData[idx].direction = idx & 1;
    }
    if (easyThreshold > 40)
    {
        for (; idx < easyThreshold; ++idx)
        {
            m_targetData[idx].kind = 1;   // 中妖
            m_targetData[idx].type = idx % 4;
            m_targetData[idx].direction = idx & 1;
        }
    }
    if (easyThreshold < m_totalTargetCount)
    {
        for (int k = easyThreshold; k < m_totalTargetCount; ++k)
        {
            m_targetData[k].kind = 2;     // 骷髏
            m_targetData[k].type = k % 3;
            m_targetData[k].direction = k & 1;
        }
    }

    // 洗牌
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < m_totalTargetCount; ++i)
    {
        uint8_t savedKind = m_targetData[i].kind;
        uint8_t savedType = m_targetData[i].type;
        DWORD t = timeGetTime();
        uint8_t swapIdx = static_cast<uint8_t>(rand() * t % m_totalTargetCount);
        m_targetData[i].kind = m_targetData[swapIdx].kind;
        m_targetData[i].type = m_targetData[swapIdx].type;
        m_targetData[swapIdx].kind = savedKind;
        m_targetData[swapIdx].type = savedType;
    }
}

// ---------------------------------------------------------------------------
// Poll — mofclient.c 0x5BB520
// ---------------------------------------------------------------------------
int cltMini_Magic_2::Poll()
{
    m_drawAlphaBox = 0;
    int frame = m_pollFrameCounter + 1;
    m_pollFrameCounter = frame;

    if ((g_cGameMagic_2State == 4 || g_cGameMagic_2State == 5 || g_cGameMagic_2State == 6)
        && frame < SETTING_FRAME)
    {
        return 0;
    }

    m_pollFrameCounter = 0;

    switch (g_cGameMagic_2State)
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

    if (m_prevState == g_cGameMagic_2State)
    {
        MoveBtnFocus(g_cGameMagic_2State);
    }
    else
    {
        m_prevState = g_cGameMagic_2State;
        InitBtnFocus();
    }

    m_pInputMgr->IsLMButtonUp();

    PollTarget();
    PollBox();

    return 0;
}

// ---------------------------------------------------------------------------
// Init_Wait — mofclient.c 0x5BB710
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    // 啟用 Wait 狀態按鈕
    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime2);
    m_buttons[1].SetActive(1);  // Ranking
    m_buttons[2].SetActive(1);  // Exit
    m_buttons[6].SetActive(1);  // Help
    m_buttons[7].SetActive(1);  // ShowPoint

    m_slots[m_uiSlotLeftBox].active = 0;
    m_slots[m_uiSlotRightBox].active = 0;
    m_slots[m_uiSlotPass].active = 0;
    m_slots[m_uiSlotFail].active = 0;
    m_slots[m_uiSlotDegreeSelect].active = 0;
    m_slots[m_uiSlotHelp].active = 0;
    m_slots[m_uiSlotShowPoint].active = 0;
    m_slots[m_uiSlotRanking].active = 0;

    m_drawNumFinal.SetActive(0);
    m_screenDamageActive = 0;
    InitMiniGameTime(0, 0);
    m_gameScore = 0;
    m_boxLeft.SetActive(0);
    m_boxRight.SetActive(0);
    m_elapsedTime = 0;
    g_cGameMagic_2State = 0;
}

// ---------------------------------------------------------------------------
// Ready — mofclient.c 0x5BB870
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Ready()
{
    if (m_pInputMgr->IsKeyDown(203)
        || m_pInputMgr->IsJoyStickPush(0, 1))
    {
        m_slots[m_uiSlotLeftBox].active = 1;
        m_slots[m_uiSlotRightBox].active = 0;

        auto* pEffect = new CEffect_MiniGame_Class_Sword();
        float ey = m_boxLeft.GetPosY() - 58.0f;
        float ex = m_boxLeft.GetPosX() - 68.0f;
        pEffect->SetEffect(ex, ey);
        g_EffectManager_MiniGame.BulletAdd(pEffect);
    }

    if (m_pInputMgr->IsKeyDown(205)
        || m_pInputMgr->IsJoyStickPush(0, 2))
    {
        m_slots[m_uiSlotLeftBox].active = 0;
        m_slots[m_uiSlotRightBox].active = 1;

        auto* pEffect = new CEffect_MiniGame_Class_Sword();
        float ey = m_boxRight.GetPosY() - 58.0f;
        float ex = m_boxRight.GetPosX() - 68.0f;
        pEffect->SetEffect(ex, ey);
        g_EffectManager_MiniGame.BulletAdd(pEffect);
    }
}

// ---------------------------------------------------------------------------
// StartGame — mofclient.c 0x5BBA20
// ---------------------------------------------------------------------------
void cltMini_Magic_2::StartGame()
{
    m_gameActive = 1;
    m_serverAck = 0;
    m_serverResult = 0;
    m_serverValid = 0;
    m_focusEnabledMagic = 0;

    m_drawNumReady.SetActive(0);
    m_drawNumRemain.SetActive(1);

    m_timeLevelIndex = 0;
    m_screenDamageCount = 0;
    m_dword148 = 0;

    unsigned int remainTime = GetRemainTime();
    m_dword148 = g_clTimerManager.CreateTimer(
        1000 * remainTime,
        reinterpret_cast<unsigned int>(this),
        1000,
        1, 0, 0,
        reinterpret_cast<void(__cdecl*)(unsigned int, unsigned int)>(OnTimer_StageClear),
        reinterpret_cast<void(__cdecl*)(unsigned int, unsigned int)>(OnTimer_DecreaseRemainTime),
        0);

    m_startTick = timeGetTime();
    g_cGameMagic_2State = 5;
}

// ---------------------------------------------------------------------------
// Gamming — mofclient.c 0x5BBAD0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Gamming()
{
    if (timeGetTime() - m_lastCreateTick > m_timeCreateTable[m_timeLevelIndex]
        && GetRemainTime() > 2
        && m_currentTargetIndex < m_totalTargetCount)
    {
        CreateNewTarget();
    }

    SetBoxYPos();
    DrawScreenEffect();

    if (m_pInputMgr->IsKeyDown(203)
        || m_pInputMgr->IsJoyStickPush(0, 1))
    {
        CheckCollision(0);
        m_slots[m_uiSlotLeftBox].active = 1;
        m_slots[m_uiSlotRightBox].active = 0;

        auto* pEffect = new CEffect_MiniGame_MagicStick_Left();
        float ey = static_cast<float>(m_screenY + 409);
        float ex = static_cast<float>(m_screenX + 269);
        pEffect->SetEffect(ex, ey);
        g_EffectManager_MiniGame.BulletAdd(pEffect);
    }

    if (m_pInputMgr->IsKeyDown(205)
        || m_pInputMgr->IsJoyStickPush(0, 2))
    {
        CheckCollision(1);
        m_slots[m_uiSlotLeftBox].active = 0;
        m_slots[m_uiSlotRightBox].active = 1;

        auto* pEffect = new CEffect_MiniGame_MagicStick_Right();
        float ey = static_cast<float>(m_screenY + 409);
        float ex = static_cast<float>(m_screenX + 497);
        pEffect->SetEffect(ex, ey);
        g_EffectManager_MiniGame.BulletAdd(pEffect);
    }
}

// ---------------------------------------------------------------------------
// SetBoxYPos — mofclient.c 0x5BBD00
// ---------------------------------------------------------------------------
void cltMini_Magic_2::SetBoxYPos()
{
    float leftX  = m_boxLeft.GetPosX();
    float rightX = m_boxRight.GetPosX();

    // Left box: find closest target
    float minDist = 2000.0f;
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (!m_targets[i].GetActive()) continue;
        float tx, ty;
        m_targets[i].GetPos(&tx, &ty);
        float dist = fabsf(leftX - tx);
        uint8_t dir = m_targets[i].GetDirection();
        if (dir)
        {
            if (tx - 22.0f > leftX) continue;
        }
        else
        {
            if (tx + 22.0f < leftX) continue;
        }
        if (minDist >= dist && dist < minDist)
        {
            minDist = dist;
            m_boxLeft.SetPosY(m_targets[i].GetPosY());
        }
    }

    // Right box: find closest target
    minDist = 2000.0f;
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (!m_targets[i].GetActive()) continue;
        float tx, ty;
        m_targets[i].GetPos(&tx, &ty);
        float dist = fabsf(rightX - tx);
        uint8_t dir = m_targets[i].GetDirection();
        if (dir)
        {
            if (tx - 22.0f > rightX) continue;
        }
        else
        {
            if (tx + 22.0f < rightX) continue;
        }
        if (minDist >= dist && dist < minDist)
        {
            minDist = dist;
            m_boxRight.SetPosY(m_targets[i].GetPosY());
        }
    }
}

// ---------------------------------------------------------------------------
// CheckCollision — mofclient.c 0x5BBEB0
// ---------------------------------------------------------------------------
int cltMini_Magic_2::CheckCollision(std::uint8_t side)
{
    static const char effectLUT[] = { 0, 2, 3, 1 };

    auto* pWizStr = new CEffect_MiniGame_Wizard_String();

    float boxX, boxY;
    if (side == 0)
    {
        boxX = m_boxLeft.GetPosX();
        boxY = m_boxLeft.GetPosY();
    }
    else
    {
        boxX = m_boxRight.GetPosX();
        boxY = m_boxRight.GetPosY();
    }

    // Sword effect
    auto* pSword = new CEffect_MiniGame_Class_Sword();
    pSword->SetEffect(boxX - 68.0f, boxY - 58.0f);
    g_EffectManager_MiniGame.BulletAdd(pSword);

    // Hit rect
    RECT rc;
    rc.left   = static_cast<LONG>(boxX - 30.0f);
    rc.top    = static_cast<LONG>(boxY - 30.0f);
    rc.right  = rc.left + 60;
    rc.bottom = rc.top + 60;

    float lastTX = 0, lastTY = 0;
    int hitIdx = -1;

    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (!m_targets[i].GetActive() || !m_targets[i].GetLive())
            continue;

        float tx, ty;
        m_targets[i].GetPos(&tx, &ty);
        float hx, hy;
        m_targets[i].GetHSize(&hx, &hy);

        LONG tlx = static_cast<LONG>(tx - hx);
        LONG tly = static_cast<LONG>(ty - hy);
        LONG brx = static_cast<LONG>(tx + hx);
        LONG bry = static_cast<LONG>(ty + hy);

        POINT p1 = { tlx, tly };
        POINT p2 = { tlx, bry };
        POINT p3 = { brx, tly };
        POINT p4 = { brx, bry };

        if (PtInRect(&rc, p1) || PtInRect(&rc, p2)
            || PtInRect(&rc, p3) || PtInRect(&rc, p4))
        {
            hitIdx = i;
            lastTX = tx;
            lastTY = ty;
            break;
        }

        lastTX = tx;
        lastTY = ty;
    }

    if (hitIdx < 0)
    {
        // Miss
        pWizStr->SetEffect(1, lastTX, lastTY);
        g_EffectManager_MiniGame.BulletAdd(pWizStr);
        if (m_gameScore > 10)
            m_gameScore -= 1;
        m_pGameSoundMgr->PlaySoundA((char*)"M0022", 0, 0);
        return 0;
    }

    // Hit
    m_targets[hitIdx].SetPos(boxX, boxY);
    m_targets[hitIdx].SetDie();

    uint8_t effectType = static_cast<uint8_t>(rand() % 3);
    int point = m_targets[hitIdx].GetPoint();

    const char* soundName;
    if (point >= 0)
    {
        m_gameScore += static_cast<uint16_t>(point);
        soundName = "M0023";
    }
    else
    {
        effectType = 3;
        m_screenDamageActive = 1;
        if (m_gameScore > 10)
            m_gameScore += static_cast<uint16_t>(point); // negative
        soundName = "M0024";
    }

    m_pGameSoundMgr->PlaySoundA((char*)soundName, 0, 0);
    pWizStr->SetEffect(effectLUT[effectType], lastTX, lastTY);
    g_EffectManager_MiniGame.BulletAdd(pWizStr);
    return 1;
}

// ---------------------------------------------------------------------------
// CreateNewTarget — mofclient.c 0x5BC220
// ---------------------------------------------------------------------------
void cltMini_Magic_2::CreateNewTarget()
{
    m_lastCreateTick = timeGetTime();

    int slot = -1;
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (!m_targets[i].GetActive())
        {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    int idx = m_currentTargetIndex;
    m_targets[slot].Initialize(
        m_targetData[idx].type,
        m_targetData[idx].kind,
        m_targetData[idx].direction,
        1);

    m_currentTargetIndex++;
}

// ---------------------------------------------------------------------------
// IncreaseTimeLevelIndex — mofclient.c 0x5BC2A0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::IncreaseTimeLevelIndex()
{
    if (m_timeLevelIndex < 4)
        m_timeLevelIndex++;
}

// ---------------------------------------------------------------------------
// EndStage — mofclient.c 0x5BC2C0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::EndStage()
{
    m_screenEffectBox.SetColor(
        1.0f, 0.0f, 0.0f, 0.0f);
    m_screenDamageCount = 0;

    for (int i = 0; i < kMaxTargets; ++i)
        m_targets[i].Release();

    if (m_gameActive)
    {
        m_elapsedTime = timeGetTime() - m_startTick;

        if (!cltValidMiniGameScore::IsValidScore(0x1F, m_gameScore))
        {
            char buf[256];
            const char* text = m_pDCTTextManager->GetText(58092);
            wsprintfA(buf, "client : %s : %i", text, static_cast<int>(m_gameScore));
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        m_totalScore = 0;
        m_baseScore = m_roundFixedScore;

        if (m_gameScore < static_cast<uint16_t>(m_passScore))
        {
            m_slots[m_uiSlotFail].active = 1;
        }
        else
        {
            m_totalScore = 1;
            m_baseScore = m_winBaseScore;
            m_slots[m_uiSlotPass].active = 1;
        }

        m_drawNumFinal.SetActive(1);

        if (m_totalScore)
        {
            unsigned int bonus = static_cast<unsigned int>(
                static_cast<float>(m_gameScore - m_passScore) * m_incrementFactor);
            unsigned int score = static_cast<unsigned int>(m_baseScore) + bonus;
            if (score > m_maxScore)
                score = m_maxScore;
            m_baseScore = static_cast<int>(score);
        }

        unsigned int finalScore = static_cast<unsigned int>(m_baseScore);
        int multiple = GetMultipleNum();
        m_displayScore = static_cast<int>(finalScore * multiple);

        SendScore(m_lessonType, m_gameScore, m_elapsedTime, m_difficulty, finalScore);
        m_gameActive = 0;
    }

    if (m_serverAck)
    {
        if (m_serverValid)
        {
            m_showTime2 = 4;
            m_serverValid = 0;
            m_focusEnabledMagic = 1;

            if (!m_serverResult)
            {
                const char* text = m_pDCTTextManager->GetText(58092);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, const_cast<char*>(text), 0, 0, 0);
                Init_Wait();
                return;
            }

            g_clLessonSystem.TraningLessonFinished(
                m_dword522,
                m_difficulty,
                m_totalScore,
                reinterpret_cast<unsigned int*>(&m_displayScore));

            if (!g_clLessonSystem.CanTraningLesson(0x1F))
                m_showTime2 = 0;

            m_buttons[12].SetActive(1);
            InitBtnFocus();
        }

        if ((m_pInputMgr->IsKeyDown(1) && m_focusEnabledMagic)
            || (m_pInputMgr->IsJoyButtonPush(1) && m_focusEnabledMagic))
        {
            Init_Wait();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
        else if (timeGetTime() - m_endTick > 5000)
        {
            Init_Wait();
        }
    }
    else
    {
        m_endTick = timeGetTime();
    }
}

// ---------------------------------------------------------------------------
// Ranking — mofclient.c 0x5BC5A0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Ranking()
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
// ShowPointText — mofclient.c 0x5BC6B0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::ShowPointText()
{
    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3342);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 70, m_uiPos[9] + 50, 0xFFFFFFFF, title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 143, m_uiPos[9] + 127, 0xFFFFFFFF, "20", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 143, m_uiPos[9] + 157, 0xFFFFFFFF, "55", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 143, m_uiPos[9] + 187, 0xFFFFFFFF, "85", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 265, m_uiPos[9] + 127, 0xFFFFFFFF, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 265, m_uiPos[9] + 157, 0xFFFFFFFF, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 265, m_uiPos[9] + 187, 0xFFFFFFFF, "200 ~ 360", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 405, m_uiPos[9] + 127, 0xFFFFFFFF, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 405, m_uiPos[9] + 157, 0xFFFFFFFF, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(
        m_uiPos[8] + 405, m_uiPos[9] + 187, 0xFFFFFFFF, "30", 1, -1, -1);
}

// ---------------------------------------------------------------------------
// ExitGame — mofclient.c 0x5BC8F0
// ---------------------------------------------------------------------------
int cltMini_Magic_2::ExitGame()
{
    return 1;
}

// ---------------------------------------------------------------------------
// Timer callbacks
// ---------------------------------------------------------------------------
void cltMini_Magic_2::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Magic_2::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Magic_2* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
    m_pGameSoundMgr->PlaySoundA((char*)"M0002", 0, 0);
}

void cltMini_Magic_2::OnTimer_DecreaseRemainTime(unsigned int /*id*/, cltMini_Magic_2* self)
{
    self->DecreaseRemainTime();
    if (!(self->GetRemainTime() % 10))
        self->IncreaseTimeLevelIndex();
}

void cltMini_Magic_2::OnTimer_StageClear(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseRemainTime();
    g_cGameMagic_2State = 6;
}

void cltMini_Magic_2::OnTimer_CreateNewTarget(unsigned int /*id*/, cltMini_Magic_2* self)
{
    self->CreateNewTarget();
}

// ---------------------------------------------------------------------------
// PrepareDrawing — mofclient.c 0x5BC9A0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::PrepareDrawing()
{
    // 背景
    GameImage* pBg = cltImageManager::GetInstance()->GetGameImage(9, m_bgResID, 0, 1);
    m_pBgImage = pBg;
    if (pBg)
    {
        pBg->SetBlockID(0);
        pBg->m_bFlag_447 = true;
        pBg->m_bFlag_446 = true;
        pBg->m_bVertexAnimation = false;
        pBg->m_fPosX = static_cast<float>(m_screenX);
        pBg->m_fPosY = static_cast<float>(m_screenY);
    }

    // Image slots
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (!m_slots[i].active) continue;
        GameImage* pImg = cltImageManager::GetInstance()->GetGameImage(
            9, m_slots[i].resID, 0, 1);
        m_slotImages[i] = pImg;
        pImg->SetBlockID(m_slots[i].blockID);
        pImg->m_bFlag_447 = true;
        pImg->m_bFlag_446 = true;
        pImg->m_bVertexAnimation = false;
        pImg->m_fPosX = static_cast<float>(m_slots[i].x);
        pImg->m_fPosY = static_cast<float>(m_slots[i].y);
    }

    // Buttons
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // DrawNums
    unsigned int readyTime = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220, readyTime, 255);

    unsigned int remainTime = GetRemainTime();
    m_drawNumRemain.PrepareDrawing(m_screenX + 55, m_screenY + 102, remainTime, 255);

    m_drawNumScore.PrepareDrawing(m_screenX + 742, m_screenY + 52, m_gameScore, 255);

    m_drawNumFinal.PrepareDrawing(m_uiPos[2], m_uiPos[3], m_displayScore, 255);

    PrepareDrawingTarget();
    PrepareDrawingBox();

    // Screen effect alpha box
    m_screenEffectBox.PrepareDrawing();

    // Overlay alpha box (for help/ranking dimming)
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();

    // Side boxes for wide screens
    if (g_Game_System_Info.ScreenWidth > 800)
    {
        m_sideBoxLeft.PrepareDrawing();
        m_sideBoxRight.PrepareDrawing();
    }
}

// ---------------------------------------------------------------------------
// Draw — mofclient.c 0x5BCBB0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::Draw()
{
    if (m_pBgImage)
        m_pBgImage->Draw();

    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    // Image slots
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (m_slots[i].active && m_slotImages[i])
            m_slotImages[i]->Draw();
    }

    // Buttons
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    // Ranking
    if (g_cGameMagic_2State == 7)
    {
        if (!m_rankDrawCounter)
            DrawRanking(m_uiPos[0], m_uiPos[1], 0);
    }

    if (g_cGameMagic_2State == 3)
        ShowPointText();

    m_drawNumReady.Draw();
    m_drawNumRemain.Draw();
    m_drawNumScore.Draw();
    m_drawNumFinal.Draw();

    DrawTarget();
    DrawBox();

    m_screenEffectBox.Draw();

    if (g_Game_System_Info.ScreenWidth > 800)
    {
        m_sideBoxLeft.Draw();
        m_sideBoxRight.Draw();
    }
}

// ---------------------------------------------------------------------------
// PollTarget — mofclient.c 0x5BCCE0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::PollTarget()
{
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (m_targets[i].GetActive())
            m_targets[i].Poll();
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawingTarget — mofclient.c 0x5BCD10
// ---------------------------------------------------------------------------
void cltMini_Magic_2::PrepareDrawingTarget()
{
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (m_targets[i].GetActive())
            m_targets[i].PrepareDrawing();
    }
}

// ---------------------------------------------------------------------------
// DrawTarget — mofclient.c 0x5BCD40
// ---------------------------------------------------------------------------
void cltMini_Magic_2::DrawTarget()
{
    for (int i = 0; i < kMaxTargets; ++i)
    {
        if (m_targets[i].GetActive())
            m_targets[i].Draw();
    }
}

// ---------------------------------------------------------------------------
// PollBox — mofclient.c 0x5BCD70
// ---------------------------------------------------------------------------
void cltMini_Magic_2::PollBox()
{
    if (m_boxLeft.GetActive())
        m_boxLeft.Poll();
    if (m_boxRight.GetActive())
        m_boxRight.Poll();
}

// ---------------------------------------------------------------------------
// PrepareDrawingBox — mofclient.c 0x5BCDB0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::PrepareDrawingBox()
{
    m_boxLeft.PrepareDrawing();
    m_boxRight.PrepareDrawing();
}

// ---------------------------------------------------------------------------
// DrawBox — mofclient.c 0x5BCDD0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::DrawBox()
{
    m_boxLeft.Draw();
    m_boxRight.Draw();
}

// ---------------------------------------------------------------------------
// DrawScreenEffect — mofclient.c 0x5BCDF0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::DrawScreenEffect()
{
    int r = rand() % 256;

    if (m_screenDamageActive)
    {
        unsigned int color = (static_cast<uint16_t>(r) << 24) | 0x77FF0000;
        float a = static_cast<float>((color >> 24) & 0xFF) * (1.0f / 255.0f);
        float red = static_cast<float>((color >> 16) & 0xFF) * (1.0f / 255.0f);
        float g = static_cast<float>((color >> 8) & 0xFF) * (1.0f / 255.0f);
        float b = static_cast<float>(color & 0xFF) * (1.0f / 255.0f);
        m_screenEffectBox.SetColor(red, g, b, a);

        m_screenDamageCount++;
        if (m_screenDamageCount > 15)
        {
            m_screenEffectBox.SetColor(1.0f, 0.0f, 0.0f, 0.0f);
            m_screenDamageCount = 0;
            m_screenDamageActive = 0;
        }
    }
}

// ---------------------------------------------------------------------------
// InitMiniGameImage — mofclient.c 0x5BCEE0
// ---------------------------------------------------------------------------
void cltMini_Magic_2::InitMiniGameImage()
{
    // Setup image slot data
    struct SlotInit {
        int active;
        uint32_t resID;
        uint16_t blockID;
        int x;
        int y;
    };

    int sx = m_screenX;
    int sy = m_screenY;

    SlotInit initData[kSlotCount] = {
        { 0, 536870960, 0,  sx + 400, sy + 600 },       // slot 0: left box bg
        { 0, 536870960, 1,  sx + 400, sy + 600 },       // slot 1: right box bg
        { 0, 570425353, 0,  sx + 177, sy + 300 },       // slot 2: pass overlay
        { 0, 570425353, 1,  sx + 177, sy + 300 },       // slot 3: fail overlay
        { 0, 570425354, 12, m_uiPos[0], m_uiPos[1] },  // slot 4: ranking
        { 0, 536870914, 0,  m_uiPos[2], m_uiPos[3] },  // slot 5: pass result
        { 0, 536870914, 1,  m_uiPos[2], m_uiPos[3] },  // slot 6: fail result
        { 0, 268435611, 20, m_uiPos[8], m_uiPos[9] },  // slot 7: degree select
        { 0, 536870993, 0,  m_uiPos[6], m_uiPos[7] },  // slot 8: help
        { 0, 268435616, 0,  m_uiPos[10], m_uiPos[11] }, // slot 9: show point
    };

    for (int i = 0; i < kSlotCount; ++i)
    {
        m_slots[i].active  = 0;
        m_slots[i].resID   = initData[i].resID;
        m_slots[i].blockID = initData[i].blockID;
        m_slots[i].x       = initData[i].x;
        m_slots[i].y       = initData[i].y;
    }

    // UI slot index assignments
    m_uiSlotLeftBox       = 0;
    m_uiSlotRightBox      = 1;
    m_uiSlotRanking       = 4;
    m_uiSlotPass          = 5;
    m_uiSlotFail          = 6;
    m_uiSlotDegreeSelect  = 7;
    m_uiSlotHelp          = 8;
    m_uiSlotShowPoint     = 9;

    // Create buttons
    m_buttons[0].CreateBtn(sx + 37, sy + 472, 9,
        0x2200000A, 0, 0x2200000A, 3, 0x2200000A, 6, 0x20000014, 9,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_Start),
        reinterpret_cast<unsigned int>(this), 1);

    m_buttons[1].CreateBtn(sx + 183, sy + 472, 9,
        0x2200000A, 1, 0x2200000A, 4, 0x2200000A, 7, 0x20000014, 10,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_Ranking),
        reinterpret_cast<unsigned int>(this), 1);

    m_buttons[2].CreateBtn(sx + 621, sy + 472, 9,
        0x2200000A, 2, 0x2200000A, 5, 0x2200000A, 8, 0x20000014, 11,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_Exit),
        reinterpret_cast<unsigned int>(this), 1);

    m_buttons[3].CreateBtn(m_uiPos[0] + 17, m_uiPos[1] + 295, 9,
        0x2200000A, 0x0D, 0x2200000A, 0x0F, 0x2200000A, 0x11, 0x2200000A, 0x13,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_RankingPre),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[4].CreateBtn(m_uiPos[0] + 62, m_uiPos[1] + 295, 9,
        0x2200000A, 0x0E, 0x2200000A, 0x10, 0x2200000A, 0x12, 0x2200000A, 0x14,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_RankingNext),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[5].CreateBtn(m_uiPos[0] + 220, m_uiPos[1] + 295, 9,
        0x2200000A, 0x15, 0x2200000A, 0x16, 0x2200000A, 0x17, 0x2200000A, 0x18,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[6].CreateBtn(sx + 329, sy + 472, 9,
        0x1000009B, 0x0C, 0x1000009B, 0x0E, 0x1000009B, 0x10, 0x1000009B, 0x12,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_Help),
        reinterpret_cast<unsigned int>(this), 1);

    m_buttons[7].CreateBtn(sx + 475, sy + 472, 9,
        0x1000009B, 0x0D, 0x1000009B, 0x0F, 0x1000009B, 0x11, 0x1000009B, 0x13,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_ShowPoint),
        reinterpret_cast<unsigned int>(this), 1);

    m_buttons[8].CreateBtn(sx + 566, sy + 513, 9,
        0x2200000A, 0x15, 0x2200000A, 0x16, 0x2200000A, 0x17, 0x2200000A, 0x18,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[9].CreateBtn(m_uiPos[8] + 36, m_uiPos[9] + 48, 9,
        0x1000009B, 0, 0x1000009B, 3, 0x1000009B, 6, 0x1000009B, 9,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_DegreeEasy),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[10].CreateBtn(m_uiPos[8] + 36, m_uiPos[9] + 102, 9,
        0x1000009B, 1, 0x1000009B, 4, 0x1000009B, 7, 0x1000009B, 10,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_DegreeNormal),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[11].CreateBtn(m_uiPos[8] + 36, m_uiPos[9] + 156, 9,
        0x1000009B, 2, 0x1000009B, 5, 0x1000009B, 8, 0x1000009B, 11,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_DegreeHard),
        reinterpret_cast<unsigned int>(this), 0);

    m_buttons[12].CreateBtn(m_uiPos[2] + 215, m_uiPos[3] + 170, 9,
        0x2200000A, 0x15, 0x2200000A, 0x16, 0x2200000A, 0x17, 0x2200000A, 0x18,
        reinterpret_cast<void(*)(unsigned int)>(OnBtn_ExitPopUp),
        reinterpret_cast<unsigned int>(this), 0);

    // Initialize DrawNums
    m_drawNumReady.InitDrawNum(9, 0x22000008, 0, 0);
    m_drawNumReady.SetActive(0);
    m_drawNumRemain.InitDrawNum(9, 0x22000016, 0, 1);
    m_drawNumRemain.SetActive(1);
    m_drawNumScore.InitDrawNum(9, 0x22000007, 0x11, 1);
    m_drawNumScore.SetActive(1);
    m_drawNumFinal.InitDrawNum(9, 0x22000016, 0, 0);
    m_drawNumFinal.SetActive(0);

    // Screen effect alpha box (red flash) — D3DXCOLOR(0x00FF0000) = invisible red
    m_screenEffectBox.Create(m_screenX, m_screenY, 800, 600,
        1.0f, 0.0f, 0.0f, 0.0f, nullptr);

    // Overlay alpha box for dimming — D3DXCOLOR(0x80000000) = semi-transparent black
    m_alphaBox.Create(m_screenX, m_screenY + 9, 800, 600,
        0.0f, 0.0f, 0.0f, 0.501960784f, nullptr);
    m_drawAlphaBox = 0;
    m_bgResID = 536871187; // 0x20000013

    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_03.ogg");

    // Side boxes for wide screens
    if (g_Game_System_Info.ScreenWidth > 800)
    {
        int sideWidth = (g_Game_System_Info.ScreenWidth - 800) / 2;
        m_sideBoxLeft.Create(0, 0, sideWidth, g_Game_System_Info.ScreenHeight,
            0.0f, 0.0f, 0.0f, 1.0f, nullptr);
        m_sideBoxRight.Create(sideWidth + 800, 0, sideWidth, g_Game_System_Info.ScreenHeight,
            0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    }

    Init_Wait();
}
