#pragma once
#include <cstdint>
#include <vector>

struct stNPCInfo {
    std::uint16_t m_wKind = 0;
    std::uint16_t _pad2 = 0;
    std::uint32_t _reserved[46]{};
    std::uint32_t m_dwNoRecall = 0;
};

class cltNPCInfo {
public:
    static uint16_t TranslateKindCode(const char* s);

    int GetTotalNPCNum() const;
    stNPCInfo* GetNPCInfoByIndex(int index);

    void SetNPCList(std::vector<stNPCInfo> list);

private:
    std::vector<stNPCInfo> m_npcs;
};
