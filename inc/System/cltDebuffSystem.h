#pragma once

#include <cstdint>
#include <cstring>
#include "Info/cltDebuffKindInfo.h"
#include "Util/cltTimer.h"

class cltDebuffSystem {
public:
    static void InitializeStaticVariable(cltDebuffKindInfo* debuffKindInfo, cltTimerManager* timerManager,
        void(__cdecl* onInitialize)(unsigned int, std::uintptr_t),
        void(__cdecl* onPoll)(unsigned int, std::uintptr_t),
        void(__cdecl* onCustom)(unsigned int, std::uintptr_t),
        void(__cdecl* onTimeOut)(unsigned int, std::uintptr_t));

    cltDebuffSystem();

    void Initialize(void* owner);
    void Free();
    int CanAddDebuff(std::uint16_t debuffKind);
    void AddDebuff(std::uint16_t debuffKind, void* target, int isBoss);
    void DeleteDebuff(void* target);
    int IsDebuffed();
    void DeleteAllDebuff();
    void OnDebuffTimeOuted(unsigned int timerID);

    static void __cdecl OnDebuffInitialize(unsigned int timerID, std::uintptr_t timerArg);
    static void __cdecl OnDebuffPoll(unsigned int timerID, std::uintptr_t timerArg);
    static void __cdecl OnDebuffCustom(unsigned int timerID, std::uintptr_t timerArg);
    static void __cdecl OnDebuffTimeOuted(unsigned int timerID, std::uintptr_t timerArg);

private:
    struct DebuffEntry {
        unsigned int timerID;
        std::uint16_t kind;
        std::uint16_t pad;
        void* target;
        void* owner;
        cltDebuffSystem* self;
    };

    DebuffEntry entries_[10]{};
    int debuffCount_ = 0;
    void* owner_ = nullptr;

    static cltDebuffKindInfo* m_pclDebuffKindInfo;
    static cltTimerManager* m_pclTimerManager;
    static void(__cdecl* m_pExternDebuffInitializeFuncPtr)(unsigned int, std::uintptr_t);
    static void(__cdecl* m_pExternDebuffPollFuncPtr)(unsigned int, std::uintptr_t);
    static void(__cdecl* m_pExternDebuffCustomFuncPtr)(unsigned int, std::uintptr_t);
    static void(__cdecl* m_pExternDebuffTimeOutFuncPtr)(unsigned int, std::uintptr_t);
};
