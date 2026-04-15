// cltMini_Sword — 劍士小遊戲，對齊 mofclient.c 原始實作。
// 此檔案以 mofclient.c 的函式呼叫為基礎邏輯性還原，並依照本專案既有的類別
// API 適度調整參數（例如 cltTimerManager::CreateTimer 的參數順序）。

#include "MiniGame/cltMini_Sword.h"

#include <cstdio>
#include <cstring>
#include <cmath>

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Sound/GameSound.h"
#include "Logic/DirectInputManager.h"
#include "Logic/cltSystemMessage.h"
#include "Logic/cltMyCharData.h"
#include "System/cltLessonSystem.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Util/cltTimer.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEffect_MiniGame_Class_Sword.h"
#include "Effect/CEffect_MiniGame_Fighter_String.h"

// 全域狀態（對齊 mofclient.c）
extern unsigned char g_cGameSwordState;
extern unsigned int  dwFrameCnt;
extern int           MonPosY[10];

namespace {

// -------------------------------------------------------------------
// 40 個影像 slot 的初始配置（依 mofclient.c 的 v23[] 資料表還原）。
// 偏移欄位為 (dx, dy)，實際位置 = screenX + dx / screenY + dy。
// 其中 sx=WORD[305]/WORD[307]/WORD[311]/WORD[313]/WORD[315] 五組 UI 錨點
// 由 cltMoF_BaseMiniGame 在建構時計算，這裡以索引 0..4 表示應該以哪個
// 錨點為基準（若為 -1 則使用 screenX/Y）。
// -------------------------------------------------------------------
struct SlotDef
{
    unsigned int res;
    unsigned short block;
    int anchor;   // -1 = screen，0..4 = m_uiPos 的某一對
    int dx;
    int dy;
};

// mofclient.c InitMiniGameImage 的資料表。
static const SlotDef kSlotTable[40] = {
    // -- 0..7: 遊戲主背景 (res 0x22000007)
    { 0x22000007u, 0,  -1, 682, 388 },
    { 0x22000007u, 1,  -1, 626, 439 },
    { 0x22000007u, 2,  -1, 682, 439 },
    { 0x22000007u, 3,  -1, 738, 439 },
    { 0x22000007u, 4,  -1, 682, 388 },
    { 0x22000007u, 5,  -1, 626, 439 },
    { 0x22000007u, 6,  -1, 682, 439 },
    { 0x22000007u, 7,  -1, 738, 439 },
    // -- 8: 怪物圖像基底 (slot index 3994 = 8)
    { 0x22000007u, 10, -1, 360,  77 },
    // -- 9: 狀態提示 (slot index ... = 9)
    { 0x22000007u, 9,  -1, 230, 200 },
    // -- 10: 另一個大圖
    { 0x22000007u, 11, -1, 360, 339 },
    // -- 11: 同一批圖
    { 0x22000007u, 8,  -1, 490, 200 },
    // -- 12: 玩家 Effect slot A (= 4000)
    { 0x22000007u, 12, -1,  33,  57 },
    // -- 13: 玩家 Effect slot B (= 4001)
    { 0x22000007u, 13, -1,  33,  57 },
    // -- 14: 玩家 Effect slot C (= 4002)
    { 0x22000007u, 14, -1,  33,  57 },
    // -- 15: Help 視窗底圖 (= 4006)
    { 0x22000007u, 15, -1, 553,  55 },
    // -- 16: Help 視窗前景
    { 0x22000007u, 16, -1, 553,  92 },
    // -- 17: Ready 背景 (= 4003)
    { 0x22000009u, 0,  -1, 177, 300 },
    // -- 18: Ready 文字 (= 4004)
    { 0x22000009u, 1,  -1, 177, 300 },
    // -- 19: Ranking 底圖 (= 604)
    { 0x2200000Au, 12,  0,   0,   0 },
    // -- 20: Win banner (= 608)
    { 0x20000002u, 0,   1,   0,   0 },
    // -- 21: Lose banner (= 609)
    { 0x20000002u, 1,   1,   0,   0 },
    // -- 22: Hit effect (= 3996)
    { 0x0B00007Eu, 0,  -1, 100,   0 },
    // -- 23: 保留
    { 0x0B000080u, 0,  -1,   0,   0 },
    // -- 24: 反向怪物 (= 3998)
    { 0x0B000082u, 0,  -1, 100,   0 },
    // -- 25: 保留
    { 0x0B00007Fu, 0,  -1, 300,   0 },
    // -- 26: Miss effect (= 3997)
    { 0x0B00007Du, 0,  -1, 100,   0 },
    // -- 27: 保留
    { 0x0B00007Cu, 0,  -1, 300,   0 },
    // -- 28: 保留
    { 0x0B000082u, 0,  -1, 100,   0 },
    // -- 29: 保留
    { 0x0B00007Bu, 0,  -1,   0,   0 },
    // -- 30: 怪物頭頂圖示 (= 3995 / 4005)
    { 0x0B000081u, 0,  -1, 100,   0 },
    // -- 31: 目標特效 (= 3999)
    { 0x22000008u, 10, -1,   0,   0 },
    // -- 32: Degree select 底圖 (= 605)
    { 0x1000001Bu, 20,  4,   0,   0 },
    // -- 33: Help 底圖 (= 606)
    { 0x1000001Au, 0,   3,   0,   0 },
    // -- 34: ShowPoint 底圖 (= 607)
    { 0x10000020u, 0,   5,   0,   0 },
    // -- 35..39: 原始 v82 memset 為 0，保留空槽
    { 0u, 0, -1, 0, 0 },
    { 0u, 0, -1, 0, 0 },
    { 0u, 0, -1, 0, 0 },
    { 0u, 0, -1, 0, 0 },
    { 0u, 0, -1, 0, 0 },
};

} // namespace

