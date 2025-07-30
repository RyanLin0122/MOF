#include "Effect/CEffect_Pet_Base.h"
#include "Effect/CEAManager.h"
#include "Pet/cltPetObject.h" // ���]�d�����󪺩w�q�b��
#include "global.h"

// �����ϲ�Ķ�X: 0x00539090
CEffect_Pet_Base::CEffect_Pet_Base()
    : m_pOwnerPet(nullptr)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x005390D0
CEffect_Pet_Base::~CEffect_Pet_Base()
{
}

// �����ϲ�Ķ�X: 0x005390E0
void CEffect_Pet_Base::SetEffect(cltPetObject* pPetOwner, unsigned short effectKindID, char* szFileName)
{
    if (!pPetOwner) return;

    // �c�ا��㪺�ɮ׸��|
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �j�w�֦���
    m_pOwnerPet = pPetOwner;
}

// �����ϲ�Ķ�X: 0x00539150
bool CEffect_Pet_Base::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00539160
void CEffect_Pet_Base::Process()
{
    // �ˬd�֦��̬O�_�s�b
    if (!m_pOwnerPet) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- �֤��޿�G����l���d����m ---
    // ��l�X: v4 = (float)(*(_DWORD *)(v2 + 2948) - dword_A73088);

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = static_cast<float>(m_pOwnerPet->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pOwnerPet->GetPosY() - g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005391E0
void CEffect_Pet_Base::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}