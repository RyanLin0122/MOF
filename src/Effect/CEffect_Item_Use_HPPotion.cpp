#include "Effect/CEffect_Item_Use_HPPotion.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00538F60
CEffect_Item_Use_HPPotion::CEffect_Item_Use_HPPotion()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 121, �ɮצW��: "Effect/efn_heal.ea"
    CEAManager::GetInstance()->GetEAData(121, "Effect/efn_heal.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00539000
CEffect_Item_Use_HPPotion::~CEffect_Item_Use_HPPotion()
{
}

// �����ϲ�Ķ�X: 0x00539010
void CEffect_Item_Use_HPPotion::SetEffect(ClientCharacter* pUser)
{
    // �j�w�֦���
    // ��l�X: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pUser;
}

// �����ϲ�Ķ�X: 0x00539020
bool CEffect_Item_Use_HPPotion::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_heal.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00539030
void CEffect_Item_Use_HPPotion::Process()
{
    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerCharacter) {
        // �p�G�֦��̮����]�Ҧp�U�u�^�A���e�����S��
        // ���M��l�X�S�����T�B�z�A���o�Oí�����@�k
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G����l�ܾ֦��̦�m ---
    // ��l�X: v1 = *((_DWORD *)this + 33);
    //         v2 = (float *)((char *)this + 36);
    //         v2[3] = (float)(*(_DWORD *)(v1 + 4384) - dword_A73088);
    //         v2[4] = (float)(*(_DWORD *)(*((_DWORD *)v2 + 24) + 4388) - dword_A7308C);

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenHeight);

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // �]�m���i���]���ŧP�_�b CCAEffect::Process ���������^
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x00539080
void CEffect_Item_Use_HPPotion::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}