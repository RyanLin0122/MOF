// cltMini_Magic — 「魔法拼圖」小遊戲，對齊 mofclient.c
// 0x5B7F80..0x5BA390 還原。4×4 滑動拼圖，玩家用方向鍵移動游標、
// 空白鍵交換空格，在時間內完成指定圖案。

#include "MiniGame/cltMini_Magic.h"

#include <cstdlib>
#include <cstring>

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

extern unsigned char g_cGameMagicState;

namespace {

// -----------------------------------------------------------------------
// 目標圖案查詢表 — 41 組 × 4 個目標位置 × (row, col)。
// 對齊 mofclient.c 中 cCase_1 (0x6CDC9C) + byte_6CDC9D (0x6CDC9D)
// 合併為一個 328-byte 陣列。索引方式：
//   kGoalData[8*caseIndex + 2*goalIdx]     = row
//   kGoalData[8*caseIndex + 2*goalIdx + 1] = col
// -----------------------------------------------------------------------
static const unsigned char kGoalData[328] = {
    // case 0
    0,0, 0,1, 1,0, 1,1,
    // case 1
    0,1, 1,2, 0,2, 1,2,
    // case 2
    0,2, 1,3, 0,3, 1,0,
    // case 3
    1,0, 2,1, 1,1, 2,1,
    // case 4
    1,1, 2,2, 1,2, 2,2,
    // case 5
    1,2, 2,3, 1,3, 2,0,
    // case 6
    2,0, 3,1, 2,1, 3,1,
    // case 7
    2,1, 3,2, 2,2, 3,2,
    // case 8
    2,2, 3,3, 2,3, 3,0,
    // case 9
    0,1, 0,1, 1,2, 1,1,
    // case 10
    0,2, 0,2, 1,3, 1,0,
    // case 11
    1,1, 1,0, 2,0, 1,1,
    // case 12
    2,1, 2,0, 3,0, 0,1,
    // case 13
    1,1, 1,2, 2,2, 1,1,
    // case 14
    2,1, 2,2, 3,2, 0,2,
    // case 15
    1,2, 1,1, 2,1, 1,1,
    // case 16
    2,2, 2,1, 3,1, 0,2,
    // case 17
    1,2, 1,3, 2,3, 1,2,
    // case 18
    2,2, 2,3, 3,3, 0,3,
    // case 19
    1,3, 1,2, 2,2, 1,3,
    // case 20
    2,3, 2,2, 3,2, 0,0,
    // case 21
    1,0, 1,1, 1,2, 1,0,
    // case 22
    2,0, 2,1, 2,2, 2,0,
    // case 23
    3,0, 3,1, 3,2, 0,0,
    // case 24
    1,0, 2,0, 2,1, 1,0,
    // case 25
    2,0, 3,0, 3,1, 0,1,
    // case 26
    1,1, 1,2, 1,3, 1,1,
    // case 27
    2,1, 2,2, 2,3, 2,1,
    // case 28
    3,1, 3,2, 3,3, 0,1,
    // case 29
    1,1, 2,1, 2,2, 1,2,
    // case 30
    2,1, 3,1, 3,2, 0,2,
    // case 31
    1,2, 2,2, 2,3, 1,2,
    // case 32
    2,2, 3,2, 3,3, 0,0,
    // case 33
    0,0, 1,0, 2,0, 3,1,
    // case 34
    0,1,1,1,1,2,1,3,
    // case 35
    2,0, 2,1, 2,2, 2,3,
    // case 36
    3,0, 3,1, 3,2, 3,3,
    // case 37
    0,0, 1,0, 2,0, 3,0,
    // case 38
    0,1, 1,1, 2,1, 3,1,
    // case 39
    0,2, 1,2, 2,2, 3,2,
    // case 40
    0,3, 1,3, 2,3, 3,3,
};

} // namespace

// ===========================================================================
// Constructor / Destructor
// ===========================================================================
cltMini_Magic::cltMini_Magic()
    : m_showTime2(0), m_difficultyByte(0)
    , m_totalScore(0), m_gameTime(0), m_winBaseScore(0)
    , m_roundFixedScore(0), m_baseScore(0), m_displayScore(0)
    , m_gameActive(0), m_serverAck(0), m_serverResult(0), m_serverValid(0)
    , m_startTick(0), m_drawAlphaBox(0), m_timerId(0), m_endTick(0)
    , m_focusEnabledMagic(0), m_incrementFactor(0.0f), m_maxScore(0)
    , m_matrix{}, m_goalImages{}, m_drawNumBaseX(0), m_drawNumBaseY(0)
    , m_goalValue(100)
    , m_caseIndex(0), m_bgAlpha(0), m_stageCompleted(0), m_totalStages(3)
    , m_uiRanking(0), m_uiSelectDegree(0), m_uiHelp(0), m_uiShowPoint(0)
    , m_uiWin(0), m_uiLose(0), m_stageBarSlotBase(0)
    , m_pGoalValueImage(nullptr), m_pCursorImage(nullptr)
    , m_pBgPat1(nullptr), m_pBgPat2(nullptr), m_pTimeBarArrow(nullptr)
    , m_matrixImages{}, m_baseX2(0), m_barY(0)
    , m_gridBaseX(0), m_gridBaseY(0), m_goalImgX(0), m_goalImgY(0)
    , m_barEndX(0), m_timeBarWidth(0), m_bgFadeDir(0)
    , m_timeLapseStarted(0), m_gameElapsedTime(0), m_pollFrame(0)
    , m_showTimeBarArrow(0), m_bgResID(0x20000120u)
    , m_prevState(100), m_rows(4), m_cols(4)
    , m_cursorRow(0), m_cursorCol(0)
{
    g_cGameMagicState = 0;

    m_drawNumBaseX = static_cast<std::int16_t>(m_screenX + 208);
    m_drawNumBaseY = static_cast<std::int16_t>(m_screenY + 25);

    m_showTime2 = 0;
    cltMoF_BaseMiniGame::ResetTime();
    memset(m_slots, 0, sizeof(m_slots));
    InitMiniGameImage();
    cltMoF_BaseMiniGame::InitBtnFocus();
    Init_Wait();
    ShowMatrix();
    m_showTime = 1;
    m_pollFrame = 0;
}

