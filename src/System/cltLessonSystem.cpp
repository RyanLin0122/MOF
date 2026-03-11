#include "System/cltLessonSystem.h"

// From mofclient.c: returns *((_DWORD *)this + 7)
// TODO: add proper member variables to header
std::uint16_t cltLessonSystem::GetSwordLessonPt() { return 0; }

// From mofclient.c: returns *((_DWORD *)this + 8)
std::uint16_t cltLessonSystem::GetBowLessonPt() { return 0; }

// From mofclient.c: returns *((_DWORD *)this + 9)
std::uint16_t cltLessonSystem::GetTheologyLessonPt() { return 0; }

// From mofclient.c: returns *((_DWORD *)this + 10)
std::uint16_t cltLessonSystem::GetMagicLessonPt() { return 0; }

void cltLessonSystem::DecLessonPt_Sword(unsigned int) {}
void cltLessonSystem::DecLessonPt_Bow(unsigned int) {}
void cltLessonSystem::DecLessonPt_Theology(unsigned int) {}
void cltLessonSystem::DecLessonPt_Magic(unsigned int) {}

int cltLessonSystem::CanTraningLesson(unsigned int) { return 0; }
int cltLessonSystem::GetTotalSwordLessonPt() { return 0; }
int cltLessonSystem::GetTotalBowLessonPt() { return 0; }
int cltLessonSystem::GetTotalTheologyLessonPt() { return 0; }
int cltLessonSystem::GetTotalMagicLessonPt() { return 0; }