// =========================================================================
// 建構 / 解構 / InitMiniGameImage
// =========================================================================
cltMini_Sword::cltMini_Sword()
    : cltMoF_BaseMiniGame()
    , m_bgResID(0x20000023u)
    , m_totalScore(0)
    , m_winMark(0)
    , m_difficultyBaseScore(0)
    , m_currentRoundScore(0)
    , m_finalScore(0)
    , m_displayScore(0)
    , m_bonusMultiplier(0.0f)
    , m_scoreCap(0)
    , m_point(0)
    , m_slots{}
    , m_drawNumTime()
    , m_drawNumPoint()
    , m_drawNumReady()
    , m_drawNumFinal()
    , m_topBlackBox()
    , m_midBlackBox()
    , m_botBlackBox()
    , m_byte3932(1)
    , m_gameDegree(0)
    , m_dword3936(0)
    , m_dword3944(0)
    , m_byte3948(0)
    , m_slotMonsterBase(8)
    , m_slotMonsterHead(30)
    , m_slotHitFX(22)
    , m_slotMissFX(26)
    , m_slotMonsterAlt(24)
    , m_slotTargetFX(31)
    , m_slotEffectA(12)
    , m_slotEffectB(13)
    , m_slotEffectC(14)
    , m_slotReadyBG(17)
    , m_slotReadyTime(18)
    , m_slotPointFrame(30)
    , m_slotHelpFrame(15)
    , m_slotBtnFrame(4)
    , m_curTarget(0)
    , m_nextTarget(0)
    , m_missFlash(0)
    , m_hitLocked(0)
    , m_needNewTarget(0)
    , m_monsterSpawned(0)
    , m_lastSpawnTick(0)
    , m_spawnInterval(1500)
    , m_startTick(0)
    , m_pollFrame(0)
    , m_prevState(100)
    , m_finalReady(0)
    , m_serverAck(0)
    , m_serverResult(0)
    , m_serverValid(0)
    , m_serverTimeMs(0)
    , m_exitTick(0)
    , m_pBgImage(nullptr)
    , m_targetAlphaState(0)
    , m_drawSlot604(19)
    , m_drawSlot605(32)
    , m_drawSlot606(33)
    , m_drawSlot607(34)
    , m_drawSlot608(20)
    , m_drawSlot609(21)
{
    g_cGameSwordState = 0;

    InitMiniGameImage();
    InitBtnFocus();
    InitMiniGameTime(0, 5u);

    m_showTime = 0;
}

cltMini_Sword::~cltMini_Sword() = default;

