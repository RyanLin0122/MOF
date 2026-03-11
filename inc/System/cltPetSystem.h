#pragma once

#include <cstdint>

class cltPetSystem {
public:
    int GetSTRAdvantage();
    int GetDEXAdvantage();
    int GetINTAdvantage();
    int GetVITAdvantage();
    int GetAPowerAdvantage();
    int GetDPowerAdvantage();
    int GetAttackSpeedAdvantage();
    int GetDropRateAdvantage();
    int GetHitRateAdvantage();
    int GetMissRateAdvantage();
    int GetSkillAPowerAdvantage(std::uint16_t charKind);
};
