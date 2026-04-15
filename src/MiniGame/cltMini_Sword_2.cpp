// cltMini_Sword_2 — 擊劍（進階）小遊戲，依 mofclient.c 原始流程還原。
// 每個函式都對齊 mofclient.c 的呼叫順序（僅以專案內已存在的類別 API 實作），
// 並共用本專案的 cltMoF_BaseMiniGame 欄位（按鈕 / 排名 / Timer / UI 座標）。

#include "MiniGame/cltMini_Sword_2.h"

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

// 與原始同樣由全域狀態機驅動整個流程。
extern unsigned char g_cGameSword_2State;

namespace {

// 6 個內嵌 slot 的資料表，對齊 mofclient.c InitMiniGameImage 裡的
// v26 / v27..v45 連續陣列（每個 slot 20 bytes = 5 ints）。
//
//   anchor = -1 表示用 screenX/Y；0..5 分別對應 m_uiPos[0..11] 的六組 x/y。
struct SlotDef
{
    unsigned int  res;
    std::uint16_t block;
    int           anchor; // 0..5 → m_uiPos 的第 anchor 組 (x,y)
};

// {
//   { 0x2200000A, 12, m_uiPos[0..1] },   // ranking 底板
//   { 0x20000002,  0, m_uiPos[2..3] },   // Win banner
//   { 0x20000002,  1, m_uiPos[2..3] },   // Lose banner
//   { 0x1000001B, 20, m_uiPos[8..9] },   // 難度選擇底圖
//   { 0x20000050,  0, m_uiPos[6..7] },   // Help 底圖
//   { 0x10000020,  0, m_uiPos[10..11] }, // ShowPoint 底圖
// }
static const SlotDef kSlotTable[cltMini_Sword_2::kSlotCount] = {
    { 0x2200000Au, 12, 0 },
    { 0x20000002u,  0, 1 },
    { 0x20000002u,  1, 1 },
    { 0x1000001Bu, 20, 4 },
    { 0x20000050u,  0, 3 },
    { 0x10000020u,  0, 5 },
};

} // namespace

// =========================================================================
// 建構 / 解構
// =========================================================================
cltMini_Sword_2::cltMini_Sword_2()
    : cltMoF_BaseMiniGame()
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
    , m_topBlackBox()
    , m_botBlackBox()
    , m_uiRanking(0)
    , m_uiSelectDegree(3)
    , m_uiHelp(4)
    , m_uiShowPoint(5)
    , m_uiWin(1)
    , m_uiLose(2)
    , m_sword2()
    , m_bgMgr()
    , m_showTime2(0)
    , m_difficulty(0)
    , m_finalReady(0)
    , m_serverAck(0)
    , m_serverResult(0)
    , m_serverValid(0)
    , m_startTick(0)
    , m_serverTimeMs(0)
    , m_exitTick(0)
    , m_incrementFactor(0.0f)
    , m_maxScore(0)
    , m_sword2Degree(0)
    , m_finalPoint(0)
    , m_pollFrame(0)
    , m_prevState(100)
{
    // mofclient.c：建構子尾段順序為 InitMiniGameImage → InitBtnFocus →
    //              m_showTime = 0 → m_prevState = 100 → m_showTime2 = 0 →
    //              m_pollFrame = 0。
    InitMiniGameImage();
    InitBtnFocus();
    m_showTime  = 0;
    m_prevState = 100;
    m_showTime2 = 0;
    m_pollFrame = 0;
}

cltMini_Sword_2::~cltMini_Sword_2() = default;