cltMini_Magic::~cltMini_Magic()
{
}

// ===========================================================================
// Button callbacks
// ===========================================================================
void cltMini_Magic::OnBtn_Start(cltMini_Magic* a1)
{
    a1->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Magic::OnBtn_Ranking(cltMini_Magic* a1)
{
    a1->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Magic::OnBtn_Exit()
{
    g_cGameMagicState = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Magic::OnBtn_RankingPre(cltMini_Magic* a1)
{
    a1->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Magic::OnBtn_RankingNext(cltMini_Magic* a1)
{
    a1->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Magic::OnBtn_Help(cltMini_Magic* a1)
{
    a1->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Magic::OnBtn_ShowPoint(cltMini_Magic* a1)
{
    a1->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Magic::OnBtn_ExitPopUp(cltMini_Magic* a1)
{
    a1->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Magic::OnBtn_DegreeEasy(cltMini_Magic* a1)
{
    a1->SetGameDegree(1);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Magic::OnBtn_DegreeNormal(cltMini_Magic* a1)
{
    a1->SetGameDegree(2);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Magic::OnBtn_DegreeHard(cltMini_Magic* a1)
{
    a1->SetGameDegree(4);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// ===========================================================================
// Timer callbacks
// ===========================================================================
void cltMini_Magic::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* a2)
{
    a2->DecreaseReadyTime();
}

void cltMini_Magic::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Magic* a2)
{
    m_pInputMgr->ResetJoyStick();
    a2->StartGame();
    m_pGameSoundMgr->PlaySoundA((char*)"M0002", 0, 0);
}

// ===========================================================================
// State initializers
// ===========================================================================
void cltMini_Magic::Init_Ranking()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_slots[m_uiRanking].active = 1;
    m_slots[m_uiWin].active = 0;
    m_slots[m_uiLose].active = 0;
    for (int i = 0; i < 3; ++i)
        m_slots[i + m_stageBarSlotBase].active = 0;

    m_drawNumRemain.SetActive(0);
    m_drawNumScore.SetActive(0);
    m_buttons[3].SetActive(1);  // +1124 = button[3]
    m_buttons[4].SetActive(1);  // +1216 = button[4]
    m_buttons[5].SetActive(1);  // +1308 = button[5]
    m_myRankingText[0] = 0;
    m_curRankPage = 0;
    memset(&m_ranking, 0, sizeof(m_ranking));
    RequestRanking(0x1Eu, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameMagicState = 7;
}

void cltMini_Magic::Ranking()
{
    if (m_pInputMgr->IsKeyDown(1)
        || m_pInputMgr->IsJoyButtonPush(1))
    {
        Init_Wait();
        m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
    }
    m_buttons[3].SetBtnState(0);
    if (m_curRankPage) {
        if (m_pInputMgr->IsKeyDown(16)
            || m_pInputMgr->IsJoyButtonPush(0))
        {
            Init_PreRanking();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
    } else {
        m_buttons[3].SetBtnState(4);
    }
    m_buttons[4].SetBtnState(0);
    if (m_curRankPage <= 0x1Cu && m_rankCount == 10) {
        if (m_pInputMgr->IsKeyDown(17)
            || m_pInputMgr->IsJoyButtonPush(0))
        {
            Init_NextRanking();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
    } else {
        m_buttons[4].SetBtnState(4);
    }
}

void cltMini_Magic::Init_PreRanking()
{
    if (m_curRankPage) {
        --m_curRankPage;
        RequestRanking(0x1Eu, m_curRankPage);
    }
}

void cltMini_Magic::Init_NextRanking()
{
    if (m_curRankPage < 0x1Du) {
        ++m_curRankPage;
        RequestRanking(0x1Eu, m_curRankPage);
    }
}

void cltMini_Magic::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    InitMiniGameTime(0, 0);
    m_elapsedTime = 0;
    m_difficultyByte = 0;
    m_gameTime = 0;
    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime2);
    m_buttons[1].SetActive(1);
    m_buttons[2].SetActive(1);
    m_buttons[6].SetActive(1);
    m_buttons[7].SetActive(1);
    m_drawNumRemain.SetActive(0);
    m_drawNumScore.SetActive(0);
    m_slots[m_uiWin].active = 0;
    m_slots[m_uiLose].active = 0;
    m_slots[m_uiRanking].active = 0;
    m_slots[m_uiSelectDegree].active = 0;
    m_slots[m_uiHelp].active = 0;
    m_slots[m_uiShowPoint].active = 0;
    g_cGameMagicState = 0;
}

void cltMini_Magic::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[9].SetActive(1);   // +1676 = button[9]
    m_buttons[10].SetActive(1);  // +1768 = button[10]
    m_buttons[11].SetActive(1);  // +1860 = button[11]
    m_drawNumRemain.SetActive(0);
    m_slots[m_uiSelectDegree].active = 1;
    g_cGameMagicState = 1;
}

void cltMini_Magic::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[8].SetActive(1);   // +1584 = button[8]
    m_buttons[8].SetPosition(
        static_cast<unsigned short>(m_uiPos[6]) + 402,
        static_cast<unsigned short>(m_uiPos[7]) + 475);
    m_drawNumRemain.SetActive(0);
    m_slots[m_uiHelp].active = 1;
    g_cGameMagicState = 2;
}

void cltMini_Magic::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(
        static_cast<unsigned short>(m_uiPos[10]) + 380,
        static_cast<unsigned short>(m_uiPos[11]) + 216);
    m_drawNumRemain.SetActive(0);
    m_slots[m_uiShowPoint].active = 1;
    g_cGameMagicState = 3;
}

void cltMini_Magic::ShowPointText()
{
    CHAR buf[32];

    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const CHAR* title = m_pDCTTextManager->GetText(3342);
    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 70,
        static_cast<unsigned short>(m_uiPos[11]) + 50,
        0xFFFFFFFF, title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");

    const char* unit = m_pDCTTextManager->GetText(3224);
    wsprintfA(buf, "%s%s", "70", unit);
    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 143,
        static_cast<unsigned short>(m_uiPos[11]) + 127,
        0xFFFFFFFF, buf, 1, -1, -1);

    unit = m_pDCTTextManager->GetText(3224);
    wsprintfA(buf, "%s%s", "55", unit);
    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 143,
        static_cast<unsigned short>(m_uiPos[11]) + 157,
        0xFFFFFFFF, buf, 1, -1, -1);

    unit = m_pDCTTextManager->GetText(3224);
    wsprintfA(buf, "%s%s", "40", unit);
    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 143,
        static_cast<unsigned short>(m_uiPos[11]) + 187,
        0xFFFFFFFF, buf, 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 265,
        static_cast<unsigned short>(m_uiPos[11]) + 127,
        0xFFFFFFFF, " 50 ~  90", 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 265,
        static_cast<unsigned short>(m_uiPos[11]) + 157,
        0xFFFFFFFF, "100 ~ 180", 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 265,
        static_cast<unsigned short>(m_uiPos[11]) + 187,
        0xFFFFFFFF, "200 ~ 360", 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 405,
        static_cast<unsigned short>(m_uiPos[11]) + 127,
        0xFFFFFFFF, "30", 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 405,
        static_cast<unsigned short>(m_uiPos[11]) + 157,
        0xFFFFFFFF, "30", 1, -1, -1);

    g_MoFFont.SetTextLineA(
        static_cast<unsigned short>(m_uiPos[10]) + 405,
        static_cast<unsigned short>(m_uiPos[11]) + 187,
        0xFFFFFFFF, "30", 1, -1, -1);
}

// ===========================================================================
// SetGameDegree
// ===========================================================================
void cltMini_Magic::SetGameDegree(std::uint8_t a2)
{
    m_difficultyByte = a2;
    m_roundFixedScore = 30;
    m_incrementFactor = 0.0f;

    switch (a2) {
    case 1:
        m_totalStages = 1;
        m_winBaseScore = 50;
        m_gameTime = 70000;
        m_maxScore = 90;
        m_incrementFactor = 2.0f;
        m_bgResID = 0x20000120u;
        break;
    case 2:
        m_totalStages = 3;
        m_winBaseScore = 100;
        m_gameTime = 55000;
        m_maxScore = 180;
        m_incrementFactor = 4.0f;
        m_bgResID = 0x20000122u;
        break;
    case 4:
        m_totalStages = 3;
        m_winBaseScore = 200;
        m_gameTime = 40000;
        m_maxScore = 360;
        m_incrementFactor = 16.0f;
        m_bgResID = 0x20000121u;
        break;
    }

    m_slots[m_uiWin].active = 0;
    m_slots[m_uiLose].active = 0;
    m_slots[m_uiSelectDegree].active = 0;
    for (int i = 0; i < 3; ++i)
        m_slots[i + m_stageBarSlotBase].active = 0;

    m_drawNumRemain.SetActive(0);
    m_drawNumScore.SetActive(0);
    ResetBar();
    m_drawNumReady.SetActive(1);
    m_drawNumScore.SetActive(0);
    m_cursorRow = 0;
    m_cursorCol = 0;
    m_stageCompleted = 0;
    m_gameActive = 1;
    m_totalScore = 0;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    InitMiniGameTime(m_gameTime, 3u);
    m_timerId = 0;
    unsigned int readyT = GetReadyTime();
    m_timerId = g_clTimerManager.CreateTimer(
        1000 * readyT,
        reinterpret_cast<std::uintptr_t>(this),
        0x3E8u, 1, 0, 0,
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(OnTimer_DecreaseReadyTime),
        0);
    g_cGameMagicState = 4;
}

// ===========================================================================
// Puzzle logic
// ===========================================================================
void cltMini_Magic::CreateNewStage()
{
    do {
        // 初始化矩陣：(col + row * cols) % 4
        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                m_matrix[r][c] = static_cast<std::int16_t>((c + r * m_cols) % 4);
            }
        }

        if (!m_emptyRow && !m_emptyCol)
            ++m_cursorRow;

        // 隨機打亂
        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                int rr = std::rand() % m_rows;
                DWORD t = timeGetTime();
                Swap(static_cast<std::uint8_t>(r),
                     static_cast<std::uint8_t>(c),
                     static_cast<std::uint8_t>(rr),
                     static_cast<std::uint8_t>(t % m_cols));
            }
        }

        m_emptyRow = static_cast<std::uint8_t>(timeGetTime() % m_rows);
        std::uint8_t randCol = static_cast<std::uint8_t>(std::rand() % m_cols);
        m_emptyCol = randCol;
        std::int16_t oldVal = m_matrix[m_emptyRow][m_emptyCol];
        m_goalValue = oldVal;
        m_matrix[m_emptyRow][m_emptyCol] = -1;
        if (m_goalValue == oldVal) {
            std::int16_t newVal;
            do {
                newVal = static_cast<std::int16_t>(std::rand() % 4);
                m_goalValue = newVal;
            } while (newVal == oldVal);
        }
        m_caseIndex = static_cast<std::uint8_t>(timeGetTime() % 0x29);
    } while (CheckSuccess());
}

