#include "Effect/CEffect_OverMind_Screen.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cstdlib>

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00539F60
CEffect_OverMind_Screen::CEffect_OverMind_Screen()
    : m_pTargetCharacter(nullptr),
    m_bIsActive(false),
    m_dwAlpha(0),
    m_pEffectImage(nullptr)
{
    // ��l�X���غc�禡���šA�Ҧ���l�Ƴ��b SetActive ���i��
}

CEffect_OverMind_Screen::~CEffect_OverMind_Screen()
{
    // GameImage �� cltImageManager �޲z�A���B���� delete
}

// �����ϲ�Ķ�X: 0x00539F80
void CEffect_OverMind_Screen::SetActive(ClientCharacter* pTarget)
{
    m_pTargetCharacter = pTarget;
    m_bIsActive = true;
    m_dwAlpha = 255; // ��l�Ƴz����/�ͩR�g���p�ɾ�
}

// �����ϲ�Ķ�X: 0x00539FB0
void CEffect_OverMind_Screen::PrepareDrawing()
{
    if (!m_bIsActive || !m_pTargetCharacter) {
        return;
    }

    // ����S�ĨϥΪ� GameImage
    m_pEffectImage = cltImageManager::GetInstance()->GetGameImage(7, 0x20000091u, 0, 1);
    if (!m_pEffectImage) {
        m_bIsActive = false; // �p�G�������Ϥ��A�h���������S��
        return;
    }

    // �p��ù��y��
    float screenX = static_cast<float>(m_pTargetCharacter->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pTargetCharacter->GetPosY() - g_Game_System_Info.ScreenHeight - 30); // �b�����Y���W��30����

    // �]�w GameImage �ݩ�
    m_pEffectImage->SetPosition(screenX, screenY);
    m_pEffectImage->SetBlockID(0);

    // �]�w�z���סA�H�ۮɶ�����
    m_pEffectImage->SetAlpha(m_dwAlpha % 255);

    // �]�w�@���H�����C��/�Y��ȡA���Ͱ{�{�ĪG
    m_pEffectImage->SetColor(rand() % 500 + 100);

    // ��s GameImage �����I�w�İ�
    m_pEffectImage->Process();

    // ��s�ͩR�g���p�ɾ�
    m_dwAlpha -= 25;
    if (m_dwAlpha < 1) {
        m_bIsActive = false; // �ͩR�g������
    }
}

// �����ϲ�Ķ�X: 0x0053A090
void CEffect_OverMind_Screen::Draw()
{
    if (m_bIsActive && m_pEffectImage && m_pEffectImage->IsInUse())
    {
        // ���M��l�X�S���]�w�V�X�Ҧ��A�������S�ĳq�`�ϥ� Alpha �V�X
        // CDeviceManager::GetInstance()->ResetRenderState();
        m_pEffectImage->Draw();
    }
}