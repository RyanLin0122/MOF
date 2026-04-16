#include "MiniGame/CMedical.h"

#include <cstdlib>
#include <cstring>
#include <new>

#include "global.h"
#include "Logic/DirectInputManager.h"
#include "Sound/GameSound.h"
#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "Effect/CEffect_MiniGame_Priest_Heal.h"
#include "Effect/CEffectManager.h"

namespace {
inline float bitsToFloat(unsigned int bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}
}

// =====================================================================
// CMedical
// =====================================================================
CMedical::CMedical()
{
    // mofclient.c: m_select 預設 4 = 未選擇
    m_select = 4;

    Mini_AniInfo info[2];

    // 按鈕 0：上方
    info[0].resourceID = 0x20000032u; info[0].blockID = 2; info[0].pad = 0;
    info[1].resourceID = 0x20000032u; info[1].blockID = 3; info[1].pad = 0;
    m_buttons[0].CreateMedicalKit_Button(info, 0);

    // 按鈕 1：左側
    info[0].resourceID = 0x20000032u; info[0].blockID = 0; info[0].pad = 0;
    info[1].resourceID = 0x20000032u; info[1].blockID = 1; info[1].pad = 0;
    m_buttons[1].CreateMedicalKit_Button(info, 1);

    // 按鈕 2：下方
    info[0].resourceID = 0x20000032u; info[0].blockID = 6; info[0].pad = 0;
    info[1].resourceID = 0x20000032u; info[1].blockID = 7; info[1].pad = 0;
    m_buttons[2].CreateMedicalKit_Button(info, 2);

    // 按鈕 3：右側
    info[0].resourceID = 0x20000032u; info[0].blockID = 4; info[0].pad = 0;
    info[1].resourceID = 0x20000032u; info[1].blockID = 5; info[1].pad = 0;
    m_buttons[3].CreateMedicalKit_Button(info, 3);

    m_btnState[0] = 0;
    m_btnState[1] = 0;
    m_btnState[2] = 0;
    m_btnState[3] = 0;
}

CMedical::~CMedical() = default;

void CMedical::OpenMedical(float x, float y)
{
    for (int i = 0; i < 4; ++i)
        m_buttons[i].OpenMedicalKit(x, y);

    m_select      = 4;
    m_btnState[0] = 0;
    m_btnState[1] = 0;
    m_btnState[2] = 0;
    m_btnState[3] = 0;
}

int CMedical::GetSelectMedical()
{
    return m_select;
}

int CMedical::Process(float dt, bool inputEnabled)
{
    m_btnState[0] = m_buttons[0].Process(dt);
    m_btnState[1] = m_buttons[1].Process(dt);
    m_btnState[2] = m_buttons[2].Process(dt);
    m_btnState[3] = m_buttons[3].Process(dt);

    if (!inputEnabled)
        return 1;

    DirectInputManager* pInput = cltMoF_BaseMiniGame::m_pInputMgr;

    if (m_btnState[0] == 1 && m_btnState[1] == 1 && m_btnState[2] == 1 && m_btnState[3] == 1)
    {
        if (pInput->IsKeyDown(200) || pInput->IsJoyStickPush(1, 1))
        {
            if (m_select != 4)
                m_buttons[m_select].SetSelect(false);
            m_select = 0;
            m_buttons[0].SetSelect(true);
        }
        else if (pInput->IsKeyDown(203) || pInput->IsJoyStickPush(0, 1))
        {
            if (m_select != 4)
                m_buttons[m_select].SetSelect(false);
            m_select = 1;
            m_buttons[1].SetSelect(true);
        }
        else if (pInput->IsKeyDown(208) || pInput->IsJoyStickPush(1, 2))
        {
            if (m_select != 4)
                m_buttons[m_select].SetSelect(false);
            m_select = 2;
            m_buttons[2].SetSelect(true);
        }
        else if (pInput->IsKeyDown(205) || pInput->IsJoyStickPush(0, 2))
        {
            if (m_select != 4)
                m_buttons[m_select].SetSelect(false);
            m_select = 3;
            m_buttons[3].SetSelect(true);
        }
        else if (pInput->IsKeyDown(57) || pInput->IsJoyButtonPush(0))
        {
            for (int i = 0; i < 4; ++i)
                m_buttons[i].CloseMedicalKit();
        }
    }

    if (m_btnState[0] == 3 && m_btnState[1] == 3 && m_btnState[2] == 3 && m_btnState[3] == 3)
        return 2;
    return 0;
}

void CMedical::Render()
{
    for (int i = 0; i < 4; ++i)
        m_buttons[i].Render();
}

// =====================================================================
// CPatient
// =====================================================================
CPatient::CPatient()
    : m_state(0)
    , m_medicalKind(0)
    , m_pad32(10)
    , m_alpha(255)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_bFirstHurt(1)
    , m_bSpeekUsed(0)
{
    // m_frameSkip 的 FrameSkip 建構子會自動設定 vftable、
    // m_fAccumulatedTime=0.0f、m_fTimePerFrame=1/60 — 對齊 ground truth。
}

