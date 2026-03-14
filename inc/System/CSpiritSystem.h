#pragma once

#include <cstdint>
#include <vector>

struct stSpiritInfo {
    std::uint16_t wKind = 0;
    std::uint16_t wTextCode = 0;
    std::uint16_t wGrade = 0;
    std::uint16_t wFromLevel = 0;
    std::uint16_t wToLevel = 0;
    std::uint16_t wSpiritKind = 0;
    std::uint16_t wUnknown6 = 0;
    std::uint16_t wUnknown7 = 0;
    std::uint16_t wUnknown8 = 0;
    std::uint16_t wUnknown9 = 0;
    std::uint16_t wUnknown10 = 0;
    std::uint16_t wUnknown11 = 0;
    std::uint16_t wResId = 0;
    std::uint16_t wBlockId = 0;
    std::uint16_t wTooltipId = 0;
    std::uint16_t wDummy = 0;
};

class CSpiritSystem {
public:
    CSpiritSystem();
    ~CSpiritSystem();
    int Initialize(char* filename);

    std::uint16_t GetSpiritKind(std::uint16_t level);
    stSpiritInfo* GetSpiritInfo(std::uint16_t spiritKind, std::uint16_t level);
    stSpiritInfo* GetSpiritInfo(std::uint16_t kind);

    std::uint16_t TranslateKindCode(char* s);

private:
    std::vector<stSpiritInfo> m_infos;
};
