#include "Effect/CEffect_Skill_SpeedUP.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533DD0
CEffect_Skill_SpeedUP::CEffect_Skill_SpeedUP()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 99, �ɮצW��: "Effect/efn_SpeedUp.ea"
    CEAManager::GetInstance()->GetEAData(99, "Effect/efn_SpeedUp.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00533E70
CEffect_Skill_SpeedUP::~CEffect_Skill_SpeedUP()
{
}

// �����ϲ�Ķ�X: 0x00533E80
void CEffect_Skill_SpeedUP::SetEffect(ClientCharacter* pOwner)
{
    if (!pOwner) return;

    // �j�w�֦���
    m_pOwnerCharacter = pOwner;

    // ��l�X: *((_DWORD *)a2 + 174) = 0;
    // �ߧY�]�w���⪺�@�ӺX�С]�i��O�u�ǳƥ[�t�v�^
    pOwner->SetPreparingSpeedUp(false); // ���]���禡
}

// �����ϲ�Ķ�X: 0x00533EA0
bool CEffect_Skill_SpeedUP::FrameProcess(float fElapsedTime)
{
    // --- �֤ߥͩR�g���PĲ�o�޿� ---

    // �ˬd�ʵe�O�_���񧹲�
    if (m_ccaEffect.FrameProcess(fElapsedTime)) {
        // �ʵe�w����
        if (m_pOwnerCharacter) {
            // ��l�X: *(_DWORD *)(v3 + 696) = 1;
            // �]�w���⨭�W�u�����u�[�t�v�X��
            m_pOwnerCharacter->SetCanSpeedUp(true); // ���]���禡
        }
        return true; // �S�ĵ���
    }

    // �ʵe�|�������A�S���~��s�b
    return false;
}

// �����ϲ�Ķ�X: 0x00533EE0
void CEffect_Skill_SpeedUP::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- ����l�ܾ֦��̦�m ---
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x00533F30
void CEffect_Skill_SpeedUP::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}