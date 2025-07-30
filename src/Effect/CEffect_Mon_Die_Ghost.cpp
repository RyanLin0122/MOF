#include "Effect/CEffect_Mon_Die_Ghost.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00531240
CEffect_Mon_Die_Ghost::CEffect_Mon_Die_Ghost()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 112, �ɮצW��: "Effect/efn-mon-dead.ea"
    CEAManager::GetInstance()->GetEAData(112, "Effect/efn-mon-dead.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x005312D0
CEffect_Mon_Die_Ghost::~CEffect_Mon_Die_Ghost()
{
}

// �����ϲ�Ķ�X: 0x005312E0
void CEffect_Mon_Die_Ghost::SetEffect(ClientCharacter* pDeadMonster)
{
    if (!pDeadMonster) {
        // �p�G�ǤJ�����Ь��šA�h���]�w��m�A�S�ĥi��b (0,0) ����Τ��i��
        m_fCurrentPosX = 0.0f;
        m_fCurrentPosY = 0.0f;
        return;
    }

    // --- �֤��޿�G�x�s��m�ַӡA�îھڴ¦V�i�氾�� ---
    // ��l�X: *((float *)this + 33) = (float)(*((_DWORD *)a2 + 1096) + (*((_DWORD *)a2 + 143) != 1 ? -15 : 15));
    //         *((float *)this + 34) = (float)*((int *)a2 + 1097);

    float offsetX = (pDeadMonster->GetActionSide() != 1) ? -15.0f : 15.0f;
    m_fCurrentPosX = static_cast<float>(pDeadMonster->GetPosX()) + offsetX;
    m_fCurrentPosY = static_cast<float>(pDeadMonster->GetPosY());
}

// �����ϲ�Ķ�X: 0x00531330
bool CEffect_Mon_Die_Ghost::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn-mon-dead.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00531340
void CEffect_Mon_Die_Ghost::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    // ��l�X: v3 = *((float *)this + 33) - (double)dword_A73088;
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    // ��l�X: v2 = CEffectBase::IsCliping(this, v3, 0.0);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        // ��l�X: *((float *)this + 12) = ...; *((float *)this + 13) = ...;
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00531390
void CEffect_Mon_Die_Ghost::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}