// =========================================================================
// InitMiniGameImage — 依 mofclient.c 填入 slot 資料並建立 13 個按鈕。
// =========================================================================
void cltMini_Sword::InitMiniGameImage()
{
    int v2 = m_screenX;
    int v3 = m_screenY;

    // 1) 填入 40 個 slot（active=0, 僅寫入 res/block/x/y）
    for (int i = 0; i < kSlotCount; ++i)
    {
        const SlotDef& d = kSlotTable[i];
        m_slots[i].active  = 0;
        m_slots[i].resID   = d.res;
        m_slots[i].blockID = d.block;
        if (d.anchor < 0)
        {
            m_slots[i].x = v2 + d.dx;
            m_slots[i].y = v3 + d.dy;
        }
        else
        {
            // m_uiPos 排列：305..316 → index 0..11，每組 x/y 兩欄位
            //   anchor 0 → (305, 306)
            //   anchor 1 → (307, 308)
            //   anchor 2 → (309, 310)
            //   anchor 3 → (311, 312)
            //   anchor 4 → (313, 314)
            //   anchor 5 → (315, 316)
            int ax = m_uiPos[d.anchor * 2 + 0];
            int ay = m_uiPos[d.anchor * 2 + 1];
            m_slots[i].x = ax + d.dx;
            m_slots[i].y = ay + d.dy;
        }
        m_slots[i].pImage = nullptr;
    }

    // 2) slot 名稱索引（對齊 mofclient.c 記憶體位置）
    m_slotBtnFrame    = 4;
    m_slotMonsterHead = 30;
    m_slotPointFrame  = 30;
    m_slotMonsterBase = 8;
    m_slotEffectA     = 12;
    m_slotEffectB     = 13;
    m_slotEffectC     = 14;
    m_slotHelpFrame   = 15;
    m_slotReadyBG     = 17;
    m_slotReadyTime   = 18;
    m_slotHitFX       = 22;
    m_slotMonsterAlt  = 24;
    m_slotMissFX      = 26;
    m_slotTargetFX    = 31;
    m_bgResID         = 0x20000023u;

    // mofclient.c：bytes 604-609 — Draw 使用的 6 個 priority 用途 slot 索引
    //   (Init 設為 19/20/21/32/33/34，整個 cltMini_Sword 生命週期不再變動)
    m_drawSlot604 = 19;
    m_drawSlot608 = 20;
    m_drawSlot609 = 21;
    m_drawSlot605 = 32;
    m_drawSlot606 = 33;
    m_drawSlot607 = 34;

    // 3) 13 個按鈕
    //   對齊 mofclient.c 對每個按鈕的 CreateBtn 呼叫 —— 僅以本專案的 API 重排參數。
    //   CreateBtn 最後一個參數是 initialActive（mofclient.c 直接寫入 *(_DWORD*)this），
    //   GT 的 13 個按鈕中只有 Main 選單的 Start/Ranking/Exit 與 Help/ShowPoint 傳 1，
    //   其餘（Ranking 彈窗的 Prev/Next/Back、Help/ShowPoint 關閉鈕、難度三顆、
    //   EndStage 關閉鈕）皆傳 0。
    auto make = [&](int idx, int x, int y,
                    unsigned int imageType,
                    unsigned int r1, uint16_t b1,
                    unsigned int r2, uint16_t b2,
                    unsigned int r3, uint16_t b3,
                    unsigned int r4, uint16_t b4,
                    void (*cb)(unsigned int),
                    unsigned int userData,
                    int initialActive)
    {
        m_buttons[idx].CreateBtn(x, y, imageType,
                                 r1, b1, r2, b2, r3, b3, r4, b4,
                                 cb, userData, initialActive);
    };

    auto cast = [](void (*fn)(cltMini_Sword*)) -> void(*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };
    auto cast2 = [](void (*fn)()) -> void(*)(unsigned int) {
        return reinterpret_cast<void (*)(unsigned int)>(fn);
    };

    const unsigned int self = reinterpret_cast<unsigned int>(this);

    // 主選單三顆（Start / Ranking / Exit）— GT: initialActive = 1
    make(0,  m_screenX + 37,  m_screenY + 472, 9u,
         0x2200000Au, 0, 0x2200000Au, 3, 0x2200000Au, 6,
         0x20000014u, 9, cast(&cltMini_Sword::OnBtn_Start),   self, 1);

    make(1,  m_screenX + 183, m_screenY + 472, 9u,
         0x2200000Au, 1, 0x2200000Au, 4, 0x2200000Au, 7,
         0x20000014u, 10, cast(&cltMini_Sword::OnBtn_Ranking), self, 1);

    make(2,  m_screenX + 621, m_screenY + 472, 9u,
         0x2200000Au, 2, 0x2200000Au, 5, 0x2200000Au, 8,
         0x20000014u, 11, cast2(&cltMini_Sword::OnBtn_Exit),   self, 1);

    // Ranking Pre/Next/Back — GT: initialActive = 0
    make(3,  m_uiPos[0] + 17,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 13, 0x2200000Au, 15, 0x2200000Au, 17,
         0x2200000Au, 19, cast(&cltMini_Sword::OnBtn_RankingPre),  self, 0);

    make(4,  m_uiPos[0] + 62,  m_uiPos[1] + 295, 9u,
         0x2200000Au, 14, 0x2200000Au, 16, 0x2200000Au, 18,
         0x2200000Au, 20, cast(&cltMini_Sword::OnBtn_RankingNext), self, 0);

    make(5,  m_uiPos[0] + 220, m_uiPos[1] + 295, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword::OnBtn_ExitPopUp),   self, 0);

    // Help / ShowPoint — GT: initialActive = 1
    make(6,  m_screenX + 329, m_screenY + 472, 9u,
         0x1000009Bu, 12, 0x1000009Bu, 14, 0x1000009Bu, 16,
         0x1000009Bu, 18, cast(&cltMini_Sword::OnBtn_Help),       self, 1);

    make(7,  m_screenX + 475, m_screenY + 472, 9u,
         0x1000009Bu, 13, 0x1000009Bu, 15, 0x1000009Bu, 17,
         0x1000009Bu, 19, cast(&cltMini_Sword::OnBtn_ShowPoint),  self, 1);

    // Help / ShowPoint popup 上的關閉鈕 — GT: initialActive = 0
    make(8,  m_screenX + 566, m_screenY + 513, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword::OnBtn_ExitPopUp),  self, 0);

    // 難度選擇：Easy / Normal / Hard — GT: initialActive = 0
    make(9,  m_uiPos[8] + 36,  m_uiPos[9] + 48,  9u,
         0x1000009Bu, 0, 0x1000009Bu, 3, 0x1000009Bu, 6,
         0x1000009Bu, 9, cast(&cltMini_Sword::OnBtn_DegreeEasy),   self, 0);

    make(10, m_uiPos[8] + 36,  m_uiPos[9] + 102, 9u,
         0x1000009Bu, 1, 0x1000009Bu, 4, 0x1000009Bu, 7,
         0x1000009Bu, 10, cast(&cltMini_Sword::OnBtn_DegreeNormal), self, 0);

    make(11, m_uiPos[8] + 36,  m_uiPos[9] + 156, 9u,
         0x1000009Bu, 2, 0x1000009Bu, 5, 0x1000009Bu, 8,
         0x1000009Bu, 11, cast(&cltMini_Sword::OnBtn_DegreeHard),  self, 0);

    // EndStage popup 關閉鈕 — GT: initialActive = 0
    make(12, m_uiPos[2] + 215, m_uiPos[3] + 170, 9u,
         0x2200000Au, 21, 0x2200000Au, 22, 0x2200000Au, 23,
         0x2200000Au, 24, cast(&cltMini_Sword::OnBtn_ExitPopUp),   self, 0);

    // DrawNum 初始化
    m_drawNumTime .InitDrawNum(9u, 0x22000007u, 0x11u, 0);
    m_drawNumPoint.InitDrawNum(9u, 0x22000008u, 0,     1u);
    m_drawNumReady.InitDrawNum(9u, 0x22000008u, 0,     0);
    m_drawNumReady.SetActive(0);
    m_drawNumFinal.InitDrawNum(9u, 0x22000016u, 0,     0);
    m_drawNumFinal.SetActive(0);

    // 三個 AlphaBox + 基底的半透明遮罩
    m_topBlackBox.Create(m_screenX,          m_screenY,       static_cast<unsigned short>(800), static_cast<unsigned short>(40),
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    m_midBlackBox.Create(m_screenX,          m_screenY + 40,  static_cast<unsigned short>(800), static_cast<unsigned short>(460),
                         1.0f, 0.0f, 0.0f, 0.0f, nullptr);
    m_botBlackBox.Create(m_screenX,          m_screenY + 500, static_cast<unsigned short>(800), static_cast<unsigned short>(100),
                         0.0f, 0.0f, 0.0f, 1.0f, nullptr);
    m_alphaBox.Create  (m_screenX,          m_screenY + 9,   static_cast<unsigned short>(800), static_cast<unsigned short>(500),
                         0.0f, 0.0f, 0.0f, 0.5f, nullptr);

    // mofclient.c：*((_DWORD *)this + 211) = 0（frame-local alphabox flag）
    m_drawAlphaBox = 0;
    m_bgResID = 0x20000023u;

    g_GameSoundManager.PlayMusic((char*)"MoFData/Music/bg_minigame.ogg");

    Init_Wait();
}

// =========================================================================
// 基本 getters / setters
// =========================================================================
unsigned int cltMini_Sword::GetExp()          { return m_displayScore; }
void     cltMini_Sword::SetPoint(uint16_t v)  { m_point = v; }
void     cltMini_Sword::IncreasePoint()       { ++m_point; }
uint16_t cltMini_Sword::GetPoint()            { return m_point; }

