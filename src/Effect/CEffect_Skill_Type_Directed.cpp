#include "Effect/CEffect_Skill_Type_Directed.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h" // ���]�Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;


// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q new)
CEffect_Skill_Type_Directed::CEffect_Skill_Type_Directed()
    : m_pCasterCharacter(nullptr), m_pTargetCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: (�b CEffectManager::FrameProcess ���Q delete)
CEffect_Skill_Type_Directed::~CEffect_Skill_Type_Directed()
{
}

// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q�I�s)
void CEffect_Skill_Type_Directed::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, unsigned char ucDirectionFlag)
{
    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�I�k�̩M�ؼ�
    m_pCasterCharacter = pCaster;
    m_pTargetCharacter = pTarget;

    // --- ½���޿� ---
    // ��l�X: if ( a6 >= 2u ) { if ( *((_DWORD *)a2 + 143) == 1 ) *((_BYTE *)this + 56) = 1; }
    // else { *((_BYTE *)this + 56) = a6 != 0; }
    if (ucDirectionFlag >= 2) {
        // �p�G�X�� >= 2�A�h½��P�_���M��I�k�̪��¦V
        if (pCaster && pCaster->GetActionSide() == 1) { // ���] GetActionSide() ����¦V
            m_bIsFlip = true;
        }
    }
    else {
        // �_�h�A�����ھںX�ШM�w�O�_½��
        m_bIsFlip = (ucDirectionFlag != 0);
    }
}

// �����ϲ�Ķ�X: (�b CEffectManager::FrameProcess ���Q�I�s)
bool CEffect_Skill_Type_Directed::FrameProcess(float fElapsedTime)
{
    // �P CEffect_Skill_Type_Once �ۦP�A�ͩR�g�������Ѥ����ʵe�M�w
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: (�b CEffectManager::Process ���Q�I�s)
void CEffect_Skill_Type_Directed::Process()
{
    // �ˬd�ؼЬO�_�s�b
    if (!m_pTargetCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // �q�ؼ������e��m
    float targetX = static_cast<float>(m_pTargetCharacter->GetPosX());
    float targetY = static_cast<float>(m_pTargetCharacter->GetPosY());

    // �]�w�S�Ħۨ���ø�s�y��
    m_fCurrentPosX = targetX;
    m_fCurrentPosY = targetY;

    // �i����ŧP�_
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // �N�@�ɮy���ഫ���ù��y�Ы�ǻ��� CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight));
        m_ccaEffect.SetFlipX(m_bIsFlip); // �N½�બ�A�ǻ��� CCAEffect
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: (�b CEffectManager::Draw ���Q�I�s)
void CEffect_Skill_Type_Directed::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}