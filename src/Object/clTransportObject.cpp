#include "Object/clTransportObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Character/ClientCharacter.h"
#include "Logic/clTransportKindInfo.h"
#include "Logic/clClientTransportKindInfo.h"
#include "Logic/clTransportAniInfo.h"
#include "Logic/cltMyCharData.h"
#include "global.h"
#include <cmath>

extern int dword_A73088;
extern int dword_A7308C;


clTransportObject::clTransportObject()
    : m_pImageUp(nullptr)
    , m_pImageDown(nullptr)
    , m_pOwner(nullptr)
    , m_pAniInfoUp(nullptr)
    , m_pAniInfoDown(nullptr)
    , m_pKindInfo(nullptr)
    , m_pCCA(nullptr)
    , _reserved8(0)
    , m_nActive(0)
    , m_wTotalFrameUp(0)
    , m_wTotalFrameDown(0)
    , m_wCurFrameUp(0)
    , m_wCurFrameDown(0)
    , m_nPosX(0)
    , m_nPosY(0)
    , m_dwActionState(0)
{
}

clTransportObject::~clTransportObject()
{
    Release();
}

void clTransportObject::Release()
{
    m_nActive = 0;
    m_pOwner = nullptr;
    m_pKindInfo = nullptr;
    m_pAniInfoUp = nullptr;
    m_pAniInfoDown = nullptr;
    m_pImageUp = nullptr;
    m_pImageDown = nullptr;
}

void clTransportObject::InitTransport(ClientCharacter* pOwner, CCA* pCCA, std::uint16_t transportKind)
{
    if (!pOwner)
        return;

    m_pOwner = pOwner;
    m_pCCA = pCCA;
    m_pKindInfo = g_clTransportKindInfo.GetTransportKindInfo(transportKind);
    if (m_pKindInfo)
    {
        m_pAniInfoUp = g_clClientTransportKindInfo.GetTransportAniInfoUp(transportKind);
        m_pAniInfoDown = g_clClientTransportKindInfo.GetTransportAniInfoDown(transportKind);
        if (m_pAniInfoUp && m_pAniInfoDown)
        {
            // Ground truth: 觸發 cltMyCharData::GetMyAccount 的副作用
            cltMyCharData::GetMyAccount(&g_clMyCharData);
        }
    }
}

void clTransportObject::SetActive(int active)
{
    m_nActive = active;
    SetActionState(*reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(m_pOwner) + 9684));
}

int clTransportObject::GetActive()
{
    return m_nActive;
}

void clTransportObject::UpdatePosition()
{
    int ownerPosX = *reinterpret_cast<int*>(reinterpret_cast<char*>(m_pOwner) + 4384);
    int ownerPosY = *reinterpret_cast<int*>(reinterpret_cast<char*>(m_pOwner) + 4388);

    if (m_pCCA)
    {
        m_nPosY = static_cast<int>(static_cast<std::int64_t>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 132)));
        m_nPosX = static_cast<int>(static_cast<std::int64_t>(*reinterpret_cast<float*>(reinterpret_cast<char*>(m_pCCA) + 128)));
    }
    else
    {
        m_nPosX = ownerPosX;
        m_nPosY = ownerPosY;
    }
}

void clTransportObject::TransportMove() { UpdatePosition(); }
void clTransportObject::TransportStop() { UpdatePosition(); }
void clTransportObject::TransportHitted() { UpdatePosition(); }

