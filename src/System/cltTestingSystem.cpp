#include "System/cltTestingSystem.h"

cltQuestionKindInfo* cltTestingSystem::m_pclQuestionKindInfo = nullptr;

void cltTestingSystem::InitializeStaticVariable(cltQuestionKindInfo* questionKindInfo) {
    m_pclQuestionKindInfo = questionKindInfo;
}

cltTestingSystem::cltTestingSystem() = default;

void cltTestingSystem::Initialize(cltMoneySystem* moneySystem, cltLessonSystem* lessonSystem, cltGradeSystem* gradeSystem, cltQuestSystem* questSystem, int lastTestingTime) {
    m_pMoneySystem = moneySystem;
    m_pLessonSystem = lessonSystem;
    m_pGradeSystem = gradeSystem;
    m_pQuestSystem = questSystem;
    m_lastTestingTime = static_cast<std::time_t>(lastTestingTime);
    m_bInTesting = 0;
}

int cltTestingSystem::CanBeginTesting(int checkTime) {
    if (checkTime && !CanNowTesting()) {
        return 1;
    }
    if (!m_pGradeSystem->CanUpGrade()) {
        return 801;
    }
    const std::uint8_t grade = m_pGradeSystem->GetGrade();
    const int cost = GetTestingCost(grade + 1);
    return m_pMoneySystem->CanDecreaseMoney(cost) ? 0 : 800;
}

void cltTestingSystem::BeginTesting(unsigned int seed) {
    GenerateQuestions(seed);
    std::time(&m_lastTestingTime);
    m_bInTesting = 1;
}

void cltTestingSystem::GenerateQuestions(unsigned int seed) {
    std::uint16_t buffer[100]{};
    const std::uint8_t grade = m_pGradeSystem->GetGrade();
    int count = m_pclQuestionKindInfo->GetQuestions(static_cast<char>(grade + 1), buffer);

    std::srand(seed);
    for (int i = 0; i < 5; ++i) {
        const int r = std::rand();
        const int pick = (count > 0) ? (r % count) : 0;
        m_wQuestions[i] = (count > 0) ? buffer[pick] : 0;
        if (count > 0 && pick < count - 1) {
            std::memmove(&buffer[pick], &buffer[pick + 1], sizeof(std::uint16_t) * (count - pick - 1));
        }
        --count;
    }
}

std::uint16_t cltTestingSystem::GetQuestion(int index) {
    return m_wQuestions[index];
}

int cltTestingSystem::DecideAnswer(int index, std::uint16_t monsterKind, std::uint16_t npcKind) {
    const std::uint16_t question = GetQuestion(index);
    strQuestionKindInfo* info = m_pclQuestionKindInfo->GetQuestionKindInfo(question);
    if (!info) {
        return 0;
    }

    if (info->ansMonsterKind != 0) {
        if (info->ansMonsterKind != npcKind) {
            return 0;
        }
    }
    else if (info->ansNpcKind != 0 && info->ansNpcKind != monsterKind) {
        return 0;
    }

    return 1;
}

int cltTestingSystem::CanFinishTesting(int passTest, std::uint16_t* const answersMonster, std::uint16_t* const answersNpc) {
    int result = IsInTesting();
    if (!result) {
        return result;
    }

    const std::uint8_t grade = m_pGradeSystem->GetGrade();
    const int cost = GetTestingCost(grade + 1);
    if (!m_pMoneySystem->CanDecreaseMoney(cost)) {
        return 0;
    }

    if (passTest == 1) {
        for (int i = 0; i < 5; ++i) {
            if (!DecideAnswer(i, answersMonster[i], answersNpc[i])) {
                return 0;
            }
        }
    }

    return 1;
}

void cltTestingSystem::FinishTesting(int passed, int* outCost, std::uint16_t* questKinds, unsigned int* questValues) {
    const std::uint8_t grade = m_pGradeSystem->GetGrade();
    const int cost = GetTestingCost(grade + 1);

    if (outCost) {
        *outCost = cost;
    }

    m_pMoneySystem->DecreaseMoney(cost);
    if (passed) {
        m_pGradeSystem->UpGrade();
        if (m_pQuestSystem) {
            m_pQuestSystem->CompleteFunctionQuest(15, questKinds, questValues);
        }
    }

    m_bInTesting = 0;
}

int cltTestingSystem::IsInTesting() {
    return m_bInTesting;
}

int cltTestingSystem::CanNowTesting() {
    std::time_t now{};
    std::time(&now);

    const std::tm nowTm = *std::localtime(&now);
    const std::tm lastTm = *std::localtime(&m_lastTestingTime);

    if (nowTm.tm_year < lastTm.tm_year) {
        return 0;
    }
    if (nowTm.tm_year == lastTm.tm_year) {
        return nowTm.tm_yday > lastTm.tm_yday;
    }
    return 1;
}

int cltTestingSystem::GetLastTestingTime() {
    return static_cast<int>(m_lastTestingTime);
}

int cltTestingSystem::GetTestingCost(std::uint8_t grade) {
    static constexpr std::array<int, 21> kTestingCost = {
        100, 200, 300, 400, 800, 1200, 2400, 4800, 7200, 14400, 28800, 57600, 86400, 172800, 259200, 450000, 700000, 950000, 1300000, 1650000, 2000000,
    };

    if (grade <= 0x14u) {
        return kTestingCost[grade];
    }
    return 0;
}
