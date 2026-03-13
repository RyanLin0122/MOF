#include "System/CQuizEventSystem.h"


cltItemKindInfo* CQuizEventSystem::m_pclItemKindInfo = nullptr;
CQuizEventParser* CQuizEventSystem::m_pclQuizEventParser = nullptr;

void CQuizEventSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, CQuizEventParser* quizEventParser) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclQuizEventParser = quizEventParser;
}

CQuizEventSystem::CQuizEventSystem() = default;
CQuizEventSystem::~CQuizEventSystem() = default;

void CQuizEventSystem::Initialize(cltBaseInventory* baseInventory, cltLevelSystem* levelSystem, cltMoneySystem* moneySystem,
                                  std::uint16_t quizID, int playedQuiz, std::uint8_t answerSize,
                                  const std::uint8_t* answerPos, const std::uint16_t* answerItemKind) {
    m_pLevelSystem = levelSystem;
    m_pBaseInventory = baseInventory;
    m_pMoneySystem = moneySystem;

    InitializeQuizEvent();
    m_quizID = quizID;
    m_playedQuiz = playedQuiz;

    for (int i = 0; i < answerSize; ++i) {
        InitQuizInfo(answerPos[i], answerItemKind[i]);
    }
}

void CQuizEventSystem::InitializeQuizEvent() {
    std::memset(m_answers.data(), 0, sizeof(m_answers));
    m_quizID = 0;
    m_answerSize = 0;
}

void CQuizEventSystem::InitQuizInfo(std::uint8_t packedPos, std::uint16_t itemKind) {
    if (!itemKind) return;

    std::uint16_t col = 0;
    std::uint16_t row = 0;
    if (m_pclItemKindInfo->IsQuizItemConsonant(itemKind) == 2) {
        row = packedPos % 6;
        col = 0;
    } else {
        col = packedPos & 3;
        row = packedPos >> 2;
    }

    m_answers[row].answer[col] = itemKind;
    ++m_answerSize;
}

void CQuizEventSystem::StartQuizEvent(std::uint16_t quizID) {
    m_quizID = quizID;
    m_playedQuiz = 0;
    m_answerSize = 0;
    std::memset(m_answers.data(), 0, sizeof(m_answers));
}

void CQuizEventSystem::BonusQuiz() {
    stQuizInfo* info = m_pclQuizEventParser->GetQuizInfo(m_quizID);
    if (!info->answerLineCount) return;

    for (int line = 0; line < info->answerLineCount; ++line) {
        for (int i = 0; i < info->answerCount[line]; ++i) {
            const std::uint16_t hangleId = info->answerHangleID[line][i];
            const std::uint16_t itemKind = info->isEnglish
                                               ? m_pclItemKindInfo->GetEnglishID2ItemKind(hangleId)
                                               : m_pclItemKindInfo->GetHangleID2ItemKind(hangleId);
            if (itemKind && m_pclItemKindInfo->IsQuizItemConsonant(itemKind) == 1) {
                m_answers[line].answer[i] = itemKind;
                ++m_answerSize;
            }
        }
    }
}

std::uint16_t CQuizEventSystem::CanSetQuiz(std::uint16_t slot, std::uint16_t answerPos, std::uint16_t isConsonant) {
    if (!m_quizID) return 1400;
    if (m_playedQuiz == 1) return 1406;

    if (!m_pBaseInventory->CanDelInventoryItem(slot, 1)) return 1401;
    strInventoryItem* inv = m_pBaseInventory->GetInventoryItem(slot);
    if (!inv) return 1401;

    if (!m_pclItemKindInfo->IsQuizItem(inv->itemKind)) return 1402;

    std::uint16_t col = 0;
    std::uint16_t row = 0;
    if (m_pclItemKindInfo->IsQuizItemConsonant(inv->itemKind) == 2) {
        if (!isConsonant) return 1410;
        row = answerPos % 6;
    } else {
        if (isConsonant == 1) return 1409;
        row = answerPos >> 2;
        col = answerPos & 3;

        if (m_pclItemKindInfo->IsQuizItemConsonant(inv->itemKind) == 1 && col == 1) return 1403;
        if (!m_pclItemKindInfo->IsQuizItemConsonant(inv->itemKind) && col != 1) return 1407;
    }

    return inv->itemKind != m_answers[row].answer[col] ? 0 : 1404;
}

