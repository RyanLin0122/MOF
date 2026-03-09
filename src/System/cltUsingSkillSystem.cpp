#include "System/cltUsingSkillSystem.h"

int cltUsingSkillSystem::GetUsingSkillNum() {
    // From mofclient.c: iterates 10 entries at offset +24,
    // each 24 bytes apart, counting non-zero first DWORDs
    // TODO: add proper member variables to header
    return 0;
}
