#include "Effect/CEffect_Field_Warp.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"


CEffect_Field_Warp::CEffect_Field_Warp()
    : m_pOwnerCharacter(nullptr)
{
}

CEffect_Field_Warp::~CEffect_Field_Warp()
{
}

// �����ϲ�Ķ�X: 0x00537610
void CEffect_Field_Warp::SetEffect(unsigned short warpType, float x, float y, ClientCharacter* pOwner)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڶǤJ�� warpType ��ܭn���J���S�ĸ귽
    switch (warpType)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 0x65: // 101
    case 0x6F: // 111
        szFileName = "Effect/efn_warp.ea";
        effectID = 103;
        break;

    case 5:
    case 0x70: // 112
        szFileName = "Effect/efn-Inwarp.ea";
        effectID = 104;
        break;

    default:
        // �p�G�ǤJ�����������A�h���ЫدS��
        return;
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �]�w��l��m�M���H�ؼ�
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_pOwnerCharacter = pOwner;
}

// �����ϲ�Ķ�X: 0x00537700
bool CEffect_Field_Warp::FrameProcess(float fElapsedTime)
{
    // �ͩR�g���Ѥ����ʵe�M�w
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00537710
void CEffect_Field_Warp::Process()
{
    // �p�G�j�w�F����A�h�����s��m���Ө��⪺��m
    if (m_pOwnerCharacter) {
        m_fCurrentPosX = static_cast<float>(m_pOwnerCharacter->GetPosX());
        m_fCurrentPosY = static_cast<float>(m_pOwnerCharacter->GetPosY());
    }

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00537770
void CEffect_Field_Warp::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}