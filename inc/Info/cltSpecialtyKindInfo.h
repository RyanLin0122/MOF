#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct strSpecialtyKindInfo {
    std::uint16_t wKind = 0;
    std::uint16_t wTextCode = 0;
    std::uint16_t wDescCode = 0;
    std::uint16_t wCategory = 0;
    std::uint16_t wRequiredSpecialtyKind = 0;
    std::uint8_t byRequiredSpecialtyPt = 0;
    std::uint8_t byRequiredGrade = 0;
    std::uint32_t dwRequiredLevel = 0;
    std::uint16_t wReserved = 0;
    std::array<std::uint16_t, 5> wAcquiredSkillKinds{};
    std::uint8_t byType = 0; // 0 generic, 1 making, 2 transform
    std::uint8_t byIsCircle = 0;
};

class cltSpecialtyKindInfo {
public:
    static std::uint16_t TranslateKindCode(char* s);

    strSpecialtyKindInfo* GetSpecialtyKindInfo(std::uint16_t kind);
    int GetSpecialtyList(char category, std::uint16_t* outList);
    int IsLastLevelSpecialty(std::uint16_t kind);

    int IsGenericSpeciatly(std::uint16_t kind);
    int IsMakingItemSpecialty(std::uint16_t kind);
    int IsTransformSpecialty(std::uint16_t kind);
    int IsCircleSpecialty(std::uint16_t kind);

    std::int16_t GetSpecialtyTotalPoint(std::uint16_t kind);

    void SetSpecialtyTable(std::vector<strSpecialtyKindInfo> entries);

private:
    std::vector<strSpecialtyKindInfo> m_entries;
};
