#include "Effect/CEffect_Skill_Type_Directed_Target.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00539550
CEffect_Skill_Type_Directed_Target::CEffect_Skill_Type_Directed_Target()
    : m_pCasterCharacter(nullptr), m_pTargetCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x00539590
CEffect_Skill_Type_Directed_Target::~CEffect_Skill_Type_Directed_Target()
{
}

// �����ϲ�Ķ�X: 0x005395A0
void CEffect_Skill_Type_Directed_Target::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName)
{
    if (!pCaster || !pTarget) return;

    // �c���ɮ׸��|�ø��J�S�ļƾ�
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�I�k�̩M�ؼ�
    m_pCasterCharacter = pCaster;
    m_pTargetCharacter = pTarget;

    // �ھڬI�k�̪��¦V�]�w½��
    // ��l�X: if ( *((_DWORD *)a2 + 143) == 1 ) *((_BYTE *)this + 56) = 1;
    if (pCaster->GetActionSide() == 1) {
        m_bIsFlip = true;
    }
}

// �����ϲ�Ķ�X: 0x00539630
bool CEffect_Skill_Type_Directed_Target::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00539640
void CEffect_Skill_Type_Directed_Target::Process()
{
    if (!m_pCasterCharacter || !m_pTargetCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G����l�ܥؼЦ�m ---
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pTargetCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pTargetCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005396D0
void CEffect_Skill_Type_Directed_Target::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}