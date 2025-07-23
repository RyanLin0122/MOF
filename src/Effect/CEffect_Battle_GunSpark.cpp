#include "Effect/CEffect_Battle_GunSpark.h"
#include "Effect/CEAManager.h"
#include "global.h" // ���]�Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052E720
CEffect_Battle_GunSpark::CEffect_Battle_GunSpark()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 5, �ɮצW��: "MoFData/Effect/efn_GunSpark.ea"
    CEAManager::GetInstance()->GetEAData(5, "MoFData/Effect/efn_GunSpark.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x0052E7B0
CEffect_Battle_GunSpark::~CEffect_Battle_GunSpark()
{
}

// �����ϲ�Ķ�X: 0x0052E7C0
void CEffect_Battle_GunSpark::SetEffect(float x, float y, bool bFlip)
{
    // �]�w�S�Ī���l�@�ɮy��
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // �]�w�S�ĬO�_�ݭn½��
    m_bIsFlip = bFlip;
}

// �����ϲ�Ķ�X: 0x0052E7E0
bool CEffect_Battle_GunSpark::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_GunSpark.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x0052E7F0
void CEffect_Battle_GunSpark::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052E830
void CEffect_Battle_GunSpark::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}