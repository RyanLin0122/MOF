#include "Effect/CEffect_Player_EnchantLevel.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00531530
CEffect_Player_EnchantLevel::CEffect_Player_EnchantLevel()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �o�����O���S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00531570
CEffect_Player_EnchantLevel::~CEffect_Player_EnchantLevel()
{
}

// �����ϲ�Ķ�X: 0x00531580
void CEffect_Player_EnchantLevel::SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName)
{
    if (!pOwner) return;

    // �c���ɮ׸��|�ø��J�S�ļƾ�
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������

    // �j�w�֦���
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x005315F0
bool CEffect_Player_EnchantLevel::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: if ( v3[2421] ) return 1; ...
    if (m_pOwnerCharacter->GetActionState() != 0 || // ���A�O���ߪ��A
        m_pOwnerCharacter->IsTransformed() ||       // �B���ܨ����A
        m_pOwnerCharacter->IsHide() ||              // �B���������A
        m_pOwnerCharacter->GetSomeOtherState()) {   // �B��Y�Ө�L�S���A
        return true; // ����@�ӱ��󺡨��A�N�����S��
    }

    // �p�G�Ҧ����󳣤������A�S���~��s�b
    return false;
}

// �����ϲ�Ķ�X: 0x00531650
void CEffect_Player_EnchantLevel::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // �ھڨ���¦V�]�w½��
    m_bIsFlip = (m_pOwnerCharacter->GetActionSide() == 1);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005316D0
void CEffect_Player_EnchantLevel::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}