void cltMini_Sword::IncLessonPt_Sword(unsigned int v)
{
    // mofclient.c 0x5A6100：
    //   cltMyCharData::IncLessonPt_Sword(cltMoF_BaseMiniGame::m_pclMyChatData, a2);
    // GT 的加分流程是 cltMini_Sword → cltMyCharData → cltLessonSystem，
    // 不是直接呼叫 g_clLessonSystem。
    cltMyCharData::IncLessonPt_Sword(cltMoF_BaseMiniGame::m_pclMyChatData, v);
}

// =========================================================================
// Poll — 依 g_cGameSwordState 分派到對應處理
// =========================================================================
int cltMini_Sword::Poll()
{
    // mofclient.c：*((_DWORD *)this + 211) = 0 — 每幀先把 m_alphaBox 顯示
    // 旗標清為 0，後面依 state 決定是否重新設為 1。這與 base class 的
    // m_rankDrawCounter (DWORD[104]) 無關。
    m_drawAlphaBox = 0;
    ++m_pollFrame;

    bool inReadyOrLater = (g_cGameSwordState == 4 || g_cGameSwordState == 5 || g_cGameSwordState == 6);
    if (inReadyOrLater && m_pollFrame < SETTING_FRAME)
        return 0;

    m_pollFrame = 0;

    switch (g_cGameSwordState)
    {
        case 1: // SelectDegree
        case 2: // Help
        case 3: // ShowPoint
            if (m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1))
            {
                Init_Wait();
                m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
            }
            m_drawAlphaBox = 1;
            break;

        case 4:  Ready();    break;
        case 5:  Gamming();  break;
        case 6:  EndStage(); break;
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

    // Button poll + 焦點
    if (!m_focusLocked)
    {
        for (int i = 0; i < kButtonCount; ++i)
        {
            if (m_buttons[i].Poll())
            {
                for (int j = 0; j < kButtonCount; ++j)
                    if (m_buttonOrder[j] == i)
                        m_focusIdx = j;
            }
        }
    }

    if (m_prevState != g_cGameSwordState)
    {
        m_prevState = g_cGameSwordState;
        InitBtnFocus();
        m_pInputMgr->IsLMButtonUp();
        return 0;
    }

    MoveBtnFocus(g_cGameSwordState);
    m_pInputMgr->IsLMButtonUp();
    return 0;
}

// =========================================================================
// PrepareDrawing — 每一幀在 Draw 前準備所有 GameImage
// =========================================================================
void cltMini_Sword::PrepareDrawing()
{
    cltImageManager* pMgr = m_pclImageMgr;

    // 半透明底色 (m_bgResID)
    // mofclient.c：取得後存入 *((_DWORD *)this + 2)，Draw 會直接 Draw(m_pBgImage)。
    GameImage* pBg = pMgr->GetGameImage(9u, m_bgResID, 0, 1);
    m_pBgImage = pBg;
    if (pBg)
    {
        pBg->SetBlockID(0);
        pBg->m_bFlag_446 = true;
        pBg->m_bFlag_447 = true;
        pBg->m_bVertexAnimation = false;
        pBg->m_fPosX = static_cast<float>(m_screenX);
        pBg->m_fPosY = static_cast<float>(m_screenY);
    }

    m_topBlackBox.PrepareDrawing();
    m_midBlackBox.PrepareDrawing();
    m_botBlackBox.PrepareDrawing();
    // mofclient.c：*((_DWORD *)this + 211) 為本幀的 alphabox 顯示旗標
    if (m_drawAlphaBox)
        m_alphaBox.PrepareDrawing();

    // 40 個 slot
    for (int i = 0; i < kSlotCount; ++i)
    {
        ImageSlot& s = m_slots[i];
        if (s.active)
        {
            GameImage* p = pMgr->GetGameImage(9u, s.resID, 0, 1);
            s.pImage = p;
            if (p)
            {
                p->SetBlockID(s.blockID);
                p->m_bFlag_446 = true;
                p->m_bFlag_447 = true;
                p->m_bVertexAnimation = false;
                p->m_fPosX = static_cast<float>(s.x);
                p->m_fPosY = static_cast<float>(s.y);
            }
        }
    }

    // Ready 計時閃爍
    if (m_slots[m_slotReadyBG].active)
    {
        unsigned int rt = GetReadyTime();
        GameImage* p = m_slots[m_slotReadyBG].pImage;
        if (p)
        {
            p->SetAlpha(255u - 40u * rt);
            p->m_bFlag_450 = true;
            p->m_bVertexAnimation = false;
        }
    }

    // Target frame 閃爍
    // mofclient.c：使用 *((_BYTE *)this + 3966) — 獨立的 byte 狀態，每幀 +=6
    // 後直接當 alpha 寫入 target FX slot；由於是 byte 運算，會自動 wrap 0..255。
    {
        m_targetAlphaState = static_cast<uint8_t>(m_targetAlphaState + 6);
        uint8_t a = m_targetAlphaState;
        if (m_slots[m_slotTargetFX].active)
        {
            GameImage* p = m_slots[m_slotTargetFX].pImage;
            if (p)
            {
                p->SetAlpha(a);
                p->m_bFlag_450 = true;
                p->m_bVertexAnimation = false;
            }
        }
    }

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].PrepareDrawing();

    unsigned int remain = GetRemainTime();
    m_drawNumTime.PrepareDrawing(m_screenX + 750, m_screenY + 56, remain, 255);
    m_drawNumPoint.PrepareDrawing(m_screenX + 10, m_screenY + 20, GetPoint(), 255);
    m_drawNumReady.PrepareDrawing(m_screenX + 450, m_screenY + 170, GetReadyTime(), 255);
    m_drawNumFinal.PrepareDrawing(m_uiPos[4], m_uiPos[5], m_displayScore, 255);
}

