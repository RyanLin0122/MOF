#pragma once

class cltWorkingPassiveSkillSystem {
public:
    int GetWorkingSkillNum();
    int GetAttackSpeedAdvantage();
    int GetDamageHP2ManaRate();
    int GetAPowerAdvantage();
    int GetDPowerAdvantage();
    int GetMissRateAdvantage();

    // Party related (被 cltPartySystem 連結需求)
    int GetPartyMemberAPowerAdvantage(int* outNum, unsigned short* outSkillKinds);
    int GetPartyMemberHitRateAdvantage();
};