CPatient::~CPatient() = default;

void CPatient::InitPatient(float x, float y)
{
    m_fX             = x;
    m_fY             = y;
    m_state          = 0;
    m_medicalKind    = 0;
    m_pad32          = 10;
    m_alpha          = 255;
    m_bFirstHurt     = 1;
    m_bSpeekUsed     = 0;

    Mini_AniInfo aniArr[31];
    std::memset(aniArr, 0, sizeof(aniArr));

    // mofclient.c：v9..v47 = 0x20000033，v49..v69 = 0x20000034
    for (int i = 0; i < 20; ++i)
    {
        aniArr[i].resourceID = 0x20000033u;
    }
    for (int i = 20; i < 31; ++i)
    {
        aniArr[i].resourceID = 0x20000034u;
    }

    // blockID 表（按 stack 變數順序：v10,v12,v14,v16,v18,...）
    aniArr[0].blockID  = 0;
    aniArr[1].blockID  = 1;
    aniArr[2].blockID  = 2;
    aniArr[3].blockID  = 10;   // v16
    aniArr[4].blockID  = 11;
    aniArr[5].blockID  = 12;
    aniArr[6].blockID  = 13;
    aniArr[7].blockID  = 14;
    aniArr[8].blockID  = 15;
    aniArr[9].blockID  = 16;
    aniArr[10].blockID = 3;
    aniArr[11].blockID = 4;
    aniArr[12].blockID = 5;
    aniArr[13].blockID = 6;
    aniArr[14].blockID = 7;
    aniArr[15].blockID = 8;
    aniArr[16].blockID = 9;    // v42
    aniArr[17].blockID = 17;
    aniArr[18].blockID = 18;
    aniArr[19].blockID = 19;
    aniArr[20].blockID = 0;
    aniArr[21].blockID = 1;
    aniArr[22].blockID = 2;
    aniArr[23].blockID = 3;
    aniArr[24].blockID = 4;
    aniArr[25].blockID = 5;
    aniArr[26].blockID = 6;
    aniArr[27].blockID = 7;
    aniArr[28].blockID = 8;
    aniArr[29].blockID = 9;    // v68
    aniArr[30].blockID = 10;   // v70

    Mini_KeyInfo keyArr[15];
    std::memset(keyArr, 0, sizeof(keyArr));
    keyArr[0]  = { 0,  0  };
    keyArr[1]  = { 1,  1  };
    keyArr[2]  = { 2,  2  };
    keyArr[3]  = { 3,  3  };
    keyArr[4]  = { 4,  5  };
    keyArr[5]  = { 6,  9  };
    keyArr[6]  = { 10, 10 };
    keyArr[7]  = { 11, 12 };
    keyArr[8]  = { 13, 16 };
    keyArr[9]  = { 17, 17 };
    keyArr[10] = { 18, 19 };
    keyArr[11] = { 20, 23 };
    keyArr[12] = { 24, 24 };
    keyArr[13] = { 25, 26 };
    keyArr[14] = { 27, 30 };

    m_aniCtrl.Create_Mini_AniCtrl(aniArr, 31, keyArr, 15, 1);
    m_aniCtrl.SetPosition(x, y);
    m_aniCtrl.Play(0);
}

int CPatient::UseMedical(int medicalKind)
{
    if (m_medicalKind == medicalKind)
    {
        g_GameSoundManager.PlaySoundA(const_cast<char*>("M0025"), 0, 0);
        m_pad32       = 100;
        m_state       = 1;
        m_aniCtrl.Play(1);

        float fy = m_fY + 10.0f;
        float fx = m_fX + 53.0f;
        m_speekThx.InitPosition(fx, fy);

        CEffect_MiniGame_Priest_Heal* pEff = nullptr;
        void* mem = operator new(0x84u);
        if (mem)
        {
            pEff = new (mem) CEffect_MiniGame_Priest_Heal();
        }
        float ey = m_fY + 80.0f;
        float ex = m_fX + 53.0f;
        pEff->SetEffect(ex, ey);
        g_EffectManager_MiniGame.BulletAdd(pEff);
        return 1;
    }
    g_GameSoundManager.PlaySoundA(const_cast<char*>("M0027"), 0, 0);
    m_pad32 = 0;
    m_state = 5;
    m_aniCtrl.Play(2);
    return -1;
}

void CPatient::RecallPatient()
{
    int r1 = std::rand();
    m_state        = 2;
    m_pad32        = 10 - r1 % 3;
    m_medicalKind  = std::rand() % 4;
    m_timer.InitTimer(1);
    m_frameSkip.m_fTimePerFrame = bitsToFloat(1006632960u);   // 0.0078125
    int patientType = m_medicalKind;
    m_alpha        = 255;
    m_bFirstHurt   = 1;
    m_bSpeekUsed   = 0;
    float speekY   = m_fY + 15.0f;
    float speekX   = m_fX + 20.0f;
    m_speekMgr.SetPosition(speekX, speekY, patientType);
}