// =========================================================================
// Draw
// =========================================================================
void cltMini_Sword::Draw()
{
    // mofclient.c：Draw 第一步先畫半透明底圖 (PrepareDrawing 取的那張)
    if (m_pBgImage)
        m_pBgImage->Draw();

    // mofclient.c：若 byte606 > 22 才會進 22..(byte606-1) 的迴圈；
    // byte606 在 Init 中固定為 33，所以實際上畫 slot 22..32。
    if (m_drawSlot606 > 22u)
    {
        for (int i = 22; i < static_cast<int>(m_drawSlot606); ++i)
        {
            if (m_slots[i].active && m_slots[i].pImage)
                m_slots[i].pImage->Draw();
        }
    }

    // 固定再畫前 22 格（slot 0..21）。
    for (int i = 0; i < 22; ++i)
    {
        if (m_slots[i].active && m_slots[i].pImage)
            m_slots[i].pImage->Draw();
    }

    // mofclient.c：Draw 依 *((_DWORD *)this + 211) 決定是否畫整塊半透明遮罩
    if (m_drawAlphaBox)
        m_alphaBox.Draw();

    m_topBlackBox.Draw();
    m_midBlackBox.Draw();

    m_drawNumTime.Draw();
    m_drawNumPoint.Draw();
    m_drawNumReady.Draw();
    m_drawNumFinal.Draw();

    m_botBlackBox.Draw();

    // mofclient.c：四個 priority slot — 順序為 byte605, byte604, byte606, byte607
    // （對應 slot 32 / 19 / 33 / 34）。
    auto drawSlot = [&](uint8_t idx) {
        if (m_slots[idx].active && m_slots[idx].pImage)
            m_slots[idx].pImage->Draw();
    };
    drawSlot(m_drawSlot605);
    drawSlot(m_drawSlot604);
    drawSlot(m_drawSlot606);
    drawSlot(m_drawSlot607);

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].Draw();

    if (g_cGameSwordState == 7)
    {
        if (!m_rankDrawCounter)
            DrawRanking(m_uiPos[0], m_uiPos[1], 0);
    }

    if (g_cGameSwordState == 3)
        ShowPointText();
}

// =========================================================================
// ExitGame
// =========================================================================
void cltMini_Sword::ExitGame()
{
    m_pGameSoundMgr->StopMusic();
    if (m_dword148)
        g_clTimerManager.ReleaseTimer(m_dword148);
    if (m_dword149)
        g_clTimerManager.ReleaseTimer(m_dword149);
    m_dword148 = 0;
    m_dword149 = 0;
}

// =========================================================================
// Ready — ready 倒數階段的每幀更新
// =========================================================================
void cltMini_Sword::Ready()
{
    ++dwFrameCnt;
    // 怪物頭頂 slot 的 Y 座標做「呼吸」抖動
    m_slots[m_slotMonsterHead].y += MonPosY[dwFrameCnt % 10];

    if (!GetReadyTime())
    {
        m_slots[m_slotReadyBG].active  = 0;
        m_slots[m_slotReadyTime].active = 1;
        m_slots[m_slotMonsterHead].active = 0;
        m_slots[m_slotMonsterAlt].active  = 1;
    }
}

