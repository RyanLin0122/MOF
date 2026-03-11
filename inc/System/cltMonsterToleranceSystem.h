#pragma once

#include <cstdint>

class cltMonsterToleranceSystem {
public:
    int GetAPowerAdvantage(std::uint16_t charKind);
    int GetDPowerAdvantage(std::uint16_t charKind);
    int GetHitRateAdvantage(std::uint16_t charKind);
    int GetCriticalHitRateAdvantage(std::uint16_t charKind);
    int GetMissRateAdvantage(std::uint16_t charKind);
    int GetSkillAPowerAdvantage(std::uint16_t charKind);
};
