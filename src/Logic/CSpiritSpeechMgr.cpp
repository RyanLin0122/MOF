#include "Logic/CSpiritSpeechMgr.h"

char* CSpiritSpeechMgr::GetSpiritSpeechScript() {
    static char empty[] = "";
    return empty;
}

void CSpiritSpeechMgr::UpdateQuestCollection(uint16_t /*itemKind*/) {
    // Stub: quest collection speech hooks are not reconstructed yet.
}

void CSpiritSpeechMgr::UpdateQuestHunt(uint16_t /*monsterKind*/) {
    // Stub: quest hunt speech hooks are not reconstructed yet.
}
