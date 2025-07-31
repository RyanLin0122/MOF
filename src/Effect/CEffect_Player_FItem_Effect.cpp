#include "Effect/CEffect_Player_FItem_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x005316E0
CEffect_Player_FItem_Effect::CEffect_Player_FItem_Effect()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x00531720
CEffect_Player_FItem_Effect::~CEffect_Player_FItem_Effect()
{
}

// �����ϲ�Ķ�X: 0x00531730
void CEffect_Player_FItem_Effect::SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName)
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

// �����ϲ�Ķ�X: 0x005317B0
bool CEffect_Player_FItem_Effect::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- �֤ߥͩR�g���P�_ ---
    ClientCharacter* pOwner = m_pOwnerCharacter;
    if (!pOwner) {
        return true; // �p�G�֦��̫��Ь��šA�S�ĵ���
    }

    // ����@: �ˬd����O�_�B��Y�ӳq�ίS���A
    // ��l�X: if ( *(_DWORD *)(v3 + 11528) ) result = 1;
    if (pOwner->GetSomeOtherState()) { // ���] GetSomeOtherState() Ū���첾 +11528 ���X��
        return true;
    }

    // ����G: �ˬdĲ�o���S�Ī��ɸˬO�_�٦b�˳Ƥ�
    // ��l�X: result = *(_DWORD *)(v3 + 4376) == 0;
    // ���] IsFashionItemActive() Ū���첾 +4376 ���X��
    if (!pOwner->IsFashionItemActive()) {
        return true;
    }

    // �p�G�Ҧ����󳣤������A�S���~��s�b
    return false;
}

// �����ϲ�Ķ�X: 0x005317F0
void CEffect_Player_FItem_Effect::Process()
{
    if (!m_pOwnerCharacter) {
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

// �����ϲ�Ķ�X: 0x00531890
void CEffect_Player_FItem_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}