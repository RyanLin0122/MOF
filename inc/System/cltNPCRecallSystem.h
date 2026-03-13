#pragma once

#include <cstdint>

class cltNPCInfo;
class cltMapInfo;
class cltMoneySystem;

class cltNPCRecallSystem {
public:
    static void InitializeStaticVariable(cltNPCInfo* npcInfo);
    static void InitializeStaticVariable(cltMapInfo* mapInfo);

    cltNPCRecallSystem();

    void Initialize(cltMoneySystem* moneySystem);
    void Free();

    unsigned int CanUseNPCRecall(std::uint16_t mapKind, std::uint16_t npcKind);
    void UseNPCRecall(std::uint16_t npcKind);

private:
    static cltNPCInfo* m_pclNPCInfo;
    static cltMapInfo* m_pclMapInfo;

    cltMoneySystem* m_pMoneySystem = nullptr;
    std::uint16_t m_wNPCRecallKind = 0;
};
