// cltMini_Exorcist — 「驅魔」小遊戲（早期卡片版本），對齊 mofclient.c
// 0x5ACFE0..0x5AF660 還原。狀態機透過全域 g_cGameExorcistState 推進；
// 玩法：3 張卡片在每回合內由暗變亮，玩家在 4 (max-lit) 階段時按下對應
// LEFT / UP / RIGHT 方向鍵即算成功，系統再依分數進入下一輪。

#include "MiniGame/cltMini_Exorcist.h"

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
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_MiniGame_Fighter_String.h"
#include "Effect/CEffect_MiniGame_Priest_Light.h"

extern unsigned char g_cGameExorcistState;
extern unsigned char g_cCardState[3];

namespace {

// -----------------------------------------------------------------------
// 19 個有效的影像 slot — 對齊 mofclient.c InitMiniGameImage 的 v22[]。
// anchor 對應 base 的座標來源：
//   0 = (screenX+84,  screenY+154)   → 卡片底圖 (slots 0..6)
//   1 = (screenX+324, screenY+433)   → 左方向鍵 off / on (slots 7,8)
//   2 = (screenX+380, screenY+433)   → 上方向鍵 off / on (slots 9,10)
//   3 = (screenX+436, screenY+433)   → 右方向鍵 off / on (slots 11,12)
//   4 = (m_uiPos[2..3])              → Win / Lose banner   (slots 13,14)
//   5 = (m_uiPos[0..1])              → Ranking 底圖        (slot 15)
//   6 = (m_uiPos[8..9])              → SelectDegree 底圖   (slot 16)
//   7 = (m_uiPos[6..7])              → Help 底圖           (slot 17)
//   8 = (m_uiPos[10..11])            → ShowPoint 底圖      (slot 18)
// -----------------------------------------------------------------------
struct SlotDef {
    unsigned int  res;
    std::uint16_t block;
    int           anchor;
};

static const SlotDef kSlotTable[19] = {
    { 0x0C000180u, 0, 0 },  // 0  card light frame 0
    { 0x0C000180u, 1, 0 },  // 1  card light frame 1
    { 0x0C000181u, 0, 0 },  // 2  card dark  frame 0
    { 0x0C000181u, 1, 0 },  // 3  card dark  frame 1
    { 0x0C000180u, 2, 0 },  // 4  card frame 2 (shared)
    { 0x0C000180u, 3, 0 },  // 5  card frame 3 (shared)
    { 0x0C000182u, 0, 0 },  // 6  card frame 4 (max lit)
    { 0x22000007u, 1, 1 },  // 7  arrow LEFT  off
    { 0x22000007u, 5, 1 },  // 8  arrow LEFT  on
    { 0x22000007u, 0, 2 },  // 9  arrow UP    off
    { 0x22000007u, 4, 2 },  // 10 arrow UP    on
    { 0x22000007u, 3, 3 },  // 11 arrow RIGHT off
    { 0x22000007u, 7, 3 },  // 12 arrow RIGHT on
    { 0x20000002u, 0, 4 },  // 13 Win banner
    { 0x20000002u, 1, 4 },  // 14 Lose banner
    { 0x2200000Au,12, 5 },  // 15 Ranking BG
    { 0x1000009Bu,20, 6 },  // 16 SelectDegree BG
    { 0x0C000183u, 0, 7 },  // 17 Help BG
    { 0x100000A0u, 0, 8 },  // 18 ShowPoint BG
};

// helper — 將 slot 條目轉成 (x, y)
struct AnchorXY { int x; int y; };
static AnchorXY ResolveAnchor(int anchor, int sx, int sy, const std::int16_t* uiPos)
{
    switch (anchor)
    {
        case 0: return { sx + 84,  sy + 154 };
        case 1: return { sx + 324, sy + 433 };
        case 2: return { sx + 380, sy + 433 };
        case 3: return { sx + 436, sy + 433 };
        case 4: return { uiPos[2],  uiPos[3]  };
        case 5: return { uiPos[0],  uiPos[1]  };
        case 6: return { uiPos[8],  uiPos[9]  };
        case 7: return { uiPos[6],  uiPos[7]  };
        case 8: return { uiPos[10], uiPos[11] };
    }
    return { sx, sy };
}

} // namespace