// =========================================================================
// InitMiniGameImage — 填 6 個 slot + 建立 13 個按鈕 + 2 個 AlphaBox
// =========================================================================
void cltMini_Sword_2::InitMiniGameImage()
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

    // 2) UI slot index 對照表（對齊原始 byte 604 / byte 2780..2784）
    m_uiRanking      = 0;
    m_uiWin          = 1;
    m_uiLose         = 2;
    m_uiSelectDegree = 3;
    m_uiHelp         = 4;
    m_uiShowPoint    = 5;

    // 3) 13 個按鈕（對齊 mofclient.c 的 CreateBtn 呼叫 — 改用本專案 API 排列）
    auto cast  = [](void (*fn)(cltMini_Sword_2*)) -> void (*)(unsigned int) {
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
         0x20000014u, 9, cast(&cltMini_Sword_2::OnBtn_Start), self, 1);

    make(1,  m_screenX + 183, m_screenY + 472, 9u,
         0x2200000Au, 1, 0x2200000Au, 4, 0x2200000Au, 7,
         0x20000014u, 10, cast(&cltMini_Sword_2::OnBtn_Ranking), self, 1);

    make(2,  m_screenX + 621, m_screenY + 472, 9u,
         0x2200000Au, 2, 0x2200000Au, 5, 0x2200000Au, 8,
         0x20000014u, 11, cast2(&cltMini_Sword_2::OnBtn_Exit), self, 1);

    // Ranking Pre / Next / Back
    make(3,  m_uiPos[0] + 17,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 13, 0x2200000Au, 15, 0x2200000Au, 17,
         0x2200000Au, 19, cast(&cltMini_Sword_2::OnBtn_RankingPre), self, 0);

    make(4,  m_uiPos[0] + 62,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 14, 0x2200000Au, 16, 0x2200000Au, 18,
         0x2200000Au, 20, cast(&cltMini_Sword_2::OnBtn_RankingNext), self, 0);

    make(5,  m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword_2::OnBtn_ExitPopUp), self, 0);

    // Help / ShowPoint
    make(6,  m_screenX + 329, m_screenY + 472, 9u,
         0x1000009Bu, 12, 0x1000009Bu, 14, 0x1000009Bu, 16,
         0x1000009Bu, 18, cast(&cltMini_Sword_2::OnBtn_Help), self, 1);

    make(7,  m_screenX + 475, m_screenY + 472, 9u,
         0x1000009Bu, 13, 0x1000009Bu, 15, 0x1000009Bu, 17,
         0x1000009Bu, 19, cast(&cltMini_Sword_2::OnBtn_ShowPoint), self, 1);

    // Help / ShowPoint popup 上的關閉鈕
    make(8,  m_screenX + 566, m_screenY + 513, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword_2::OnBtn_ExitPopUp), self, 0);

    // 難度選擇：Easy / Normal / Hard
    make(9,  m_uiPos[8] + 36,  m_uiPos[9] + 48,  9u,
         0x1000009Bu, 0, 0x1000009Bu, 3, 0x1000009Bu, 6,
         0x1000009Bu, 9, cast(&cltMini_Sword_2::OnBtn_DegreeEasy), self, 0);

    make(10, m_uiPos[8] + 36,  m_uiPos[9] + 102, 9u,
         0x1000009Bu, 1, 0x1000009Bu, 4, 0x1000009Bu, 7,
         0x1000009Bu, 10, cast(&cltMini_Sword_2::OnBtn_DegreeNormal), self, 0);

    make(11, m_uiPos[8] + 36,  m_uiPos[9] + 156, 9u,
         0x1000009Bu, 2, 0x1000009Bu, 5, 0x1000009Bu, 8,
         0x1000009Bu, 11, cast(&cltMini_Sword_2::OnBtn_DegreeHard), self, 0);

    // EndStage popup 關閉鈕
    make(12, m_uiPos[2] + 215, m_uiPos[3] + 170, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword_2::OnBtn_ExitPopUp), self, 0);

    // 2 個 DrawNum（ready 計時 + 結算分數）
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumReady.SetActive(0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumFinal.SetActive(0);

    // 半透明 base 遮罩（對齊 mofclient.c：x=screenX,y=0,w=800,h=screenY+600,
    //   color = 0x80000000 → 50% alpha 黑）
    m_alphaBox.Create(m_screenX, 0,
                      static_cast<unsigned short>(800),
                      static_cast<unsigned short>(m_screenY + 600),
                      0.0f, 0.0f, 0.0f, 0.5f, nullptr);
    // mofclient.c 352604：*((_DWORD *)this + 211) = 0（frame-local alphabox flag）
    m_drawAlphaBox = 0;

    // 上黑條：x=screenX, y=0, w=800, h=screenY+50, 黑不透明
    m_topBlackBox.Create(m_screenX, 0,
                         static_cast<unsigned short>(800),
                         static_cast<unsigned short>(m_screenY + 50),
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    // 下黑條：x=screenX, y=screenY+500, w=800, h=screenY+100, 黑不透明
    m_botBlackBox.Create(m_screenX, m_screenY + 500,
                         static_cast<unsigned short>(800),
                         static_cast<unsigned short>(m_screenY + 100),
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);

    // 音樂 + 初始背景滾動
    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame.ogg");

    float bx = static_cast<float>(m_screenX);
    float by = static_cast<float>(m_screenY) - 140.0f;
    m_bgMgr.InitBackGroundMgr(0, bx, by);

    Init_Wait();
}

// =========================================================================
// Poll — 狀態分派；同一 Poll 內兼負責按鈕 Poll + 焦點處理
// =========================================================================
int cltMini_Sword_2::Poll()
{
    // mofclient.c：*((_DWORD *)this + 211) = 0 — 每幀先把 m_alphaBox 顯示
    // 旗標清為 0，後面依 state 決定是否重新設為 1。與 base class 的
    // m_rankDrawCounter (DWORD[104]) 無關。
    unsigned int prevFrame = m_pollFrame;
    m_drawAlphaBox = 0;
    m_pollFrame = prevFrame + 1;

    // Ready / Gamming / EndStage 三態採 frame-skip；其它狀態立即處理。
    bool inGamePhase =
        (g_cGameSword_2State == 4 ||
         g_cGameSword_2State == 5 ||
         g_cGameSword_2State == 6);

    if (inGamePhase && m_pollFrame < SETTING_FRAME)
        return 0;

    m_pollFrame = 0;

    switch (g_cGameSword_2State)
    {
        case 1:   // SelectDegree
        case 2:   // Help
        case 3:   // ShowPoint
            if (m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1))
            {
                Init_Wait();
                m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
            }
            m_drawAlphaBox = 1;
            break;

        case 4:   // Ready
            Ready();
            break;

        case 6:   // EndStage
            EndStage();
            break;

        case 7:   // Ranking
            Ranking();
            m_drawAlphaBox = 1;
            break;

        case 100: // Exit
            return ExitGame();

        default:
            break;
    }

    // Button Poll + 焦點處理
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

    if (m_prevState != g_cGameSword_2State)
    {
        m_prevState = g_cGameSword_2State;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }

    MoveBtnFocus(g_cGameSword_2State);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// =========================================================================
// PrepareDrawing
// =========================================================================
void cltMini_Sword_2::PrepareDrawing()
{
    m_bgMgr.Process(0.0f);

    // 原始：在 Gamming 階段同樣會在 PrepareDrawing 內驅動 Sword2::Process
    // （不是只有 Gamming() 被呼叫時），以保證每一幀都推進子遊戲。
    if (g_cGameSword_2State == 5 && m_sword2.Process())
    {
        m_finalPoint   = static_cast<std::uint32_t>(m_sword2.GetTotalScore());
        m_serverTimeMs = timeGetTime() - m_startTick;
        g_cGameSword_2State = 6;
    }

    // 6 個 slot：更新 GameImage 位置、block id、旗標
    cltImageManager* pMgr = m_pclImageMgr;
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
        p->m_bFlag_446 = true;
        p->m_bFlag_447 = true;
        p->m_bVertexAnimation = false;
        p->m_fPosX = static_cast<float>(s.x);
        p->m_fPosY = static_cast<float>(s.y);
    }

    // 13 個按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    // Ready 倒數數字與最終得分數字
    unsigned int readySec = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220, readySec, 255);
    m_drawNumFinal.PrepareDrawing(m_uiPos[4], m_uiPos[5],
                                  static_cast<unsigned int>(m_displayScore), 255);

    // AlphaBox 三塊：top / bottom 永遠處理；中央半透明遮罩只在排行榜/選單顯示。
    m_topBlackBox.PrepareDrawing();
    m_botBlackBox.PrepareDrawing();
    // mofclient.c 352211：if (*((_DWORD *)this + 211)) m_alphaBox.PrepareDrawing()
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();
}