void clTransportObject::PrepareDrawing(int param)
{
    (void)param;
    if (!m_nActive)
        return;

    if (!m_pOwner)
        return;

    unsigned int resUp = 0;
    unsigned int resDown = 0;
    std::uint16_t frameUp[2] = {0, 0};
    std::uint16_t frameDown[2] = {0, 0};

    unsigned int actionState = *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(m_pOwner) + 9684);
    if (actionState)
    {
        unsigned int v = actionState - 1;
        if (v)
        {
            if (v == 3)
                TransportHitted();
        }
        else
        {
            TransportMove();
        }
    }
    else
    {
        TransportStop();
    }

    DecideDrawFrame(&resUp, &resDown, frameUp, frameDown);
    m_pImageUp = cltImageManager::GetInstance()->GetGameImage(1u, resUp, 0, 1);
    m_pImageDown = cltImageManager::GetInstance()->GetGameImage(1u, resDown, 0, 1);
    if (!m_pImageUp || !m_pImageDown)
        return;

    int drawX = m_nPosX;
    float drawY = static_cast<float>(m_nPosY);
    if (!m_pCCA)
    {
        drawX = m_nPosX - dword_A73088;
        drawY = static_cast<float>(m_nPosY - dword_A7308C);
    }

    const float fDrawX = static_cast<float>(drawX);
    m_pImageUp->m_bFlag_447 = true;
    m_pImageUp->SetBlockID(frameUp[0]);
    m_pImageUp->m_bFlag_446 = true;
    m_pImageUp->SetPosition(fDrawX, drawY);
    m_pImageUp->m_bVertexAnimation = false;
    m_pImageUp->m_bFlipX = (*reinterpret_cast<int*>(reinterpret_cast<char*>(m_pOwner) + 572) == 0);

    m_pImageDown->SetPosition(fDrawX, drawY);
    m_pImageDown->SetBlockID(frameDown[0]);
    m_pImageDown->m_bFlag_447 = true;
    m_pImageDown->m_bFlag_446 = true;
    m_pImageDown->m_bVertexAnimation = false;
    m_pImageDown->m_bFlipX = (*reinterpret_cast<int*>(reinterpret_cast<char*>(m_pOwner) + 572) == 0);

    if (m_pImageUp->m_pGIData)
    {
        int* blockData = reinterpret_cast<int*>(*(reinterpret_cast<int*>(m_pImageUp->m_pGIData) + 8));
        if (blockData)
        {
            const int blockOff24 = *reinterpret_cast<int*>(reinterpret_cast<char*>(blockData) + 52 * frameUp[0] + 24);
            const int blockOff32 = *reinterpret_cast<int*>(reinterpret_cast<char*>(blockData) + 52 * frameUp[0] + 32);
            if (static_cast<double>(blockOff24) < fabs(static_cast<double>(blockOff32)))
                *reinterpret_cast<float*>(reinterpret_cast<char*>(m_pImageUp) + 352) = static_cast<float>(-(blockOff32 + blockOff24));
        }
        m_pImageUp->m_bDrawPart2 = true;
    }

    if (m_pImageDown->m_pGIData)
    {
        int* blockData = reinterpret_cast<int*>(*(reinterpret_cast<int*>(m_pImageDown->m_pGIData) + 8));
        if (blockData)
        {
            const int blockOff24 = *reinterpret_cast<int*>(reinterpret_cast<char*>(blockData) + 52 * frameDown[0] + 24);
            const int blockOff32 = *reinterpret_cast<int*>(reinterpret_cast<char*>(blockData) + 52 * frameDown[0] + 32);
            if (static_cast<double>(blockOff24) < fabs(static_cast<double>(blockOff32)))
                *reinterpret_cast<float*>(reinterpret_cast<char*>(m_pImageDown) + 352) = 15.0f - static_cast<float>(blockOff32 + blockOff24);
        }
        m_pImageDown->m_bDrawPart2 = true;
    }
}

void clTransportObject::DrawUp(int param)
{
    (void)param;
    if (m_nActive && m_pImageUp)
        m_pImageUp->Draw();
}

void clTransportObject::DrawDown(int param)
{
    (void)param;
    if (m_nActive && m_pImageDown)
        m_pImageDown->Draw();
}

void clTransportObject::DecideDrawFrame(unsigned int* outResUp, unsigned int* outResDown,
                                        std::uint16_t* outFrameUp, std::uint16_t* outFrameDown)
{
    unsigned int actionState = m_dwActionState;
    if (actionState)
    {
        unsigned int v = actionState - 1;
        if (v)
        {
            if (v == 3)
            {
                m_pAniInfoUp->GetFrameInfo(4u, m_wCurFrameUp, outResUp, outFrameUp);
                m_pAniInfoDown->GetFrameInfo(4u, m_wCurFrameDown, outResDown, outFrameDown);
            }
        }
        else
        {
            m_pAniInfoUp->GetFrameInfo(1u, m_wCurFrameUp, outResUp, outFrameUp);
            m_pAniInfoDown->GetFrameInfo(1u, m_wCurFrameDown, outResDown, outFrameDown);
        }
    }
    else
    {
        m_pAniInfoUp->GetFrameInfo(0, m_wCurFrameUp, outResUp, outFrameUp);
        m_pAniInfoDown->GetFrameInfo(0, m_wCurFrameDown, outResDown, outFrameDown);
    }
}

void clTransportObject::Poll()
{
    if (!m_nActive)
        return;

    std::uint16_t frameUp = ++m_wCurFrameUp;
    std::uint16_t frameDown = ++m_wCurFrameDown;
    if (frameUp >= m_wTotalFrameUp)
        m_wCurFrameUp = 0;
    if (frameDown >= m_wTotalFrameDown)
        m_wCurFrameDown = 0;
}

void clTransportObject::SetActionState(unsigned int actionState)
{
    m_dwActionState = actionState;
    m_wTotalFrameUp = 0;
    m_wTotalFrameDown = 0;
    switch (actionState)
    {
    case 0:
        m_wTotalFrameUp = m_pAniInfoUp ? m_pAniInfoUp->GetTotalFrameNum(0) : 0;
        m_wTotalFrameDown = m_pAniInfoDown ? m_pAniInfoDown->GetTotalFrameNum(0) : 0;
        break;
    case 1:
        m_wTotalFrameUp = m_pAniInfoUp ? m_pAniInfoUp->GetTotalFrameNum(1) : 0;
        m_wTotalFrameDown = m_pAniInfoDown ? m_pAniInfoDown->GetTotalFrameNum(1) : 0;
        break;
    case 4:
        m_wTotalFrameUp = m_pAniInfoUp ? m_pAniInfoUp->GetTotalFrameNum(4) : 0;
        m_wTotalFrameDown = m_pAniInfoDown ? m_pAniInfoDown->GetTotalFrameNum(4) : 0;
        break;
    default:
        break;
    }
    m_wCurFrameUp = 0;
    m_wCurFrameDown = 0;
}
