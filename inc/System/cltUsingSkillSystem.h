#pragma once

class cltUsingSkillSystem {
public:
    int GetUsingSkillNum();
    int GetTotalStrOfUsingSkill();
    int GetTotalDexOfUsingSkill();
    int GetTotalIntOfUsingSkill();
    int GetTotalVitOfUsingSkill();
    int GetAttackSpeedAdvantage();
    int IsActiveFastRunSkill();
    int IsActiveNonDelayAttack();
    int IsActiveMultiAttack();
    int IsActiveManashieldSkill();
    int GetCriticalRate();
    int GetMissRateAdvantage();
    int GetTotalAPowerOfUsingSkill();
    int GetTotalDPowerOfUsingSkill();
    int GetExpAdvantage(int includePassive);
    int GetMaxFaintingInfo(int* outChance, int* outDuration);
    int GetMaxConfusionInfo(int* outChance, int* outDuration);
    int GetMaxFreezingInfo(int* outChance, int* outDuration);
};
