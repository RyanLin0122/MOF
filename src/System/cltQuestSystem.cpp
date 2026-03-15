#include "System/cltQuestSystem.h"

void* cltQuestSystem::GetPlayingQuestInfoByNPCID(std::uint16_t) { return nullptr; }
void* cltQuestSystem::GetPlayingQuestInfoByQuestID(std::uint16_t) { return nullptr; }
int cltQuestSystem::CanReward(int) { return 0; }

int cltQuestSystem::IsGiveupQuestPermanently() { return 0; }
int cltQuestSystem::IsCompleteQuest(std::uint16_t) { return 0; }

int cltQuestSystem::IsQuestMonster(std::uint16_t) { return 0; }
void cltQuestSystem::CompleteFunctionQuest(int, std::uint16_t*, unsigned int*) {}
