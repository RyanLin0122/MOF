#include "System/cltTransformSystem.h"

#include <cstdlib>

cltTransformKindInfo* cltTransformSystem::m_pclTransformKindInfo = nullptr;
cltTimerManager* cltTransformSystem::m_pclTimerManager = nullptr;
void(__cdecl* cltTransformSystem::m_pExternTransformBeginFuncPtr)(unsigned int, std::uint16_t) = nullptr;
void(__cdecl* cltTransformSystem::m_pExternTransformCustomFuncPtr)(unsigned int, unsigned int) = nullptr;
void(__cdecl* cltTransformSystem::m_pExternTransformFinishedFuncPtr)(unsigned int, std::uint16_t) = nullptr;

void cltTransformSystem::InitializeStaticVaraible(cltTransformKindInfo* transformKindInfo, cltTimerManager* timerManager,
    void(__cdecl* onBegin)(unsigned int, std::uint16_t),
    void(__cdecl* onCustom)(unsigned int, unsigned int),
    void(__cdecl* onFinished)(unsigned int, std::uint16_t)) {
    m_pclTransformKindInfo = transformKindInfo;
    m_pclTimerManager = timerManager;
    m_pExternTransformBeginFuncPtr = onBegin;
    m_pExternTransformCustomFuncPtr = onCustom;
    m_pExternTransformFinishedFuncPtr = onFinished;
}

cltTransformSystem::cltTransformSystem() = default;

void cltTransformSystem::Initialize(unsigned int userData1) {
    transformKind_ = 0;
    userData1_ = userData1;
}

void cltTransformSystem::Free() {
    if (timerID_) {
        m_pclTimerManager->ReleaseTimer(timerID_);
        timerID_ = 0;
    }
    transformKind_ = 0;
}

int cltTransformSystem::IsTransformed() { return transformKind_ != 0; }

void cltTransformSystem::TransformBegin(std::uint16_t transformKind) {
    if (timerID_) {
        m_pclTimerManager->ReleaseTimer(timerID_);
        timerID_ = 0;
    }

    transformKind_ = transformKind;

    const strTransformKindInfo* info = m_pclTransformKindInfo ? m_pclTransformKindInfo->GetTransfromKindInfo(transformKind) : nullptr;
    const unsigned int interval = info ? static_cast<unsigned int>(info->influenceInterval) : 0;

    if (interval && m_pExternTransformCustomFuncPtr) {
        timerID_ = m_pclTimerManager->CreateTimer(0, static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(this)), interval, 1,
            nullptr, nullptr, nullptr, m_pExternTransformCustomFuncPtr, nullptr);
    }

    if (m_pExternTransformBeginFuncPtr) m_pExternTransformBeginFuncPtr(userData1_, transformKind);
}

void cltTransformSystem::TransformFinished() {
    if (timerID_) {
        m_pclTimerManager->ReleaseTimer(timerID_);
        timerID_ = 0;
    }

    if (m_pExternTransformFinishedFuncPtr) m_pExternTransformFinishedFuncPtr(userData1_, transformKind_);
    transformKind_ = 0;
}

std::uint16_t cltTransformSystem::GetTransformKind() { return transformKind_; }
int cltTransformSystem::CanMove() { return m_pclTransformKindInfo->GetTransfromKindInfo(transformKind_)->canMove; }
int cltTransformSystem::CanAttack() { return m_pclTransformKindInfo->GetTransfromKindInfo(transformKind_)->canAttack; }
int cltTransformSystem::CanBeAttacked() { return m_pclTransformKindInfo->GetTransfromKindInfo(transformKind_)->canBeAttacked; }
unsigned int cltTransformSystem::GetUserData1() { return userData1_; }

std::uint16_t cltTransformSystem::GenerateTransformKind(unsigned int seed, std::uint16_t* const candidates) {
    int count = 0;
    while (count < 3 && candidates[count]) ++count;
    if (count <= 1) return candidates[0];

    std::srand(seed);
    return candidates[std::rand() % count];
}
