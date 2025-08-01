#include "Effect/CEffect_Skill_Take.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00533F40
CEffect_Skill_Take::CEffect_Skill_Take()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 93, �ɮצW��: "Effect/efn_TakeSkill.ea"
    CEAManager::GetInstance()->GetEAData(93, "Effect/efn_TakeSkill.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00533FE0
CEffect_Skill_Take::~CEffect_Skill_Take()
{
}

// �����ϲ�Ķ�X: 0x00533FF0
void CEffect_Skill_Take::SetEffect(ClientCharacter* pOwner)
{
    // �j�w�֦���
    // ��l�X: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00534000
bool CEffect_Skill_Take::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_TakeSkill.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00534010
void CEffect_Skill_Take::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G����l�ܾ֦��̦�m ---
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00534090
void CEffect_Skill_Take::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}