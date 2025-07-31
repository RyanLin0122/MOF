#include "Effect/CEffect_Skill_Type_Sustain.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h" // ���]�Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q new)
CEffect_Skill_Type_Sustain::CEffect_Skill_Type_Sustain()
    : m_pOwnerCharacter(nullptr), m_sSustainSkillID(0)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: (�b CEffectManager::FrameProcess ���Q delete)
CEffect_Skill_Type_Sustain::~CEffect_Skill_Type_Sustain()
{
}

// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q�I�s)
void CEffect_Skill_Type_Sustain::SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName, unsigned short sustainSkillID)
{
    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    // ����ʯS�ĳq�`�O�`������
    m_ccaEffect.Play(0, true);

    // �j�w�֦��̩M�ޯ� ID
    m_pOwnerCharacter = pCaster;
    m_sSustainSkillID = sustainSkillID;
}

// �����ϲ�Ķ�X: CEffect_Skill_Type_Sustain::FrameProcess
bool CEffect_Skill_Type_Sustain::FrameProcess(float fElapsedTime)
{
    // �����A�������� CCAEffect ��s��ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�֦��̬O�_�s�b�H�Χޯબ�A�O�_�٦b
    if (m_pOwnerCharacter && m_pOwnerCharacter->GetSustainSkillState(m_sSustainSkillID)) {
        // �p�G�ޯબ�A�٦b�A�S�ĴN�~��s�b
        return false;
    }

    // �p�G�֦��̤��s�b�Χޯબ�A�w�����A�h�S�ĵ���
    return true;
}

// �����ϲ�Ķ�X: (�~�Ӧ� CEffect_Skill_Type_Once ���޿�)
void CEffect_Skill_Type_Sustain::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �ˬd����O�_�B�󤣥i���ίS���A ---
    // ��l�X: if ( v2[175] || v2[2882] )
    if (m_pOwnerCharacter->IsHide() || m_pOwnerCharacter->IsTransparent()) { // ���]���禡
        m_bIsVisible = FALSE;
        return;
    }

    // �q�֦��������e��m
    float ownerX = static_cast<float>(m_pOwnerCharacter->GetPosX());
    float ownerY = static_cast<float>(m_pOwnerCharacter->GetPosY());

    m_fCurrentPosX = ownerX;
    m_fCurrentPosY = ownerY;

    // �i����ŧP�_
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // �N�@�ɮy���ഫ���ù��y�Ы�ǻ��� CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY));
        m_ccaEffect.SetFlipX(m_pOwnerCharacter->GetActionSide() != 0); // �P�I�k�̤�V�P�B
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: (�~�Ӧ� CEffect_Skill_Type_Once ���޿�)
void CEffect_Skill_Type_Sustain::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}