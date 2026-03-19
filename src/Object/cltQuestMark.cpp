#include "Object/cltQuestMark.h"
#include <cstring>

// -------------------------------------------------------------------------
// cltQuestMark
// -------------------------------------------------------------------------
cltQuestMark::cltQuestMark()
    : CBaseObject()
{
}

cltQuestMark::~cltQuestMark()
{
}

void cltQuestMark::Initialize(std::uint16_t npcID, float posX, float posY, unsigned int type)
{
    m_siField4 = npcID;                      // WORD offset 14 = npcID
    m_siField5 = (unsigned short)type;       // WORD offset 10 = type (101 for quest)
    m_nPosX = (int)posX;
    m_nPosY = (int)posY;
}

// -------------------------------------------------------------------------
// cltRewardMark
// -------------------------------------------------------------------------
cltRewardMark::cltRewardMark()
    : CBaseObject()
{
}

cltRewardMark::~cltRewardMark()
{
}

void cltRewardMark::Initialize(std::uint16_t npcID, float posX, float posY, unsigned int type)
{
    m_siField4 = npcID;                      // WORD offset 14 = npcID
    m_siField5 = (unsigned short)type;       // WORD offset 10 = type (102 for reward)
    m_nPosX = (int)posX;
    m_nPosY = (int)posY;
}
