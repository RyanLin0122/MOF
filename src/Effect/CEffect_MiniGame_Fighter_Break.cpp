#include "Effect/CEffect_MiniGame_Fighter_Break.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00538080
CEffect_MiniGame_Fighter_Break::CEffect_MiniGame_Fighter_Break()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 117, �ɮצW��: "Effect/MinigameBreak.ea"
    CEAManager::GetInstance()->GetEAData(117, "Effect/MinigameBreak.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00538110
CEffect_MiniGame_Fighter_Break::~CEffect_MiniGame_Fighter_Break()
{
}

// �����ϲ�Ķ�X: 0x00538120
void CEffect_MiniGame_Fighter_Break::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x00538140
bool CEffect_MiniGame_Fighter_Break::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "MinigameBreak.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00538150
void CEffect_MiniGame_Fighter_Break::Process()
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

// �����ϲ�Ķ�X: 0x00538170
void CEffect_MiniGame_Fighter_Break::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}