// =========================================================================
// OnBtn_Start / Init_SelectDegree
// =========================================================================
void cltMini_Sword::OnBtn_Start(cltMini_Sword* self)
{
    self->Init_SelectDegree();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword::Init_SelectDegree()
{
    // 僅保留 9, 10, 11 三顆難度按鈕
    for (int i = 0; i < 8; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[9].SetActive(1);
    m_buttons[10].SetActive(1);
    m_buttons[11].SetActive(1);
    m_slots[m_slotBtnFrame].active = 1;
    g_cGameSwordState = 1;
}

// =========================================================================
// OnBtn_Ranking / Init_Ranking / Ranking / Init_Pre/Next
// =========================================================================
void cltMini_Sword::OnBtn_Ranking(cltMini_Sword* self)
{
    self->Init_Ranking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword::Init_Ranking()
{
    // 排名用 3 顆按鈕
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[19].active = 1;    // ranking 底板
    m_buttons[3].SetActive(1); // prev
    m_buttons[4].SetActive(1); // next
    m_buttons[5].SetActive(1); // back

    m_slots[m_slotReadyBG].active = 0;
    m_slots[20].active = 0;
    m_slots[21].active = 0;
    m_missFlash = 0;

    m_drawNumPoint.SetActive(0);
    m_drawNumFinal.SetActive(0);

    std::memset(m_myRankingText, 0, sizeof(m_myRankingText));
    m_curRankPage = 0;
    std::memset(m_ranking, 0, sizeof(m_ranking));

    RequestRanking(10u, m_curRankPage);
    m_rankDrawCounter = 1;
    g_cGameSwordState = 7;
}

void cltMini_Sword::Ranking()
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

void cltMini_Sword::Init_PreRanking()
{
    uint8_t p = m_curRankPage;
    if (p)
    {
        m_curRankPage = p - 1;
        RequestRanking(10u, static_cast<uint8_t>(p - 1));
    }
}

void cltMini_Sword::Init_NextRanking()
{
    uint8_t p = m_curRankPage;
    if (p < 0x1Du)
    {
        m_curRankPage = p + 1;
        RequestRanking(10u, static_cast<uint8_t>(p + 1));
    }
}

// =========================================================================
// 其他按鈕 callback
// =========================================================================
void cltMini_Sword::OnBtn_Exit()
{
    g_cGameSwordState = 100;
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword::OnBtn_RankingPre(cltMini_Sword* self)
{
    self->Init_PreRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword::OnBtn_RankingNext(cltMini_Sword* self)
{
    self->Init_NextRanking();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword::OnBtn_Help(cltMini_Sword* self)
{
    self->Init_Help();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword::OnBtn_ShowPoint(cltMini_Sword* self)
{
    self->Init_ShowPoint();
    m_pGameSoundMgr->PlaySoundA((char*)"J0004", 0, 0);
}

void cltMini_Sword::OnBtn_ExitPopUp(cltMini_Sword* self)
{
    self->Init_Wait();
    m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
}

void cltMini_Sword::OnBtn_DegreeEasy(cltMini_Sword* self)
{
    self->SetGameDegree(1);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Sword::OnBtn_DegreeNormal(cltMini_Sword* self)
{
    self->SetGameDegree(2);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

void cltMini_Sword::OnBtn_DegreeHard(cltMini_Sword* self)
{
    self->SetGameDegree(4);
    m_pGameSoundMgr->PlaySoundA((char*)"J0003", 0, 0);
}

// =========================================================================
// Init_Wait — 回到主選單
// =========================================================================
void cltMini_Sword::Init_Wait()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);

    m_dword148 = 0;
    m_stage = 0;
    m_slots[m_slotMonsterHead].active = 1;

    m_buttons[0].SetActive(1);
    m_buttons[0].SetBtnState(m_showTime);
    m_buttons[1].SetActive(1);
    m_buttons[2].SetActive(1);
    m_buttons[6].SetActive(1);
    m_buttons[7].SetActive(1);

    m_slots[m_slotHelpFrame].active = 0;
    m_slots[20].active = 0;
    m_slots[21].active = 0;
    m_slots[m_slotBtnFrame].active = 0;
    m_slots[m_slotEffectC].active = 0;
    m_slots[33].active = 0;
    m_slots[34].active = 0;
    m_slots[19].active = 0;
    m_missFlash = 0;

    m_drawNumFinal.SetActive(0);
    m_drawNumTime.SetActive(0);
    m_drawNumPoint.SetActive(0);

    g_cGameSwordState = 0;
}

// =========================================================================
// SetGameDegree — 依難度設置各項常數並啟動 Ready 計時
// =========================================================================
void cltMini_Sword::SetGameDegree(uint8_t degree)
{
    m_stage = degree;
    m_currentRoundScore = 30;
    // mofclient.c：*((_DWORD*)this + 520) = 0 在 switch 前先清為 0
    m_bonusMultiplier = 0.0f;

    switch (degree)
    {
        case 1:
            m_winMark             = 50;
            m_difficultyBaseScore = 20;
            m_spawnInterval       = 800;
            m_gameDegree          = 20;
            // 每 point > 20 後，額外加 (point-20)*2.0 分，上限 90
            m_scoreCap            = 90;
            m_bonusMultiplier     = 2.0f;   // 0x40000000
            m_bgResID             = 0x20000023u;
            break;
        case 2:
            m_winMark             = 100;
            m_difficultyBaseScore = 55;
            m_spawnInterval       = 700;
            m_gameDegree          = 30;
            m_scoreCap            = 180;
            m_bonusMultiplier     = 8.0f;   // 0x41000000
            m_bgResID             = 0x20000025u;
            break;
        case 4:
            m_winMark             = 200;
            m_difficultyBaseScore = 40;
            m_spawnInterval       = 600;
            m_gameDegree          = 40;
            m_scoreCap            = 360;
            m_bonusMultiplier     = 4.0f;   // 0x40800000
            m_bgResID             = 0x20000024u;
            break;
    }

    m_slots[m_slotHelpFrame].active = 1;
    m_slots[m_slotReadyBG].active = 1;
    m_slots[20].active = 0;
    m_slots[21].active = 0;
    m_missFlash = 0;

    InitMiniGameTime(0x32u, 3u);
    m_drawNumReady.SetActive(1);
    m_drawNumTime.SetActive(1);
    m_drawNumPoint.SetActive(1);

    m_dword149 = 0;
    unsigned int rt = GetReadyTime();
    m_dword149 = g_clTimerManager.CreateTimer(
        1000u * rt,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword::OnTimer_TimeOutReadyTime),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword::OnTimer_DecreaseReadyTime),
        nullptr);

    m_drawNumFinal.SetActive(0);
    m_totalScore = 0;
    // mofclient.c 348544：*((_DWORD *)this + 138) = 0 — 清 m_finalReady。
    // 早先版本誤寫成 m_rankDrawCounter = 0，會把 Init_Ranking 剛設好的
    // DWORD[104] gate 清掉，導致 state==7 的排行榜在資料還沒到前就被畫出。
    m_finalReady = 0;

    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_slots[m_slotBtnFrame].active = 0;

    SetPoint(0);
    g_cGameSwordState = 4;
}

// =========================================================================
// Init_Help / Init_ShowPoint
// =========================================================================
void cltMini_Sword::Init_Help()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[6] + 402, m_uiPos[7] + 475);
    m_slots[33].active = 1;
    m_slots[m_slotMonsterHead].active = 0;
    g_cGameSwordState = 2;
}

void cltMini_Sword::Init_ShowPoint()
{
    for (int i = 0; i < kButtonCount; ++i)
        m_buttons[i].SetActive(0);
    m_buttons[8].SetActive(1);
    m_buttons[8].SetPosition(m_uiPos[10] + 380, m_uiPos[11] + 216);
    m_slots[34].active = 1;
    g_cGameSwordState = 3;
}

// =========================================================================
// ShowPointText — 將得分表文字顯示到螢幕
// =========================================================================
void cltMini_Sword::ShowPointText()
{
    g_MoFFont.SetFont("MiniShowGetPointTitle");
    char* title = m_pDCTTextManager->GetText(3340);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 70, m_uiPos[11] + 50, 0xFFFFFFFFu, title, 0, -1, -1);

    g_MoFFont.SetFont("MiniShowGetPoint");
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 127, 0xFFFFFFFFu, "20", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 157, 0xFFFFFFFFu, "55", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 143, m_uiPos[11] + 187, 0xFFFFFFFFu, "40", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 127, 0xFFFFFFFFu, " 50 ~  90", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 157, 0xFFFFFFFFu, "100 ~ 180", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 265, m_uiPos[11] + 187, 0xFFFFFFFFu, "200 ~ 360", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 127, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 157, 0xFFFFFFFFu, "30", 1, -1, -1);
    g_MoFFont.SetTextLineA(m_uiPos[10] + 405, m_uiPos[11] + 187, 0xFFFFFFFFu, "30", 1, -1, -1);
}

// =========================================================================
// StartGame — Ready 倒數結束後呼叫
// =========================================================================
void cltMini_Sword::StartGame()
{
    m_finalReady    = 1;
    m_serverAck     = 0;
    m_serverResult  = 0;
    m_serverValid   = 0;
    m_focusEnabled  = 0;

    SetPoint(0);
    m_drawNumReady.SetActive(0);
    m_slots[m_slotReadyBG].active   = 0;
    m_slots[m_slotReadyTime].active = 0;
    m_slots[m_slotMonsterHead].active = 1;
    m_slots[m_slotMonsterAlt].active  = 0;

    // mofclient.c：*((_BYTE *)this + 3966) = -106
    m_targetAlphaState = static_cast<uint8_t>(-106);
    // mofclient.c：*((_DWORD *)this + 992) = 0 — hit lock 需在開局時重置
    m_hitLocked     = 0;
    m_needNewTarget = 0;
    m_curTarget     = 0;

    m_dword148 = 0;
    unsigned int remain = GetRemainTime();
    m_dword148 = g_clTimerManager.CreateTimer(
        1000u * remain,
        reinterpret_cast<unsigned int>(this),
        0x3E8u,
        1,
        nullptr, nullptr,
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword::OnTimer_StageClear),
        reinterpret_cast<cltTimer::TimerCallback>(&cltMini_Sword::OnTimer_DecreaseRemainTime),
        nullptr);

    m_startTick = static_cast<int>(timeGetTime());
    g_cGameSwordState = 5;
}

