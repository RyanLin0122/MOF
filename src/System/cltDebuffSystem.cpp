#include "System/cltDebuffSystem.h"


cltDebuffKindInfo* cltDebuffSystem::m_pclDebuffKindInfo = nullptr;
cltTimerManager* cltDebuffSystem::m_pclTimerManager = nullptr;
void(__cdecl* cltDebuffSystem::m_pExternDebuffInitializeFuncPtr)(unsigned int, std::uintptr_t) = nullptr;
void(__cdecl* cltDebuffSystem::m_pExternDebuffPollFuncPtr)(unsigned int, std::uintptr_t) = nullptr;
void(__cdecl* cltDebuffSystem::m_pExternDebuffCustomFuncPtr)(unsigned int, std::uintptr_t) = nullptr;
void(__cdecl* cltDebuffSystem::m_pExternDebuffTimeOutFuncPtr)(unsigned int, std::uintptr_t) = nullptr;

void cltDebuffSystem::InitializeStaticVariable(cltDebuffKindInfo* debuffKindInfo, cltTimerManager* timerManager,
    void(__cdecl* onInitialize)(unsigned int, std::uintptr_t),
    void(__cdecl* onPoll)(unsigned int, std::uintptr_t),
    void(__cdecl* onCustom)(unsigned int, std::uintptr_t),
    void(__cdecl* onTimeOut)(unsigned int, std::uintptr_t)) {
    m_pclDebuffKindInfo = debuffKindInfo;
    m_pclTimerManager = timerManager;
    m_pExternDebuffInitializeFuncPtr = onInitialize;
    m_pExternDebuffPollFuncPtr = onPoll;
    m_pExternDebuffCustomFuncPtr = onCustom;
    m_pExternDebuffTimeOutFuncPtr = onTimeOut;
}

cltDebuffSystem::cltDebuffSystem() = default;

void cltDebuffSystem::Initialize(void* owner) { owner_ = owner; }

void cltDebuffSystem::Free() {
    for (int i = 0; i < debuffCount_; ++i) {
        if (entries_[i].timerID) {
            m_pclTimerManager->ReleaseTimer(entries_[i].timerID);
            entries_[i].timerID = 0;
        }
    }
    debuffCount_ = 0;
    owner_ = nullptr;
}

int cltDebuffSystem::CanAddDebuff(std::uint16_t debuffKind) {
    if (debuffCount_ >= 10) return 0;
    for (int i = 0; i < debuffCount_; ++i) {
        if (entries_[i].kind == debuffKind) return 0;
    }
    return 1;
}

void cltDebuffSystem::AddDebuff(std::uint16_t debuffKind, void* target, int isBoss) {
    strDebuffKindInfo* info = m_pclDebuffKindInfo->GetDebuffKindInfo(debuffKind);
    DebuffEntry* entry = &entries_[debuffCount_];

    entry->kind = debuffKind;
    entry->target = target;
    entry->owner = owner_;
    entry->self = this;

    const unsigned int duration = isBoss ? info->dwBossDuration : info->dwDuration;
    const unsigned int interval = isBoss ? info->dwBossRepeatCycle : info->dwRepeatCycle;

    entry->timerID = m_pclTimerManager->CreateTimer(duration, reinterpret_cast<std::uintptr_t>(entry), interval, 1,
        OnDebuffInitialize, OnDebuffPoll, OnDebuffTimeOuted, OnDebuffCustom, nullptr);

    ++debuffCount_;
}

void cltDebuffSystem::DeleteDebuff(void* target) {
    for (int i = 0; i < debuffCount_; ++i) {
        if (entries_[i].target == target) {
            if (entries_[i].timerID) {
                m_pclTimerManager->ReleaseTimer(entries_[i].timerID);
                entries_[i].timerID = 0;
            }

            std::memmove(&entries_[i], &entries_[i + 1], sizeof(DebuffEntry) * (debuffCount_ - i - 1));
            --debuffCount_;
            --i;
        }
    }
}

int cltDebuffSystem::IsDebuffed() { return debuffCount_ != 0; }

void cltDebuffSystem::DeleteAllDebuff() {
    for (int i = 0; i < debuffCount_; ++i) {
        if (entries_[i].timerID) {
            m_pclTimerManager->ReleaseTimer(entries_[i].timerID);
            entries_[i].timerID = 0;
        }
    }
    debuffCount_ = 0;
}

void cltDebuffSystem::OnDebuffTimeOuted(unsigned int timerID) {
    for (int i = 0; i < debuffCount_; ++i) {
        if (entries_[i].timerID == timerID) {
            std::memmove(&entries_[i], &entries_[i + 1], sizeof(DebuffEntry) * (debuffCount_ - i - 1));
            --debuffCount_;
            return;
        }
    }
}

void __cdecl cltDebuffSystem::OnDebuffInitialize(unsigned int timerID, std::uintptr_t timerArg) {
    if (m_pExternDebuffInitializeFuncPtr) m_pExternDebuffInitializeFuncPtr(timerID, timerArg);
}

void __cdecl cltDebuffSystem::OnDebuffPoll(unsigned int timerID, std::uintptr_t timerArg) {
    if (m_pExternDebuffPollFuncPtr) m_pExternDebuffPollFuncPtr(timerID, timerArg);
}

void __cdecl cltDebuffSystem::OnDebuffCustom(unsigned int timerID, std::uintptr_t timerArg) {
    if (m_pExternDebuffCustomFuncPtr) m_pExternDebuffCustomFuncPtr(timerID, timerArg);
}

void __cdecl cltDebuffSystem::OnDebuffTimeOuted(unsigned int timerID, std::uintptr_t timerArg) {
    cltDebuffSystem* self = reinterpret_cast<DebuffEntry*>(timerArg)->self;
    if (m_pExternDebuffTimeOutFuncPtr) m_pExternDebuffTimeOutFuncPtr(timerID, timerArg);
    self->OnDebuffTimeOuted(timerID);
}
