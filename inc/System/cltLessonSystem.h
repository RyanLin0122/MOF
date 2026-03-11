#pragma once

#include <cstdint>

class cltLessonSystem {
public:
    std::uint16_t GetSwordLessonPt();
    std::uint16_t GetBowLessonPt();
    std::uint16_t GetTheologyLessonPt();
    std::uint16_t GetMagicLessonPt();

    void DecLessonPt_Sword(unsigned int value);
    void DecLessonPt_Bow(unsigned int value);
    void DecLessonPt_Theology(unsigned int value);
    void DecLessonPt_Magic(unsigned int value);

    int CanTraningLesson(unsigned int lessonKind);
    int GetTotalSwordLessonPt();
    int GetTotalBowLessonPt();
    int GetTotalTheologyLessonPt();
    int GetTotalMagicLessonPt();
};