// =========================================================================
// Gamming — 5 狀態下的主要幀邏輯
// =========================================================================
void cltMini_Sword::Gamming()
{
    CheckGameDegree();

    if (timeGetTime() - m_lastSpawnTick > m_spawnInterval)
        m_needNewTarget = 1;

    if (m_needNewTarget)
    {
        // 四個攻擊方向中隨機挑一個（避免重複）
        int r = std::rand() % 4;
        if (m_nextTarget == r)
            r = std::rand() % 4;
        m_curTarget  = m_nextTarget;
        m_nextTarget = static_cast<uint8_t>(r);

        m_missFlash = 0;
        m_topBlackBox.SetColor(1.0f, 0.0f, 0.0f, 0.0f);

        if (m_curTarget == 2)
        {
            m_slots[m_slotMonsterAlt].active = 1;
            m_slots[m_slotMonsterHead].active = 0;
        }
        else
        {
            m_slots[m_slotMonsterAlt].active = 0;
            m_slots[m_slotMonsterHead].active = 1;
        }
        m_slots[m_slotTargetFX].active = 1;
        m_lastSpawnTick = timeGetTime();
        m_monsterSpawned = 1;
        m_hitLocked = 0;
    }

    ++dwFrameCnt;
    m_slots[m_slotMonsterHead].y += MonPosY[dwFrameCnt % 10];

    DrawEffect(0);

    if (!m_hitLocked)
    {
        if (m_pInputMgr->IsKeyDown(200) || m_pInputMgr->IsJoyStickPush(1, 1))
            KeyHit(0);
        else if (m_pInputMgr->IsKeyDown(203) || m_pInputMgr->IsJoyStickPush(0, 1))
            KeyHit(1);
        else if (m_pInputMgr->IsKeyDown(208) || m_pInputMgr->IsJoyStickPush(1, 2))
            KeyHit(2);
        else if (m_pInputMgr->IsKeyDown(205) || m_pInputMgr->IsJoyStickPush(0, 2))
            KeyHit(3);
    }
}

// =========================================================================
// Timer callbacks
// =========================================================================
void cltMini_Sword::OnTimer_DecreaseReadyTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseReadyTime();
}

void cltMini_Sword::OnTimer_TimeOutReadyTime(unsigned int /*id*/, cltMini_Sword* self)
{
    m_pInputMgr->ResetJoyStick();
    self->StartGame();
    m_pGameSoundMgr->PlaySoundA((char*)"M0002", 0, 0);
}

void cltMini_Sword::OnTimer_DecreaseRemainTime(unsigned int /*id*/, cltMoF_BaseMiniGame* self)
{
    self->DecreaseRemainTime();
}

void cltMini_Sword::OnTimer_StageClear(unsigned int /*id*/, cltMini_Sword* self)
{
    self->EndStage();
    g_cGameSwordState = 6;
}

// =========================================================================
// EndStage — 遊戲結束結算
// =========================================================================
void cltMini_Sword::EndStage()
{
    if (m_dword148) g_clTimerManager.ReleaseTimer(m_dword148);
    if (m_dword149) g_clTimerManager.ReleaseTimer(m_dword149);
    m_dword148 = 0;
    m_dword149 = 0;

    // 關閉所有 effect slot
    m_slots[m_slotTargetFX].active = 0;
    m_slots[m_slotEffectA].active = 0;
    m_slots[m_slotEffectB].active = 0;
    m_slots[m_slotEffectC].active = 0;
    m_slots[m_slotMonsterHead].active = 1;
    m_slots[m_slotMonsterAlt].active  = 0;

    InitMiniGameTime(0, 5u);
    m_topBlackBox.SetColor(1.0f, 0.0f, 0.0f, 0.0f);

    if (m_finalReady)
    {
        if (!m_serverAck)
        {
            uint16_t pt = GetPoint();
            if (!cltValidMiniGameScore::IsValidScore(10u, pt))
            {
                char buf[256];
                char* txt = m_pDCTTextManager->GetText(58092);
                wsprintfA(buf, "client : %s : %i", txt, GetPoint());
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buf, 0, 0, 0);
                Init_Wait();
                return;
            }

            int total = m_winMark;
            m_totalScore = 0;
            m_finalScore = total;
            if (m_difficultyBaseScore > GetPoint())
            {
                m_slots[21].active = 1;
                m_finalScore = m_currentRoundScore;
            }
            else
            {
                m_slots[20].active = 1;
                m_totalScore = 1;
            }

            m_serverTimeMs = timeGetTime() - static_cast<unsigned int>(m_startTick);

            if (m_totalScore)
            {
                // mofclient.c：bonus = (point - baseScore) * m_bonusMultiplier
                //   加到 m_finalScore（此時等於 m_winMark），再依 m_scoreCap 截頂
                double frac = static_cast<double>(GetPoint() - m_difficultyBaseScore);
                int bonus = static_cast<int>(frac * static_cast<double>(m_bonusMultiplier));
                m_finalScore += bonus;
                if (m_scoreCap < m_finalScore)
                    m_finalScore = m_scoreCap;
            }

            int finalValue = m_finalScore;
            int mult = GetMultipleNum();
            m_displayScore = finalValue * mult;

            SendScore(m_stage, GetPoint(), m_serverTimeMs, m_stage, finalValue);
            m_finalReady = 0;
            g_cGameSwordState = 6;
        }

        if (!m_serverAck)
        {
            m_exitTick = timeGetTime();
            m_drawNumFinal.SetActive(1);
            return;
        }
    }

    if (m_serverValid)
    {
        int res = static_cast<int>(m_serverResult);
        m_showTime = 4;
        m_serverValid = 0;
        m_focusEnabled = 1;

        if (!res)
        {
            char* txt = m_pDCTTextManager->GetText(58092);
            cltSystemMessage::SetSystemMessage(&g_clSysemMessage, txt, 0, 0, 0);
            Init_Wait();
            return;
        }

        unsigned int lessonPt = m_displayScore;
        g_clLessonSystem.TraningLessonFinished(m_dword522, m_stage, m_totalScore, &lessonPt);
        m_displayScore = lessonPt;

        if (!g_clLessonSystem.CanTraningLesson(10u))
            m_showTime = 0;

        m_buttons[12].SetActive(1);
        InitBtnFocus();
    }

    if ((m_pInputMgr->IsKeyDown(1) || m_pInputMgr->IsJoyButtonPush(1)) && m_focusEnabled)
    {
        Init_Wait();
        m_pGameSoundMgr->PlaySoundA((char*)"J0002", 0, 0);
        return;
    }

    if (timeGetTime() - m_exitTick > 0x1388u)
    {
        Init_Wait();
        return;
    }

    m_drawNumFinal.SetActive(1);
}

