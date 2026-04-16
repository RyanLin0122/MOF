// cltMini_Exorcist_2 — 「驅魔（醫院）」小遊戲，依 mofclient.c 0x5B1000..0x5B2900 還原。
// 與 cltMini_Sword_2 採用相同的 cltMoF_BaseMiniGame 流程框架，只是把實際玩法
// 替換成 CHospital + 治療包選單，並改用獨立的 g_cGameExorcist_2State 狀態機。

#include "MiniGame/cltMini_Exorcist_2.h"

#include <cstdio>
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

extern unsigned char g_cGameExorcist_2State;

namespace {

// 6 個內嵌 slot 的資料表（對齊 mofclient.c InitMiniGameImage 的局部變數
// 連續陣列）。anchor 表示要套用 m_uiPos 中的第幾組 (x, y)。
struct SlotDef {
    unsigned int  res;
    std::uint16_t block;
    int           anchor; // 0..5 → m_uiPos[2*anchor .. 2*anchor+1]
};

// {
//   { 0x2200000A, 12, m_uiPos[0..1]  },  // ranking 底板
//   { 0x20000002,  0, m_uiPos[2..3]  },  // Win banner
//   { 0x20000002,  1, m_uiPos[2..3]  },  // Lose banner
//   { 0x1000009B, 20, m_uiPos[8..9]  },  // SelectDegree 底圖
//   { 0x2000004F,  0, m_uiPos[6..7]  },  // Help 底圖
//   { 0x100000A0,  0, m_uiPos[10..11]},  // ShowPoint 底圖
// }
static const SlotDef kSlotTable[cltMini_Exorcist_2::kSlotCount] = {
    { 0x2200000Au, 12, 0 },
    { 0x20000002u,  0, 1 },
    { 0x20000002u,  1, 1 },
    { 0x1000009Bu, 20, 4 },
    { 0x2000004Fu,  0, 3 },
    { 0x100000A0u,  0, 5 },
};

} // namespace

// =========================================================================
// 建構 / 解構
// =========================================================================
cltMini_Exorcist_2::cltMini_Exorcist_2()
    : cltMoF_BaseMiniGame()
    , m_pBgImage(nullptr)
    , m_bgResID(0)
    , m_totalScore(0)
    , m_difficultyBaseScore(0)
    , m_winMark(0)
    , m_currentRoundScore(0)
    , m_finalScore(0)
    , m_displayScore(0)
    , m_slots{}
    , m_drawNumReady()
    , m_drawNumFinal()
    , m_drawNumScore()
    , m_hospital()
    , m_difficulty(0)
    , m_score(0)
    , m_pollFrame(0)
    , m_uiRanking(0)
    , m_uiSelectDegree(3)
    , m_uiHelp(4)
    , m_uiShowPoint(5)
    , m_uiWin(1)
    , m_uiLose(2)
    , m_showTime2(0)
    , m_difficultyByte(0)
    , m_prevState(100)
    , m_finalReady(0)
    , m_serverAck(0)
    , m_serverResult(0)
    , m_serverValid(0)
    , m_startTick(0)
    , m_serverTimeMs(0)
    , m_exitTick(0)
    , m_incrementFactor(0.0f)
    , m_maxScore(0)
    , m_drawAlphaBox(0)
{
    // mofclient.c：建構子順序為 ctor 子物件 → m_difficulty=0 → m_score=0 →
    //   m_showTime=0 → m_showTime2=0 → m_prevState=100 →
    //   InitMiniGameImage → InitBtnFocus → m_pollFrame=0
    m_showTime  = 0;
    m_showTime2 = 0;
    m_prevState = 100;
    InitMiniGameImage();
    InitBtnFocus();
    m_pollFrame = 0;
}

cltMini_Exorcist_2::~cltMini_Exorcist_2() = default;

