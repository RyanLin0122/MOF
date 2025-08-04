#include "Effect/CEffect_UI_SkillSet.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00532ED0
CEffect_UI_SkillSet::CEffect_UI_SkillSet()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 68, �ɮצW��: "Effect/efi_skillset.ea"
    CEAManager::GetInstance()->GetEAData(68, "Effect/efi_skillset.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00532F60
CEffect_UI_SkillSet::~CEffect_UI_SkillSet()
{
}

// �����ϲ�Ķ�X: 0x00532F70
void CEffect_UI_SkillSet::SetEffect(float x, float y)
{
    // --- �֤��޿�G�����]�w CCAEffect ���ù��y�� ---
    // ��l�X: *((float *)this + 12) = a2;
    //         *((float *)this + 13) = a3;
    m_ccaEffect.SetPosition(x, y);
}

// �����ϲ�Ķ�X: 0x00532F90
bool CEffect_UI_SkillSet::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efi_skillset.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00532FA0
void CEffect_UI_SkillSet::Process()
{
    // �����N�B�z���ȩe�U�������� CCAEffect
    // �]���y�Фw�b SetEffect ���]�w�A�L�ݦA����s
    m_ccaEffect.Process();

    // UI �S���`�O�i��
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x00532FB0
void CEffect_UI_SkillSet::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}