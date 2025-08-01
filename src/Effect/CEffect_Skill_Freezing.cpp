#include "Effect/CEffect_Skill_Freezing.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// =======================================================================
// CEffect_Skill_Freezing (�W�h�S��)
// =======================================================================

// �����ϲ�Ķ�X: 0x00533240
CEffect_Skill_Freezing::CEffect_Skill_Freezing()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    CEAManager::GetInstance()->GetEAData(97, "Effect/Efn-Pvp-Ice-Top.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�������`���A�� FrameProcess ���������
}

// �����ϲ�Ķ�X: 0x005332E0
CEffect_Skill_Freezing::~CEffect_Skill_Freezing()
{
}

// �����ϲ�Ķ�X: 0x005332F0
void CEffect_Skill_Freezing::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY());
}

// �����ϲ�Ķ�X: 0x00533320
bool CEffect_Skill_Freezing::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: return (~*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 4) != 0;
    // �ˬd���⪺�S���A�X�Ф��A�N��u�B��v���줸�O�_�Q�M��
    if (m_pOwnerCharacter->IsFreezing()) { // ���] IsFreezing() ��{�F�Ӧ줸�ˬd
        return false; // �p�G���b�B��A�S���~��
    }

    return true; // �_�h�A�S�ĵ���
}

// �����ϲ�Ķ�X: 0x00533350
void CEffect_Skill_Freezing::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005333B0
void CEffect_Skill_Freezing::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}


// =======================================================================
// CEffect_Skill_Freezing_Sub (���h�S��)
// =======================================================================

// �����ϲ�Ķ�X: 0x005333D0
CEffect_Skill_Freezing_Sub::CEffect_Skill_Freezing_Sub()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    CEAManager::GetInstance()->GetEAData(98, "Effect/Efn-Pvp-Ice-Bottom.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false);
}

// �����ϲ�Ķ�X: 0x00533470
CEffect_Skill_Freezing_Sub::~CEffect_Skill_Freezing_Sub()
{
}

// �����ϲ�Ķ�X: 0x00533480
void CEffect_Skill_Freezing_Sub::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY());
}

// �����ϲ�Ķ�X: 0x005334B0
bool CEffect_Skill_Freezing_Sub::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ (�P�W�h�S�ĬۦP) ---
    if (m_pOwnerCharacter->IsFreezing()) {
        return false;
    }

    return true;
}

// �����ϲ�Ķ�X: 0x005334E0
void CEffect_Skill_Freezing_Sub::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00533540
void CEffect_Skill_Freezing_Sub::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}