#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Network/CMofMsg.h"
#include "System/cltLevelSystem.h"
#include "System/cltMoneySystem.h"
#include "Logic/CQuizEventParser.h"

class CQuizEventSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo, CQuizEventParser* quizEventParser);

    CQuizEventSystem();
    ~CQuizEventSystem();

    void Initialize(cltBaseInventory* baseInventory, cltLevelSystem* levelSystem, cltMoneySystem* moneySystem,
                    std::uint16_t quizID, int playedQuiz, std::uint8_t answerSize,
                    const std::uint8_t* answerPos, const std::uint16_t* answerItemKind);

    void InitializeQuizEvent();
    void InitQuizInfo(std::uint8_t packedPos, std::uint16_t itemKind);
    void StartQuizEvent(std::uint16_t quizID);
    void BonusQuiz();

    std::uint16_t CanSetQuiz(std::uint16_t slot, std::uint16_t answerPos, std::uint16_t isConsonant);
    int SetQuiz(std::uint16_t slot, std::uint16_t answerPos);

    int CanDelQuiz();
    void DelQuiz();

    int CanGetQuiz();
    stQuizAnswer* GetQuiz();

    std::uint16_t CanCompleteQuiz();
    std::int64_t CompleteQuiz();

    double GetRewardPer(std::uint16_t level);
    std::uint16_t GetAnswerID(std::uint16_t answerPos);

    void FillOutQuiz(CMofMsg* msg);

    std::uint16_t GetQuizID();
    std::uint8_t GetAnswerSize();
    int GetPlayedQuiz();

    static cltItemKindInfo* m_pclItemKindInfo;
    static CQuizEventParser* m_pclQuizEventParser;

private:
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltMoneySystem* m_pMoneySystem = nullptr;

    std::array<stQuizAnswer, 6> m_answers{};
    std::uint16_t m_quizID = 0;
    std::uint8_t m_answerSize = 0;
    std::uint8_t m_reserved = 0;
    int m_playedQuiz = 0;
};
