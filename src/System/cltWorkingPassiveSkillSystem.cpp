#include "System/cltWorkingPassiveSkillSystem.h"

int cltWorkingPassiveSkillSystem::GetWorkingSkillNum() {
    // From mofclient.c: returns *((_DWORD *)this + 40)
    // TODO: add proper member variables to header
    return 0;
}

int cltWorkingPassiveSkillSystem::GetAttackSpeedAdvantage() { return 0; }
int cltWorkingPassiveSkillSystem::GetDamageHP2ManaRate() { return 0; }
int cltWorkingPassiveSkillSystem::GetAPowerAdvantage() { return 0; }
int cltWorkingPassiveSkillSystem::GetDPowerAdvantage() { return 0; }