void cltMini_Magic::ShowMatrix()
{
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_matrix[r][c] = static_cast<std::int16_t>((c + r * m_cols) % 4);
        }
    }
    if (g_cGameMagicState == 4) {
        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                int rr = std::rand() % m_rows;
                DWORD t = timeGetTime();
                Swap(static_cast<std::uint8_t>(r),
                     static_cast<std::uint8_t>(c),
                     static_cast<std::uint8_t>(rr),
                     static_cast<std::uint8_t>(t % m_cols));
            }
        }
    }
}

void cltMini_Magic::Swap(std::uint8_t r1, std::uint8_t c1,
                          std::uint8_t r2, std::uint8_t c2)
{
    if (CheckIndex(r1) && CheckIndex(c1) && CheckIndex(r2) && CheckIndex(c2)) {
        std::int16_t tmp = m_matrix[r1][c1];
        m_matrix[r1][c1] = m_matrix[r2][c2];
        m_matrix[r2][c2] = tmp;
    }
}

BOOL cltMini_Magic::CheckIndex(int a2)
{
    return a2 >= 0 && a2 <= m_rows && a2 <= m_cols;
}

int cltMini_Magic::CheckSuccess()
{
    for (int i = 0; i < 4; ++i) {
        int idx = 8 * m_caseIndex + 2 * i;
        std::uint8_t row = kGoalData[idx];
        std::uint8_t col = kGoalData[idx + 1];
        if (m_matrix[row][col] != m_goalValue)
            return 0;
    }
    return 1;
}

