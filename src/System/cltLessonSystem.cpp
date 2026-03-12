#include "System/cltLessonSystem.h"

#include <algorithm>
#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Info/cltLessonKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "Network/CMofMsg.h"
#include "System/cltEmblemSystem.h"

cltLessonKindInfo* cltLessonSystem::m_pclLessonKindInfo = nullptr;
cltItemKindInfo* cltLessonSystem::m_pclItemKindInfo = nullptr;

cltLessonSystem::~cltLessonSystem() {
    Free();
}

void cltLessonSystem::InitializeStaticVariable(cltLessonKindInfo* lessonKindInfo, cltItemKindInfo* itemKindInfo) {
    m_pclLessonKindInfo = lessonKindInfo;
    m_pclItemKindInfo = itemKindInfo;
}

void cltLessonSystem::Initialize(cltBaseInventory* baseInventory, cltEmblemSystem* emblemSystem,
                                 std::uint16_t traningItemKind, const std::uint8_t* lessonSchedule,
                                 const std::uint8_t* lessonState, unsigned int swordPt, unsigned int bowPt,
                                 unsigned int theologyPt, unsigned int magicPt, unsigned int totalSwordPt,
                                 unsigned int totalBowPt, unsigned int totalTheologyPt, unsigned int totalMagicPt,
                                 const strLessonHistory (*lessonHistory)[3]) {
    if (!lessonSchedule || !lessonState || !lessonHistory) return;

    m_pBaseInventory = baseInventory;
    m_pEmblemSystem = emblemSystem;
    m_currentSwordLessonPt = swordPt;
    m_currentBowLessonPt = bowPt;
    m_currentTheologyLessonPt = theologyPt;
    m_currentMagicLessonPt = magicPt;
    m_totalSwordLessonPt = totalSwordPt;
    m_totalBowLessonPt = totalBowPt;
    m_totalTheologyLessonPt = totalTheologyPt;
    m_totalMagicLessonPt = totalMagicPt;
    m_wTraningItemKind = traningItemKind;

    std::memcpy(m_lessonSchedule.data(), lessonSchedule, 4);
    std::memcpy(m_lessonState.data(), lessonState, 4);
    std::memcpy(m_lessonHistory.data(), lessonHistory, sizeof(m_lessonHistory));
}

void cltLessonSystem::Initialize(cltBaseInventory* baseInventory, CMofMsg* msg) {
    m_pBaseInventory = baseInventory;
    msg->Get_WORD(&m_wTraningItemKind);
    msg->Get(m_lessonSchedule.data(), 4);
    msg->Get(m_lessonState.data(), 4);
    msg->Get_DWORD(&m_currentSwordLessonPt);
    msg->Get_DWORD(&m_currentBowLessonPt);
    msg->Get_DWORD(&m_currentTheologyLessonPt);
    msg->Get_DWORD(&m_currentMagicLessonPt);
    msg->Get_DWORD(&m_totalSwordLessonPt);
    msg->Get_DWORD(&m_totalBowLessonPt);
    msg->Get_DWORD(&m_totalTheologyLessonPt);
    msg->Get_DWORD(&m_totalMagicLessonPt);
    msg->Get(reinterpret_cast<unsigned char*>(m_lessonHistory.data()), sizeof(m_lessonHistory));
}

void cltLessonSystem::Free() {
    m_pBaseInventory = nullptr;
    m_pEmblemSystem = nullptr;
    std::fill(m_lessonSchedule.begin(), m_lessonSchedule.end(), 0);
    std::fill(m_lessonState.begin(), m_lessonState.end(), 0);
}

std::uint8_t* cltLessonSystem::GetLessonSchedule() { return m_lessonSchedule.data(); }
std::uint8_t cltLessonSystem::GetLessonSchedule(int index) { return m_lessonSchedule[static_cast<std::size_t>(index)]; }
std::uint8_t* cltLessonSystem::GetLessonState() { return m_lessonState.data(); }
std::uint8_t cltLessonSystem::GetLessonState(int index) { return m_lessonState[static_cast<std::size_t>(index)]; }

int cltLessonSystem::CanCancelSchedule() { return 1; }

void cltLessonSystem::CancelSchedule() {
    m_wTraningItemKind = 0;
    std::fill(m_lessonSchedule.begin(), m_lessonSchedule.end(), 0);
}

strLessonHistory* cltLessonSystem::GetLessonHistory(std::uint8_t lessonType, std::uint8_t rankOrMode) {
    return &m_lessonHistory[lessonType][rankOrMode];
}

std::uint16_t cltLessonSystem::GetTraningItemKind() { return m_wTraningItemKind; }