// =========================================================================
// InitMiniGameImage — 6 個 slot + 13 個按鈕 + 3 個 DrawNum + alphaBox + 音樂
// =========================================================================
void cltMini_Exorcist_2::InitMiniGameImage()
{
    // 1) 6 個 slot
    for (int i = 0; i < kSlotCount; ++i)
    {
        const SlotDef& d = kSlotTable[i];
        int ax = m_uiPos[d.anchor * 2 + 0];
        int ay = m_uiPos[d.anchor * 2 + 1];
        m_slots[i].active  = 0;
        m_slots[i].resID   = d.res;
        m_slots[i].blockID = d.block;
        m_slots[i].x       = ax;
        m_slots[i].y       = ay;
        m_slots[i].pImage  = nullptr;
    }

    // 2) UI slot 對照表
    m_uiRanking      = 0;
    m_uiWin          = 1;
    m_uiLose         = 2;
    m_uiSelectDegree = 3;
    m_uiHelp         = 4;
    m_uiShowPoint    = 5;

    // 3) 13 個按鈕（對齊 mofclient.c 的 CreateBtn 呼叫順序）
    auto cast  = [](void (*fn)(cltMini_Exorcist_2*)) -> void (*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };
    auto cast2 = [](void (*fn)()) -> void (*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };
    auto make = [&](int idx, int x, int y,
                    unsigned int imageType,
                    unsigned int r1, std::uint16_t b1,
                    unsigned int r2, std::uint16_t b2,
                    unsigned int r3, std::uint16_t b3,
                    unsigned int r4, std::uint16_t b4,
                    void (*cb)(unsigned int),
                    unsigned int userData,
                    int reserved)
    {
        m_buttons[idx].CreateBtn(x, y, imageType,
                                 r1, b1, r2, b2, r3, b3, r4, b4,
                                 cb, userData, reserved);
    };

    const unsigned int self = reinterpret_cast<unsigned int>(this);

    // 主選單：Start / Ranking / Exit
    make(0,  m_screenX + 37,  m_screenY + 472, 9u,
         0x2200000Au, 0, 0x2200000Au, 3, 0x2200000Au, 6,
         0x20000014u, 9, cast(&cltMini_Exorcist_2::OnBtn_Start), self, 1);

    make(1,  m_screenX + 183, m_screenY + 472, 9u,
         0x2200000Au, 1, 0x2200000Au, 4, 0x2200000Au, 7,
         0x20000014u, 10, cast(&cltMini_Exorcist_2::OnBtn_Ranking), self, 1);

    make(2,  m_screenX + 621, m_screenY + 472, 9u,
         0x2200000Au, 2, 0x2200000Au, 5, 0x2200000Au, 8,
         0x20000014u, 11, cast2(&cltMini_Exorcist_2::OnBtn_Exit), self, 1);

    // Ranking Pre / Next / Back
    make(3,  m_uiPos[0] + 17,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 13, 0x2200000Au, 15, 0x2200000Au, 17,
         0x2200000Au, 19, cast(&cltMini_Exorcist_2::OnBtn_RankingPre), self, 0);

    make(4,  m_uiPos[0] + 62,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 14, 0x2200000Au, 16, 0x2200000Au, 18,
         0x2200000Au, 20, cast(&cltMini_Exorcist_2::OnBtn_RankingNext), self, 0);

    make(5,  m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Exorcist_2::OnBtn_ExitPopUp), self, 0);

    // Help / ShowPoint
    make(6,  m_screenX + 329, m_screenY + 472, 9u,
         0x1000009Bu, 12, 0x1000009Bu, 14, 0x1000009Bu, 16,
         0x1000009Bu, 18, cast(&cltMini_Exorcist_2::OnBtn_Help), self, 1);

    make(7,  m_screenX + 475, m_screenY + 472, 9u,
         0x1000009Bu, 13, 0x1000009Bu, 15, 0x1000009Bu, 17,
         0x1000009Bu, 19, cast(&cltMini_Exorcist_2::OnBtn_ShowPoint), self, 1);

    // Help / ShowPoint popup 上的關閉鈕
    make(8,  m_screenX + 566, m_screenY + 513, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Exorcist_2::OnBtn_ExitPopUp), self, 0);

    // 難度選擇：Easy / Normal / Hard
    make(9,  m_uiPos[8] + 36,  m_uiPos[9] + 48,  9u,
         0x1000009Bu, 0, 0x1000009Bu, 3, 0x1000009Bu, 6,
         0x1000009Bu, 9, cast(&cltMini_Exorcist_2::OnBtn_DegreeEasy), self, 0);

    make(10, m_uiPos[8] + 36,  m_uiPos[9] + 102, 9u,
         0x1000009Bu, 1, 0x1000009Bu, 4, 0x1000009Bu, 7,
         0x1000009Bu, 10, cast(&cltMini_Exorcist_2::OnBtn_DegreeNormal), self, 0);

    make(11, m_uiPos[8] + 36,  m_uiPos[9] + 156, 9u,
         0x1000009Bu, 2, 0x1000009Bu, 5, 0x1000009Bu, 8,
         0x1000009Bu, 11, cast(&cltMini_Exorcist_2::OnBtn_DegreeHard), self, 0);

    // EndStage popup 關閉鈕
    make(12, m_uiPos[2] + 215, m_uiPos[3] + 170, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Exorcist_2::OnBtn_ExitPopUp), self, 0);

    // 3 個 DrawNum
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumReady.SetActive(0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumFinal.SetActive(0);
    m_drawNumScore.InitDrawNum(9u, 0x22000016u, 0, 1u);
    m_drawNumScore.SetActive(1);

    // 中央半透明遮罩 — 0x80000000 = 黑色 50% alpha
    m_alphaBox.Create(m_screenX, m_screenY + 9,
                      static_cast<unsigned short>(800),
                      static_cast<unsigned short>(600),
                      0.0f, 0.0f, 0.0f, 0.5f, nullptr);
    m_drawAlphaBox = 0;

    // 預設背景資源 = Easy 的 0x2000005A
    m_bgResID = 0x2000005Au;

    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_01.ogg");
    Init_Wait();
}