// =========================================================================
// 建構 / 解構
// =========================================================================
cltMini_Exorcist::cltMini_Exorcist()
    : cltMoF_BaseMiniGame()
    , m_slotImages{}
    , m_slots{}
    , m_pBgImage(nullptr)
    , m_bgResID(0)
    , m_totalScore(0)
    , m_difficultyBaseScore(0)
    , m_winMark(0)
    , m_currentRoundScore(0)
    , m_finalScore(0)
    , m_displayScore(0)
    , m_drawNumRemain()
    , m_drawNumReady()
    , m_drawNumScore()
    , m_drawNumFinal()
    , m_score(0)
    , m_targetCard{}
    , m_cardImageIdx{}
    , m_remainingHits(0)
    , m_inputCooldown(0)
    , m_cardX{}
    , m_cardLightSlots{}
    , m_cardDarkSlots{}
    , m_cardViewValue{ 4, 4, 4 }
    , m_cardYPos(0)
    , m_uiRanking(0)
    , m_uiHelp(0)
    , m_uiSelectDegree(0)
    , m_uiShowPoint(0)
    , m_uiWin(0)
    , m_uiLose(0)
    , m_cardSlots{}
    , m_inputReceivedThisFrame(0)
    , m_openTimerId(0)
    , m_hitTimerId(0)
    , m_openTimeMs(0)
    , m_hitTimeMs(0)
    , m_pollFrame(0)
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
    // mofclient.c：cltMoF_BaseMiniGame ctor → 4 個 cltMiniGame_DrawNum
    // ctor → memset(slots/卡片區) → 設置卡片 x → InitMiniGameImage →
    // InitBtnFocus → 卡片動畫常數 → g_cGameExorcistState=0 → Init_Wait

    // m_slots / m_slotImages 已由建構 in-class 預設值清 0。
    std::memset(m_targetCard,    0, sizeof(m_targetCard));
    std::memset(m_cardImageIdx,  0, sizeof(m_cardImageIdx));

    // 卡片 x 座標 (WORD[1800..1802])
    m_cardX[0] = static_cast<std::uint16_t>(m_screenX + 86);
    m_cardX[1] = static_cast<std::uint16_t>(m_screenX + 296);
    m_cardX[2] = static_cast<std::uint16_t>(m_screenX + 506);

    m_showTime2 = 0;

    InitMiniGameImage();
    InitBtnFocus();

    // 卡片動畫常數（BYTE[3613..3625]）
    m_cardLightSlots[0] = 0;
    m_cardLightSlots[1] = 1;
    m_cardLightSlots[2] = 4;
    m_cardLightSlots[3] = 5;
    m_cardLightSlots[4] = 6;

    m_cardDarkSlots[0] = 2;
    m_cardDarkSlots[1] = 3;
    m_cardDarkSlots[2] = 4;
    m_cardDarkSlots[3] = 5;
    m_cardDarkSlots[4] = 6;

    m_cardViewValue[0] = 4;
    m_cardViewValue[1] = 4;
    m_cardViewValue[2] = 4;

    g_cGameExorcistState = 0;
    m_prevState = 100;
    m_showTime  = 0;

    m_openTimerId = 0;
    m_hitTimerId  = 0;
    m_pollFrame   = 0;

    Init_Wait();
}

cltMini_Exorcist::~cltMini_Exorcist() = default;

