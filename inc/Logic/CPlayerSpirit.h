#pragma once

#include <cstdint>

struct stSpiritInfo;

class CPlayerSpirit {
public:
    CPlayerSpirit();
    ~CPlayerSpirit() = default;

    void Initialize();

    // UpdatePlayerSpiritInfo(spiritKind, equipFlag)
    int UpdatePlayerSpiritInfo(std::uint16_t spiritKind, int equipFlag);

    // UpdatePlayerSpiritInfo(spiritKind, level, equipFlag)
    int UpdatePlayerSpiritInfo(std::uint16_t spiritKind, std::uint16_t level, int equipFlag);

    // UpdatePlayerSpiritInfo(level) — uses stored spiritKind
    int UpdatePlayerSpiritInfo(std::uint16_t level);

    void SetSpiritInfo(stSpiritInfo* info);

private:
    std::uint16_t m_wSpiritAttribute;  // wSpiritAttribute
    std::uint16_t m_wIncStr;           // strength increase
    std::uint16_t m_wIncInt;           // intelligence increase
    std::uint16_t m_wIncDex;           // dexterity increase
    std::uint16_t m_wIncVit;           // vitality increase
    std::uint16_t m_wSpiritKind;       // stored spirit kind for level-only update
};
