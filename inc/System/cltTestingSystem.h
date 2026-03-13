#pragma once

#include <cstdint>
#include <ctime>
#include <array>
#include <cstdlib>
#include <cstring>

#include "Info/cltQuestionKindInfo.h"
#include "System/cltGradeSystem.h"
#include "System/cltMoneySystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltLessonSystem.h"


class cltTestingSystem {
public:
    static void InitializeStaticVariable(cltQuestionKindInfo* questionKindInfo);

    cltTestingSystem();

    void Initialize(cltMoneySystem* moneySystem, cltLessonSystem* lessonSystem, cltGradeSystem* gradeSystem, cltQuestSystem* questSystem, int lastTestingTime);

    int CanBeginTesting(int checkTime);
    void BeginTesting(unsigned int seed);
    void GenerateQuestions(unsigned int seed);
    std::uint16_t GetQuestion(int index);
    int DecideAnswer(int index, std::uint16_t monsterKind, std::uint16_t npcKind);
    int CanFinishTesting(int passTest, std::uint16_t* const answersMonster, std::uint16_t* const answersNpc);
    void FinishTesting(int passed, int* outCost, std::uint16_t* questKinds, unsigned int* questValues);

    int IsInTesting();
    int CanNowTesting();
    int GetLastTestingTime();
    int GetTestingCost(std::uint8_t grade);

private:
    static cltQuestionKindInfo* m_pclQuestionKindInfo;

    cltMoneySystem* m_pMoneySystem = nullptr;
    cltLessonSystem* m_pLessonSystem = nullptr;
    cltGradeSystem* m_pGradeSystem = nullptr;
    cltQuestSystem* m_pQuestSystem = nullptr;

    std::uint16_t m_wQuestions[5]{};
    int m_bInTesting = 0;
    std::time_t m_lastTestingTime = 0;
};
