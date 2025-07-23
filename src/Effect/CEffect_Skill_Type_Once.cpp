#include "Effect/CEffect_Skill_Type_Once.h"
#include "Effect/CEAManager.h"       // �ݭn�q CEAManager ����S�ļƾ�
#include "Character/ClientCharacter.h" // �ݭn ClientCharacter ���w�q
#include "global.h"                      // �Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;


// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q new)
CEffect_Skill_Type_Once::CEffect_Skill_Type_Once()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: (�b CEffectManager::FrameProcess ���Q delete)
CEffect_Skill_Type_Once::~CEffect_Skill_Type_Once()
{
}

// �����ϲ�Ķ�X: (�b CEffectManager::AddEffect ���Q�I�s)
void CEffect_Skill_Type_Once::SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName)
{
    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A�åB���`��

    // �j�w�֦���
    m_pOwnerCharacter = pCaster;
}

// �����ϲ�Ķ�X: (�b CEffectManager::FrameProcess ���Q�I�s)
bool CEffect_Skill_Type_Once::FrameProcess(float fElapsedTime)
{
    // �����N�ͩR�g���޲z�����ȩe�U�������� CCAEffect ����C
    // �� CCAEffect ���ʵe���񧹲��ɡA���禡�|�^�� true�A
    // CEffectManager �H��|�N���S�Ĺ�ҾP���C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: (�b CEffectManager::Process ���Q�I�s)
void CEffect_Skill_Type_Once::Process()
{
    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // �q�֦��������e��m
    float ownerX = static_cast<float>(m_pOwnerCharacter->GetPosX());
    float ownerY = static_cast<float>(m_pOwnerCharacter->GetPosY());

    // �]�w�S�Ħۨ���ø�s�y��
    // ��l�X: *((float *)this + 12) = (float)(v2[1096] - g_Game_System_Info.ScreenX);
    //         *((float *)this + 13) = (float)(v2[1097] - g_Game_System_Info.ScreenY);
    m_fCurrentPosX = ownerX;
    m_fCurrentPosY = ownerY;

    // �i����ŧP�_
    // ��h��v���y�СA�ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f); // ��l�X�u�ˬd X �b

    // �p�G�i���A�h��s���� CCAEffect �����A�óB�z��ʵe
    if (m_bIsVisible) {
        // �N�@�ɮy���ഫ���ù��y�Ы�ǻ��� CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY));
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: (�b CEffectManager::Draw ���Q�I�s)
void CEffect_Skill_Type_Once::Draw()
{
    // �u���b Process() �禡���Q�аO���i���ɤ~�i��ø�s
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}