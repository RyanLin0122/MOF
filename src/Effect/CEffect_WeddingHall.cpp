#include "Effect/CEffect_WeddingHall.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00537E50
CEffect_WeddingHall::CEffect_WeddingHall()
    : m_pOwnerPlayer(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x00537E90
CEffect_WeddingHall::~CEffect_WeddingHall()
{
}

// �����ϲ�Ķ�X: 0x00537EA0
void CEffect_WeddingHall::Init()
{
    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 107, �ɮצW��: "Effect/efn_wedding_hall.ea"
    CEAManager::GetInstance()->GetEAData(107, "Effect/efn_wedding_hall.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // �B§�S�����`������
}

// �����ϲ�Ķ�X: 0x00537ED0
void CEffect_WeddingHall::SetEffect(float x, float y, ClientCharacter* pPlayer)
{
    if (!pPlayer) return;

    // ��l�ƯS�ĸ귽
    Init();

    // �x�s��m�M�j�w�����a
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_pOwnerPlayer = pPlayer;
}


// �����ϲ�Ķ�X: 0x00537F00
bool CEffect_WeddingHall::FrameProcess(float fElapsedTime)
{
    if (!m_pOwnerPlayer) {
        return true; // �p�G���A�֦��̤��s�b�A�h�S�ĥߧY����
    }

    // --- �֤ߥͩR�g���P�_ ---
    // �ˬd�֦��̪��a�� ID �O�_�� 4
    // ��l�X: if ( *(_WORD *)(*((_DWORD *)this + 33) + 592) == 4 ) ...
    if (m_pOwnerPlayer->GetMapID() == 4) { // ���] GetMapID() Ū���첾 +592 ���ݩ�
        // ���A�ǰt�A��s�ʵe�����S���~��s�b
        m_ccaEffect.FrameProcess(fElapsedTime);
        return false;
    }
    else {
        // ���A���ǰt�A�S�ĵ���
        return true;
    }
}

// �����ϲ�Ķ�X: 0x00537F30
void CEffect_WeddingHall::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00537F70
void CEffect_WeddingHall::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}