void cltMini_Magic::MoveCursor(std::uint8_t dir)
{
    switch (dir) {
    case 0: // up
        if (m_cursorRow)
            --m_cursorRow;
        break;
    case 1: // left
        if (m_cursorCol)
            --m_cursorCol;
        break;
    case 2: // down
        if (m_cursorRow < m_rows - 1)
            ++m_cursorRow;
        break;
    case 3: // right
        if (m_cursorCol < m_cols - 1)
            ++m_cursorCol;
        break;
    }
}

void cltMini_Magic::ChangeRun()
{
    if (m_cursorRow != m_emptyRow) {
        if (m_cursorCol == m_emptyCol) {
            // 同列不同行 → 滑動整行
            if (m_cursorRow <= m_emptyRow) {
                for (int i = 0; i < m_emptyRow - m_cursorRow; ++i)
                    Swap(m_emptyRow - i, m_cursorCol, m_emptyRow - i - 1, m_cursorCol);
            } else {
                for (int i = 0; i < m_cursorRow - m_emptyRow; ++i)
                    Swap(i + m_emptyRow, m_cursorCol, i + m_emptyRow + 1, m_cursorCol);
            }
            m_emptyRow = m_cursorRow;
        } else if (m_cursorCol != m_emptyCol) {
            // 不同行不同列 → 不操作
            return;
        } else {
            // should not reach
            return;
        }
    } else if (m_cursorCol == m_emptyCol) {
        // 同一格 → 不操作
        return;
    } else {
        // 同行不同列 → 滑動整列
        m_pGameSoundMgr->PlaySoundA((char*)"M0011", 0, 0);
        if (m_cursorCol <= m_emptyCol) {
            for (int i = 0; i < m_emptyCol - m_cursorCol; ++i)
                Swap(m_emptyRow, m_emptyCol - i, m_emptyRow, m_emptyCol - i - 1);
        } else {
            for (int i = 0; i < m_cursorCol - m_emptyCol; ++i)
                Swap(m_emptyRow, i + m_emptyCol, m_emptyRow, i + m_emptyCol + 1);
        }
        m_emptyCol = m_cursorCol;
    }

    if (CheckSuccess()) {
        ++m_stageCompleted;
        SetStageBar();
        if (m_stageCompleted >= m_totalStages) {
            if (m_stageCompleted == m_totalStages) {
                m_gameElapsedTime = timeGetTime() - m_startTick;
                g_cGameMagicState = 6;
            }
        } else {
            CreateNewStage();
        }
        m_pGameSoundMgr->PlaySoundA((char*)"M0012", 0, 0);
    }
}