// =========================================================================
// Draw
// =========================================================================
void cltMini_Sword_2::Draw()
{
    m_bgMgr.Render();
    m_sword2.Render();

    // mofclient.c：Draw 依 *((_DWORD *)this + 211) 決定是否畫整塊半透明遮罩
    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    m_topBlackBox.Draw();
    m_botBlackBox.Draw();

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

    // 排行榜 / ShowPoint 文字繪製
    if (g_cGameSword_2State == 7)
    {
        if (!m_rankDrawCounter)
            DrawRanking(static_cast<std::uint16_t>(m_uiPos[0]),
                        static_cast<std::uint16_t>(m_uiPos[1]), 0);
    }

    if (g_cGameSword_2State == 3)
        ShowPointText();

    m_drawNumReady.Draw();
    m_drawNumFinal.Draw();
}

// =========================================================================
// ExitGame
// =========================================================================
int cltMini_Sword_2::ExitGame()
{
    return 1;
}

// =========================================================================
// Ready — 狀態 4 時每幀呼叫；實體僅驅動狀態機，等待 timer callback 觸發
// =========================================================================
void cltMini_Sword_2::Ready()
{
    // 原始 mofclient.c 中 cltMini_Sword_2::Ready 為 IDB 保留但無顯式實作，
    // 實際的倒數由 OnTimer_DecreaseReadyTime / OnTimer_TimeOutReadyTime 處理。
    // 我們在這裡只作每幀 dwFrameCnt 推進，行為等價。
    ++dwFrameCnt;
}

