#include "Logic/cltMoF_ClientMine.h"

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "global.h"

cltMoF_ClientMine::cltMoF_ClientMine() = default;

cltMoF_ClientMine::~cltMoF_ClientMine()
{
    DeleteMine();
}

void cltMoF_ClientMine::CreateMine(unsigned int handle,
                                   int hp, int maxHp,
                                   int range, int maxRange,
                                   unsigned int disappearMs,
                                   unsigned int resourceIdHex,
                                   uint16_t totalFlame,
                                   char airGround,
                                   char attackCountType,
                                   char moveType)
{
    m_nActive = 1;
    m_handle = handle;
    m_hp = hp;
    m_maxHp = maxHp;
    m_range = range;
    m_maxRange = maxRange;
    m_disappearMs = disappearMs;
    m_resourceIdHex = resourceIdHex;
    m_totalFlame = totalFlame;
    m_airGround = airGround;
    m_attackCountType = attackCountType;
    m_moveType = moveType;
    m_bSearched = 0;
    m_fScreenX = 0.0f;
    m_fScreenY = 0.0f;
}

void cltMoF_ClientMine::DeleteMine()
{
    m_nActive = 0;
    m_handle = 0;
    m_hp = 0;
    m_bSearched = 0;

    if (m_pImage)
    {
        cltImageManager::GetInstance()->ReleaseGameImage(m_pImage);
        m_pImage = nullptr;
    }
}

int cltMoF_ClientMine::GetActive()
{
    return m_nActive;
}

void cltMoF_ClientMine::ResetSearched()
{
    m_bSearched = 0;
}

void cltMoF_ClientMine::Poll()
{
}

void cltMoF_ClientMine::AttackOneMine()
{
}

void cltMoF_ClientMine::AttackMultiMine()
{
}

void cltMoF_ClientMine::SearchAndMove()
{
}

void cltMoF_ClientMine::PrepareDrawing()
{
    if (!m_nActive)
        return;

    if (!m_pImage)
    {
        m_pImage = cltImageManager::GetInstance()->GetGameImage(6u, m_resourceIdHex, 0, 0);
    }

    if (!m_pImage)
        return;

    m_pImage->SetBlockID(0);
    m_pImage->SetPosition(m_fScreenX, m_fScreenY);
    m_pImage->SetAlpha(255u);
    m_pImage->Process();
}

void cltMoF_ClientMine::Draw()
{
    if (m_nActive && m_pImage)
        m_pImage->Draw();
}

int cltMoF_ClientMine::IsValidSearch(ClientCharacter* /*pChar*/)
{
    return 0;
}

int cltMoF_ClientMine::IsValidAttack(ClientCharacter* /*pChar*/)
{
    return 0;
}
