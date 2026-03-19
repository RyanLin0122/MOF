#include "Object/CPortalObject.h"
#include "Effect/CEffect_Portal.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"
#include <cstring>

CPortalObject::CPortalObject()
    : CBaseObject()
    , m_pEffect(nullptr)
{
    m_siType = 1;
}

CPortalObject::~CPortalObject()
{
    if (m_pEffect)
    {
        delete m_pEffect;
        m_pEffect = nullptr;
    }
}

void CPortalObject::CreatePortal(unsigned short wPortalType, int nPosX, int nPosY)
{
    char szEffect1[128];
    char szEffect2[128];
    memset(szEffect1, 0, sizeof(szEffect1));
    memset(szEffect2, 0, sizeof(szEffect2));

    switch (wPortalType)
    {
    case 1:
        strcpy(szEffect1, "E0749");
        nPosY += 25;
        break;
    case 2:
        nPosY += 100;
        strcpy(szEffect1, "E0743");
        break;
    case 3:
    case 0x65:
        nPosY += 25;
        strcpy(szEffect1, "E0743");
        break;
    case 4:
    case 0x6F:
        nPosY += 70;
        strcpy(szEffect1, "E0746");
        strcpy(szEffect2, "E0747");
        break;
    case 5:
        strcpy(szEffect1, "E0745");
        nPosY += 25;
        break;
    case 6:
        nPosY += 70;
        strcpy(szEffect1, "E0744");
        strcpy(szEffect2, "E0750");
        break;
    case 0x70:
        nPosY += 25;
        strcpy(szEffect1, "E0742");
        strcpy(szEffect2, "E0741");
        break;
    default:
        break;
    }

    m_pEffect = nullptr;

    // Create the primary portal effect
    if (szEffect1[0] != 0)
    {
        CEffect_Portal* pPortal = new CEffect_Portal();
        m_pEffect = pPortal;
        pPortal->SetEffect(szEffect1, (float)nPosX, (float)nPosY);
    }

    // Create secondary portal effect if specified
    if (szEffect2[0] != 0)
    {
        CEffect_Portal* pPortal2 = new CEffect_Portal();
        pPortal2->SetEffect(szEffect2, (float)nPosX, (float)nPosY);
        CEffectManager::GetInstance()->BulletAdd(pPortal2);
    }
}

void CPortalObject::FrameProcess(float fElapsedTime)
{
    if (m_pEffect)
        m_pEffect->FrameProcess(fElapsedTime);
}

void CPortalObject::Process()
{
    if (m_pEffect)
        m_pEffect->Process();
}

void CPortalObject::Draw()
{
    if (m_pEffect)
    {
        m_pEffect->Draw();
        CDeviceManager::GetInstance()->ResetRenderState();
    }
}
