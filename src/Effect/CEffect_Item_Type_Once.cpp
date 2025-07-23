#include "Effect/CEffect_Item_Type_Once.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00539200
CEffect_Item_Type_Once::CEffect_Item_Type_Once()
    : m_pOwnerCharacter(nullptr),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x00539250
CEffect_Item_Type_Once::~CEffect_Item_Type_Once()
{
}

// �����ϲ�Ķ�X: 0x00539260
void CEffect_Item_Type_Once::SetEffect(ClientCharacter* pUser, unsigned short effectKindID, char* szFileName)
{
    if (!pUser) return;

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�֦���
    m_pOwnerCharacter = pUser;

    // --- �֤��޿�G�x�s��m�ַ� ---
    // ��l�X: v5 = *((_DWORD *)this + 33); 
    //         *((_DWORD *)this + 34) = *(_DWORD *)(v5 + 4384);
    //         *((_DWORD *)this + 35) = *(_DWORD *)(v5 + 4388);
    m_fInitialPosX = static_cast<float>(pUser->GetPosX());
    m_fInitialPosY = static_cast<float>(pUser->GetPosY());

    // �ھڨ���¦V�]�w½��
    // ��l�X: *((_BYTE *)this + 56) = *((_DWORD *)a2 + 143) != 0;
    m_bIsFlip = (pUser->GetActionSide() != 0);
}

// �����ϲ�Ķ�X: 0x00539300
bool CEffect_Item_Type_Once::FrameProcess(float fElapsedTime)
{
    // �ͩR�g�������Ѥ����ʵe�M�w
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00539310
void CEffect_Item_Type_Once::Process()
{
    // �ˬd�֦��̬O�_�٦s�b�]�Ҧp�A�i��b�S�ļ�������U�u�Φ��`�^
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G�ϥ��x�s����l��m ---
    // ��l�X: v3 = (float)(*((_DWORD *)this + 34) - dword_A73088);
    //         v2 = CEffectBase::IsCliping(this, v3, 0.0);
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // �N�@�ɮy���ഫ���ù��y�Ы�ǻ��� CCAEffect
        // ��l�X: *((float *)this + 12) = (float)(*((_DWORD *)this + 34) - dword_A73088);
        //         *((float *)this + 13) = (float)(*((_DWORD *)this + 35) - dword_A7308C);
        m_ccaEffect.SetPosition(screenX, m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenHeight));
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00539390
void CEffect_Item_Type_Once::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}