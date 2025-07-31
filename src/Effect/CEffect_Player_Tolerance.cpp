#include "Effect/CEffect_Player_Tolerance.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// =======================================================================
// CEffect_Player_Tolerance (�W�h�S��)
// =======================================================================

CEffect_Player_Tolerance::CEffect_Player_Tolerance()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

CEffect_Player_Tolerance::~CEffect_Player_Tolerance()
{
}

// �����ϲ�Ķ�X: 0x005324E0
void CEffect_Player_Tolerance::SetEffect(ClientCharacter* pOwner, unsigned char toleranceType)
{
    if (!pOwner) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    switch (toleranceType) {
    case 1:
        effectID = 43;
        szFileName = "Effect/effect-mon01-Top.ea";
        break;
    case 2:
        effectID = 44;
        szFileName = "Effect/effect-mon02-Top.ea";
        break;
    case 3:
        effectID = 45;
        szFileName = "Effect/effect-mon03-Top.ea";
        break;
    default:
        return; // ��������
    }

    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����@��

    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00532550
bool CEffect_Player_Tolerance::FrameProcess(float fElapsedTime)
{
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00532560
void CEffect_Player_Tolerance::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005325F0
void CEffect_Player_Tolerance::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}


// =======================================================================
// CEffect_Player_Tolerance_Sub (���h�S��)
// =======================================================================

CEffect_Player_Tolerance_Sub::CEffect_Player_Tolerance_Sub()
    : m_pOwnerCharacter(nullptr)
{
}

CEffect_Player_Tolerance_Sub::~CEffect_Player_Tolerance_Sub()
{
}

// �����ϲ�Ķ�X: 0x00532660
void CEffect_Player_Tolerance_Sub::SetEffect(ClientCharacter* pOwner, unsigned char toleranceType)
{
    if (!pOwner) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    switch (toleranceType) {
    case 1:
        effectID = 46;
        szFileName = "Effect/effect-mon01-Bottom.ea";
        break;
    case 2:
        effectID = 47;
        szFileName = "Effect/effect-mon02-Bottom.ea";
        break;
    case 3:
        effectID = 48;
        szFileName = "Effect/effect-mon03-Bottom.ea";
        break;
    default:
        return; // ��������
    }

    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����@��

    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x005326D0
bool CEffect_Player_Tolerance_Sub::FrameProcess(float fElapsedTime)
{
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005326E0
void CEffect_Player_Tolerance_Sub::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00532770
void CEffect_Player_Tolerance_Sub::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}