// =========================================================================
// InitMiniGameImage
// =========================================================================
void cltMini_Exorcist::InitMiniGameImage()
{
    // m_cardYPos = screenY + 154
    m_cardYPos = m_screenY + 154;

    // 19 個 slot 寫入；其餘 slot 保持 active=0
    for (int i = 0; i < 19; ++i)
    {
        const SlotDef& d = kSlotTable[i];
        AnchorXY a = ResolveAnchor(d.anchor, m_screenX, m_screenY, m_uiPos);
        m_slots[i].active  = 0;
        m_slots[i].resID   = d.res;
        m_slots[i].blockID = d.block;
        m_slots[i].x       = a.x;
        m_slots[i].y       = a.y;
    }
    for (int i = 19; i < kSlotCount; ++i)
        m_slots[i].active = 0;

    // 卡片 / 方向鍵 slot 索引
    m_cardSlots[0] = 6;   // BYTE[3606]
    m_cardSlots[1] = 7;   // BYTE[3607]
    m_cardSlots[2] = 8;   // BYTE[3608] arrow LEFT  on
    m_cardSlots[3] = 9;   // BYTE[3609] arrow UP    off
    m_cardSlots[4] = 10;  // BYTE[3610] arrow UP    on
    m_cardSlots[5] = 11;  // BYTE[3611] arrow RIGHT off
    m_cardSlots[6] = 12;  // BYTE[3612] arrow RIGHT on

    // UI slot 索引
    m_uiWin          = 13; // BYTE[608]
    m_uiLose         = 14; // BYTE[609]
    m_uiRanking      = 15; // BYTE[604]
    m_uiHelp         = 16; // BYTE[605]
    m_uiSelectDegree = 17; // BYTE[606]
    m_uiShowPoint    = 18; // BYTE[607]

    // mofclient.c：在 InitMiniGameImage 末尾把 slot 7、9、11（三個 OFF
    // 方向鍵底圖）預先打開，讓 idle/select 階段就能看見方向鍵殼。
    m_slots[7].active  = 1;
    m_slots[9].active  = 1;
    m_slots[11].active = 1;

    // 13 顆按鈕 — 對齊 mofclient.c 的 CreateBtn 順序
    auto cast  = [](void (*fn)(cltMini_Exorcist*)) -> void (*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };
    auto cast2 = [](void (*fn)()) -> void (*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };

    const unsigned int self = reinterpret_cast<unsigned int>(this);

    // 主選單：Start / Ranking / Exit
    m_buttons[0].CreateBtn(m_screenX + 37,  m_screenY + 472, 9u,
        0x2200000Au, 0,  0x2200000Au, 3,  0x2200000Au, 6,
        0x20000014u, 9,
        cast(&cltMini_Exorcist::OnBtn_Start), self, 1);

    m_buttons[1].CreateBtn(m_screenX + 183, m_screenY + 472, 9u,
        0x2200000Au, 1,  0x2200000Au, 4,  0x2200000Au, 7,
        0x20000014u, 10,
        cast(&cltMini_Exorcist::OnBtn_Ranking), self, 1);

    m_buttons[2].CreateBtn(m_screenX + 621, m_screenY + 472, 9u,
        0x2200000Au, 2,  0x2200000Au, 5,  0x2200000Au, 8,
        0x20000014u, 11,
        cast2(&cltMini_Exorcist::OnBtn_Exit), self, 1);

    // Ranking pre / next / popup-close
    m_buttons[3].CreateBtn(m_uiPos[0] + 17,  m_uiPos[1] + 295, 9u,
        0x2200000Au, 13, 0x2200000Au, 15, 0x2200000Au, 17,
        0x2200000Au, 19,
        cast(&cltMini_Exorcist::OnBtn_RankingPre), self, 0);

    m_buttons[4].CreateBtn(m_uiPos[0] + 62,  m_uiPos[1] + 295, 9u,
        0x2200000Au, 14, 0x2200000Au, 16, 0x2200000Au, 18,
        0x2200000Au, 20,
        cast(&cltMini_Exorcist::OnBtn_RankingNext), self, 0);

    m_buttons[5].CreateBtn(m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
        0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
        0x2200000Au, 24,
        cast(&cltMini_Exorcist::OnBtn_ExitPopUp), self, 0);

    // Help / ShowPoint
    m_buttons[6].CreateBtn(m_screenX + 329, m_screenY + 472, 9u,
        0x1000009Bu, 12, 0x1000009Bu, 14, 0x1000009Bu, 16,
        0x1000009Bu, 18,
        cast(&cltMini_Exorcist::OnBtn_Help), self, 1);

    m_buttons[7].CreateBtn(m_screenX + 475, m_screenY + 472, 9u,
        0x1000009Bu, 13, 0x1000009Bu, 15, 0x1000009Bu, 17,
        0x1000009Bu, 19,
        cast(&cltMini_Exorcist::OnBtn_ShowPoint), self, 1);

    // Help / ShowPoint popup 上的關閉鈕
    m_buttons[8].CreateBtn(m_screenX + 566, m_screenY + 513, 9u,
        0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
        0x2200000Au, 24,
        cast(&cltMini_Exorcist::OnBtn_ExitPopUp), self, 0);

    // 難度選擇：Easy / Normal / Hard
    m_buttons[9].CreateBtn(m_uiPos[8] + 36,  m_uiPos[9] + 48, 9u,
        0x1000009Bu, 0,  0x1000009Bu, 3,  0x1000009Bu, 6,
        0x1000009Bu, 9,
        cast(&cltMini_Exorcist::OnBtn_DegreeEasy), self, 0);

    m_buttons[10].CreateBtn(m_uiPos[8] + 36, m_uiPos[9] + 102, 9u,
        0x1000009Bu, 1,  0x1000009Bu, 4,  0x1000009Bu, 7,
        0x1000009Bu, 10,
        cast(&cltMini_Exorcist::OnBtn_DegreeNormal), self, 0);

    m_buttons[11].CreateBtn(m_uiPos[8] + 36, m_uiPos[9] + 156, 9u,
        0x1000009Bu, 2,  0x1000009Bu, 5,  0x1000009Bu, 8,
        0x1000009Bu, 11,
        cast(&cltMini_Exorcist::OnBtn_DegreeHard), self, 0);

    // EndStage popup 關閉鈕
    m_buttons[12].CreateBtn(m_uiPos[2] + 215, m_uiPos[3] + 170, 9u,
        0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
        0x2200000Au, 24,
        cast(&cltMini_Exorcist::OnBtn_ExitPopUp), self, 0);

    // 4 個 DrawNum
    m_drawNumRemain.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0, 0);
    m_drawNumScore.InitDrawNum(9u, 0x22000016u, 0, 0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0, 0);

    m_drawNumRemain.SetActive(0);
    m_drawNumReady.SetActive(0);
    m_drawNumScore.SetActive(1);
    m_drawNumFinal.SetActive(0);

    // 中央半透明遮罩
    m_alphaBox.Create(m_screenX, m_screenY,
                      static_cast<unsigned short>(800),
                      static_cast<unsigned short>(600),
                      0.0f, 0.0f, 0.0f, 0.5f, nullptr);
    m_drawAlphaBox = 0;

    // 預設背景資源
    m_bgResID = 0x2000005Du;

    m_pGameSoundMgr->PlayMusic((char*)"MoFData/Music/bg_minigame_01.ogg");
}

// =========================================================================
// Poll — 狀態分派 + 按鈕焦點處理
// =========================================================================
int cltMini_Exorcist::Poll()
{
    unsigned int prevFrame = m_pollFrame;
    m_drawAlphaBox = 0;
    m_pollFrame    = prevFrame + 1;

    // mofclient.c：state ∈ {4,5,6} 採 frame skip；其它每幀執行。
    bool inFrameSkip = (g_cGameExorcistState == 4
                     || g_cGameExorcistState == 5
                     || g_cGameExorcistState == 6);
    if (inFrameSkip && m_pollFrame < static_cast<unsigned int>(SETTING_FRAME))
        return 0;

    m_pollFrame = 0;

    bool drawAlpha = false;

    switch (g_cGameExorcistState)
    {
        case 1: // SelectDegree
        case 2: // Help
        case 3: // ShowPoint
            if (m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1))
            {
                Init_Wait();
                m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
            }
            drawAlpha = true;
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
            drawAlpha = true;
            break;

        case 100:
            ExitGame();
            return 1;

        default:
            break;
    }

    m_drawAlphaBox = drawAlpha ? 1 : 0;

    // 按鈕 Poll + 焦點同步
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

    if (m_prevState != g_cGameExorcistState)
    {
        m_prevState = g_cGameExorcistState;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }

    MoveBtnFocus(g_cGameExorcistState);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// =========================================================================
