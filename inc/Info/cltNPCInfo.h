#pragma once
#include <cstdint>
#include <vector>

// NPC 資訊結構 — 記憶體佈局與反編譯相符
// 總長度 ≥ 342 bytes (offset 200 起存放最多 71 筆任務 ID)
struct stNPCInfo {
    std::uint16_t m_wKind = 0;              // offset 0
    std::uint16_t _pad2 = 0;               // offset 2
    std::uint32_t _reserved[46]{};         // offset 4~187  (184 bytes)
    std::uint32_t m_dwNoRecall = 0;        // offset 188
    std::uint32_t _field_192 = 0;          // offset 192 (GetNPCIDByLevel 用)
    std::uint16_t _pad196[2]{};            // offset 196~199
    std::uint16_t m_wQuestIDs[71]{};       // offset 200~341 (142 bytes)
                                            // index 0 = 首個任務 ID
                                            // index 1..70 = 後續任務鏈
};

class cltNPCInfo {
public:
    static uint16_t TranslateKindCode(const char* s);

    int GetTotalNPCNum() const;
    stNPCInfo* GetNPCInfoByIndex(int index);
    stNPCInfo* GetNPCInfoByID(std::uint16_t npcKind);
    std::uint16_t GetMapID(std::uint16_t npcKind);
    std::uint16_t GetNPCIDByQuestID(int questID, int* outNPCIndex);

    void SetNPCList(std::vector<stNPCInfo> list);

private:
    std::vector<stNPCInfo> m_npcs;
};
