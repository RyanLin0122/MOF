#include "MiniGame/cltMoF_MiniGame_Mgr.h"

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMini_Sword.h"
#include "MiniGame/cltMini_Sword_2.h"
#include "MiniGame/cltMini_Bow.h"
#include "MiniGame/cltMini_Bow_2.h"
#include "MiniGame/cltMini_Magic.h"
#include "MiniGame/cltMini_Magic_2.h"
#include "MiniGame/cltMini_Exorcist.h"
#include "MiniGame/cltMini_Exorcist_2.h"
#include "Logic/cltMyCharData.h"
#include "System/cltLessonSystem.h"
#include "Info/cltLessonKindInfo.h"
#include "Logic/DirectInputManager.h"
#include "UI/CUIManager.h"
#include "UI/CUIBase.h"
#include "global.h"

// ---------------------------------------------------------------------------
// cltMoF_MiniGame_Mgr::cltMoF_MiniGame_Mgr — mofclient.c 0x5BF1E0
// ---------------------------------------------------------------------------
cltMoF_MiniGame_Mgr::cltMoF_MiniGame_Mgr()
    : m_gameKind(0)
    , m_pMiniGame(nullptr)
{
}

// ---------------------------------------------------------------------------
// ~cltMoF_MiniGame_Mgr — mofclient.c (implicit)
// ---------------------------------------------------------------------------
cltMoF_MiniGame_Mgr::~cltMoF_MiniGame_Mgr()
{
    if (m_pMiniGame)
    {
        delete m_pMiniGame;
        m_pMiniGame = nullptr;
    }
}

// ---------------------------------------------------------------------------
// InitMiniGame — mofclient.c 0x5BF200
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::InitMiniGame()
{
    if (cltMoF_BaseMiniGame::m_pInputMgr)
        cltMoF_BaseMiniGame::m_pInputMgr->FreeAllKey();

    m_gameKind = g_clMyCharData.GetMiniGameKind();
    if (!m_gameKind)
        return;

    // 判定是否為進階課程 (lesson v4 & 1)
    int lessonSlot = 0;
    bool foundActive = false;
    char v4 = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (g_clLessonSystem.GetLessonState(i) == 1)
        {
            foundActive = true;
            lessonSlot = i;
            break;
        }
    }

    if (foundActive)
    {
        uint8_t schedule = g_clLessonSystem.GetLessonSchedule(lessonSlot);
        strLessonKindInfo* pInfo = g_clLessonKindInfo.GetLessonKindInfo(schedule);
        if (pInfo)
            v4 = *reinterpret_cast<char*>(pInfo);
    }

    cltMoF_BaseMiniGame* pGame = nullptr;

    if ((v4 & 1) != 0)
    {
        // 進階版
        switch (m_gameKind)
        {
            case 1: pGame = new cltMini_Sword_2(); break;
            case 2: pGame = new cltMini_Bow_2(); break;
            case 3: pGame = new cltMini_Magic_2(); break;
            case 4: pGame = new cltMini_Exorcist_2(); break;
            default: break;
        }
    }
    else
    {
        // 基本版
        switch (m_gameKind)
        {
            case 1: pGame = new cltMini_Sword(); break;
            case 2: pGame = new cltMini_Bow(); break;
            case 3: pGame = new cltMini_Magic(); break;
            case 4: pGame = new cltMini_Exorcist(); break;
            default: break;
        }
    }

    m_pMiniGame = pGame;

    if (g_UIMgr)
    {
        CUIBase* pUI = g_UIMgr->GetUIWindow(0);
        if (pUI)
            pUI->Hide_QuestAlarm(1);
    }
}

// ---------------------------------------------------------------------------
// Poll — mofclient.c 0x5BF450
// ---------------------------------------------------------------------------
int cltMoF_MiniGame_Mgr::Poll()
{
    if (!m_pMiniGame)
        return 0;

    int result = 0;
    if (m_pMiniGame->Poll())
    {
        result = 1;
        extern int g_stGsGameFieldArg;
        g_stGsGameFieldArg = g_clLessonSystem.IsEmptySchedule();
    }
    return result;
}

// ---------------------------------------------------------------------------
// PrepareDrawing — mofclient.c 0x5BF480
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::PrepareDrawing()
{
    if (m_pMiniGame)
        m_pMiniGame->PrepareDrawing();
}

// ---------------------------------------------------------------------------
// Draw — mofclient.c 0x5BF490
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::Draw()
{
    if (m_pMiniGame)
        m_pMiniGame->Draw();
}

// ---------------------------------------------------------------------------
// EndMiniGame — mofclient.c 0x5BF4A0
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::EndMiniGame()
{
    if (m_pMiniGame)
    {
        delete m_pMiniGame;
        m_pMiniGame = nullptr;
    }

    if (g_UIMgr)
    {
        CUIBase* pUI = g_UIMgr->GetUIWindow(0);
        if (pUI)
            pUI->OpenQuestAlarm();
    }
}

// ---------------------------------------------------------------------------
// IsActive — mofclient.c 0x5BF4D0
// ---------------------------------------------------------------------------
BOOL cltMoF_MiniGame_Mgr::IsActive()
{
    return m_pMiniGame != nullptr;
}

// ---------------------------------------------------------------------------
// SetMiniGameResult — mofclient.c 0x5BF4E0
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::SetMiniGameResult(int result, unsigned int seed)
{
    if (m_pMiniGame)
    {
        m_pMiniGame->m_serverResult = result;
        m_pMiniGame->m_dword522     = static_cast<int>(seed);
        m_pMiniGame->m_serverAck    = 1;
        m_pMiniGame->m_serverValid  = 1;
    }
}

// ---------------------------------------------------------------------------
// SetRankingData — mofclient.c 0x5BF510
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::SetRankingData(std::uint8_t index, char* name,
                                          std::uint8_t classKind, int score)
{
    if (m_pMiniGame)
        m_pMiniGame->SetRankingData(index, name, classKind, score);
}

// ---------------------------------------------------------------------------
// SetReceiveRankingData — mofclient.c 0x5BF540
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::SetReceiveRankingData(std::uint8_t count)
{
    if (m_pMiniGame)
        m_pMiniGame->SetReceiveRankingData(count);
}

// ---------------------------------------------------------------------------
// SetMyRanking — mofclient.c 0x5BF560
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::SetMyRanking(int rank)
{
    if (m_pMiniGame)
        m_pMiniGame->SetMyRanking(rank);
}

// ---------------------------------------------------------------------------
// InvalidScore — mofclient.c 0x5BF580
// GT: (*(void (__thiscall **)(int))(*(_DWORD *)v1 + 24))(v1);
// This is a virtual call at vtable offset 24 (6th slot).
// In our vtable: slot 0 = destructor, slot 1 = Poll.
// For now this is a virtual call on the mini-game object.
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::InvalidScore()
{
    // GT calls vtable+24 on the mini-game, which is a virtual function.
    // The exact vtable slot depends on the class. For now, leave as stub
    // since no derived class has a matching virtual at that slot yet.
    // TODO: 需要確認 vtable 佈局後補齊
}