// =========================================================================
// KeyHit — 玩家按鍵打擊
// =========================================================================
void cltMini_Sword::KeyHit(uint8_t a2)
{
    m_hitLocked = 1;
    m_slots[m_slotMissFX].active = 0;
    m_slots[m_slotMonsterAlt].active = 0;
    m_slots[m_slotMonsterHead].active = 0;
    m_slots[m_slotHitFX].active = 0;
    m_slots[m_slotMissFX].active = 0;
    m_byte3948 = a2;

    // 設定 effect 位置（相對於主目標）
    ImageSlot& base = m_slots[m_slotMonsterBase + a2];
    m_slots[m_slotEffectA].x = base.x - 43;
    m_slots[m_slotEffectA].y = base.y + 8;
    m_slots[m_slotEffectB].x = base.x - 20;
    m_slots[m_slotEffectB].y = base.y + 8;
    m_slots[m_slotEffectC].x = base.x - 77;
    m_slots[m_slotEffectC].y = base.y + 8;

    CEffect_MiniGame_Fighter_String* pText = new CEffect_MiniGame_Fighter_String();

    if (m_curTarget == a2)
    {
        m_topBlackBox.SetColor(1.0f, 0.0f, 0.0f, 0.0f);
        IncreasePoint();

        m_slots[m_slotMonsterBase + a2].active = 0;
        m_slots[m_slotTargetFX].active = 0;
        m_slots[m_slotEffectA].active = 0;
        m_slots[m_slotEffectC].active = 0;
        m_slots[m_slotHitFX + a2].active = 1;
        m_missFlash = 0;

        CEffect_MiniGame_Class_Sword* pSword = new CEffect_MiniGame_Class_Sword();
        pSword->SetEffect(static_cast<float>(m_slots[m_slotEffectB].x),
                          static_cast<float>(m_slots[m_slotEffectB].y - 30));
        g_EffectManager_MiniGame.BulletAdd(pSword);

        pText->SetEffect(13,
                         static_cast<float>(m_slots[m_slotEffectB].x + 100),
                         static_cast<float>(m_slots[m_slotEffectB].y - 30));
        g_EffectManager_MiniGame.BulletAdd(pText);
        m_pGameSoundMgr->PlaySoundA((char*)"M0003", 0, 0);
    }
    else
    {
        if (GetPoint() > 10)
            SetPoint(GetPoint() - 1);

        char frame = 12;
        float fx = static_cast<float>(m_slots[m_slotEffectB].x + 58);
        float fy = static_cast<float>(m_slots[m_slotEffectB].y - 30);
        if (m_curTarget == 2)
        {
            m_missFlash = 1;
            frame = 14;
        }
        pText->SetEffect(frame, fx, fy);
        g_EffectManager_MiniGame.BulletAdd(pText);
        m_slots[m_slotEffectB].active = 0;
        m_slots[m_slotMissFX + a2].active = 1;
        m_pGameSoundMgr->PlaySoundA((char*)"M0004", 0, 0);
    }
}

// =========================================================================
// DrawEffect — 失誤時讓背景閃紅
// =========================================================================
void cltMini_Sword::DrawEffect(uint8_t /*slot*/)
{
    int r = std::rand() % 256;
    if (m_missFlash)
    {
        unsigned int packed = (static_cast<unsigned int>(r & 0xFFu) << 24) | 0x77FF0000u;
        float fa = static_cast<float>((packed >> 24) & 0xFFu) * (1.0f / 255.0f);
        float fr = static_cast<float>((packed >> 16) & 0xFFu) * (1.0f / 255.0f);
        float fg = static_cast<float>((packed >> 8)  & 0xFFu) * (1.0f / 255.0f);
        float fb = static_cast<float>(packed         & 0xFFu) * (1.0f / 255.0f);
        m_topBlackBox.SetColor(fr, fg, fb, fa);
    }
}

// =========================================================================
// CheckGameDegree — 隨時間流逝動態調整怪物出現間隔
// =========================================================================
void cltMini_Sword::CheckGameDegree()
{
    m_spawnInterval = static_cast<unsigned int>(m_gameDegree) - 0; // 原邏輯為空殼
    // mofclient.c：m_spawnInterval = m_winMark - m_gameDegree * ((50 - remain) / 10)
    m_spawnInterval = static_cast<unsigned int>(
        static_cast<int>(m_winMark) -
        static_cast<int>(m_gameDegree) * ((50 - static_cast<int>(GetRemainTime())) / 10));
}