int cltLessonSystem::CanSetupSchedule(int traningItemKind, const std::uint8_t* lessonSchedule) {
    int validCount = 0;

    if (!(m_pEmblemSystem->GetTraningCardItemKind() && m_pEmblemSystem->GetTraningCardItemKind() == static_cast<std::uint16_t>(traningItemKind))) {
        if (!m_pclItemKindInfo->IsTraningCard(static_cast<unsigned short>(traningItemKind))) return 303;
        if (!m_pBaseInventory->GetInventoryItemQuantity(traningItemKind)) return 303;
        if (m_pBaseInventory->IsLock() == 1) return 1;
    }

    for (int i = 0; i < 4; ++i) {
        const std::uint8_t kind = lessonSchedule[i];
        if (kind == 0) return validCount != 0 ? 0 : 300;
        if (!m_pclLessonKindInfo->IsValidLessonKind(kind)) return 304;
        ++validCount;
    }

    return validCount != 0 ? 0 : 300;
}

void cltLessonSystem::SetupSchedule(std::uint16_t traningItemKind, const std::uint8_t* lessonSchedule, cltItemList* itemList, std::uint16_t* outUsingCardKind) {
    int consumeCard = 1;
    if (m_pEmblemSystem && m_pEmblemSystem->GetTraningCardItemKind() && m_pEmblemSystem->GetTraningCardItemKind() == traningItemKind) consumeCard = 0;

    std::memcpy(m_lessonSchedule.data(), lessonSchedule, 4);
    for (int i = 0; i < 4; ++i) {
        m_lessonState[i] = m_lessonSchedule[i] ? 1 : 0;
    }

    m_wTraningItemKind = traningItemKind;
    if (consumeCard) {
        m_pBaseInventory->DelInventoryItemKind(traningItemKind, 1u, itemList, nullptr);
        if (outUsingCardKind) *outUsingCardKind = 0;
    } else if (outUsingCardKind) {
        *outUsingCardKind = m_pEmblemSystem ? m_pEmblemSystem->GetTraningCardItemKind() : 0;
    }
}

int cltLessonSystem::IsEmptySchedule() {
    for (std::uint8_t kind : m_lessonSchedule) if (kind != 0) return 0;
    return 1;
}

std::uint32_t cltLessonSystem::GetSwordLessonPt() { return m_currentSwordLessonPt; }
std::uint32_t cltLessonSystem::GetBowLessonPt() { return m_currentBowLessonPt; }
std::uint32_t cltLessonSystem::GetTheologyLessonPt() { return m_currentTheologyLessonPt; }
std::uint32_t cltLessonSystem::GetMagicLessonPt() { return m_currentMagicLessonPt; }

std::uint32_t cltLessonSystem::GetTotalSwordLessonPt() { return m_totalSwordLessonPt; }
std::uint32_t cltLessonSystem::GetTotalBowLessonPt() { return m_totalBowLessonPt; }
std::uint32_t cltLessonSystem::GetTotalTheologyLessonPt() { return m_totalTheologyLessonPt; }
std::uint32_t cltLessonSystem::GetTotalMagicLessonPt() { return m_totalMagicLessonPt; }

int cltLessonSystem::GetTotalAllLessonPt() {
    return static_cast<int>(m_totalSwordLessonPt + m_totalBowLessonPt + m_totalTheologyLessonPt + m_totalMagicLessonPt);
}

static constexpr std::uint32_t kMaxLessonTotal = 2000000000U;

void cltLessonSystem::IncLessonPt_Sword(unsigned int value) {
    m_currentSwordLessonPt += value;
    std::uint32_t total = m_totalSwordLessonPt + value;
    m_totalSwordLessonPt = (total >= 0x77359400U) ? kMaxLessonTotal : total;
}
void cltLessonSystem::IncLessonPt_Bow(unsigned int value) {
    m_currentBowLessonPt += value;
    std::uint32_t total = m_totalBowLessonPt + value;
    m_totalBowLessonPt = (total >= 0x77359400U) ? kMaxLessonTotal : total;
}
void cltLessonSystem::IncLessonPt_Theology(unsigned int value) {
    m_currentTheologyLessonPt += value;
    std::uint32_t total = m_totalTheologyLessonPt + value;
    m_totalTheologyLessonPt = (total >= 0x77359400U) ? kMaxLessonTotal : total;
}
void cltLessonSystem::IncLessonPt_Magic(unsigned int value) {
    m_currentMagicLessonPt += value;
    std::uint32_t total = m_totalMagicLessonPt + value;
    m_totalMagicLessonPt = (total >= 0x77359400U) ? kMaxLessonTotal : total;
}

void cltLessonSystem::DecLessonPt_Sword(unsigned int value) { m_currentSwordLessonPt = (m_currentSwordLessonPt > value) ? (m_currentSwordLessonPt - value) : 0; }
void cltLessonSystem::DecLessonPt_Bow(unsigned int value) { m_currentBowLessonPt = (m_currentBowLessonPt > value) ? (m_currentBowLessonPt - value) : 0; }
void cltLessonSystem::DecLessonPt_Theology(unsigned int value) { m_currentTheologyLessonPt = (m_currentTheologyLessonPt > value) ? (m_currentTheologyLessonPt - value) : 0; }
void cltLessonSystem::DecLessonPt_Magic(unsigned int value) { m_currentMagicLessonPt = (m_currentMagicLessonPt > value) ? (m_currentMagicLessonPt - value) : 0; }

