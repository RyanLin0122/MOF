#pragma once

#include "Object/CBaseObject.h"
#include <cstdint>

class GameImage;

class cltQuestMark : public CBaseObject {
public:
    cltQuestMark();
    virtual ~cltQuestMark();

    void Process() override;
    void Draw() override;
    void Initialize(std::uint16_t npcID, float posX, float posY, std::uint8_t type);

protected:
    GameImage*    m_pGameImage;
    std::uint16_t m_wCurrentFrame;
    std::uint16_t _pad;
    float         m_fAniFrame;
    unsigned int  m_dwResourceID;
};

class cltRewardMark : public CBaseObject {
public:
    cltRewardMark();
    virtual ~cltRewardMark();

    void Process() override;
    void Draw() override;
    void Initialize(std::uint16_t npcID, float posX, float posY, std::uint8_t type);

protected:
    GameImage*    m_pGameImage;
    std::uint16_t m_wCurrentFrame;
    std::uint16_t _pad;
    float         m_fAniFrame;
    unsigned int  m_dwResourceID;
};