// ===========================================================================
// Poll
// ===========================================================================
int cltMini_Magic::Poll()
{
    m_drawAlphaBox = 0;
    ++m_pollFrame;

    if ((g_cGameMagicState == 4 || g_cGameMagicState == 5 || g_cGameMagicState == 6)
        && m_pollFrame < static_cast<std::uint32_t>(SETTING_FRAME))
    {
        return 0;
    }

    m_pollFrame = 0;

    switch (g_cGameMagicState) {
    case 1: case 2: case 3:
        if (m_pInputMgr->IsKeyDown(1)
            || m_pInputMgr->IsJoyButtonPush(1))
        {
            Init_Wait();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
        m_drawAlphaBox = 1;
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

    if (!m_focusLocked) {
        for (int i = 0; i < kButtonCount; ++i) {
            if (m_buttons[i].Poll()) {
                for (int j = 0; j < kButtonCount; ++j) {
                    if (m_buttonOrder[j] == static_cast<std::uint8_t>(i))
                        m_focusIdx = j;
                }
            }
        }
    }
    if (m_prevState != g_cGameMagicState) {
        m_prevState = g_cGameMagicState;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }
    MoveBtnFocus(g_cGameMagicState);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// ===========================================================================
// StartGame / Gamming / EndGame / ExitGame
// ===========================================================================
void cltMini_Magic::StartGame()
{
    m_startTick = timeGetTime();
    m_timeLapseStarted = 0;
    m_gameActive = 1;
    m_serverAck = 0;
    m_serverResult = 0;
    m_serverValid = 0;
    m_focusEnabledMagic = 0;
    m_stageCompleted = 0;
    m_drawNumReady.SetActive(0);
    CreateNewStage();
    m_drawNumRemain.SetActive(1);
    g_cGameMagicState = 5;
}

void cltMini_Magic::Gamming()
{
    if (m_pInputMgr->IsKeyDown(200)
        || m_pInputMgr->IsJoyStickPush(1, 1))
    {
        MoveCursor(0);
    }
    else if (m_pInputMgr->IsKeyDown(203)
             || m_pInputMgr->IsJoyStickPush(0, 1))
    {
        MoveCursor(1);
    }
    else if (m_pInputMgr->IsKeyDown(208)
             || m_pInputMgr->IsJoyStickPush(1, 2))
    {
        MoveCursor(2);
    }
    else if (m_pInputMgr->IsKeyDown(205)
             || m_pInputMgr->IsJoyStickPush(0, 2))
    {
        MoveCursor(3);
    }
    else if (m_pInputMgr->IsKeyDown(57)
             || m_pInputMgr->IsJoyButtonPush(0))
    {
        ChangeRun();
    }

    SetTimeBar();
    DWORD remain = static_cast<DWORD>(m_gameTime) + m_startTick - timeGetTime();
    m_remainTime = remain;
    if (remain >= static_cast<DWORD>(m_gameTime) && m_timeLapseStarted) {
        m_remainTime = 0;
        m_gameElapsedTime = timeGetTime() - m_startTick;
        g_cGameMagicState = 6;
    }
    m_timeLapseStarted = 1;
}

void cltMini_Magic::EndGame()
{
    m_drawNumRemain.SetActive(1);
    m_drawNumScore.SetActive(1);

    if (m_gameActive) {
        if (!m_validScore.IsValidScore(0x1Eu, m_gameElapsedTime)) {
            CHAR buf[256];
            const char* txt = m_pDCTTextManager->GetText(58092);
            wsprintfA(buf, "client : %s :%i", txt, m_gameElapsedTime);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage,buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        m_baseScore = m_roundFixedScore;
        if (m_stageCompleted == m_totalStages) {
            m_baseScore = m_winBaseScore;
            m_slots[m_uiWin].active = 1;
            m_totalScore = 1;
        } else {
            m_slots[m_uiLose].active = 1;
            InitMiniGameTime(0, 0);
        }

        if (m_totalScore) {
            unsigned int remainMs = static_cast<unsigned int>(m_gameTime) - m_gameElapsedTime;
            unsigned int remainSec = remainMs / 1000u;
            int bonus = static_cast<int>(
                static_cast<__int64>(static_cast<double>(remainSec) * static_cast<double>(m_incrementFactor)));
            m_baseScore += bonus;
            if (m_maxScore < static_cast<unsigned int>(m_baseScore))
                m_baseScore = static_cast<int>(m_maxScore);
        }

        unsigned int finalBase = static_cast<unsigned int>(m_baseScore);
        int multiple = GetMultipleNum();
        m_displayScore = static_cast<int>(finalBase * multiple);
        SendScore(0x1Eu, m_gameElapsedTime, m_gameElapsedTime,
                  m_difficultyByte, finalBase);

        if (m_timerId)
            g_clTimerManager.ReleaseTimer(m_timerId);
        m_gameActive = 0;
    }

    if (m_serverAck) {
        if (m_serverValid) {
            m_showTime2 = 4;
            m_serverValid = 0;
            m_focusEnabledMagic = 1;
            if (!m_serverResult) {
                const char* txt = m_pDCTTextManager->GetText(58092);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage,const_cast<char*>(txt), 0, 0, 0);
                Init_Wait();
                return;
            }
            g_clLessonSystem.TraningLessonFinished(
                m_dword522, m_difficultyByte,
                m_totalScore,
                reinterpret_cast<unsigned int*>(&m_displayScore));
            if (!g_clLessonSystem.CanTraningLesson(0x1Eu))
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
        else if (timeGetTime() - m_endTick > 0x1388) {
            Init_Wait();
        }
    } else {
        m_endTick = timeGetTime();
    }
}

int cltMini_Magic::ExitGame()
{
    m_pGameSoundMgr->StopMusic();
    if (m_timerId)
        g_clTimerManager.ReleaseTimer(m_timerId);
    return 1;
}

// ===========================================================================
// PrepareDrawing / Draw
// ===========================================================================
void cltMini_Magic::PrepareDrawing()
{
    // 背景
    GameImage* bg = m_pclImageMgr->GetGameImage(9u, m_bgResID, 0, 1);
    m_pBgImage = bg;
    if (bg) {
        *((float*)bg + 83) = static_cast<float>(m_screenX);
        *((float*)bg + 84) = static_cast<float>(m_screenY);
        *((__int16*)bg + 186) = 0;
        *((std::uint8_t*)bg + 447) = 1;
        *((std::uint8_t*)bg + 446) = 1;
        *((std::uint8_t*)bg + 444) = 0;
    }

    // Slot images
    for (int i = 0; i < kSlotCount; ++i) {
        if (m_slots[i].active) {
            GameImage* img = m_pclImageMgr->GetGameImage(9u, m_slots[i].resID, 0, 1);
            m_slotImages[i] = img;
            *((float*)img + 83) = static_cast<float>(m_slots[i].x);
            *((float*)img + 84) = static_cast<float>(m_slots[i].y);
            *((__int16*)img + 186) = m_slots[i].blockID;
            *((std::uint8_t*)img + 447) = 1;
            *((std::uint8_t*)img + 446) = 1;
            *((std::uint8_t*)img + 444) = 0;
        }
    }

    // Buttons
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // Gaming state: goal, matrix, cursor, etc.
    if (g_cGameMagicState == 5) {
        PrepareDrawingGoal();
        PrepareDrawingMatrix();

        GameImage* gvImg = m_pclImageMgr->GetGameImage(9u, 0x100000A1u, 0, 1);
        m_pGoalValueImage = gvImg;
        *((float*)gvImg + 83) = static_cast<float>(m_goalImgX);
        *((__int16*)gvImg + 186) = static_cast<std::int16_t>(m_goalValue + 2);
        *((std::uint8_t*)gvImg + 447) = 1;
        *((float*)gvImg + 84) = static_cast<float>(m_goalImgY);
        *((std::uint8_t*)gvImg + 446) = 1;
        *((std::uint8_t*)gvImg + 444) = 0;

        *((std::uint8_t*)((char*)m_pGoalValueImage + 450)) = 1;
        *(int*)((char*)m_pGoalValueImage + 380) = 255 - static_cast<int>(m_bgAlpha);
        *((std::uint8_t*)((char*)m_pGoalValueImage + 444)) = 0;

        PrepareDrawingCursor();
        m_timeBarBox.PrepareDrawing();
    }

    // Alpha box overlay
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();

    // Remain time DrawNum
    m_drawNumRemain.PrepareDrawing(
        static_cast<int>(static_cast<unsigned short>(m_drawNumBaseX)) + 545,
        static_cast<int>(static_cast<unsigned short>(m_drawNumBaseY)) + 43,
        (static_cast<unsigned int>(m_gameTime) - m_remainTime) / 10u,
        255);

    // Time bar arrow
    if (m_showTimeBarArrow) {
        GameImage* arrow = m_pclImageMgr->GetGameImage(9u, 0x20000079u, 0, 1);
        m_pTimeBarArrow = arrow;
        *((__int16*)arrow + 186) = 0;
        *((std::uint8_t*)arrow + 447) = 1;
        *((std::uint8_t*)arrow + 446) = 1;
        *((std::uint8_t*)arrow + 444) = 0;
        *((float*)arrow + 83) = static_cast<float>(static_cast<unsigned short>(m_drawNumBaseX) + 509);
        *((float*)arrow + 84) = static_cast<float>(static_cast<unsigned short>(m_drawNumBaseY) + 58);
    }

    // Ready time DrawNum
    unsigned int readyT = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220, readyT, 255);

    // Score DrawNum
    m_drawNumScore.PrepareDrawing(
        static_cast<int>(static_cast<unsigned short>(m_uiPos[4])),
        static_cast<int>(static_cast<unsigned short>(m_uiPos[5])),
        static_cast<unsigned int>(m_displayScore), 255);
}

void cltMini_Magic::Draw()
{
    // Background
    if (m_pBgImage)
        m_pBgImage->Draw();

    // Slot images (up to m_uiHelp count)
    for (int i = 0; i < m_uiHelp; ++i) {
        if (m_slots[i].active && m_slotImages[i])
            m_slotImages[i]->Draw();
    }

    // Gaming state
    if (g_cGameMagicState == 5) {
        DrawGoal();
        if (m_pGoalValueImage)
            m_pGoalValueImage->Draw();
        DrawMatrix();
        DrawCursor();
        m_timeBarBox.Draw();
    }

    // Remain time
    m_drawNumRemain.Draw();

    // Time bar arrow
    if (m_showTimeBarArrow && m_pTimeBarArrow)
        m_pTimeBarArrow->Draw();

    // Alpha box overlay
    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    // Ranking panel
    if (m_slots[m_uiRanking].active && m_slotImages[m_uiRanking])
        m_slotImages[m_uiRanking]->Draw();

    // Help panel
    if (m_slots[m_uiHelp].active && m_slotImages[m_uiHelp])
        m_slotImages[m_uiHelp]->Draw();

    // ShowPoint panel
    if (m_slots[m_uiShowPoint].active && m_slotImages[m_uiShowPoint])
        m_slotImages[m_uiShowPoint]->Draw();

    // Buttons
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    // Ready time + score
    m_drawNumReady.Draw();
    m_drawNumScore.Draw();

    // Ranking draw
    if (g_cGameMagicState == 7) {
        if (m_rankDrawCounter)
            return;
        DrawRanking(m_uiPos[0], m_uiPos[1], 1u);
    }

    if (g_cGameMagicState == 3)
        ShowPointText();
}

// ===========================================================================
// Drawing sub-functions
// ===========================================================================
void cltMini_Magic::PrepareDrawingMatrix()
{
    int yOffset = 0;
    for (int r = 0; r < m_rows; ++r) {
        int xOffset = 0;
        for (int c = 0; c < m_cols; ++c) {
            if (m_matrix[r][c] <= 100) {
                GameImage* img = m_pclImageMgr->GetGameImage(9u, 0x100000A1u, 0, 1);
                m_matrixImages[r][c] = img;
                *((float*)img + 83) = static_cast<float>(xOffset + m_gridBaseX);
                *((float*)img + 84) = static_cast<float>(yOffset + m_gridBaseY);
                *((__int16*)img + 186) = static_cast<std::int16_t>(m_matrix[r][c] + 2);
                *((std::uint8_t*)img + 447) = 1;
                *((std::uint8_t*)img + 446) = 1;
                *((std::uint8_t*)img + 444) = 0;
            }
            xOffset += 60;
        }
        yOffset += 60;
    }
}

void cltMini_Magic::DrawMatrix()
{
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_matrix[r][c] <= 100) {
                GameImage* img = m_matrixImages[r][c];
                if (img)
                    img->Draw();
            }
        }
    }
}

