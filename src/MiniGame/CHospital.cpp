#include "MiniGame/CHospital.h"

#include <cstdlib>

#include "global.h"
#include "Logic/DirectInputManager.h"
#include "Sound/GameSound.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"

// =========================================================================
// CHospital — 對齊 mofclient.c 0x5B0720..0x5B1000
// =========================================================================

CHospital::CHospital()
    : m_timeRemaining(0)
    , m_state(0)
    , m_beds{}
    , m_medical()
    , m_recallMgr()
    , m_aniCtrl()
    , m_timer()
    , m_imgNumber()
    , m_selectRow(0)
    , m_selectCol(0)
    , m_difficulty(0)
{
    // mofclient.c：建構子不寫入 m_difficulty（由 InitHospital 設定 0/1/2）；
    // LOBYTE(v11)=5 是 MSVC __ehstate，非 m_difficulty。
    // 建構子歸零 g_GAMESCORE 及 m_state，並建立游標動畫 (resID 0x20000032 blocks 8/9)。
    g_GAMESCORE = 0;
    m_state     = 0;

    Mini_AniInfo aniArr[2];
    aniArr[0].resourceID = 0x20000032u;
    aniArr[0].blockID    = 8;
    aniArr[0].pad        = 0;
    aniArr[1].resourceID = 0x20000032u;
    aniArr[1].blockID    = 9;
    aniArr[1].pad        = 0;

    Mini_KeyInfo keyArr[1];
    keyArr[0].startFrame = 0;
    keyArr[0].lastFrame  = 1;

    m_aniCtrl.Create_Mini_AniCtrl(aniArr, 2, keyArr, 1, 2);
    m_aniCtrl.Play(0);

    std::srand(timeGetTime());
}

CHospital::~CHospital() = default;

void CHospital::InitHospital(int difficulty)
{
    // mofclient.c：難度 0/1/2 都把總秒數設為 50；其它值不覆寫。
    m_difficulty = difficulty;
    if (difficulty == 0 || difficulty == 1 || difficulty == 2)
        m_timeRemaining = 50;

    m_state = 1;

    // 3x3 病床擺位：第一格 (col=0,row=0) 在螢幕中央偏左上的固定座標。
    const float baseX = static_cast<float>((g_Game_System_Info.ScreenWidth  - 800) / 2) + 207.0f;
    const float baseY = static_cast<float>((g_Game_System_Info.ScreenHeight - 600) / 2) + 69.0f;

    int linearIdx = 0;
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            float bx = static_cast<float>(col) * 146.0f + baseX;
            float by = static_cast<float>(row) * 140.0f + baseY;
            m_beds[linearIdx].InitBedstead(linearIdx, bx, by);
            ++linearIdx;
        }
    }

    m_timer.InitTimer(1);
    m_selectRow = 1;
    m_selectCol = 1;
    m_beds[1 * 3 + 1].SelectBedstead(true);

    m_recallMgr.InitPatientRecallMgr(difficulty, &m_beds[0]);

    m_imgNumber.InitMini_IMG_Number(0x22000007u, 0x1Bu, 0x24u);
    float numY = static_cast<float>((g_Game_System_Info.ScreenHeight - 600) / 2) + 60.0f;
    float numX = static_cast<float>((g_Game_System_Info.ScreenWidth  - 800) / 2) + 770.0f;
    m_imgNumber.SetNumber(m_timeRemaining, numX, numY);
    m_imgNumber.Process();

    g_GAMESCORE = 0;
}

void CHospital::SelectBedstead_Move_Up()
{
    int r = m_selectRow;
    if (r - 1 >= 0 && r - 1 < 3)
    {
        m_beds[r * 3 + m_selectCol].SelectBedstead(false);
        --m_selectRow;
        m_beds[m_selectRow * 3 + m_selectCol].SelectBedstead(true);
    }
}

void CHospital::SelectBedstead_Move_Down()
{
    int r = m_selectRow;
    if (r + 1 >= 0 && r + 1 < 3)
    {
        m_beds[r * 3 + m_selectCol].SelectBedstead(false);
        ++m_selectRow;
        m_beds[m_selectRow * 3 + m_selectCol].SelectBedstead(true);
    }
}

