#include "System/cltPetSkillSystem.h"

#include "Network/CMofMsg.h"

void cltPetSkillSystem::Initialize(CMofMsg*) {}
void cltPetSkillSystem::Free() {}
void cltPetSkillSystem::OnPetCreated() {}
void cltPetSkillSystem::OnPetDeleted() {}

int cltPetSkillSystem::GetSkillAPowerAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetAPowerAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetDPowerAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetHitRateAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetAutoRecoverHPAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetAutoRecoverManaAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::CanPickupItem(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetDropRateAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetSTRAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetVITAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetDEXAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetINTAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::GetAttackSpeedAdvantage(std::uint16_t) { return 0; }
int cltPetSkillSystem::CanSetPetUsingSkill(std::uint8_t, std::uint16_t) { return 0; }
void cltPetSkillSystem::SetPetUsingSkill(std::uint8_t, std::uint16_t) {}
unsigned int cltPetSkillSystem::CanAddPetSkill(std::uint16_t) { return 0; }
void cltPetSkillSystem::AddPetSkill(std::uint16_t) {}
std::uint16_t cltPetSkillSystem::GetPetSkillNum() { return 0; }

