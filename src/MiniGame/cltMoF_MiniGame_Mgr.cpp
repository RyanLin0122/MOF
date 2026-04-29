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
#include "Character/cltMyCharData.h"
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
// GT 沒有顯式 destructor delete m_pMiniGame，只歸零指標。
// ---------------------------------------------------------------------------
cltMoF_MiniGame_Mgr::~cltMoF_MiniGame_Mgr()
{
}

// ---------------------------------------------------------------------------
// InitMiniGame — mofclient.c 0x5BF200
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::InitMiniGame()
{
    cltMoF_BaseMiniGame::m_pInputMgr->FreeAllKey();

    m_gameKind = cltMyCharData::GetMiniGameKind(&g_clMyCharData);
    if (!m_gameKind)
        return;

    int v3 = 0;
    char v4; // GT: possibly undefined (v16) when no lesson state==1

    while (g_clLessonSystem.GetLessonState(v3) != 1)
    {
        if (++v3 >= 4)
            goto LABEL_7;
    }
    {
        uint8_t schedule = g_clLessonSystem.GetLessonSchedule(v3);
        v4 = *reinterpret_cast<char*>(g_clLessonKindInfo.GetLessonKindInfo(schedule));
    }

LABEL_7:
    if ((v4 & 1) != 0)
    {
        switch (m_gameKind)
        {
            case 1: m_pMiniGame = new cltMini_Sword_2(); break;
            case 2: m_pMiniGame = new cltMini_Bow_2(); break;
            case 3: m_pMiniGame = new cltMini_Magic_2(); break;
            case 4: m_pMiniGame = new cltMini_Exorcist_2(); break;
            default: break;
        }
    }
    else
    {
        switch (m_gameKind)
        {
            case 1: m_pMiniGame = new cltMini_Sword(); break;
            case 2: m_pMiniGame = new cltMini_Bow(); break;
            case 3: m_pMiniGame = new cltMini_Magic(); break;
            case 4: m_pMiniGame = new cltMini_Exorcist(); break;
            default: break;
        }
    }

    CUIBase* pUI = g_UIMgr->GetUIWindow(0);
    pUI->Hide_QuestAlarm(1);
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
// GT 不 delete m_pMiniGame，只歸零指標。
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::EndMiniGame()
{
    if (m_pMiniGame)
        m_pMiniGame = nullptr;

    CUIBase* pUI = g_UIMgr->GetUIWindow(0);
    pUI->OpenQuestAlarm();
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
// ---------------------------------------------------------------------------
void cltMoF_MiniGame_Mgr::InvalidScore()
{
    if (m_pMiniGame)
        m_pMiniGame->InvalidScore();
}