// PrepareDrawing
// =========================================================================
void cltMini_Exorcist::PrepareDrawing()
{
    cltImageManager* pMgr = m_pclImageMgr;

    // 背景圖
    GameImage* pBg = pMgr->GetGameImage(9u, m_bgResID, 0, 1);
    m_pBgImage = pBg;
    if (pBg)
    {
        pBg->SetBlockID(0);
        pBg->m_bFlag_447        = true;
        pBg->m_bFlag_446        = true;
        pBg->m_bVertexAnimation = false;
        pBg->m_fPosX            = static_cast<float>(m_screenX);
        pBg->m_fPosY            = static_cast<float>(m_screenY);
    }

    PrepareDrawingCard();

    // mofclient.c 從 slot[3] 開始迴圈共 47 次，slot 0..2 由 PrepareDrawingCard 處理
    for (int i = 3; i < kSlotCount; ++i)
    {
        ImageSlot& s = m_slots[i];
        if (!s.active)
            continue;

        GameImage* p = pMgr->GetGameImage(9u, s.resID, 0, 1);
        m_slotImages[i] = p;
        if (!p)
            continue;

        p->SetBlockID(s.blockID);
        p->m_bFlag_447        = true;
        p->m_bFlag_446        = true;
        p->m_bVertexAnimation = false;
        p->m_fPosX            = static_cast<float>(s.x);
        p->m_fPosY            = static_cast<float>(s.y);
    }

    // 4 個 DrawNum
    unsigned int remainSec = GetRemainTime();
    m_drawNumRemain.PrepareDrawing(m_screenX + 652, m_screenY + 88,
                                   remainSec, 255);

    unsigned int readySec = GetReadyTime();
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 220,
                                  readySec, 255);

    m_drawNumScore.PrepareDrawing(m_screenX + 721, m_screenY + 88,
                                  static_cast<unsigned int>(m_score), 255);

    m_drawNumFinal.PrepareDrawing(m_uiPos[4], m_uiPos[5],
                                  static_cast<unsigned int>(m_displayScore), 255);

    // 13 顆按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();
}

// =========================================================================
// PrepareDrawingCard — 解析 3 張卡片現在要畫哪個 slot
// =========================================================================
void cltMini_Exorcist::PrepareDrawingCard()
{
    cltImageManager* pMgr = m_pclImageMgr;

    for (int i = 0; i < 3; ++i)
    {
        std::uint8_t slotIdx = m_cardImageIdx[i];
        if (slotIdx >= kSlotCount)
            continue;

        const ImageSlot& src = m_slots[slotIdx];
        GameImage* p = pMgr->GetGameImage(9u, src.resID, 0, 1);
        m_slotImages[i] = p;
        if (!p)
            continue;

        p->SetBlockID(src.blockID);
        p->m_bFlag_447        = true;
        p->m_bFlag_446        = true;
        p->m_bVertexAnimation = false;
        p->m_fPosX            = static_cast<float>(m_cardX[i]);
        p->m_fPosY            = static_cast<float>(m_cardYPos);
    }
}

// =========================================================================
// Draw
// =========================================================================
void cltMini_Exorcist::Draw()
{
    if (m_pBgImage)
        m_pBgImage->Draw();

    // 50 個影像 slot
    for (int i = 0; i < kSlotCount; ++i)
    {
        if (m_slots[i].active && m_slotImages[i])
            m_slotImages[i]->Draw();
    }

    m_drawNumRemain.Draw();
    m_drawNumReady.Draw();
    m_drawNumScore.Draw();
    m_drawNumFinal.Draw();

    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    // 對齊 mofclient.c：若四個 UI slot active，重新覆繪一次
    auto drawIfActive = [this](std::uint8_t slot) {
        if (slot < kSlotCount && m_slots[slot].active)
        {
            GameImage* p = m_slotImages[slot];
            if (p) p->Draw();
        }
    };
    drawIfActive(m_uiRanking);
    drawIfActive(m_uiHelp);
    drawIfActive(m_uiSelectDegree);
    drawIfActive(m_uiShowPoint);

    // mofclient.c 在 Draw 結尾覆寫 BYTE[605..607] = 16/17/18
    m_uiHelp         = 16;
    m_uiSelectDegree = 17;
    m_uiShowPoint    = 18;

    // 13 顆按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    if (g_cGameExorcistState == 7)
    {
        if (m_rankDrawCounter == 0)
            DrawRanking(static_cast<std::uint16_t>(m_uiPos[0]),
                        static_cast<std::uint16_t>(m_uiPos[1]), 0);
    }

    if (g_cGameExorcistState == 3)
        ShowPointText();
}

// =========================================================================
// ExitGame
// =========================================================================
int cltMini_Exorcist::ExitGame()
{
    m_pGameSoundMgr->StopMusic();
    return 1;
}

