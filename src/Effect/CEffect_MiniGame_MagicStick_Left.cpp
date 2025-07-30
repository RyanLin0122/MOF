#include "Effect/CEffect_MiniGame_MagicStick_Left.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00538810
CEffect_MiniGame_MagicStick_Left::CEffect_MiniGame_MagicStick_Left()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 118, �ɮצW��: "Effect/efn_Minigame_MagicLeft.ea"
    CEAManager::GetInstance()->GetEAData(118, "Effect/efn_Minigame_MagicLeft.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x005388A0
CEffect_MiniGame_MagicStick_Left::~CEffect_MiniGame_MagicStick_Left()
{
}

// �����ϲ�Ķ�X: 0x005388B0
void CEffect_MiniGame_MagicStick_Left::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x005388D0
bool CEffect_MiniGame_MagicStick_Left::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_Minigame_MagicLeft.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x005388E0
void CEffect_MiniGame_MagicStick_Left::Process()
{
    // --- �֤��޿�G�����N�@�ɮy�Ч@���ù��y�� ---
    // �o�P��L�S�Ĥ��P�A������h��v���y�� (g_Game_System_Info.ScreenX/Y)
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

// �����ϲ�Ķ�X: 0x00538900
void CEffect_MiniGame_MagicStick_Left::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}