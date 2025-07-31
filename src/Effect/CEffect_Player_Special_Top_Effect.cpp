#include "Effect/CEffect_Player_Special_Top_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00532290
CEffect_Player_Special_Top_Effect::CEffect_Player_Special_Top_Effect()
    : m_pOwnerCharacter(nullptr), m_ucEffectType(0)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x005322D0
CEffect_Player_Special_Top_Effect::~CEffect_Player_Special_Top_Effect()
{
}

// �����ϲ�Ķ�X: 0x005322E0
void CEffect_Player_Special_Top_Effect::SetEffect(ClientCharacter* pOwner, unsigned char effectType)
{
    if (!pOwner) return;

    m_ucEffectType = effectType;

    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڶǤJ�� effectType ��ܭn���J���S�ĸ귽
    if (effectType == 1) {
        effectID = 51;
        szFileName = "Effect/efn-LiveeventMc-Top.ea";
    }
    else {
        // ��l�X�� v5 ���w�q�A��ܦp�G effectType ���O 1�A�i��|�ɭP���~
        // �o�̧ڭ̰��]���@�ӹw�]�ο��~�B�z
        return;
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�����i�ण�`���A�� FrameProcess ���������

    // �j�w�֦���
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00532370
bool CEffect_Player_Special_Top_Effect::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- �֤ߥͩR�g���P�_ ---
    // ��l�X: return *((_DWORD *)this + 33) == 0;
    // �p�G�֦��̫��гQ�~���]�� nullptr�A�h�S�ĵ���
    return (m_pOwnerCharacter == nullptr);
}

// �����ϲ�Ķ�X: 0x005323B0
void CEffect_Player_Special_Top_Effect::Process()
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

// �����ϲ�Ķ�X: 0x00532470
void CEffect_Player_Special_Top_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}