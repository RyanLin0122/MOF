#pragma once

#include <cstdint>

class cltUsingSkillSystem {
public:
    int GetSkillCoolTimeByBaseCoolTime(int a2, int baseCoolTime);
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

    // Party related (被 cltPartySystem 連結需求)
    int GetPartyMemberAPowerAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberDPowerAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyExpAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberStrAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberDexAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberVitAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberIntAdvantage(int* outNum, std::uint16_t* outSkillKinds);
    int GetPartyMemberAttackAtb(void* outAtb, unsigned int* outFound, std::uint16_t* outAtbCode);
};