int CQuizEventSystem::SetQuiz(std::uint16_t slot, std::uint16_t answerPos) {
    strInventoryItem* inv = m_pBaseInventory->GetInventoryItem(slot);
    if (!inv) return 0;

    const std::uint16_t itemKind = inv->itemKind;
    m_pBaseInventory->DelInventoryItem(slot, 1, nullptr);

    std::uint16_t row = 0;
    std::uint16_t col = 0;
    if (m_pclItemKindInfo->IsQuizItemConsonant(itemKind) == 2) {
        row = answerPos % 6;
    } else {
        row = answerPos >> 2;
        col = answerPos & 3;
    }

    m_answers[row].answer[col] = itemKind;
    ++m_answerSize;
    return 1;
}

int CQuizEventSystem::CanDelQuiz() {
    return m_quizID != 0 ? 0 : 1400;
}

void CQuizEventSystem::DelQuiz() {
    stQuizInfo* info = m_pclQuizEventParser->GetQuizInfo(m_quizID);

    for (int line = 0; line < info->answerLineCount; ++line) {
        for (int i = 0; i < info->answerCount[line]; ++i) {
            if (m_answers[line].answer[i]) {
                m_answers[line].answer[i] = 0;
                --m_answerSize;
            }
        }
    }
}

int CQuizEventSystem::CanGetQuiz() {
    return m_quizID != 0 ? 0 : 1400;
}

stQuizAnswer* CQuizEventSystem::GetQuiz() {
    return m_answers.data();
}

std::uint16_t CQuizEventSystem::CanCompleteQuiz() {
    if (!m_quizID) return 1400;
    if (m_playedQuiz == 1) return 1406;

    stQuizInfo* info = m_pclQuizEventParser->GetQuizInfo(m_quizID);
    if (!info->answerLineCount) return 0;

    for (int line = 0; line < info->answerLineCount; ++line) {
        for (int i = 0; i < info->answerCount[line]; ++i) {
            if (!m_answers[line].answer[i]) return 1405;
            if (m_pclItemKindInfo->GetQuizItemHangleID(m_answers[line].answer[i]) != info->answerHangleID[line][i]) return 1405;
        }
    }

    return 0;
}

std::int64_t CQuizEventSystem::CompleteQuiz() {
    m_playedQuiz = 1;
    const std::int64_t totalExp = m_pLevelSystem->GetTotalExpOfLevel();
    const auto lv = m_pLevelSystem->GetLevel();
    const double rewardPer = GetRewardPer(lv);

    m_pMoneySystem->IncreaseMoney(0);
    InitializeQuizEvent();

    return static_cast<std::int64_t>((static_cast<double>(totalExp / 100) * rewardPer));
}

double CQuizEventSystem::GetRewardPer(std::uint16_t level) {
    if (level < 20) return 1.0;
    if (level < 40) return 3.0;
    if (level < 60) return 3.0;
    if (level < 90) return 2.0;
    return 1.0;
}

std::uint16_t CQuizEventSystem::GetAnswerID(std::uint16_t answerPos) {
    int row = answerPos >> 2;
    std::uint16_t col = answerPos & 3;

    stQuizInfo* info = m_pclQuizEventParser->GetQuizInfo(m_quizID);
    if (!info) return 0;

    if (info->isEnglish == 1) {
        col = 0;
        row = answerPos % 6;
    }

    return m_answers[row].answer[col];
}

void CQuizEventSystem::FillOutQuiz(CMofMsg* msg) {
    stQuizInfo* info = m_pclQuizEventParser->GetQuizInfo(m_quizID);

    msg->Put_WORD(m_quizID);
    msg->Put_BYTE(static_cast<std::uint8_t>(m_playedQuiz));
    msg->Put_BYTE(m_answerSize);

    if (!info->isEnglish) {
        for (int line = 0; line < 6; ++line) {
            for (int i = 0; i < 4; ++i) {
                if (m_answers[line].answer[i]) {
                    msg->Put_BYTE(static_cast<std::uint8_t>(i + 4 * line));
                    msg->Put_WORD(m_answers[line].answer[i]);
                }
            }
        }
    }

    if (info->isEnglish == 1) {
        for (int line = 0; line < 6; ++line) {
            if (m_answers[line].answer[0]) {
                msg->Put_BYTE(static_cast<std::uint8_t>(line));
                msg->Put_WORD(m_answers[line].answer[0]);
            }
        }
    }
}

std::uint16_t CQuizEventSystem::GetQuizID() { return m_quizID; }
std::uint8_t CQuizEventSystem::GetAnswerSize() { return m_answerSize; }
int CQuizEventSystem::GetPlayedQuiz() { return m_playedQuiz; }