void cltMini_Magic::PrepareDrawingCursor()
{
    GameImage* img = m_pclImageMgr->GetGameImage(9u, 0x100000A1u, 0, 1);
    m_pCursorImage = img;
    float y = static_cast<float>(m_gridBaseY + 60 * m_cursorRow);
    float x = static_cast<float>(m_gridBaseX + 60 * m_cursorCol);
    *((__int16*)img + 186) = 1;
    *((std::uint8_t*)img + 447) = 1;
    *((std::uint8_t*)img + 446) = 1;
    *((std::uint8_t*)img + 444) = 0;
    *((float*)img + 83) = x;
    *((float*)img + 84) = y;
}

void cltMini_Magic::DrawCursor()
{
    if (m_pCursorImage)
        m_pCursorImage->Draw();
}

void cltMini_Magic::PrepareDrawingGoal()
{
    for (int i = 0; i < 4; ++i) {
        int idx = 2 * (i + 4 * m_caseIndex);
        std::uint8_t row = kGoalData[idx];
        std::uint8_t col = kGoalData[idx + 1];

        GameImage* img = m_pclImageMgr->GetGameImage(9u, 0x100000A1u, 0, 1);
        m_goalImages[i] = img;
        float y = static_cast<float>(m_gridBaseY + 60 * row + 1);
        float x = static_cast<float>(m_gridBaseX + 60 * col);
        *((__int16*)img + 186) = 9;
        *((std::uint8_t*)img + 444) = 0;
        *((std::uint8_t*)img + 447) = 1;
        *((std::uint8_t*)img + 446) = 1;
        *((float*)img + 83) = x;
        *((float*)img + 84) = y;
    }
}

