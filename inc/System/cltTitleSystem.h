#pragma once

#include <cstdint>

#include "Info/cltCharKindInfo.h"
#include "Info/cltQuestKindInfo.h"
#include "Info/cltTitleKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltLevelSystem.h"

class cltTitleSystem {
public:
    static void InitializeStaticVariable(cltTitleKindInfo*, cltQuestKindInfo*, cltCharKindInfo*, void (*)(void*, std::uint16_t));

    cltTitleSystem();

    void Initialize(void*, std::uint16_t, cltLevelSystem*, cltBaseInventory*);
    void Free();
    int CanSetTitleKind(std::uint16_t);
    void SetTitleKind(std::uint16_t);
    std::uint16_t GetTitleKind();
    std::uint16_t GetNewTitleKind();
    void OnNewTitle(std::uint16_t);
    void OnEvent_killmonster(std::uint16_t, int);
    void OnEvent_clearquest(std::uint16_t);
    void OnEvent_setemblem(std::uint16_t);
    void OnEvent_acquireskill(std::uint16_t);
    void OnEvent_getitem(int, int);

private:
    std::uint16_t titleKind_{};
    void* owner_{};
    cltLevelSystem* levelSystem_{};
    cltBaseInventory* inventory_{};
    std::uint16_t newTitleKind_{};

    static cltTitleKindInfo* m_pclTitleKindInfo;
    static cltQuestKindInfo* m_pclQuestKindInfo;
    static cltCharKindInfo* m_pclCharKindInfo;
    static void (*m_pOnNewTitleFuncPtr)(void*, std::uint16_t);
};