// =========================================================================
// Poll — 狀態分派 + 按鈕焦點處理
// =========================================================================
int cltMini_Exorcist_2::Poll()
{
    unsigned int prevFrame = m_pollFrame;
    m_drawAlphaBox = 0;
    m_pollFrame = prevFrame + 1;

    // mofclient.c：state ∈ {4, 6} 採 frame skip；其它每幀執行。
    bool inFrameSkip =
        (g_cGameExorcist_2State == 4 || g_cGameExorcist_2State == 6);
    if (inFrameSkip && m_pollFrame < static_cast<unsigned int>(SETTING_FRAME))
        return 0;

    m_pollFrame = 0;

    bool drawAlpha = false;

    switch (g_cGameExorcist_2State)
    {
        case 1:   // SelectDegree
        case 2:   // Help
        case 3:   // ShowPoint
            if (m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1))
            {
                Init_Wait();
                m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
            }
            drawAlpha = true;
            break;

        case 4:   // Ready
            Ready();
            break;

        case 5:   // Gamming
            Gamming();
            break;

        case 6:   // EndStage
            EndStage();
            break;

        case 7:   // Ranking
            Ranking();
            drawAlpha = true;
            break;

        case 100: // Exit
            return ExitGame();

        default:
            break;
    }

    m_drawAlphaBox = drawAlpha ? 1 : 0;

    // Button Poll + 焦點同步
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

    if (m_prevState != g_cGameExorcist_2State)
    {
        m_prevState = g_cGameExorcist_2State;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }

    MoveBtnFocus(g_cGameExorcist_2State);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// =========================================================================
// PrepareDrawing
// =========================================================================
void cltMini_Exorcist_2::PrepareDrawing()
{
    // 背景圖（DWORD[2]）— 由 m_bgResID 決定，依難度切換
    cltImageManager* pMgr = m_pclImageMgr;
    GameImage* pBg = pMgr->GetGameImage(9u, m_bgResID, 0, 1);
    m_pBgImage = pBg;
    if (pBg)
    {
        pBg->SetBlockID(0);
        pBg->m_bFlag_447       = true;
        pBg->m_bFlag_446       = true;
        pBg->m_bVertexAnimation = false;
        pBg->m_fPosX           = static_cast<float>(m_screenX);
        pBg->m_fPosY           = static_cast<float>(m_screenY);
    }

    // 6 個 slot
    for (int i = 0; i < kSlotCount; ++i)
    {
        ImageSlot& s = m_slots[i];
        if (!s.active)
            continue;

        GameImage* p = pMgr->GetGameImage(9u, s.resID, 0, 1);
        s.pImage = p;
        if (!p)
            continue;

        p->SetBlockID(s.blockID);
        p->m_bFlag_446        = true;
        p->m_bFlag_447        = true;
        p->m_bVertexAnimation = false;
        p->m_fPosX            = static_cast<float>(s.x);
        p->m_fPosY            = static_cast<float>(s.y);
    }

    // 13 個按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // 3 個 DrawNum
    unsigned int readySec = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220,
                                  readySec, 255);
    m_drawNumFinal.PrepareDrawing(m_uiPos[4], m_uiPos[5],
                                  static_cast<unsigned int>(m_displayScore), 255);
    m_drawNumScore.PrepareDrawing(m_screenX + 52, m_screenY + 109,
                                  static_cast<unsigned int>(m_score), 255);

    // 中央半透明遮罩
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();
}

