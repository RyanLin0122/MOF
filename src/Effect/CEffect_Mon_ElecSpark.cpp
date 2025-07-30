#include "Effect/CEffect_Mon_ElecSpark.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00530FE0
CEffect_Mon_ElecSpark::CEffect_Mon_ElecSpark()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 110, �ɮצW��: "Effect/efn_Mop_ElecSpark.ea"
    CEAManager::GetInstance()->GetEAData(110, "Effect/efn_Mop_ElecSpark.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00531080
CEffect_Mon_ElecSpark::~CEffect_Mon_ElecSpark()
{
}

// �����ϲ�Ķ�X: 0x00531090
void CEffect_Mon_ElecSpark::SetEffect(ClientCharacter* pTarget)
{
    // �j�w�֦���
    // ��l�X: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pTarget;
}

// �����ϲ�Ķ�X: 0x005310A0
bool CEffect_Mon_ElecSpark::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_Mop_ElecSpark.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005310B0
void CEffect_Mon_ElecSpark::Process()
{
    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G����l�ܾ֦��̦�m ---
    // ��l�X: v2[3] = (float)(*(_DWORD *)(v1 + 4384) - dword_A73088);
    //         v2[4] = (float)(*(_DWORD *)(*((_DWORD *)v2 + 24) + 4388) - dword_A7308C);

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenHeight);

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // �]�m���i���]���ŧP�_�b CCAEffect::Process ���������^
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x00531100
void CEffect_Mon_ElecSpark::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}