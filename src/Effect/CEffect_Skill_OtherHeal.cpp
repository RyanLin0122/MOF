#include "Effect/CEffect_Skill_OtherHeal.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533B20
CEffect_Skill_OtherHeal::CEffect_Skill_OtherHeal()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00533B60
CEffect_Skill_OtherHeal::~CEffect_Skill_OtherHeal()
{
}

// �����ϲ�Ķ�X: 0x00533B70
void CEffect_Skill_OtherHeal::SetEffect(ClientCharacter* pTarget, unsigned char skillLevel)
{
    if (!pTarget) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڧޯ൥�ſ�ܤ��P���S�ĸ귽
    if (skillLevel == 1) {
        effectID = 78;
        szFileName = "Effect/efn_OtherHeal_TopLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 79;
        szFileName = "Effect/efn_OtherHeal_TopLv3.ea";
    }
    else { // �w�]������ 0
        effectID = 77;
        szFileName = "Effect/efn_OtherHeal.ea";
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�֦���
    m_pOwnerCharacter = pTarget;
}

// �����ϲ�Ķ�X: 0x00533BE0
bool CEffect_Skill_OtherHeal::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00533BF0
void CEffect_Skill_OtherHeal::Process()
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

// �����ϲ�Ķ�X: 0x00533C70
void CEffect_Skill_OtherHeal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}