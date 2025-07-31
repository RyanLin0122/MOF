#include "Effect/CEffect_Player_MapConqueror.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacterManager.h" // �ݭn IsMapConqueror �禡
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00531CD0
CEffect_Player_MapConqueror::CEffect_Player_MapConqueror()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �o�����O���S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00531D10
CEffect_Player_MapConqueror::~CEffect_Player_MapConqueror()
{
}

// �����ϲ�Ķ�X: 0x00531D20
void CEffect_Player_MapConqueror::SetEffect(ClientCharacter* pOwner)
{
    if (!pOwner) return;

    // ���J�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(50, "Effect/efe_ZoneConquest.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������

    // �j�w�֦���
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00531D60
bool CEffect_Player_MapConqueror::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: result = !ClientCharacterManager::IsMapConqueror(&g_ClientCharMgr, (char *)(v3 + 460));
    if (ClientCharacterManager::GetInstance()->IsMapConqueror(m_pOwnerCharacter->GetName())) {
        // �p�G���⤴�M�O�a�Ϧ���̡A�S���~��s�b
        return false;
    }
    else {
        // �_�h�A�S�ĵ���
        return true;
    }
}

// �����ϲ�Ķ�X: 0x00531DA0
void CEffect_Player_MapConqueror::Process()
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

// �����ϲ�Ķ�X: 0x00531E60
void CEffect_Player_MapConqueror::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}