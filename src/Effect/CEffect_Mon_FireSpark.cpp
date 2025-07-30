#include "Effect/CEffect_Mon_FireSpark.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00531110
CEffect_Mon_FireSpark::CEffect_Mon_FireSpark()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 109, �ɮצW��: "Effect/efn_Mop_FireSpark.ea"
    CEAManager::GetInstance()->GetEAData(109, "Effect/efn_Mop_FireSpark.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x005311B0
CEffect_Mon_FireSpark::~CEffect_Mon_FireSpark()
{
}

// �����ϲ�Ķ�X: 0x005311C0
void CEffect_Mon_FireSpark::SetEffect(ClientCharacter* pTarget)
{
    // �j�w�֦���
    // ��l�X: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pTarget;
}

// �����ϲ�Ķ�X: 0x005311D0
bool CEffect_Mon_FireSpark::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_Mop_FireSpark.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005311E0
void CEffect_Mon_FireSpark::Process()
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

// �����ϲ�Ķ�X: 0x00531230
void CEffect_Mon_FireSpark::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}