int cltLessonSystem::CanTraningLesson(unsigned int lessonKind) {
    if (IsEmptySchedule() == 1) return 300;

    int idx = 0;
    while (idx < 4 && m_lessonState[idx] != 1) {
        if (++idx >= 4) return 1;
    }
    return lessonKind != m_lessonSchedule[idx] ? 0x12E : 0;
}

int cltLessonSystem::CanTraningLessonByType(std::uint8_t lessonType) {
    if (IsEmptySchedule() == 1) return 300;

    int idx = 0;
    while (idx < 4 && m_lessonState[idx] != 1) {
        if (++idx >= 4) return 1;
    }

    strLessonKindInfo* info = m_pclLessonKindInfo->GetLessonKindInfo(m_lessonSchedule[idx]);
    return (info->bLessonType != lessonType) ? 0x12E : 0;
}

int cltLessonSystem::TraningLessonFinished(unsigned int seed, std::uint8_t hitType, int success, unsigned int* inoutPt) {
    int idx = 0;
    while (idx < 4 && m_lessonState[idx] != 1) {
        if (++idx >= 4) {
            goto finish_check;
        }
    }
    {
        strLessonKindInfo* lessonInfo = m_pclLessonKindInfo->GetLessonKindInfo(m_lessonSchedule[idx]);
        switch (lessonInfo->bLessonType) {
            case 0: {
                const unsigned int p = *inoutPt;
                *inoutPt = p + p * m_pEmblemSystem->GetSwordLessonPtAdvantage() / 1000;
                IncLessonPt_Sword(*inoutPt);
                m_pEmblemSystem->OnEvent_CompleteSwordLesson(seed);
                break;
            }
            case 1: {
                const unsigned int p = *inoutPt;
                *inoutPt = p + p * m_pEmblemSystem->GetBowLessonPtAdvantage() / 1000;
                IncLessonPt_Bow(*inoutPt);
                m_pEmblemSystem->OnEvent_CompleteBowLesson(seed);
                break;
            }
            case 2: {
                const unsigned int p = *inoutPt;
                *inoutPt = p + p * m_pEmblemSystem->GetMagicLessonPtAdvantage() / 1000;
                IncLessonPt_Magic(*inoutPt);
                m_pEmblemSystem->OnEvent_CompleteMagicLesson(seed);
                break;
            }
            case 3: {
                const unsigned int p = *inoutPt;
                *inoutPt = p + p * m_pEmblemSystem->GetTheologyLessonPtAdvantage() / 1000;
                IncLessonPt_Theology(*inoutPt);
                m_pEmblemSystem->OnEvent_CompleteTheologyLesson(seed);
                break;
            }
            default: break;
        }

        std::uint8_t mapped = hitType;
        if (mapped == 1) mapped = 0;
        else if (mapped == 2) mapped = 1;
        else if (mapped == 4) mapped = 2;

        strLessonHistory* hist = GetLessonHistory(lessonInfo->bLessonType, mapped);
        if (success == 1) {
            m_lessonState[idx] = 3;
            if (hist) ++hist->successCount;
        } else {
            m_lessonState[idx] = 2;
            if (hist) ++hist->failCount;
        }
    }

finish_check:
    if (idx != 3 && m_lessonState[idx + 1]) return 0;
    ScheduleFinished();
    return 1;
}

void cltLessonSystem::ScheduleFinished() {
    m_wTraningItemKind = 0;
    std::fill(m_lessonSchedule.begin(), m_lessonSchedule.end(), 0);
    std::fill(m_lessonState.begin(), m_lessonState.end(), 0);
}

int cltLessonSystem::GetThisLessonInfo(std::uint8_t* outIndex, std::uint8_t* outType, std::uint8_t* outKind) {
    if (IsEmptySchedule() == 1) return 0;

    int idx = 0;
    while (idx < 4 && m_lessonState[idx] != 1) {
        if (++idx >= 4) return 0;
    }

    strLessonKindInfo* info = m_pclLessonKindInfo->GetLessonKindInfo(m_lessonSchedule[idx]);

    *outIndex = static_cast<std::uint8_t>(idx);
    *outType = info->bLessonType;
    *outKind = info->bClassType;
    return 1;
}

void cltLessonSystem::FillOutLessonInfo(CMofMsg* msg) {
    msg->Put_WORD(m_wTraningItemKind);
    msg->Put(reinterpret_cast<char*>(m_lessonSchedule.data()), 4);
    msg->Put(reinterpret_cast<char*>(m_lessonState.data()), 4);
    msg->Put_DWORD(m_currentSwordLessonPt);
    msg->Put_DWORD(m_currentBowLessonPt);
    msg->Put_DWORD(m_currentTheologyLessonPt);
    msg->Put_DWORD(m_currentMagicLessonPt);
    msg->Put_DWORD(m_totalSwordLessonPt);
    msg->Put_DWORD(m_totalBowLessonPt);
    msg->Put_DWORD(m_totalTheologyLessonPt);
    msg->Put_DWORD(m_totalMagicLessonPt);
    msg->Put(reinterpret_cast<char*>(m_lessonHistory.data()), sizeof(m_lessonHistory));
}
