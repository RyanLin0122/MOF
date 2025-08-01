#include "Effect/CEffect_Skill_HealPray.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533880
CEffect_Skill_HealPray::CEffect_Skill_HealPray()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x005338C0
CEffect_Skill_HealPray::~CEffect_Skill_HealPray()
{
}

// �����ϲ�Ķ�X: 0x005338D0
void CEffect_Skill_HealPray::SetEffect(ClientCharacter* pCaster, unsigned char skillLevel)
{
    if (!pCaster) return;

    const char* szFileName = nullptr;
    int effectID = 100; // ��¦ ID

    // �ھڧޯ൥�ſ�ܤ��P���S�ĸ귽
    if (skillLevel == 1) {
        szFileName = "Effect/efn_HealPray-TopLv2.ea";
    }
    else if (skillLevel == 2) {
        szFileName = "Effect/efn_HealPray-TopLv3.ea";
    }
    else { // �w�]������ 0
        szFileName = "Effect/efn_HealPray.ea";
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�֦���
    m_pOwnerCharacter = pCaster;
}

// �����ϲ�Ķ�X: 0x00533940
bool CEffect_Skill_HealPray::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00533950
void CEffect_Skill_HealPray::Process()
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

// �����ϲ�Ķ�X: 0x005339E0
void CEffect_Skill_HealPray::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}