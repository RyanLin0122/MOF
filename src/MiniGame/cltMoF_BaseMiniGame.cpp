// 必須在 windows.h 之前載入 winsock2.h，否則 winsock.h 會先被 windows.h 拉進來造成重定義。
#include <winsock2.h>

#include "MiniGame/cltMoF_BaseMiniGame.h"

#include "Network/CMoFNetwork.h"
#include "global.h"

#include <cstdio>
#include <cstring>

#include "Logic/cltMyCharData.h"
#include "Image/cltImageManager.h"
#include "Logic/DirectInputManager.h"
#include "Sound/GameSound.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "System/cltLessonSystem.h"
#include "Info/cltItemKindInfo.h"

// 原始碼 (mofclient.c 0x5BE910/0x5BE920) 取用的 lesson system 位於
// cltMyCharData 物件的 +6568 偏移處：
//     cltLessonSystem::GetTraningItemKind((char*)m_pclMyChatData + 6568)
// 本還原專案中 cltMyCharData 為不含內嵌子系統的精簡 stub，因此將所有
// lesson system 操作統一放到獨立全域 g_clLessonSystem。整個專案
// (cltMini_Sword.cpp / cltMini_Sword_2.cpp 等) 都走這個全域，所以
// 語意上等同於 GT — 不存在「兩份 lesson system 不同步」的風險。
extern cltLessonSystem g_clLessonSystem;

// --- 靜態成員定義 ---
cltMyCharData*          cltMoF_BaseMiniGame::m_pclMyChatData   = nullptr;
cltImageManager*        cltMoF_BaseMiniGame::m_pclImageMgr     = nullptr;
CMoFNetwork*            cltMoF_BaseMiniGame::m_pNetwork        = nullptr;
ClientCharacterManager* cltMoF_BaseMiniGame::m_pCharMgr        = nullptr;
DirectInputManager*     cltMoF_BaseMiniGame::m_pInputMgr       = nullptr;
GameSound*              cltMoF_BaseMiniGame::m_pGameSoundMgr   = nullptr;
DCTTextManager*         cltMoF_BaseMiniGame::m_pDCTTextManager = nullptr;

cltMoF_BaseMiniGame::cltMoF_BaseMiniGame()
    : m_validScore()
    , m_score(0)
    , m_stage(0)
    , m_showTime(0)
    , m_byte50(0)
    , m_byte51(0)
    , m_ranking{}
    , m_curRankPage(0)
    , m_pad_413{}
    , m_rankDrawCounter(0)
    , m_rankCount(0)
    , m_myRankingText{}
    , m_alphaBox()
    , m_gameActive(0)
    , m_serverAck(0)
    , m_serverResult(0)
    , m_serverValid(0)
    , m_readyTime(0)
    , m_remainTime(0)
    , m_startTick(0)
    , m_elapsedTime(0)
    , m_dword148(0)
    , m_dword149(0)
    , m_endTick(0)
    , m_uiPos{}
    , m_focusIdx(0)
    , m_focusLocked(0)
    , m_focusEnabled(1)
    , m_prevMouseX(0)
    , m_prevMouseY(0)
    , m_buttonOrder{0, 1, 6, 7, 2, 11, 10, 9, 3, 4, 5, 8, 12}
    , m_dword522(0)
    , m_screenX((g_Game_System_Info.ScreenWidth - 800) / 2)
    , m_screenY((g_Game_System_Info.ScreenHeight - 600) / 2)
{
    // mofclient.c 以螢幕偏移低 16 位作為 UI 座標基準。
    const std::int16_t v3 = static_cast<std::int16_t>(m_screenX);
    const std::int16_t v4 = static_cast<std::int16_t>(m_screenY);
    m_uiPos[0]  = static_cast<std::int16_t>(v3 + 240); // WORD[305]
    m_uiPos[1]  = static_cast<std::int16_t>(v4 + 105); // WORD[306]
    m_uiPos[2]  = static_cast<std::int16_t>(v3 + 240); // WORD[307]
    m_uiPos[3]  = static_cast<std::int16_t>(v4 + 170); // WORD[308]
    m_uiPos[4]  = static_cast<std::int16_t>(v3 + 433); // WORD[309]
    m_uiPos[5]  = static_cast<std::int16_t>(v4 + 273); // WORD[310]
    m_uiPos[6]  = static_cast<std::int16_t>(v3 + 164); // WORD[311]
    m_uiPos[7]  = static_cast<std::int16_t>(v4 + 38);  // WORD[312]
    m_uiPos[8]  = static_cast<std::int16_t>(v3 + 315); // WORD[313]
    m_uiPos[9]  = static_cast<std::int16_t>(v4 + 145); // WORD[314]
    m_uiPos[10] = static_cast<std::int16_t>(v3 + 154); // WORD[315]
    m_uiPos[11] = static_cast<std::int16_t>(v4 + 170); // WORD[316]
}

