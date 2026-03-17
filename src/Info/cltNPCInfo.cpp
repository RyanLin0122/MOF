#include "Info/cltNPCInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <utility>

uint16_t cltNPCInfo::TranslateKindCode(const char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;

    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) {
        return static_cast<uint16_t>(hi | num);
    }
    return 0;
}

int cltNPCInfo::GetTotalNPCNum() const {
    return static_cast<int>(m_npcs.size());
}

stNPCInfo* cltNPCInfo::GetNPCInfoByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_npcs.size())) {
        return nullptr;
    }
    return &m_npcs[index];
}

stNPCInfo* cltNPCInfo::GetNPCInfoByID(std::uint16_t npcKind) {
    for (auto& npc : m_npcs) {
        if (npc.m_wKind == npcKind)
            return &npc;
    }
    return nullptr;
}

std::uint16_t cltNPCInfo::GetNPCIDByQuestID(int questID, int* outNPCIndex) {
    for (auto& npc : m_npcs) {
        if (!npc.m_wQuestIDs[0])
            continue;
        for (int j = 0; j < 71; j++) {
            if (npc.m_wQuestIDs[j] == static_cast<std::uint16_t>(questID)) {
                if (outNPCIndex)
                    *outNPCIndex = j;
                return npc.m_wKind;
            }
            if (!npc.m_wQuestIDs[j])
                break;
        }
    }
    return 0;
}

void cltNPCInfo::SetNPCList(std::vector<stNPCInfo> list) {
    m_npcs = std::move(list);
}