int CPatient::GetPatientState()
{
    return m_state;
}

void CPatient::Process(float dt)
{
    if (m_state == 0)
        return;

    switch (m_pad32)
    {
        case 0:
        {
            m_state = 5;
            m_aniCtrl.SetFrameRate(1);
            m_aniCtrl.Play(2);
            if (m_bFirstHurt)
            {
                g_GameSoundManager.PlaySoundA(const_cast<char*>("M0027"), 0, 0);
                g_GAMESCORE -= 2;
                m_bFirstHurt = 0;
            }
            break;
        }
        case 3:
        {
            if (!m_bSpeekUsed)
            {
                m_speekMgr.SetUsed(true);
                m_bSpeekUsed = 1;
            }
            m_state = 4;
            m_aniCtrl.SetFrameRate(5);
            int v = m_medicalKind;
            if (v == 1)
                m_aniCtrl.Play(5);
            else if (v == 0)
                m_aniCtrl.Play(8);
            else if (v == 2)
                m_aniCtrl.Play(11);
            else if (v == 3)
                m_aniCtrl.Play(14);
            break;
        }
        case 4:
        case 5:
        case 6:
        {
            m_state = 3;
            m_aniCtrl.SetFrameRate(2);
            int v = m_medicalKind;
            if (v == 1)
                m_aniCtrl.Play(4);
            else if (v == 0)
                m_aniCtrl.Play(7);
            else if (v == 2)
                m_aniCtrl.Play(10);
            else if (v == 3)
                m_aniCtrl.Play(13);
            break;
        }
        case 7:
        case 8:
        case 9:
        case 10:
        {
            m_state = 2;
            m_aniCtrl.SetFrameRate(1);
            int v = m_medicalKind;
            if (v == 1)
                m_aniCtrl.Play(3);
            else if (v == 0)
                m_aniCtrl.Play(6);
            else if (v == 2)
                m_aniCtrl.Play(9);
            else if (v == 3)
                m_aniCtrl.Play(12);
            break;
        }
        default:
            break;
    }

    if (m_state == 5 || m_state == 1)
    {
        if (m_bSpeekUsed)
            m_speekMgr.SetUsed(false);

        float accum = dt + m_frameSkip.m_fAccumulatedTime;
        bool below = accum < m_frameSkip.m_fTimePerFrame;
        m_frameSkip.m_fAccumulatedTime = accum;
        long long n = 0;
        if (!below)
        {
            n = static_cast<long long>(accum / m_frameSkip.m_fTimePerFrame);
            if (n)
                m_frameSkip.m_fAccumulatedTime = accum - static_cast<float>(static_cast<int>(n)) * m_frameSkip.m_fTimePerFrame;
        }
        int newAlpha = m_alpha - static_cast<int>(n);
        m_alpha = newAlpha;
        m_aniCtrl.SetAlpha(newAlpha);
        if (m_alpha < 0)
        {
            m_state = 0;
            m_aniCtrl.Play(0);
            m_alpha = 255;
        }
    }
    else
    {
        int delta = m_timer.GetCurrentSecondDelta();
        if (delta)
        {
            int v = m_pad32 - delta;
            m_pad32 = v;
            if (v < 0)
                m_pad32 = 0;
        }
    }

    m_aniCtrl.Process(dt);
    m_speekMgr.Process();
    m_speekThx.Process(dt);
}

void CPatient::Render()
{
    if (m_state)
    {
        m_aniCtrl.Render();
        m_speekMgr.Render();
        m_speekThx.Render();
    }
}

// =====================================================================
// CBedstead
// =====================================================================
CBedstead::CBedstead()
    : m_bedKind(0)
    , m_fX(0.0f)
    , m_fY(0.0f)
    , m_bSelected(0)
{
}

CBedstead::~CBedstead() = default;

void CBedstead::InitBedstead(int bedKind, float x, float y)
{
    m_bedKind   = bedKind;
    m_fX        = x;
    m_fY        = y;
    m_bSelected = 0;
    m_patient.InitPatient(x, y);
}

void CBedstead::PatientRecall()
{
    m_patient.RecallPatient();
}

bool CBedstead::IsPatient()
{
    int s = m_patient.GetPatientState();
    return s != 0 && s != 1 && s != 5;
}

int CBedstead::IsPatientState()
{
    return m_patient.GetPatientState();
}

void CBedstead::SelectBedstead(bool selected)
{
    m_bSelected = selected ? 1 : 0;
}

int CBedstead::HealPatient(int medicalKind)
{
    return m_patient.UseMedical(medicalKind);
}

void CBedstead::Process(float dt)
{
    m_patient.Process(dt);
}

void CBedstead::Render()
{
    m_patient.Render();
}