cltMoF_BaseMiniGame::~cltMoF_BaseMiniGame() = default;

int cltMoF_BaseMiniGame::Poll()
{
    return 0;
}

void cltMoF_BaseMiniGame::PrepareDrawing() {}
void cltMoF_BaseMiniGame::Draw() {}
void cltMoF_BaseMiniGame::InvalidScore() {}

void cltMoF_BaseMiniGame::InitializeStaticVariable(cltMyCharData* myCharData,
                                                   cltImageManager* imageMgr,
                                                   CMoFNetwork* network,
                                                   ClientCharacterManager* charMgr,
                                                   DirectInputManager* inputMgr,
                                                   GameSound* gameSound,
                                                   DCTTextManager* textMgr)
{
    m_pclMyChatData   = myCharData;
    m_pclImageMgr     = imageMgr;
    m_pNetwork        = network;
    m_pCharMgr        = charMgr;
    m_pInputMgr       = inputMgr;
    m_pGameSoundMgr   = gameSound;
    m_pDCTTextManager = textMgr;
}

int cltMoF_BaseMiniGame::SendScore(std::uint8_t lessonType, std::uint32_t score,
                                   std::uint32_t seed, std::uint8_t hitType, std::uint32_t finishMode)
{
    std::uint16_t itemKind = GetTrainningItemKind();
    return m_pNetwork->LessonFinished(lessonType, score, seed, hitType, itemKind, finishMode);
}

void cltMoF_BaseMiniGame::SetStage(std::uint8_t stage)   { m_stage = stage; }
void cltMoF_BaseMiniGame::SetScore(std::uint32_t score)  { m_score = score; }
std::uint8_t  cltMoF_BaseMiniGame::GetStage()            { return m_stage; }
std::uint32_t cltMoF_BaseMiniGame::GetScore()            { return m_score; }

int cltMoF_BaseMiniGame::RequestRanking(std::uint8_t lessonType, std::uint8_t /*pageHint*/)
{
    // mofclient.c：pageHint 被忽略；實際送出的頁碼永遠取 this->m_curRankPage。
    m_rankCount = 0;
    return m_pNetwork->LessonRanking(lessonType, m_curRankPage);
}

void cltMoF_BaseMiniGame::SetRankingData(std::uint8_t index, char* name, std::uint8_t classKind, int score)
{
    RankingEntry& entry = m_ranking[index];
    // rank 編號：page*10 + index + 1
    wsprintfA(entry.rank, "%02i",
               static_cast<int>(index) + 10 * static_cast<int>(m_curRankPage) + 1);
    wsprintfA(entry.name, "%s", name);
    entry.score = score;

    int textId;
    switch (classKind)
    {
        case 1: textId = 3345; break;
        case 2: textId = 3346; break;
        case 4: textId = 3347; break;
        default:
            return;
    }
    const char* classText = m_pDCTTextManager->GetText(textId);
    wsprintfA(entry.classLabel, "%s", classText);
}

void cltMoF_BaseMiniGame::SetReceiveRankingData(std::uint8_t count)
{
    m_rankDrawCounter = 0;
    m_rankCount = count;
}

