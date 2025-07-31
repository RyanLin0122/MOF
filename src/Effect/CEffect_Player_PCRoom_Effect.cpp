#include "Effect/CEffect_Player_PCRoom_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00531E80
CEffect_Player_PCRoom_Effect::CEffect_Player_PCRoom_Effect()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �o�����O���S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00531EC0
CEffect_Player_PCRoom_Effect::~CEffect_Player_PCRoom_Effect()
{
}

// �����ϲ�Ķ�X: 0x00531ED0
void CEffect_Player_PCRoom_Effect::SetEffect(ClientCharacter* pOwner)
{
    if (!pOwner) return;

    // ���J�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(49, "Effect/efn_PCroom.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������

    // �j�w�֦���
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00531F10
bool CEffect_Player_PCRoom_Effect::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- �֤ߥͩR�g���P�_ ---
    ClientCharacter* pOwner = m_pOwnerCharacter;
    if (!pOwner) {
        return true; // �p�G�֦��̫��Ь��šA�S�ĵ���
    }

    // �ˬd����O�_���B�� PC Room ���A
    // ��l�X: if ( *(_DWORD *)(v3 + 11556) ) return 0;
    //         if ( *(_BYTE *)(v3 + 11560) == 1 ) return 0;
    if (pOwner->IsPCRoomUser() || pOwner->IsPCRoomPremium()) { // ���]���禡
        // �p�G�O�A�S���~��s�b
        return false;
    }

    // �_�h�A�S�ĵ���
    return true;
}

// �����ϲ�Ķ�X: 0x00531F80
void CEffect_Player_PCRoom_Effect::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // �p�G����B���������S���A�A�h�����
    if (m_pOwnerCharacter->IsHide() || m_pOwnerCharacter->GetSomeOtherState()) {
        m_bIsVisible = FALSE;
        return;
    }

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // �ھڨ���¦V�]�w½��
    m_bIsFlip = (m_pOwnerCharacter->GetActionSide() != 0);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00532040
void CEffect_Player_PCRoom_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}