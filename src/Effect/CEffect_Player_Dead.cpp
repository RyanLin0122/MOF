#include "Effect/CEffect_Player_Dead.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <windows.h> // for timeGetTime


// �����ϲ�Ķ�X: 0x005313A0
CEffect_Player_Dead::CEffect_Player_Dead()
    : m_pOwnerCharacter(nullptr),
    m_bIsPK_Mode(0)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 12, �ɮצW��: "Effect/efn_dead.ea"
    CEAManager::GetInstance()->GetEAData(12, "Effect/efn_dead.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �O���Ыخɪ��ɶ��W�A�Ω�W�ɧP�_
    m_dwStartTime = timeGetTime();
}

// �����ϲ�Ķ�X: 0x00531450
CEffect_Player_Dead::~CEffect_Player_Dead()
{
}

// �����ϲ�Ķ�X: 0x00531460
void CEffect_Player_Dead::SetEffect(ClientCharacter* pDeadChar, float x, float y, int isPkMode)
{
    // �]�w�S�Ī���l�@�ɮy��
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // �j�w���⪫��MPK�Ҧ��X��
    m_pOwnerCharacter = pDeadChar;
    m_bIsPK_Mode = isPkMode;
}

// �����ϲ�Ķ�X: 0x00531490
bool CEffect_Player_Dead::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        return true; // �p�G������Х��ġA���������S��
    }

    // --- �֤ߥͩR�g���P�_ ---

    // ����@: �ˬd����O�_�w�_��
    // ��l�X: if ( *(_DWORD *)(*((_DWORD *)this + 33) + 9684) != 7 ) return 1;
    if (m_pOwnerCharacter->GetActionState() != 7) { // ���] 7 �N���`���A
        return true;
    }

    // ����G: �ˬd�O�_�W�� (30��) �B���b PK �Ҧ�
    // ��l�X: return timeGetTime() - *((_DWORD *)this + 34) > 0x7530 && !*((_DWORD *)this + 35);
    if (!m_bIsPK_Mode && (timeGetTime() - m_dwStartTime > 30000)) {
        return true;
    }

    // �p�G�H�W���󳣤������A�h�S���~��s�b
    return false;
}

// �����ϲ�Ķ�X: 0x005314E0
void CEffect_Player_Dead::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00531520
void CEffect_Player_Dead::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}