void cltMoF_BaseMiniGame::DrawRanking(std::uint16_t x, std::uint16_t y, std::uint8_t decimalMode)
{
    g_MoFFont.SetFont("MiniGameRank");

    int textY = static_cast<int>(static_cast<std::uint16_t>(y + 55));
    int textX = static_cast<int>(static_cast<std::uint16_t>(x + 40));

    const char* t1 = m_pDCTTextManager->GetText(3334);
    g_MoFFont.SetTextLineA(textX, textY - 20, 0xFFFFFFFFu, t1, 1, -1, -1);
    const char* t2 = m_pDCTTextManager->GetText(3335);
    g_MoFFont.SetTextLineA(textX + 77, textY - 20, 0xFFFFFFFFu, t2, 1, -1, -1);
    const char* t3 = m_pDCTTextManager->GetText(3336);
    g_MoFFont.SetTextLineA(textX + 145, textY - 20, 0xFFFFFFFFu, t3, 1, -1, -1);

    int scoreHeaderId = m_showTime ? 3670 : 3337;
    const char* t4 = m_pDCTTextManager->GetText(scoreHeaderId);
    g_MoFFont.SetTextLineA(textX + 215, textY - 20, 0xFFFFFFFFu, t4, 1, -1, -1);

    if (m_rankCount)
    {
        for (int i = 0; i < m_rankCount; ++i)
        {
            RankingEntry& e = m_ranking[i];
            g_MoFFont.SetTextLineA(textX,       textY, 0xFFFFFFFFu, e.rank,       1, -1, -1);
            g_MoFFont.SetTextLineA(textX + 75,  textY, 0xFFFFFFFFu, e.name,       1, -1, -1);
            g_MoFFont.SetTextLineA(textX + 145, textY, 0xFFFFFFFFu, e.classLabel, 1, -1, -1);

            char scoreBuf[32];
            if (decimalMode)
                wsprintfA(scoreBuf, "%i.%02i", e.score / 100, e.score % 100);
            else
                wsprintfA(scoreBuf, "%i", e.score);

            g_MoFFont.SetTextLineA(textX + 215, textY, 0xFFFFFFFFu, scoreBuf, 1, -1, -1);
            textY += 24;
        }
    }

    g_MoFFont.SetFont("MiniGameMyRank");
    g_MoFFont.SetTextLineA((g_Game_System_Info.ScreenWidth - 800) / 2 + 400,
                           (g_Game_System_Info.ScreenHeight - 600) / 2 + 520,
                           0xFFFFFFFFu, m_myRankingText, 1, -1, -1);

    g_MoFFont.SetFont("MiniGameUpdateNotice");
    const char* notice = m_pDCTTextManager->GetText(3853);
    g_MoFFont.SetTextLineA((g_Game_System_Info.ScreenWidth - 800) / 2 + 400,
                           (g_Game_System_Info.ScreenHeight - 600) / 2 + 560,
                           0xFFFFFFFFu, notice, 1, -1, -1);
}

void cltMoF_BaseMiniGame::SetMyRanking(int rank)
{
    int textId;
    int rankValue;
    cltMyCharData* name;
    // mofclient.c 0x5BE370：兩個分支都在 if/else 內呼叫 GetMyCharName，
    // 結果會跟 rankValue、textId 一起當作 _wsprintfA 的可變參數。
    if (rank == -1)
    {
        rankValue = 300;
        name = cltMyCharData::GetMyCharName(m_pclMyChatData);
        textId = 3338;
    }
    else
    {
        rankValue = rank + 1;
        name = cltMyCharData::GetMyCharName(m_pclMyChatData);
        textId = 3339;
    }
    const char* fmt = m_pDCTTextManager->GetText(textId);
    // GT：_wsprintfA((LPSTR)this + 421, fmt, v5, v6)
    //   v5 = GetMyCharName(m_pclMyChatData) 回傳指標 → 在 cltMyCharData 起點
    //        放著 NUL 結尾的角色名稱，被 %s 解讀為 C-string。
    //   v6 = rankValue。
    wsprintfA(m_myRankingText, fmt, reinterpret_cast<const char*>(name), rankValue);
}

void cltMoF_BaseMiniGame::InitMiniGameTime(unsigned int remainSec, unsigned int readySec)
{
    m_remainTime = remainSec;
    m_readyTime = readySec;
}

void cltMoF_BaseMiniGame::ResetTime()
{
    m_remainTime = 0;
    m_readyTime = 0;
}

unsigned int cltMoF_BaseMiniGame::GetRemainTime() { return m_remainTime; }
unsigned int cltMoF_BaseMiniGame::GetReadyTime()  { return m_readyTime; }

void cltMoF_BaseMiniGame::DecreaseReadyTime()
{
    unsigned int newVal = m_readyTime - 1;
    m_readyTime = newVal;
    if (newVal)
        m_pGameSoundMgr->PlaySoundA((char*)"M0001", 0, 0);
}

void cltMoF_BaseMiniGame::DecreaseRemainTime()
{
    unsigned int newVal = m_remainTime - 1;
    m_remainTime = newVal;
    if (newVal < 5)
        m_pGameSoundMgr->PlaySoundA((char*)"M0013", 0, 0);
}

void cltMoF_BaseMiniGame::InitBtnFocus()
{
    m_focusLocked = 0;
    for (int orderIdx = 0; orderIdx < kButtonCount; ++orderIdx)
    {
        cltMiniGame_Button& btn = m_buttons[m_buttonOrder[orderIdx]];
        if (btn.IsActive() && btn.GetState() != 4)
        {
            btn.SetBtnState(1);
            m_focusIdx = orderIdx;
            return;
        }
    }
    // 全部不可用時不改 m_focusIdx。
}

