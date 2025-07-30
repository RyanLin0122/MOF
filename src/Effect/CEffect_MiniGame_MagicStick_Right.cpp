#include "Effect/CEffect_MiniGame_MagicStick_Right.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00538910
CEffect_MiniGame_MagicStick_Right::CEffect_MiniGame_MagicStick_Right()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 119, �ɮצW��: "Effect/efn_Minigame_MagicRight.ea"
    CEAManager::GetInstance()->GetEAData(119, "Effect/efn_Minigame_MagicRight.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x005389A0
CEffect_MiniGame_MagicStick_Right::~CEffect_MiniGame_MagicStick_Right()
{
}

// �����ϲ�Ķ�X: 0x005389B0
void CEffect_MiniGame_MagicStick_Right::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x005389D0
bool CEffect_MiniGame_MagicStick_Right::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_Minigame_MagicRight.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005389E0
void CEffect_MiniGame_MagicStick_Right::Process()
{
    // --- �֤��޿�G�����N�@�ɮy�Ч@���ù��y�� ---
    // ��l�X: *((float *)this + 12) = *((float *)this + 2);
    //         *((float *)this + 13) = *((float *)this + 3);
    float screenX = m_fCurrentPosX;
    float screenY = m_fCurrentPosY;

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // �]���O UI �S�ġA�`�O�b�i���d�򤺡A���ݭn����
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x00538A00
void CEffect_MiniGame_MagicStick_Right::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}