// =========================================================================
// Button handlers / Init_* 系列
// =========================================================================
void cltMini_Sword_2::OnBtn_Start(cltMini_Sword_2* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword_2::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[9].SetActive(1);
    m_buttons[10].SetActive(1);
    m_buttons[11].SetActive(1);
    m_slots[m_uiSelectDegree].active = 1;
    g_cGameSword_2State = 1;
}

void cltMini_Sword_2::OnBtn_Ranking(cltMini_Sword_2* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword_2::Init_Ranking()
{
    m_slots[m_uiRanking].active = 1;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[3].SetActive(1);
    m_buttons[4].SetActive(1);
    m_buttons[5].SetActive(1);

    m_slots[m_uiWin].active  = 0;
    m_slots[m_uiLose].active = 0;

    // 原始：byte 421 (myRankingText 首位) = 0；byte 412 = 0（curRankPage）
    m_myRankingText[0] = 0;
    m_curRankPage = 0;
    std::memset(m_ranking, 0, sizeof(m_ranking));

    RequestRanking(0xBu, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameSword_2State = 7;
}

void cltMini_Sword_2::OnBtn_Exit()
{
    g_cGameSword_2State = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword_2::OnBtn_RankingPre(cltMini_Sword_2* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword_2::Init_PreRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p)
    {
        m_curRankPage = static_cast<std::uint8_t>(p - 1);
        RequestRanking(0xBu, static_cast<std::uint8_t>(p - 1));
    }
}

void cltMini_Sword_2::OnBtn_RankingNext(cltMini_Sword_2* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword_2::Init_NextRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p < 0x1Du)
    {
        m_curRankPage = static_cast<std::uint8_t>(p + 1);
        RequestRanking(0xBu, static_cast<std::uint8_t>(p + 1));
    }
}

void cltMini_Sword_2::OnBtn_Help(cltMini_Sword_2* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword_2::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);
    m_slots[m_uiHelp].active = 1;
    g_cGameSword_2State = 2;
}