// =========================================================================
// Init_Wait — 回到主選單
// =========================================================================
void cltMini_Exorcist::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_cardImageIdx[0] = 2;
    m_cardImageIdx[1] = 0;
    m_cardImageIdx[2] = 2;
    m_score           = 0;
    m_serverTimeMs    = 0;
    m_difficultyByte  = 0;

    if (m_uiRanking < kSlotCount) m_slots[m_uiRanking].active = 1;
    if (m_uiHelp < kSlotCount)    m_slots[m_uiHelp].active    = 1;
    if (m_uiSelectDegree < kSlotCount) m_slots[m_uiSelectDegree].active = 1;

    // 主選單 5 顆按鈕
    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime2);
    m_buttons[1].SetActive(1);
    m_buttons[2].SetActive(1);
    m_buttons[6].SetActive(1);
    m_buttons[7].SetActive(1);

    // 關閉所有狀態相關 UI slot
    if (m_uiWin < kSlotCount)          m_slots[m_uiWin].active          = 0;
    if (m_uiLose < kSlotCount)         m_slots[m_uiLose].active         = 0;
    if (m_uiRanking < kSlotCount)      m_slots[m_uiRanking].active      = 0;
    if (m_uiHelp < kSlotCount)         m_slots[m_uiHelp].active         = 0;
    if (m_uiSelectDegree < kSlotCount) m_slots[m_uiSelectDegree].active = 0;
    if (m_uiShowPoint < kSlotCount)    m_slots[m_uiShowPoint].active    = 0;

    m_drawNumFinal.SetActive(0);
    g_cGameExorcistState = 0;
}

// =========================================================================
// Init_SelectDegree
// =========================================================================
void cltMini_Exorcist::Init_SelectDegree()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[9].SetActive(1);
    m_buttons[10].SetActive(1);
    m_buttons[11].SetActive(1);

    if (m_uiHelp < kSlotCount) m_slots[m_uiHelp].active = 1;
    g_cGameExorcistState = 1;
}

// =========================================================================
// Ranking 系列
// =========================================================================
void cltMini_Exorcist::Init_Ranking()
{
    if (m_uiRanking < kSlotCount) m_slots[m_uiRanking].active = 1;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[3].SetActive(1);
    m_buttons[4].SetActive(1);
    m_buttons[5].SetActive(1);

    if (m_uiWin < kSlotCount)  m_slots[m_uiWin].active  = 0;
    if (m_uiLose < kSlotCount) m_slots[m_uiLose].active = 0;

    m_myRankingText[0] = 0;
    m_curRankPage      = 0;
    std::memset(m_ranking, 0, sizeof(m_ranking));
    RequestRanking(0x28u, m_curRankPage);

    m_rankDrawCounter = 1;
    g_cGameExorcistState = 7;
}

void cltMini_Exorcist::Ranking()
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

void cltMini_Exorcist::Init_PreRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p)
    {
        m_curRankPage = static_cast<std::uint8_t>(p - 1);
        RequestRanking(0x28u, static_cast<std::uint8_t>(p - 1));
    }
}

void cltMini_Exorcist::Init_NextRanking()
{
    std::uint8_t p = m_curRankPage;
    if (p < 0x1Du)
    {
        m_curRankPage = static_cast<std::uint8_t>(p + 1);
        RequestRanking(0x28u, static_cast<std::uint8_t>(p + 1));
    }
}

// =========================================================================
// Help / ShowPoint
// =========================================================================
void cltMini_Exorcist::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);

    if (m_uiSelectDegree < kSlotCount)
        m_slots[m_uiSelectDegree].active = 1;
    g_cGameExorcistState = 2;
}

void cltMini_Exorcist::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);

    if (m_uiShowPoint < kSlotCount)
        m_slots[m_uiShowPoint].active = 1;
    g_cGameExorcistState = 3;
}

// =========================================================================
// ShowPointText — ShowPoint 介面顯示得分對照表
// =========================================================================
void cltMini_Exorcist::ShowPointText()
{
    g_MoFFont.SetFont("MiniShowGetPointTitle");
    const char* title = m_pDCTTextManager->GetText(3343);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 70, m_uiPos[11] + 50, 0xFFFFFFFFu,
                           title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 127, 0xFFFFFFFFu, "10", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 157, 0xFFFFFFFFu, "45", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 187, 0xFFFFFFFFu, "75", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 127, 0xFFFFFFFFu, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 157, 0xFFFFFFFFu, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 187, 0xFFFFFFFFu, "200 ~ 360", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 127, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 157, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 187, 0xFFFFFFFFu, "30", 1, -1, -1);
}

