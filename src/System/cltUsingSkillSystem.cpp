#include "System/cltUsingSkillSystem.h"


int cltUsingSkillSystem::GetUsingSkillNum() { return 0; }
int cltUsingSkillSystem::GetTotalStrOfUsingSkill(){ return 0; }
int cltUsingSkillSystem::GetTotalDexOfUsingSkill(){ return 0; }
int cltUsingSkillSystem::GetTotalIntOfUsingSkill(){ return 0; }
int cltUsingSkillSystem::GetTotalVitOfUsingSkill(){ return 0; }
int cltUsingSkillSystem::GetAttackSpeedAdvantage(){ return 0; }
int cltUsingSkillSystem::IsActiveFastRunSkill(){ return 0; }
int cltUsingSkillSystem::IsActiveNonDelayAttack(){ return 0; }
int cltUsingSkillSystem::IsActiveMultiAttack() { return 0; }
int cltUsingSkillSystem::IsActiveManashieldSkill(){ return 0; }
int cltUsingSkillSystem::GetCriticalRate(){ return 0; }
int cltUsingSkillSystem::GetMissRateAdvantage() { return 0; }
int cltUsingSkillSystem::GetTotalAPowerOfUsingSkill(){ return 0; }
int cltUsingSkillSystem::GetTotalDPowerOfUsingSkill() { return 0; }
int cltUsingSkillSystem::GetExpAdvantage(int) { return 0; }
int cltUsingSkillSystem::GetMaxFaintingInfo(int* outChance, int* outDuration) {
    if (outChance) *outChance = 0;
    if (outDuration) *outDuration = 0;
    return 0;
}
int cltUsingSkillSystem::GetMaxConfusionInfo(int* outChance, int* outDuration) {
    if (outChance) *outChance = 0;
    if (outDuration) *outDuration = 0;
    return 0;
}
int cltUsingSkillSystem::GetMaxFreezingInfo(int* outChance, int* outDuration) {
    if (outChance) *outChance = 0;
    if (outDuration) *outDuration = 0;
    return 0;
}
