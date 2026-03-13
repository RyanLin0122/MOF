#include "System/cltNPCRecallSystem.h"

#include "Info/cltMapInfo.h"
#include "Info/cltNPCInfo.h"
#include "System/cltMoneySystem.h"

cltNPCInfo* cltNPCRecallSystem::m_pclNPCInfo = nullptr;
cltMapInfo* cltNPCRecallSystem::m_pclMapInfo = nullptr;

void cltNPCRecallSystem::InitializeStaticVariable(cltNPCInfo* npcInfo) {
    m_pclNPCInfo = npcInfo;
}

void cltNPCRecallSystem::InitializeStaticVariable(cltMapInfo* mapInfo) {
    m_pclMapInfo = mapInfo;
}

cltNPCRecallSystem::cltNPCRecallSystem() = default;

void cltNPCRecallSystem::Initialize(cltMoneySystem* moneySystem) {
    m_pMoneySystem = moneySystem;
}

void cltNPCRecallSystem::Free() {
    m_wNPCRecallKind = 0;
    m_pMoneySystem = nullptr;
}

unsigned int cltNPCRecallSystem::CanUseNPCRecall(std::uint16_t mapKind, std::uint16_t npcKind) {
    const std::uint16_t areaType = m_pclMapInfo->GetAreaVorF(mapKind);
    if (areaType != 2 && areaType != 4 && areaType != 5 && areaType != 6) {
        return 1600;
    }

    const int totalNPC = m_pclNPCInfo->GetTotalNPCNum();
    if (totalNPC <= 0) {
        return m_pMoneySystem->CanDecreaseMoney(1000) ? 0u : 1602u;
    }

    for (int i = 0; i < totalNPC; ++i) {
        stNPCInfo* info = m_pclNPCInfo->GetNPCInfoByIndex(i);
        if (info && info->m_wKind == npcKind && info->m_dwNoRecall != 1) {
            return 1601;
        }
    }

    return m_pMoneySystem->CanDecreaseMoney(1000) ? 0u : 1602u;
}

void cltNPCRecallSystem::UseNPCRecall(std::uint16_t npcKind) {
    m_pMoneySystem->DecreaseMoney(1000);
    m_wNPCRecallKind = npcKind;
}
