#include "Effect/CEffect_MiniGame_Priest_Light.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00538E60
CEffect_MiniGame_Priest_Light::CEffect_MiniGame_Priest_Light()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 116, �ɮצW��: "Effect/efn_MiniGame_Class_Sword.ea"
    // �`�N�G���B���J���O�C�h���S�ĸ귽
    CEAManager::GetInstance()->GetEAData(116, "Effect/efn_MiniGame_Class_Sword.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��
}

// �����ϲ�Ķ�X: 0x00538EF0
CEffect_MiniGame_Priest_Light::~CEffect_MiniGame_Priest_Light()
{
}

// �����ϲ�Ķ�X: 0x00538F00
void CEffect_MiniGame_Priest_Light::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x00538F20
bool CEffect_MiniGame_Priest_Light::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00538F30
void CEffect_MiniGame_Priest_Light::Process()
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

// �����ϲ�Ķ�X: 0x00538F50
void CEffect_MiniGame_Priest_Light::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}