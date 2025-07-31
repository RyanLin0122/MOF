#include "Effect/CEffect_Player_WeatherPreView.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00532790
CEffect_Player_WeatherPreView::CEffect_Player_WeatherPreView()
    : m_fScreenPosX(0.0f), m_fScreenPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

// �����ϲ�Ķ�X: 0x005327D0
CEffect_Player_WeatherPreView::~CEffect_Player_WeatherPreView()
{
}

// �����ϲ�Ķ�X: 0x005327E0
void CEffect_Player_WeatherPreView::SetEffect(float x, float y, int baseEffectID, char* szFileName)
{
    // �c���ɮ׸��|
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);

    // �ھڰ�¦ ID �p��̲ת� Effect Kind ID
    int finalEffectID = baseEffectID + 53;

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(finalEffectID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �x�s�T�w���ù��y��
    m_fScreenPosX = x;
    m_fScreenPosY = y;
}

// �����ϲ�Ķ�X: 0x00532860
bool CEffect_Player_WeatherPreView::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe�v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- �֤ߥͩR�g���޿� ---
    // ��l�X: return 0;
    // �û���^ false�A��ܦ��S�Ĥ��|�۰ʵ����C
    return false;
}

// �����ϲ�Ķ�X: 0x00532880
void CEffect_Player_WeatherPreView::Process()
{
    // --- �֤��޿�G�����N�x�s���y�Ч@���ù��y�� ---
    // ��l�X: *((float *)this + 12) = *((float *)this + 33);
    //         *((float *)this + 13) = *((float *)this + 34);

    // ��s���� CCAEffect �����A�A���i����v���y���ഫ
    m_ccaEffect.SetPosition(m_fScreenPosX, m_fScreenPosY);
    m_ccaEffect.Process();

    // UI �S���`�O�i��
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x005328A0
void CEffect_Player_WeatherPreView::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}