void cltMini_Magic::DrawGoal()
{
    for (int i = 0; i < 4; ++i) {
        if (m_goalImages[i])
            m_goalImages[i]->Draw();
    }
}

void cltMini_Magic::PrepareDrawingBgPat()
{
    if (m_bgAlpha > 0x32u || !m_bgAlpha)
        m_bgFadeDir = (m_bgFadeDir == 0) ? 1 : 0;

    if (m_bgFadeDir)
        m_bgAlpha = static_cast<std::uint8_t>(m_bgAlpha + 1);
    else
        m_bgAlpha = static_cast<std::uint8_t>(m_bgAlpha - 1);

    int cx = m_screenX + 400;
    int cy = m_screenY + 300;

    GameImage* img1 = m_pclImageMgr->GetGameImage(9u, 0x22000014u, 0, 1);
    m_pBgPat1 = img1;
    *((__int16*)img1 + 186) = 15;
    *((std::uint8_t*)img1 + 447) = 1;
    *((std::uint8_t*)img1 + 446) = 1;
    *((std::uint8_t*)img1 + 444) = 0;
    *((float*)img1 + 83) = static_cast<float>(cx);
    *((float*)img1 + 84) = static_cast<float>(cy);
    *(int*)((char*)m_pBgPat1 + 376) = 300;
    *((std::uint8_t*)((char*)m_pBgPat1 + 449)) = 1;
    *((std::uint8_t*)((char*)m_pBgPat1 + 444)) = 0;
    *(int*)((char*)m_pBgPat1 + 380) = static_cast<int>(m_bgAlpha) + 27;
    *((std::uint8_t*)((char*)m_pBgPat1 + 450)) = 1;
    *((std::uint8_t*)((char*)m_pBgPat1 + 444)) = 0;

    GameImage* img2 = m_pclImageMgr->GetGameImage(9u, 0x22000014u, 0, 1);
    m_pBgPat2 = img2;
    *((__int16*)img2 + 186) = 15;
    *((std::uint8_t*)img2 + 447) = 1;
    *((std::uint8_t*)img2 + 446) = 1;
    *((std::uint8_t*)img2 + 444) = 0;
    *((float*)img2 + 83) = static_cast<float>(cx);
    *((float*)img2 + 84) = static_cast<float>(cy);
    *(int*)((char*)m_pBgPat2 + 380) = 177 - static_cast<int>(m_bgAlpha);
    *((std::uint8_t*)((char*)m_pBgPat2 + 450)) = 1;
    *((std::uint8_t*)((char*)m_pBgPat2 + 444)) = 0;
}

void cltMini_Magic::DrawBgPat()
{
    if (m_pBgPat1)
        m_pBgPat1->Draw();
    if (m_pBgPat2)
        m_pBgPat2->Draw();
}

// ===========================================================================
// Bar management
// ===========================================================================
void cltMini_Magic::SetTimeBar()
{
    unsigned int remain = static_cast<unsigned int>(m_gameTime) - m_remainTime;
    double ratio = static_cast<double>(remain) / static_cast<double>(m_gameTime);
    m_timeBarWidth = static_cast<std::int16_t>(static_cast<__int64>(ratio * 112.0));
    m_timeBarBox.SetPos(
        m_barEndX - static_cast<int>(static_cast<unsigned short>(m_timeBarWidth)),
        m_barY + 142);
}

void cltMini_Magic::SetStageBar()
{
    for (int i = 0; i < m_stageCompleted; ++i) {
        m_slots[i + m_stageBarSlotBase].active = 1;
    }
}

void cltMini_Magic::ResetBar()
{
    m_timeBarWidth = 1;
}