void CHospital::SelectBedstead_Move_Left()
{
    int c = m_selectCol;
    if (c - 1 >= 0 && c - 1 < 3)
    {
        m_beds[m_selectRow * 3 + c].SelectBedstead(false);
        --m_selectCol;
        m_beds[m_selectRow * 3 + m_selectCol].SelectBedstead(true);
    }
}

void CHospital::SelectBedstead_Move_Right()
{
    int c = m_selectCol;
    if (c + 1 >= 0 && c + 1 < 3)
    {
        m_beds[m_selectRow * 3 + c].SelectBedstead(false);
        ++m_selectCol;
        m_beds[m_selectRow * 3 + m_selectCol].SelectBedstead(true);
    }
}

int CHospital::GetGameScore()
{
    return g_GAMESCORE;
}

int CHospital::Process()
{
    float dt = static_cast<float>(m_timer.GetCurrentFrameTime());

    // mofclient.c：state==0 直接結束。
    if (m_state == 0)
        return 0;

    int delta = m_timer.GetCurrentSecondDelta();
    if (delta)
    {
        m_timeRemaining -= delta;
        float numY = static_cast<float>((g_Game_System_Info.ScreenHeight - 600) / 2) + 60.0f;
        float numX = static_cast<float>((g_Game_System_Info.ScreenWidth  - 800) / 2) + 770.0f;
        m_imgNumber.SetNumber(m_timeRemaining, numX, numY);
    }
    m_imgNumber.Process();

    // 游標動畫追隨選中病床。
    CBedstead& selBed = m_beds[m_selectRow * 3 + m_selectCol];
    m_aniCtrl.SetPosition(selBed.m_fX, selBed.m_fY);
    m_aniCtrl.Process(dt);

    for (int i = 0; i < 9; ++i)
        m_beds[i].Process(dt);

    DirectInputManager* pInput = cltMoF_BaseMiniGame::m_pInputMgr;

    if (m_state == 1)
    {
        if (pInput->IsKeyDown(200) || pInput->IsJoyStickPush(1, 1))
        {
            g_GameSoundManager.PlaySoundA(const_cast<char*>("M0028"), 0, 0);
            SelectBedstead_Move_Up();
        }
        else if (pInput->IsKeyDown(208) || pInput->IsJoyStickPush(1, 2))
        {
            g_GameSoundManager.PlaySoundA(const_cast<char*>("M0028"), 0, 0);
            SelectBedstead_Move_Down();
        }
        else if (pInput->IsKeyDown(203) || pInput->IsJoyStickPush(0, 1))
        {
            g_GameSoundManager.PlaySoundA(const_cast<char*>("M0028"), 0, 0);
            SelectBedstead_Move_Left();
        }
        else if (pInput->IsKeyDown(205) || pInput->IsJoyStickPush(0, 2))
        {
            g_GameSoundManager.PlaySoundA(const_cast<char*>("M0028"), 0, 0);
            SelectBedstead_Move_Right();
        }
        else if ((pInput->IsKeyDown(57) || pInput->IsJoyButtonPush(0))
                 && m_beds[m_selectRow * 3 + m_selectCol].IsPatient())
        {
            g_GameSoundManager.PlaySoundA(const_cast<char*>("M0029"), 0, 0);
            CBedstead& cur = m_beds[m_selectRow * 3 + m_selectCol];
            float openX = cur.m_fX + 25.0f;
            float openY = cur.m_fY + 35.0f;
            m_medical.OpenMedical(openX, openY);
            m_state = 2;
        }
    }
    else if (m_state == 2)
    {
        bool inputEnabled = m_beds[m_selectRow * 3 + m_selectCol].IsPatient();
        int r = m_medical.Process(dt, inputEnabled);
        if (r)
        {
            if (r == 1)
            {
                m_state = 1;
            }
            else if (r == 2)
            {
                int kind = m_medical.GetSelectMedical();
                g_GAMESCORE += m_beds[m_selectRow * 3 + m_selectCol].HealPatient(kind);
                m_state = 1;
            }
        }
    }

    if (g_GAMESCORE < 1)
        g_GAMESCORE = 0;

    m_recallMgr.Process();

    if (m_timeRemaining > 0)
        return 0;

    m_state = 0;
    return 1;
}

void CHospital::Render()
{
    m_aniCtrl.Render();
    if (m_state)
    {
        for (int i = 0; i < 9; ++i)
            m_beds[i].Render();
        if (m_state == 2)
            m_medical.Render();
        m_imgNumber.Render();
    }
}
