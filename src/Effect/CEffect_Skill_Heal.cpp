#include "Effect/CEffect_Skill_Heal.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533710
CEffect_Skill_Heal::CEffect_Skill_Heal()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00533750
CEffect_Skill_Heal::~CEffect_Skill_Heal()
{
}

// �����ϲ�Ķ�X: 0x00533760
void CEffect_Skill_Heal::SetEffect(ClientCharacter* pTarget, unsigned char skillLevel)
{
    if (!pTarget) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڧޯ൥�ſ�ܤ��P���S�ĸ귽
    if (skillLevel == 1) {
        effectID = 75;
        szFileName = "Effect/efn_healLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 76;
        szFileName = "Effect/efn_healLv3.ea";
    }
    else { // �w�]������ 0
        effectID = 74;
        szFileName = "Effect/efn_heal.ea";
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�֦���
    m_pOwnerCharacter = pTarget;
}

// �����ϲ�Ķ�X: 0x005337D0
bool CEffect_Skill_Heal::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005337E0
void CEffect_Skill_Heal::Process()
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

// �����ϲ�Ķ�X: 0x00533860
void CEffect_Skill_Heal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}