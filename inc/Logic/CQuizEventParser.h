#pragma once
#include <array>
#include <cstdint>
#include <cstring>

struct stQuizAnswer {
    std::array<std::uint16_t, 4> answer{};
};

struct stQuizInfo {
    std::uint16_t id;
    std::uint16_t unk;
    std::uint16_t answerLineCount;
    std::uint16_t isEnglish;
    std::uint16_t answerCount[6];
    std::uint16_t reserved[16];
    std::uint16_t answerHangleID[6][11];
};

class CQuizEventParser {
public:
    stQuizInfo* GetQuizInfo(std::uint16_t quizID);
};