void cltMini_Sword_2::OnBtn_ShowPoint(cltMini_Sword_2* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword_2::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);
    m_slots[m_uiShowPoint].active = 1;
    g_cGameSword_2State = 3;
}

void cltMini_Sword_2::OnBtn_ExitPopUp(cltMini_Sword_2* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword_2::OnBtn_DegreeEasy(cltMini_Sword_2* self)
{
    self->SetGameDegree(1u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Sword_2::OnBtn_DegreeNormal(cltMini_Sword_2* self)
{
    self->SetGameDegree(2u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Sword_2::OnBtn_DegreeHard(cltMini_Sword_2* self)
{
    self->SetGameDegree(4u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// =========================================================================
// SetGameDegree — 依難度設常數 + 啟動 ready timer
// =========================================================================
void cltMini_Sword_2::SetGameDegree(std::uint8_t degree)
{
    m_difficulty        = degree;
    m_currentRoundScore = 30;
    m_incrementFactor   = 0.0f;

    const float bgX = static_cast<float>(m_screenX);
    const float bgY = static_cast<float>(m_screenY) - 140.0f;

    switch (degree)
    {
        case 1: // Easy
            m_winMark             = 50;
            m_difficultyBaseScore = 12;
            m_maxScore            = 90;
            m_incrementFactor     = 5.0f;            // IEEE 754 1084227584
            m_sword2Degree        = 1;
            m_bgMgr.InitBackGroundMgr(0, bgX, bgY);
            break;

        case 2: // Normal
            m_winMark             = 100;
            m_difficultyBaseScore = 20;
            m_maxScore            = 180;
            m_incrementFactor     = 8.0f;            // IEEE 754 1090519040
            m_sword2Degree        = 2;
            m_bgMgr.InitBackGroundMgr(1, bgX, bgY);
            break;

        case 4: // Hard
            m_winMark             = 200;
            m_difficultyBaseScore = 27;
            m_maxScore            = 360;
            m_incrementFactor     = 40.0f;           // IEEE 754 1101004800
            m_sword2Degree        = 2;
            m_bgMgr.InitBackGroundMgr(2, bgX, bgY);
            break;

        default:
            break;
    }

    // 關掉 Win / Lose / SelectDegree 底圖與全部按鈕
    m_slots[m_uiWin].active          = 0;
    m_slots[m_uiLose].active         = 0;
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[m_uiSelectDegree].active = 0;

    // Ready 倒數 3 秒 + 啟動 timer
    // mofclient.c：第一參數為 *((_DWORD *)this + 6)，即剛設定好的
    // m_difficultyBaseScore (Easy=12, Normal=20, Hard=27)。
    InitMiniGameTime(static_cast<unsigned int>(m_difficultyBaseScore), 3u);
    m_dword149 = 0;
    unsigned int rt = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000u * rt,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword_2::OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword_2::OnTimer_DecreaseReadyTime),
        nullptr);

    m_drawNumReady.SetActive(1);
    m_finalReady = 1;
    m_totalScore = 1;       // 原始 *(this+5) = 1
    g_cGameSword_2State = 4;
}

// =========================================================================
// Timer callbacks
// =========================================================================
void cltMini_Sword_2::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Sword_2::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Sword_2* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
}

void cltMini_Sword_2::OnTimer_DecreaseRemainTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseRemainTime();
}

// =========================================================================
// Init_Wait — 回到主選單狀態
// =========================================================================
void cltMini_Sword_2::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    // 啟用主選單四顆按鈕 + Help/ShowPoint
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
    m_serverTimeMs = 0;

    g_cGameSword_2State = 0;
}

// =========================================================================
// StartGame — Ready 計時結束後
// =========================================================================
void cltMini_Sword_2::StartGame()
{
    m_serverAck    = 0;
    m_serverResult = 0;
    m_serverValid  = 0;
    m_focusEnabled = 0;

    m_drawNumReady.SetActive(0);
    m_startTick    = timeGetTime();
    g_cGameSword_2State = 5;
    m_sword2.InitSword2(static_cast<int>(m_sword2Degree));
}

// =========================================================================
// Gamming — 直接把控制權讓給 Sword2::Process；結束時切 6
// =========================================================================
void cltMini_Sword_2::Gamming()
{
    if (m_sword2.Process())
    {
        m_finalPoint   = static_cast<std::uint32_t>(m_sword2.GetTotalScore());
        m_serverTimeMs = timeGetTime() - m_startTick;
        g_cGameSword_2State = 6;
    }
}

// =========================================================================
// EndStage — 結算 + 送分 + 回 Wait
// =========================================================================
void cltMini_Sword_2::EndStage()
{
    if (m_finalReady)
    {
        // 首次進入：驗證分數，計算顯示分，送到伺服器
        if (!cltValidMiniGameScore::IsValidScore(0xBu, m_finalPoint))
        {
            char buf[256];
            char* txt = m_pDCTTextManager->GetText(58092);
            wsprintfA(buf, "client : %s : %i", txt, static_cast<int>(m_finalPoint));
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        unsigned int point = m_finalPoint;
        int baseScore      = m_winMark;
        int roundScore     = m_currentRoundScore;
        m_totalScore = 0;
        m_finalScore = roundScore;

        if (point >= static_cast<unsigned int>(m_difficultyBaseScore) && m_finalReady)
        {
            m_totalScore = 1;
            m_finalScore = baseScore;
        }

        m_drawNumFinal.SetActive(1);

        if (m_totalScore)
        {
            m_slots[m_uiWin].active = 1;
            unsigned int bonus = static_cast<unsigned int>(
                static_cast<double>(m_finalPoint - static_cast<unsigned int>(m_difficultyBaseScore))
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

        int mult   = GetMultipleNum();
        unsigned int finalPoint = static_cast<unsigned int>(m_finalScore);
        m_displayScore = static_cast<int>(finalPoint * static_cast<unsigned int>(mult));

        // 原始：SendScore(this, *((_BYTE *)this + 20), m_finalPoint, m_serverTimeMs,
        //                  m_difficulty, finalPoint)
        //   this+20（低位）= m_totalScore（0/1）→ 作為 lessonType 低位傳入
        SendScore(static_cast<std::uint8_t>(m_totalScore & 0xFF),
                  m_finalPoint, m_serverTimeMs, m_difficulty, finalPoint);

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
            g_clLessonSystem.TraningLessonFinished(m_dword522, m_difficulty,
                                                   m_totalScore, &lessonPt);
            m_displayScore = static_cast<int>(lessonPt);

            if (!g_clLessonSystem.CanTraningLesson(0xBu))
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
// Ranking — 在排行榜狀態下每幀更新按鈕狀態
// =========================================================================
void cltMini_Sword_2::Ranking()
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
// ShowPointText — ShowPoint 介面顯示得分表
// =========================================================================
void cltMini_Sword_2::ShowPointText()
{
    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3340);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 70, m_uiPos[11] + 50, 0xFFFFFFFFu,
                           title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 127, 0xFFFFFFFFu, "12", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 157, 0xFFFFFFFFu, "20", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 187, 0xFFFFFFFFu, "27", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 127, 0xFFFFFFFFu, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 157, 0xFFFFFFFFu, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 187, 0xFFFFFFFFu, "200 ~ 360", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 127, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 157, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 187, 0xFFFFFFFFu, "30", 1, -1, -1);
}
