#pragma once

#include <cstdint>

#include "Info/cltTransformKindInfo.h"
#include "Util/cltTimer.h"

class cltTransformSystem {
public:
    static void InitializeStaticVaraible(cltTransformKindInfo* transformKindInfo, cltTimerManager* timerManager,
        void(__cdecl* onBegin)(unsigned int, std::uint16_t),
        void(__cdecl* onCustom)(unsigned int, unsigned int),
        void(__cdecl* onFinished)(unsigned int, std::uint16_t));

    cltTransformSystem();

    void Initialize(unsigned int userData1);
    void Free();
    int IsTransformed();
    void TransformBegin(std::uint16_t transformKind);
    void TransformFinished();
    std::uint16_t GetTransformKind();
    int CanMove();
    int CanAttack();
    int CanBeAttacked();
    unsigned int GetUserData1();

    static std::uint16_t GenerateTransformKind(unsigned int seed, std::uint16_t* const candidates);

private:
    std::uint16_t transformKind_ = 0;
    std::uint16_t pad_ = 0;
    unsigned int timerID_ = 0;
    unsigned int userData1_ = 0;

    static cltTransformKindInfo* m_pclTransformKindInfo;
    static cltTimerManager* m_pclTimerManager;
    static void(__cdecl* m_pExternTransformBeginFuncPtr)(unsigned int, std::uint16_t);
    static void(__cdecl* m_pExternTransformCustomFuncPtr)(unsigned int, unsigned int);
    static void(__cdecl* m_pExternTransformFinishedFuncPtr)(unsigned int, std::uint16_t);
};