// ===========================================================================
// InitMiniGameImage — 建立所有按鈕、影像 slot、DrawNum
// ===========================================================================
void cltMini_Magic::InitMiniGameImage()
{
    int sx = m_screenX;
    int sy = m_screenY;
    int bx = sx + 208;

    m_baseX2 = bx;
    m_gridBaseX = bx + 70;
    m_gridBaseY = sy + 25 + 188;
    m_goalImgX = bx + 59;
    m_goalImgY = sy + 25 + 73;
    m_barY = sy + 25;
    m_barEndX = bx + 324;

    // --- Fill slot data ---
    struct SlotInit { unsigned int resID; int blockID; int x; int y; };
    SlotInit slotInits[kSlotCount] = {
        { 0x100000A1u, 0,  bx,         sy + 25 },        // slot 0
        { 0x100000A1u, 7,  bx + 212,   sy + 25 + 142 },  // slot 1
        { 0x100000A1u, 8,  bx + 171,   sy + 25 + 69 },   // slot 2
        { 0x100000A1u, 8,  bx + 223,   sy + 25 + 69 },   // slot 3
        { 0x100000A1u, 8,  bx + 275,   sy + 25 + 69 },   // slot 4
        { 0x2200000Au, 12, static_cast<int>(static_cast<unsigned short>(m_uiPos[0])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[1])) },  // slot 5
        { 0x20000002u, 0,  static_cast<int>(static_cast<unsigned short>(m_uiPos[2])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[3])) },  // slot 6
        { 0x20000002u, 1,  static_cast<int>(static_cast<unsigned short>(m_uiPos[2])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[3])) },  // slot 7
        { 0x1000009Bu, 20, static_cast<int>(static_cast<unsigned short>(m_uiPos[8])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[9])) },  // slot 8
        { 0x0C00017Eu, 0,  static_cast<int>(static_cast<unsigned short>(m_uiPos[6])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[7])) },  // slot 9
        { 0x100000A0u, 0,  static_cast<int>(static_cast<unsigned short>(m_uiPos[10])),
                            static_cast<int>(static_cast<unsigned short>(m_uiPos[11])) }, // slot 10
    };

    for (int i = 0; i < kSlotCount; ++i) {
        m_slots[i].active = 0;
        m_slots[i].resID = slotInits[i].resID;
        m_slots[i].blockID = static_cast<std::uint16_t>(slotInits[i].blockID);
        m_slots[i].x = slotInits[i].x;
        m_slots[i].y = slotInits[i].y;
    }

    // --- UI slot indices and initial active states ---
    m_slots[0].active = 1;
    m_slots[1].active = 1;
    m_stageBarSlotBase = 2;
    m_uiRanking = 5;
    m_uiWin = 6;
    m_uiLose = 7;
    m_uiSelectDegree = 8;
    m_uiHelp = 9;
    m_uiShowPoint = 10;

    // --- Buttons ---
    // Button 0: Start
    m_buttons[0].CreateBtn(
        sx + 37, sy + 472, 9u,
        0x2200000Au, 0, 0x2200000Au, 3u, 0x2200000Au, 6u,
        0x20000014u, 9u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Start),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 1: Ranking
    m_buttons[1].CreateBtn(
        sx + 183, sy + 472, 9u,
        0x2200000Au, 1u, 0x2200000Au, 4u, 0x2200000Au, 7u,
        0x20000014u, 0xAu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Ranking),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 2: Exit
    m_buttons[2].CreateBtn(
        sx + 621, sy + 472, 9u,
        0x2200000Au, 2u, 0x2200000Au, 5u, 0x2200000Au, 8u,
        0x20000014u, 0xBu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Exit),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 3: RankingPre
    m_buttons[3].CreateBtn(
        static_cast<unsigned short>(m_uiPos[0]) + 17,
        static_cast<unsigned short>(m_uiPos[1]) + 295, 9u,
        0x2200000Au, 0xDu, 0x2200000Au, 0xFu, 0x2200000Au, 0x11u,
        0x2200000Au, 0x13u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_RankingPre),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 4: RankingNext
    m_buttons[4].CreateBtn(
        static_cast<unsigned short>(m_uiPos[0]) + 62,
        static_cast<unsigned short>(m_uiPos[1]) + 295, 9u,
        0x2200000Au, 0xEu, 0x2200000Au, 0x10u, 0x2200000Au, 0x12u,
        0x2200000Au, 0x14u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_RankingNext),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 5: RankingExitPopUp
    m_buttons[5].CreateBtn(
        static_cast<unsigned short>(m_uiPos[0]) + 220,
        static_cast<unsigned short>(m_uiPos[1]) + 295, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 6: Help
    m_buttons[6].CreateBtn(
        sx + 329, sy + 472, 9u,
        0x1000009Bu, 0xCu, 0x1000009Bu, 0xEu, 0x1000009Bu, 0x10u,
        0x1000009Bu, 0x12u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_Help),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 7: ShowPoint
    m_buttons[7].CreateBtn(
        sx + 475, sy + 472, 9u,
        0x1000009Bu, 0xDu, 0x1000009Bu, 0xFu, 0x1000009Bu, 0x11u,
        0x1000009Bu, 0x13u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ShowPoint),
        reinterpret_cast<std::uintptr_t>(this), 1);

    // Button 8: ExitPopUp (for Help/ShowPoint)
    m_buttons[8].CreateBtn(
        sx + 566, sy + 513, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 9: DegreeEasy
    m_buttons[9].CreateBtn(
        static_cast<unsigned short>(m_uiPos[8]) + 36,
        static_cast<unsigned short>(m_uiPos[9]) + 48, 9u,
        0x1000009Bu, 0, 0x1000009Bu, 3u, 0x1000009Bu, 6u,
        0x1000009Bu, 9u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeEasy),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 10: DegreeNormal
    m_buttons[10].CreateBtn(
        static_cast<unsigned short>(m_uiPos[8]) + 36,
        static_cast<unsigned short>(m_uiPos[9]) + 102, 9u,
        0x1000009Bu, 1u, 0x1000009Bu, 4u, 0x1000009Bu, 7u,
        0x1000009Bu, 0xAu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeNormal),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 11: DegreeHard
    m_buttons[11].CreateBtn(
        static_cast<unsigned short>(m_uiPos[8]) + 36,
        static_cast<unsigned short>(m_uiPos[9]) + 156, 9u,
        0x1000009Bu, 2u, 0x1000009Bu, 5u, 0x1000009Bu, 8u,
        0x1000009Bu, 0xBu,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_DegreeHard),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // Button 12: EndGame ExitPopUp
    m_buttons[12].CreateBtn(
        static_cast<unsigned short>(m_uiPos[2]) + 215,
        static_cast<unsigned short>(m_uiPos[3]) + 170, 9u,
        0x2200000Au, 0x15u, 0x2200000Au, 0x16u, 0x2200000Au, 0x17u,
        0x2200000Au, 0x18u,
        reinterpret_cast<void(__cdecl*)(std::uintptr_t)>(OnBtn_ExitPopUp),
        reinterpret_cast<std::uintptr_t>(this), 0);

    // --- DrawNums ---
    m_drawNumRemain.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumScore.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumRemain.SetActive(0);
    m_drawNumReady.SetActive(0);
    m_drawNumScore.SetActive(0);

    // --- Alpha boxes ---
    m_alphaBox.Create(sx, sy + 9,
        static_cast<unsigned short>(800),
        static_cast<unsigned short>(600),
        0.0f, 0.0f, 0.0f, 128.0f / 255.0f, nullptr);

    m_drawAlphaBox = 0;

    m_timeBarBox.Create(m_barEndX, m_barY + 142,
        static_cast<unsigned short>(1),
        static_cast<unsigned short>(10),
        43.0f / 255.0f, 46.0f / 255.0f, 49.0f / 255.0f, 1.0f, nullptr);

    // --- Background resource ---
    m_bgResID = 0x20000120u;

    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_03.ogg");
}
