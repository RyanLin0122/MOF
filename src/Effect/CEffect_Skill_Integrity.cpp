#include "Effect/CEffect_Skill_Integrity.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533560
CEffect_Skill_Integrity::CEffect_Skill_Integrity()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 95, �ɮצW��: "Effect/efn_Integrity.ea"
    CEAManager::GetInstance()->GetEAData(95, "Effect/efn_Integrity.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������
}

// �����ϲ�Ķ�X: 0x00533600
CEffect_Skill_Integrity::~CEffect_Skill_Integrity()
{
}

// �����ϲ�Ķ�X: 0x00533610
void CEffect_Skill_Integrity::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    // �j�w�֦���
    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;

    // --- �֤��޿�G�x�s�a��������������m�ַ� ---
    // ��l�X: *((float *)this + 36) = (float)*((int *)a2 + 1096);
    //         v3 = *((_DWORD *)a2 + 1097);
    //         *((float *)this + 37) = (float)(v3 - ClientCharacter::GetCharHeight(a2) + 70);
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY() - pTarget->GetCharHeight() + 70);
}

// �����ϲ�Ķ�X: 0x00533660
bool CEffect_Skill_Integrity::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: return (*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 1) == 0;
    // �ˬd���⪺�S���A�X�Ф��A�N��uIntegrity�v���줸�O�_�Q�M��
    if (m_pOwnerCharacter->IsIntegrityActive()) { // ���] IsIntegrityActive() ��{�F�Ӧ줸�ˬd
        return false; // �p�G���b���A���A�S���~��
    }

    return true; // �_�h�A�S�ĵ���
}

// �����ϲ�Ķ�X: 0x00533690
void CEffect_Skill_Integrity::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005336F0
void CEffect_Skill_Integrity::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}