// =========================================================================
// Draw
// =========================================================================
void cltMini_Exorcist_2::Draw()
{
    if (m_pBgImage)
        m_pBgImage->Draw();

    m_hospital.Render();

    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    // 6 個 slot
    for (int i = 0; i < kSlotCount; ++i)
    {
        ImageSlot& s = m_slots[i];
        if (s.active && s.pImage)
            s.pImage->Draw();
    }

    // 13 個按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    if (g_cGameExorcist_2State == 7)
    {
        if (!m_rankDrawCounter)
            DrawRanking(static_cast<std::uint16_t>(m_uiPos[0]),
                        static_cast<std::uint16_t>(m_uiPos[1]), 0);
    }

    if (g_cGameExorcist_2State == 3)
        ShowPointText();

    m_drawNumReady.Draw();
    m_drawNumFinal.Draw();
    m_drawNumScore.Draw();
}

int cltMini_Exorcist_2::ExitGame()
{
    return 1;
}

void cltMini_Exorcist_2::Ready()
{
    // mofclient.c：cltMini_Exorcist_2::Ready 在 IDB 中僅有宣告但無函式本體；
    // 倒數實際由 OnTimer_DecreaseReadyTime / OnTimer_TimeOutReadyTime 推進。
}

// =========================================================================
// Button handlers / Init_* 系列
// =========================================================================
void cltMini_Exorcist_2::OnBtn_Start(cltMini_Exorcist_2* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist_2::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[9].SetActive(1);
    m_buttons[10].SetActive(1);
    m_buttons[11].SetActive(1);
    m_slots[m_uiSelectDegree].active = 1;
    g_cGameExorcist_2State = 1;
}

void cltMini_Exorcist_2::OnBtn_Ranking(cltMini_Exorcist_2* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist_2::Init_Ranking()
{
    m_slots[m_uiRanking].active = 1;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[3].SetActive(1);
    m_buttons[4].SetActive(1);
    m_buttons[5].SetActive(1);

    m_slots[m_uiWin].active  = 0;
    m_slots[m_uiLose].active = 0;

    m_myRankingText[0] = 0;
    m_curRankPage      = 0;
    std::memset(m_ranking, 0, sizeof(m_ranking));

    RequestRanking(0x29u, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameExorcist_2State = 7;
}

void cltMini_Exorcist_2::OnBtn_Exit()
{
    g_cGameExorcist_2State = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist_2::OnBtn_RankingPre(cltMini_Exorcist_2* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist_2::Init_PreRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p)
    {
        m_curRankPage = static_cast<std::uint8_t>(p - 1);
        RequestRanking(0x29u, static_cast<std::uint8_t>(p - 1));
    }
}

void cltMini_Exorcist_2::OnBtn_RankingNext(cltMini_Exorcist_2* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist_2::Init_NextRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p < 0x1Du)
    {
        m_curRankPage = static_cast<std::uint8_t>(p + 1);
        RequestRanking(0x29u, static_cast<std::uint8_t>(p + 1));
    }
}

void cltMini_Exorcist_2::OnBtn_Help(cltMini_Exorcist_2* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist_2::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);
    m_slots[m_uiHelp].active = 1;
    g_cGameExorcist_2State = 2;
}

void cltMini_Exorcist_2::OnBtn_ShowPoint(cltMini_Exorcist_2* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist_2::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);
    m_slots[m_uiShowPoint].active = 1;
    g_cGameExorcist_2State = 3;
}

