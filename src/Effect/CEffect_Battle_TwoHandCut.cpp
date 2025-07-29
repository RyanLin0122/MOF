#include "Effect/CEffect_Battle_TwoHandCut.h"
#include "Effect/CEAManager.h"
#include "global.h" // ���]�Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052F630
CEffect_Battle_TwoHandCut::CEffect_Battle_TwoHandCut()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 7, �ɮצW��: "Effect/efn_twohandcut.ea"
    CEAManager::GetInstance()->GetEAData(7, "Effect/efn_twohandcut.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x0052F6C0
CEffect_Battle_TwoHandCut::~CEffect_Battle_TwoHandCut()
{
}

// �����ϲ�Ķ�X: 0x0052F6D0
void CEffect_Battle_TwoHandCut::SetEffect(float x, float y, bool bFlip, unsigned char a5)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // �]�w�S�ĬO�_�ݭn½��
    // ��l�X: *((_BYTE *)this + 56) = a4;
    m_bIsFlip = bFlip;
}

// �����ϲ�Ķ�X: 0x0052F6F0
bool CEffect_Battle_TwoHandCut::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_twohandcut.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x0052F700
void CEffect_Battle_TwoHandCut::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    // ��l�X: v3 = *((float *)this + 2) - (double)dword_A73088;
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    // ��l�X: v2 = CEffectBase::IsCliping(this, v3, 0.0); *((_DWORD *)this + 8) = v2;
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        // ��l�X: *((float *)this + 12) = ...; *((float *)this + 13) = ...;
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052F740
void CEffect_Battle_TwoHandCut::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}