void cltMoF_BaseMiniGame::MoveBtnFocus(std::uint8_t gameStateFilter)
{
    // mofclient.c：State==4 或 5 時完全停用焦點移動，
    // 另外只有 m_focusEnabled 為真時才處理。
    if (gameStateFilter == 5 || gameStateFilter == 4 || !m_focusEnabled)
        return;

    // 滑鼠移動偵測：滑鼠只要移過就解鎖焦點。
    if (m_prevMouseX != m_pInputMgr->GetMouse_X()
        || m_prevMouseY != m_pInputMgr->GetMouse_Y())
    {
        m_prevMouseX = m_pInputMgr->GetMouse_X();
        m_prevMouseY = m_pInputMgr->GetMouse_Y();
        m_focusLocked = 0;
    }

    auto findBackward = [this](int start) -> int {
        for (int i = start; i >= 0; --i)
        {
            cltMiniGame_Button& btn = m_buttons[m_buttonOrder[i]];
            if (btn.IsActive() && btn.GetState() != 4)
                return i;
        }
        return -1;
    };
    auto findForward = [this](int start) -> int {
        for (int i = start; i < kButtonCount; ++i)
        {
            cltMiniGame_Button& btn = m_buttons[m_buttonOrder[i]];
            if (btn.IsActive() && btn.GetState() != 4)
                return i;
        }
        return kButtonCount;
    };

    // 左 / 上：往前找；找不到則從末端環繞回找。
    if (m_pInputMgr->IsKeyDown(203)
        || m_pInputMgr->IsKeyDown(208)
        || m_pInputMgr->IsJoyStickPush(0, 1)
        || m_pInputMgr->IsJoyStickPush(1, 2))
    {
        for (int i = 0; i < kButtonCount; ++i)
        {
            cltMiniGame_Button& btn = m_buttons[i];
            if (btn.IsActive() && btn.GetState() != 4)
                btn.SetBtnState(0);
        }

        int v = m_focusIdx - 1;
        m_focusIdx = v;
        int found = -1;
        if (v >= 0)
        {
            found = findBackward(v);
            if (found != -1)
                m_buttons[m_buttonOrder[found]].SetBtnState(1);
            if (found == -1)
            {
                found = findBackward(kButtonCount - 1);
            }
        }
        else
        {
            found = findBackward(kButtonCount - 1);
        }

        if (found != -1)
        {
            m_buttons[m_buttonOrder[found]].SetBtnState(1);
            m_focusIdx = found;
        }
        m_focusLocked = 1;
    }

    // 右 / 下：往後找；找不到則從頭環繞。
    if (m_pInputMgr->IsKeyDown(205)
        || m_pInputMgr->IsKeyDown(200)
        || m_pInputMgr->IsJoyStickPush(0, 2)
        || m_pInputMgr->IsJoyStickPush(1, 1))
    {
        for (int i = 0; i < kButtonCount; ++i)
        {
            cltMiniGame_Button& btn = m_buttons[i];
            if (btn.IsActive() && btn.GetState() != 4)
                btn.SetBtnState(0);
        }

        int v = m_focusIdx + 1;
        m_focusIdx = v;
        int found = kButtonCount;
        if (v < kButtonCount)
        {
            found = findForward(v);
            if (found != kButtonCount)
                m_buttons[m_buttonOrder[found]].SetBtnState(1);
            if (found == kButtonCount)
            {
                found = findForward(0);
            }
        }
        else
        {
            found = findForward(0);
        }

        if (found != kButtonCount)
        {
            m_buttons[m_buttonOrder[found]].SetBtnState(1);
            m_focusIdx = found;
        }
        m_focusLocked = 1;
    }

    // 確保焦點高亮，並處理 Enter / joy button 0 執行按鍵動作。
    int fi = m_focusIdx;
    if (fi < kButtonCount && fi >= 0)
    {
        cltMiniGame_Button& btn = m_buttons[m_buttonOrder[fi]];
        if (btn.IsActive() && btn.GetState() != 4)
            btn.SetBtnState(1);
        if (m_pInputMgr->IsKeyDown(28) || m_pInputMgr->IsJoyButtonPush(0))
        {
            cltMiniGame_Button& b2 = m_buttons[m_buttonOrder[m_focusIdx]];
            if (b2.IsActive() && b2.GetState() != 4)
                b2.ButtonAction();
        }
    }
}

std::uint16_t cltMoF_BaseMiniGame::GetTrainningItemKind()
{
    return g_clLessonSystem.GetTraningItemKind();
}

int cltMoF_BaseMiniGame::GetMultipleNum()
{
    std::uint16_t kind = g_clLessonSystem.GetTraningItemKind();
    stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(kind);
    if (info)
        return info->Training.m_byMultiplier;
    return 1;
}
