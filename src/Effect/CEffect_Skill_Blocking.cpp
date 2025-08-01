#include "Effect/CEffect_Skill_Blocking.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x005330E0
CEffect_Skill_Blocking::CEffect_Skill_Blocking()
    : m_pOwnerCharacter(nullptr),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 96, �ɮצW��: "Effect/efn_Weight_01.ea"
    CEAManager::GetInstance()->GetEAData(96, "Effect/efn_Weight_01.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������
}

// �����ϲ�Ķ�X: 0x00533180
CEffect_Skill_Blocking::~CEffect_Skill_Blocking()
{
}

// �����ϲ�Ķ�X: 0x00533190
void CEffect_Skill_Blocking::SetEffect(ClientCharacter* pTarget)
{
    if (!pTarget) return;

    // �j�w�֦���
    m_pOwnerCharacter = pTarget;

    // --- �֤��޿�G�x�s�a��������������m�ַ� ---
    // ��l�X: *((float *)this + 34) = (float)*((int *)a2 + 1096);
    //         *((float *)this + 35) = (float)(*((_DWORD *)a2 + 1097) - *((unsigned __int16 *)a2 + 2216) + 70);
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY() - pTarget->GetCharHeight() + 70);
}

// �����ϲ�Ķ�X: 0x005331D0
bool CEffect_Skill_Blocking::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: return (~*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 2) != 0;
    // �ˬd���⪺�S���A�X�Ф��A�N��u�L�k���ʡv���줸�O�_�Q�M��
    if (m_pOwnerCharacter->CanMove()) { // ���] CanMove() ��{�F�Ӧ줸�ˬd
        return true; // �p�G�i�H���ʤF�A�S�ĵ���
    }

    // �_�h�A�S���~��s�b
    return false;
}

// �����ϲ�Ķ�X: 0x00533200
void CEffect_Skill_Blocking::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G�ϥ��x�s����l��m ---
    // ��l�X: v1[3] = v1[25] - (double)dword_A73088;
    //         v1[4] = v1[26] - (double)dword_A7308C;
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // �]����m�T�w�A���ŧP�_�i�H²��
    m_bIsVisible = IsCliping(screenX, 0.0f);
}

// �����ϲ�Ķ�X: 0x00533230
void CEffect_Skill_Blocking::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}