// =========================================================================
// Button handlers
// =========================================================================
void cltMini_Exorcist::OnBtn_Start(cltMini_Exorcist* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist::OnBtn_Ranking(cltMini_Exorcist* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist::OnBtn_Exit()
{
    g_cGameExorcistState = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist::OnBtn_RankingPre(cltMini_Exorcist* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist::OnBtn_RankingNext(cltMini_Exorcist* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist::OnBtn_Help(cltMini_Exorcist* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist::OnBtn_ShowPoint(cltMini_Exorcist* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Exorcist::OnBtn_ExitPopUp(cltMini_Exorcist* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Exorcist::OnBtn_DegreeEasy(cltMini_Exorcist* self)
{
    self->SetGameDegree(1u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Exorcist::OnBtn_DegreeNormal(cltMini_Exorcist* self)
{
    self->SetGameDegree(2u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Exorcist::OnBtn_DegreeHard(cltMini_Exorcist* self)
{
    self->SetGameDegree(4u);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// =========================================================================
// SetGameDegree — 依難度設常數 + 啟動 ready timer
// =========================================================================
void cltMini_Exorcist::SetGameDegree(std::uint8_t degree)
{
    m_difficultyByte    = degree;
    m_currentRoundScore = 30;
    m_incrementFactor   = 0.0f;

    switch (degree)
    {
        case 1u:  // Easy
            m_winMark             = 50;
            m_difficultyBaseScore = 10;
            m_maxScore            = 90;
            m_incrementFactor     = 2.0f;          // 0x40000000
            m_bgResID             = 0x2000005Du;   // 536871197
            break;
        case 2u:  // Normal
            m_winMark             = 100;
            m_difficultyBaseScore = 45;
            m_maxScore            = 180;
            m_incrementFactor     = 4.0f;          // 0x40800000 (1082130432)
            m_bgResID             = 0x2000005Fu;   // 536871199
            break;
        case 4u:  // Hard
            m_winMark             = 200;
            m_difficultyBaseScore = 75;
            m_maxScore            = 360;
            m_incrementFactor     = 16.0f;         // 0x41800000 (1098907648)
            m_bgResID             = 0x2000005Eu;   // 536871198
            break;
        default:
            break;
    }

    if (m_uiWin < kSlotCount)  m_slots[m_uiWin].active  = 0;
    if (m_uiLose < kSlotCount) m_slots[m_uiLose].active = 0;
    m_drawNumFinal.SetActive(0);
    m_score = 0;

    m_finalReady = 1;
    m_totalScore = 1;
    InitMiniGameTime(0x32u, 3u);

    m_dword149 = 0;
    unsigned int rt = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000u * rt,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_DecreaseReadyTime),
        nullptr);

    m_drawNumReady.SetActive(1);

    // 卡片預設都顯示「最大 lit」frame (slot 6)
    m_cardImageIdx[0] = m_cardSlots[0]; // 6
    m_cardImageIdx[1] = m_cardSlots[0]; // 6
    m_cardImageIdx[2] = m_cardSlots[0]; // 6

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    if (m_uiHelp < kSlotCount) m_slots[m_uiHelp].active = 0;
    g_cGameExorcistState = 4;
}

// =========================================================================
// Ready — 偵測方向鍵高亮（倒數階段預覽用）
// =========================================================================
void cltMini_Exorcist::Ready()
{
    // 預設關閉三個方向鍵 ON 圖
    if (m_cardSlots[2] < kSlotCount) m_slots[m_cardSlots[2]].active = 0; // LEFT  on
    if (m_cardSlots[4] < kSlotCount) m_slots[m_cardSlots[4]].active = 0; // UP    on
    if (m_cardSlots[6] < kSlotCount) m_slots[m_cardSlots[6]].active = 0; // RIGHT on

    if (m_pInputMgr->IsKeyPressed(203) || m_pInputMgr->IsJoyStickPush(0, 1))
    {
        if (m_cardSlots[2] < kSlotCount) m_slots[m_cardSlots[2]].active = 1;
    }
    if (m_pInputMgr->IsKeyPressed(200) || m_pInputMgr->IsJoyStickPush(1, 1))
    {
        if (m_cardSlots[4] < kSlotCount) m_slots[m_cardSlots[4]].active = 1;
    }
    if (m_pInputMgr->IsKeyPressed(205) || m_pInputMgr->IsJoyStickPush(0, 2))
    {
        if (m_cardSlots[6] < kSlotCount) m_slots[m_cardSlots[6]].active = 1;
    }
}

// =========================================================================
// StartGame
// =========================================================================
void cltMini_Exorcist::StartGame()
{
    std::srand(timeGetTime());
    InitCardTime(0x5DCu, 0x5DCu);

    m_dword148 = 0;
    unsigned int v3 = GetRemainTime();
    m_dword148 = g_clTimerManager.CreateTimer(
        1000u * v3,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_StageClear),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_DecreaseRemainTime),
        nullptr);

    m_drawNumReady.SetActive(0);
    m_drawNumRemain.SetActive(1);
    m_score = 0;

    g_cCardState[0] = 3;
    g_cCardState[1] = 3;
    g_cCardState[2] = 3;

    CreateNewCard();

    m_startTick    = timeGetTime();
    m_serverAck    = 0;
    m_serverResult = 0;
    m_serverValid  = 0;
    m_focusEnabled = 0;

    g_cGameExorcistState = 5;
}

// =========================================================================
// Gamming — 接受 LEFT/UP/RIGHT 輸入並更新 3 張卡片動畫
// =========================================================================
void cltMini_Exorcist::Gamming()
{
    if (m_pInputMgr->IsKeyDown(203) || m_pInputMgr->IsJoyStickPush(0, 1))
        m_inputReceivedThisFrame = KeyHit(0xCBu);
    if (m_pInputMgr->IsKeyDown(200) || m_pInputMgr->IsJoyStickPush(1, 1))
        m_inputReceivedThisFrame = KeyHit(0xC8u);
    if (m_pInputMgr->IsKeyDown(205) || m_pInputMgr->IsJoyStickPush(0, 2))
        m_inputReceivedThisFrame = KeyHit(0xCDu);

    for (int i = 0; i < 3; ++i)
        DecideCardViewValue(static_cast<std::uint8_t>(i));

    if (m_inputReceivedThisFrame)
        ++m_inputCooldown;
    if (m_inputCooldown > 10u)
    {
        g_cCardState[0] = 5;
        g_cCardState[1] = 5;
        g_cCardState[2] = 5;
    }
}

// =========================================================================
// EndGame — 結算 + 送分 + 回 Wait
// =========================================================================
void cltMini_Exorcist::EndGame()
{
    g_cCardState[0] = 2;
    g_cCardState[1] = 2;
    g_cCardState[2] = 2;

    if (m_dword149)
        g_clTimerManager.ReleaseTimer(m_dword149);
    if (m_dword148)
    {
        g_clTimerManager.ReleaseTimer(m_dword148);
        m_remainTime = 0;
    }

    if (m_finalReady)
    {
        m_serverTimeMs = timeGetTime() - m_startTick;
        unsigned int score = m_score;

        if (!cltValidMiniGameScore::IsValidScore(0x28u, score))
        {
            char buf[256];
            char* txt = m_pDCTTextManager->GetText(58092);
            wsprintfA(buf, "client : %s %i", txt, m_score);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
            Init_Wait();
            return;
        }

        unsigned int baseScore  = static_cast<unsigned int>(m_difficultyBaseScore);
        int          roundScore = m_currentRoundScore;
        m_totalScore = 0;
        m_finalScore = roundScore;

        if (score < baseScore)
        {
            if (m_uiLose < kSlotCount) m_slots[m_uiLose].active = 1;
        }
        else
        {
            m_finalScore = m_winMark;
            m_totalScore = 1;
            if (m_uiWin < kSlotCount) m_slots[m_uiWin].active = 1;
        }

        m_drawNumFinal.SetActive(1);

        if (m_totalScore)
        {
            unsigned int bonus = static_cast<unsigned int>(
                static_cast<double>(score - baseScore)
                * static_cast<double>(m_incrementFactor));
            unsigned int v = static_cast<unsigned int>(m_finalScore) + bonus;
            if (m_maxScore < v)
                v = m_maxScore;
            m_finalScore = static_cast<int>(v);
        }

        unsigned int finalPoint = static_cast<unsigned int>(m_finalScore);
        int mult = GetMultipleNum();
        m_displayScore = static_cast<int>(finalPoint * static_cast<unsigned int>(mult));

        SendScore(static_cast<std::uint8_t>(m_totalScore & 0xFF),
                  static_cast<std::uint32_t>(m_score),
                  m_serverTimeMs, m_difficultyByte, finalPoint);

        m_finalReady = 0;
        if (m_dword149)
            g_clTimerManager.ReleaseTimer(m_dword149);
        if (m_dword148)
            g_clTimerManager.ReleaseTimer(m_dword148);
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

            if (!g_clLessonSystem.CanTraningLesson(0x28u))
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
// Timer callbacks
// =========================================================================
void cltMini_Exorcist::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Exorcist::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Exorcist* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
    m_pGameSoundMgr->PlaySoundA((char*)"M0002", 0, 0);
}

void cltMini_Exorcist::OnTimer_DecreaseRemainTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseRemainTime();
}

void cltMini_Exorcist::OnTimer_StageClear(unsigned int /*id*/, cltMini_Exorcist* self)
{
    self->EndGame();
    g_cGameExorcistState = 6;
}

void cltMini_Exorcist::OnTimer_TimeOutOpenCardTime()
{
    g_cCardState[0] = 3;
    g_cCardState[1] = 3;
    g_cCardState[2] = 3;
}

void cltMini_Exorcist::OnTimer_TimeOutHitCardTime()
{
    g_cCardState[0] = 5;
    g_cCardState[1] = 5;
    g_cCardState[2] = 5;
}

// =========================================================================
// InitCardTime
// =========================================================================
void cltMini_Exorcist::InitCardTime(unsigned int openTimeMs, unsigned int hitTimeMs)
{
    m_openTimeMs = openTimeMs;
    m_hitTimeMs  = hitTimeMs;
}

// =========================================================================
// DecideCardViewValue
// =========================================================================
void cltMini_Exorcist::DecideCardViewValue(std::uint8_t cardIdx)
{
    if (cardIdx >= 3) return;

    auto applyView = [&]() {
        std::uint8_t v = m_cardViewValue[cardIdx];
        if (v > 4) v = 4;
        if (m_targetCard[cardIdx])
            m_cardImageIdx[cardIdx] = m_cardDarkSlots[v];
        else
            m_cardImageIdx[cardIdx] = m_cardLightSlots[v];
    };

    switch (g_cCardState[cardIdx])
    {
        case 1:
            m_cardViewValue[cardIdx] = 0;
            applyView();
            return;

        case 2:
        {
            std::uint8_t v = m_cardViewValue[cardIdx];
            if (!v)
            {
                m_pGameSoundMgr->PlaySoundA((char*)"M0010", 0, 0);
                g_cCardState[cardIdx] = 1;
            }
            else
            {
                m_cardViewValue[cardIdx] = static_cast<std::uint8_t>(v - 1);
            }
            applyView();
            return;
        }

        case 3:
        {
            std::uint8_t v = m_cardViewValue[cardIdx];
            if (v == 4)
            {
                m_pGameSoundMgr->PlaySoundA((char*)"M0010", 0, 0);
                g_cCardState[cardIdx] = 4;
            }
            else
            {
                m_cardViewValue[cardIdx] = static_cast<std::uint8_t>(v + 1);
            }
            applyView();
            return;
        }

        case 4:
            m_cardViewValue[cardIdx] = 4;
            applyView();
            return;

        case 5:
            if (cardIdx == 2)
            {
                if (m_openTimerId)
                    g_clTimerManager.ReleaseTimer(m_openTimerId);
                if (m_hitTimerId)
                    g_clTimerManager.ReleaseTimer(m_hitTimerId);
                CreateNewCard();
            }
            m_inputReceivedThisFrame = 0;
            m_inputCooldown          = 0;
            return;

        default:
            applyView();
            return;
    }
}

// =========================================================================
// KeyHit
// =========================================================================
int cltMini_Exorcist::KeyHit(std::uint16_t scancode)
{
    if (g_cCardState[0] != 4 && g_cCardState[1] != 4 && g_cCardState[2] != 4)
        return 0;

    int result;
    switch (scancode)
    {
        case 0xC8u: // UP
            if (m_cardSlots[4] < kSlotCount) m_slots[m_cardSlots[4]].active = 1;
            result = CheckSuccess(1u);
            break;
        case 0xCBu: // LEFT
            if (m_cardSlots[2] < kSlotCount) m_slots[m_cardSlots[2]].active = 1;
            result = CheckSuccess(0u);
            break;
        case 0xCDu: // RIGHT
            if (m_cardSlots[6] < kSlotCount) m_slots[m_cardSlots[6]].active = 1;
            result = CheckSuccess(2u);
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

// =========================================================================
// CheckSuccess — 依該卡片是否為目標決定加 / 扣分並噴特效
// =========================================================================
int cltMini_Exorcist::CheckSuccess(std::uint8_t cardIdx)
{
    if (cardIdx > 2u)
        return 0;
    if (g_cCardState[cardIdx] != 4)
        return 0;
    if (!m_remainingHits)
        return 0;

    CEffect_MiniGame_Fighter_String* fxStr = new CEffect_MiniGame_Fighter_String();

    const char* sndId;
    if (m_targetCard[cardIdx] == 1 && g_cCardState[cardIdx] == 4)
    {
        // 命中目標
        --m_remainingHits;
        ++m_score;

        float effY = static_cast<float>(m_cardYPos);
        float effX = static_cast<float>(m_cardX[cardIdx]) + 210.0f;
        fxStr->SetEffect(13, effX, effY);
        g_EffectManager_MiniGame.BulletAdd(fxStr);

        CEffect_MiniGame_Priest_Light* fxLight = new CEffect_MiniGame_Priest_Light();
        float lightX = static_cast<float>(m_cardX[cardIdx]) + 50.0f;
        float lightY = static_cast<float>(m_cardYPos) + 100.0f;
        fxLight->SetEffect(lightX, lightY);
        g_EffectManager_MiniGame.BulletAdd(fxLight);

        sndId = "M0008";
    }
    else
    {
        // 誤擊：扣 2 分（最低 0）
        if (m_score >= 3u)
            m_score = static_cast<std::uint8_t>(m_score - 2);

        float effY = static_cast<float>(m_cardYPos);
        float effX = static_cast<float>(m_cardX[cardIdx]) + 210.0f;
        fxStr->SetEffect(12, effX, effY);
        g_EffectManager_MiniGame.BulletAdd(fxStr);

        sndId = "M0009";
    }

    m_pGameSoundMgr->PlaySoundA((char*)sndId, 0, 0);
    g_cCardState[cardIdx] = 2;
    return 1;
}

// =========================================================================
// CreateNewCard
// =========================================================================
void cltMini_Exorcist::CreateNewCard()
{
    m_remainingHits  = 0;
    m_targetCard[0]  = 0;
    m_targetCard[1]  = 0;
    m_targetCard[2]  = 0;

    std::uint8_t v4 = m_score;
    std::uint8_t targetCount;
    unsigned int t;
    if (v4 < 3u)
    {
        InitCardTime(0x3E8u, 0x3E8u);
        targetCount = 1;
    }
    else if (v4 < 13u)
    {
        InitCardTime(0x384u, 0x384u);
        targetCount = 2;
    }
    else
    {
        if (v4 >= 25u)
            t = 500;
        else
            t = 700;
        InitCardTime(t, t);
        targetCount = 3;
    }

    g_cCardState[0] = 2;
    g_cCardState[1] = 2;
    g_cCardState[2] = 2;

    for (int i = 0; i < targetCount; ++i)
    {
        m_targetCard[std::rand() % 3] = 1;
    }
    for (int i = 0; i < 3; ++i)
    {
        if (m_targetCard[i] == 1)
            ++m_remainingHits;
    }

    // open timer：openTimeMs 後將所有卡狀態切到 3 (count up)
    m_openTimerId = 0;
    m_openTimerId = g_clTimerManager.CreateTimer(
        m_openTimeMs,
        0u,
        0x64u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_TimeOutOpenCardTime),
        nullptr,
        nullptr);

    // hit timer：openTime + hitTime 後將所有卡切到 5 (miss → new card)
    m_hitTimerId = 0;
    m_hitTimerId = g_clTimerManager.CreateTimer(
        m_openTimeMs + m_hitTimeMs,
        0u,
        0x64u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Exorcist::OnTimer_TimeOutHitCardTime),
        nullptr,
        nullptr);

    m_inputReceivedThisFrame = 0;
    m_inputCooldown          = 0;

    // 重置三個方向鍵 ON 圖
    if (m_cardSlots[2] < kSlotCount) m_slots[m_cardSlots[2]].active = 0;
    if (m_cardSlots[4] < kSlotCount) m_slots[m_cardSlots[4]].active = 0;
    if (m_cardSlots[6] < kSlotCount) m_slots[m_cardSlots[6]].active = 0;
}