void cltMini_Exorcist_2::OnBtn_ExitPopUp(cltMini_Exorcist_2* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist_2::OnBtn_DegreeEasy(cltMini_Exorcist_2* self)
{
    self->SetGameDegree(1u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Exorcist_2::OnBtn_DegreeNormal(cltMini_Exorcist_2* self)
{
    self->SetGameDegree(2u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Exorcist_2::OnBtn_DegreeHard(cltMini_Exorcist_2* self)
{
    self->SetGameDegree(4u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// =========================================================================
// SetGameDegree — 依難度設常數 + 啟動 ready timer
// =========================================================================
void cltMini_Exorcist_2::SetGameDegree(std::uint8_t degree)
{
    m_difficultyByte    = degree;
    m_currentRoundScore = 30;
    m_score             = 0;
    m_incrementFactor   = 0.0f;

    switch (degree)
    {
        case 1: // Easy
            m_winMark             = 50;
            m_difficultyBaseScore = 10;
            m_maxScore            = 90;
            m_incrementFactor     = 4.0f;          // 1082130432
            m_bgResID             = 0x2000005Au;   // 536871194
            m_difficulty          = 0;
            break;

        case 2: // Normal
            m_winMark             = 100;
            m_difficultyBaseScore = 25;
            m_maxScore            = 180;
            m_incrementFactor     = 8.0f;          // 1090519040
            m_bgResID             = 0x2000005Bu;   // 536871195
            m_difficulty          = 1;
            break;

        case 4: // Hard
            m_winMark             = 200;
            m_difficultyBaseScore = 40;
            m_maxScore            = 360;
            m_incrementFactor     = 16.0f;         // 1098907648
            m_bgResID             = 0x2000005Cu;   // 536871196
            m_difficulty          = 2;
            break;

        default:
            break;
    }

    // 關掉 Win / Lose 底圖
    m_slots[m_uiWin].active  = 0;
    m_slots[m_uiLose].active = 0;
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[m_uiSelectDegree].active = 0;

    // mofclient.c：Init 用 m_difficultyBaseScore 當 remainSec、3 秒當 readySec
    InitMiniGameTime(static_cast<unsigned int>(m_difficultyBaseScore), 3u);
    m_dword149 = 0;
    unsigned int rt = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000u * rt,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist_2::OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist_2::OnTimer_DecreaseReadyTime),
        nullptr);

    m_drawNumReady.SetActive(1);
    m_finalReady = 1;
    m_totalScore = 1;
    g_cGameExorcist_2State = 4;
}

// =========================================================================
// Timer callbacks
// =========================================================================
void cltMini_Exorcist_2::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Exorcist_2::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Exorcist_2* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
}

void cltMini_Exorcist_2::OnTimer_DecreaseRemainTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseRemainTime();
}

// =========================================================================
// Init_Wait — 回到主選單
// =========================================================================
void cltMini_Exorcist_2::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime2);
    m_buttons[1].SetActive(1);
    m_buttons[2].SetActive(1);
    m_buttons[6].SetActive(1);
    m_buttons[7].SetActive(1);

    m_slots[m_uiWin].active          = 0;
    m_slots[m_uiLose].active         = 0;
    m_slots[m_uiSelectDegree].active = 0;
    m_slots[m_uiHelp].active         = 0;
    m_slots[m_uiShowPoint].active    = 0;
    m_slots[m_uiRanking].active      = 0;

    m_drawNumFinal.SetActive(0);
    m_score        = 0;
    m_serverTimeMs = 0;
    g_cGameExorcist_2State = 0;
}

// =========================================================================
// StartGame
// =========================================================================
void cltMini_Exorcist_2::StartGame()
{
    m_drawNumReady.SetActive(0);
    m_startTick = timeGetTime();
    g_cGameExorcist_2State = 5;

    m_hospital.InitHospital(m_difficulty);

    m_serverAck    = 0;
    m_serverResult = 0;
    m_serverValid  = 0;
    m_focusEnabled = 0;
}

// =========================================================================
// Gamming — 把控制權交給 CHospital；它回傳 1 表示時間用盡 → 切 EndStage
// =========================================================================
void cltMini_Exorcist_2::Gamming()
{
    int r = m_hospital.Process();
    m_score = m_hospital.GetGameScore();
    if (r == 1)
    {
        m_serverTimeMs = timeGetTime() - m_startTick;
        g_cGameExorcist_2State = 6;
    }
}

// =========================================================================
// EndStage — 結算 + 送分 + 回 Wait
// =========================================================================
void cltMini_Exorcist_2::EndStage()
{
    if (m_finalReady)
    {
        if (!cltValidMiniGameScore::IsValidScore(0x29u, static_cast<std::uint32_t>(m_score)))
        {
            char buf[256];
            char* txt = m_pDCTTextManager->GetText(58092);
            wsprintfA(buf, "client : %s : %i", txt, m_score);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        unsigned int point = static_cast<unsigned int>(m_score);
        int baseScore      = m_winMark;
        int roundScore     = m_currentRoundScore;
        m_totalScore = 0;
        m_finalScore = roundScore;
        if (point >= static_cast<unsigned int>(m_difficultyBaseScore))
        {
            m_totalScore = 1;
            m_finalScore = baseScore;
        }

        m_drawNumFinal.SetActive(1);

        if (m_totalScore)
        {
            m_slots[m_uiWin].active = 1;
            unsigned int bonus = static_cast<unsigned int>(
                static_cast<double>(point - static_cast<unsigned int>(m_difficultyBaseScore))
                * static_cast<double>(m_incrementFactor));
            unsigned int score = static_cast<unsigned int>(m_finalScore) + bonus;
            if (m_maxScore < score)
                score = m_maxScore;
            m_finalScore = static_cast<int>(score);
        }
        else
        {
            m_slots[m_uiLose].active = 1;
        }

        int mult = GetMultipleNum();
        unsigned int finalPoint = static_cast<unsigned int>(m_finalScore);
        m_displayScore = static_cast<int>(finalPoint * static_cast<unsigned int>(mult));

        SendScore(static_cast<std::uint8_t>(m_totalScore & 0xFF),
                  static_cast<std::uint32_t>(m_score),
                  m_serverTimeMs, m_difficultyByte, finalPoint);

        m_finalReady = 0;
    }

    if (m_serverAck)
    {
        if (m_serverValid)
        {
            int res = static_cast<int>(m_serverResult);
            m_showTime2    = 4;
            m_serverValid  = 0;
            m_focusEnabled = 1;

            if (!res)
            {
                char* txt = m_pDCTTextManager->GetText(58092);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, txt, 0, 0, 0);
                Init_Wait();
                return;
            }

            unsigned int lessonPt = static_cast<unsigned int>(m_displayScore);
            g_clLessonSystem.TraningLessonFinished(m_dword522, m_difficultyByte,
                                                   m_totalScore, &lessonPt);
            m_displayScore = static_cast<int>(lessonPt);

            if (!g_clLessonSystem.CanTraningLesson(0x29u))
                m_showTime2 = 0;

            m_buttons[12].SetActive(1);
            InitBtnFocus();
        }

        if ((m_pInputMgr->IsKeyDown(1)    && m_focusEnabled)
            || (m_pInputMgr->IsJoyButtonPush(1) && m_focusEnabled))
        {
            Init_Wait();
            m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        }
        else if (timeGetTime() - m_exitTick > 0x1388u)
        {
            Init_Wait();
        }
    }
    else
    {
        m_exitTick = timeGetTime();
    }
}

// =========================================================================
// Ranking — 排行榜狀態下每幀更新按鈕
// =========================================================================
void cltMini_Exorcist_2::Ranking()
{
    if (m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1))
    {
        Init_Wait();
        m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
    }

    m_buttons[3].SetBtnState(0);
    if (m_curRankPage)
    {
        if (m_pInputMgr->IsKeyDown(16) || m_pInputMgr->IsJoyButtonPush(0))
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
    if (m_curRankPage <= 0x1Cu && m_rankCount == 10)
    {
        if (m_pInputMgr->IsKeyDown(17) || m_pInputMgr->IsJoyButtonPush(0))
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

// =========================================================================
// ShowPointText — ShowPoint 介面顯示得分對照表（10/25/40 → 50~90/100~180/200~360）
// =========================================================================
void cltMini_Exorcist_2::ShowPointText()
{
    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3343);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 70, m_uiPos[11] + 50, 0xFFFFFFFFu,
                           title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 127, 0xFFFFFFFFu, "10", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 157, 0xFFFFFFFFu, "25", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 187, 0xFFFFFFFFu, "40", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 127, 0xFFFFFFFFu, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 157, 0xFFFFFFFFu, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 187, 0xFFFFFFFFu, "200 ~ 360", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 127, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 157, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 187, 0xFFFFFFFFu, "30", 1, -1, -1);
}
