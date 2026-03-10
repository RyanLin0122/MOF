#include "System/cltUsingItemSystem.h"

#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Info/cltMapInfo.h"
#include "Logic/cltBaseInventory.h"

cltItemKindInfo* cltUsingItemSystem::m_pclItemKindInfo = nullptr;
cltPandoraKindInfo* cltUsingItemSystem::m_pclPandoraKindInfo = nullptr;
cltPetKindInfo* cltUsingItemSystem::m_pclPetKindInfo = nullptr;
cltTimerManager* cltUsingItemSystem::m_pclTimerManager = nullptr;
int (*cltUsingItemSystem::m_pIsPlayerDeadFuncPtr)(void*) = nullptr;
void (*cltUsingItemSystem::m_pResurrectPlayerFuncPtr)(void*) = nullptr;
void (*cltUsingItemSystem::m_pShoutFuncPtr)(int, void*, char*) = nullptr;
void (*cltUsingItemSystem::m_pUseMapUsingItemFuncPtr)(void*, std::uint16_t) = nullptr;
int (*cltUsingItemSystem::m_pIsRelayServerConnectedFuncPtr)() = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanUsePostItItemFuncPtr)(void*, std::uint16_t, char*) = nullptr;
void (*cltUsingItemSystem::m_pPostItFuncPtr)(int, void*, char*, char*) = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanPlayerCureFuncPtr)(void*) = nullptr;
int (*cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pPlayerCureFuncPtr)(void*) = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanChangeCoupleRingFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pChangeCoupleRingFuncPtr)(void*, std::uint16_t, int) = nullptr;
int (*cltUsingItemSystem::m_pCanUseChangeSexItemFuncPtr)(void*) = nullptr;
void (*cltUsingItemSystem::m_pOpenGiftBoxFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemInitializeFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemPollFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemCustomFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemTimeOutFuncPtr)(unsigned int, unsigned int) = nullptr;

namespace {
constexpr int kUsingItemSlotCount = 10;
}

void cltUsingItemSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltPandoraKindInfo* pandoraKindInfo, cltPetKindInfo* petKindInfo, cltTimerManager* timerManager,
    int (*isPlayerDeadFunc)(void*), void (*resurrectPlayerFunc)(void*), void (*shoutFunc)(int, void*, char*), void (*openGiftBoxFunc)(void*, std::uint16_t), int (*isRelayServerConnectedFunc)(),
    unsigned int (*canUsePostItItemFunc)(void*, std::uint16_t, char*), void (*postItFunc)(int, void*, char*, char*), int (*canUseChangeSexItemFunc)(void*),
    int (*canUseMapUsingItemFunc)(void*, std::uint16_t), void (*useMapUsingItemFunc)(void*, std::uint16_t), unsigned int (*canPlayerCureFunc)(void*), void (*playerCureFunc)(void*),
    unsigned int (*canChangeCoupleRingFunc)(void*, std::uint16_t), void (*changeCoupleRingFunc)(void*, std::uint16_t, int), void (*externUsingItemInitializeFunc)(unsigned int, unsigned int),
    void (*externUsingItemPollFunc)(unsigned int, unsigned int), void (*externUsingItemCustomFunc)(unsigned int, unsigned int), void (*externUsingItemTimeOutFunc)(unsigned int, unsigned int)) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclPandoraKindInfo = pandoraKindInfo;
    m_pclPetKindInfo = petKindInfo;
    m_pclTimerManager = timerManager;
    m_pIsPlayerDeadFuncPtr = isPlayerDeadFunc;
    m_pResurrectPlayerFuncPtr = resurrectPlayerFunc;
    m_pShoutFuncPtr = shoutFunc;
    m_pOpenGiftBoxFuncPtr = openGiftBoxFunc;
    m_pIsRelayServerConnectedFuncPtr = isRelayServerConnectedFunc;
    m_pCanUsePostItItemFuncPtr = canUsePostItItemFunc;
    m_pPostItFuncPtr = postItFunc;
    m_pCanUseChangeSexItemFuncPtr = canUseChangeSexItemFunc;
    m_pCanUseMapUsingItemFuncPtr = canUseMapUsingItemFunc;
    m_pUseMapUsingItemFuncPtr = useMapUsingItemFunc;
    m_pCanPlayerCureFuncPtr = canPlayerCureFunc;
    m_pPlayerCureFuncPtr = playerCureFunc;
    m_pCanChangeCoupleRingFuncPtr = canChangeCoupleRingFunc;
    m_pChangeCoupleRingFuncPtr = changeCoupleRingFunc;
    m_pExternUsingItemInitializeFuncPtr = externUsingItemInitializeFunc;
    m_pExternUsingItemPollFuncPtr = externUsingItemPollFunc;
    m_pExternUsingItemCustomFuncPtr = externUsingItemCustomFunc;
    m_pExternUsingItemTimeOutFuncPtr = externUsingItemTimeOutFunc;
}

cltUsingItemSystem::cltUsingItemSystem() = default;

void cltUsingItemSystem::Initialize(void* owner, cltBaseInventory* baseInventory, cltPlayerAbility* playerAbility, cltSpecialtySystem* specialtySystem, cltMakingItemSystem* makingItemSystem, cltUsingSkillSystem* usingSkillSystem, cltWorkingPassiveSkillSystem* workingPassiveSkillSystem, cltEmblemSystem* emblemSystem, cltSkillSystem* skillSystem, cltClassSystem* classSystem, cltSexSystem* sexSystem, cltEquipmentSystem* equipmentSystem, cltBasicAppearSystem* basicAppearSystem, cltPetSystem* petSystem, cltMyItemSystem* myItemSystem, cltMarriageSystem* marriageSystem) {
    m_pOwner = owner;
    m_pBaseInventory = baseInventory;
    m_pPlayerAbility = playerAbility;
    m_pSpecialtySystem = specialtySystem;
    m_pMakingItemSystem = makingItemSystem;
    m_pUsingSkillSystem = usingSkillSystem;
    m_pWorkingPassiveSkillSystem = workingPassiveSkillSystem;
    m_pEmblemSystem = emblemSystem;
    m_pSkillSystem = skillSystem;
    m_pClassSystem = classSystem;
    m_pSexSystem = sexSystem;
    m_pEquipmentSystem = equipmentSystem;
    m_pBasicAppearSystem = basicAppearSystem;
    m_pPetSystem = petSystem;
    m_pMyItemSystem = myItemSystem;
    m_pMarriageSystem = marriageSystem;
    std::memset(m_usingItems, 0, sizeof(m_usingItems));
}

void cltUsingItemSystem::Free() {
    m_pOwner = nullptr;
    m_pBaseInventory = nullptr;
    m_pPlayerAbility = nullptr;
    m_pSpecialtySystem = nullptr;
    m_pMakingItemSystem = nullptr;
    m_pUsingSkillSystem = nullptr;
    m_pWorkingPassiveSkillSystem = nullptr;
    m_pEmblemSystem = nullptr;
    m_pSkillSystem = nullptr;
    m_pClassSystem = nullptr;
    m_pSexSystem = nullptr;
    m_pEquipmentSystem = nullptr;
    m_pBasicAppearSystem = nullptr;
    m_pPetSystem = nullptr;
    m_pMyItemSystem = nullptr;
    m_pMarriageSystem = nullptr;
    std::memset(m_usingItems, 0, sizeof(m_usingItems));
}

bool cltUsingItemSystem::IsUseItem(std::uint16_t itemKind) {
    return m_pclItemKindInfo && m_pclItemKindInfo->IsUseItem(itemKind);
}

bool cltUsingItemSystem::IsSpecialUseItem(std::uint16_t itemKind) {
    return m_pclItemKindInfo && m_pclItemKindInfo->GetSpecialUseItem(itemKind) != 0;
}

std::uint8_t cltUsingItemSystem::GetSpecialItemType(std::uint16_t itemKind) {
    return m_pclItemKindInfo ? m_pclItemKindInfo->GetSpecialUseItem(itemKind) : 0;
}

bool cltUsingItemSystem::CanUseReturnItem(std::uint16_t itemKind) { return IsUseItem(itemKind); }
bool cltUsingItemSystem::CanUseTeleportItem(std::uint16_t itemKind) { return IsUseItem(itemKind); }
bool cltUsingItemSystem::CanMoveTeleportItem(std::uint16_t itemKind) { return IsUseItem(itemKind); }
bool cltUsingItemSystem::CanUseTeleportDragon(std::uint16_t itemKind) { return IsUseItem(itemKind); }
bool cltUsingItemSystem::CanUseTakeShip(std::uint16_t itemKind) { return IsUseItem(itemKind); }
BOOL cltUsingItemSystem::CanUseTownPortalItem(std::uint16_t itemKind) { return IsUseItem(itemKind); }

int cltUsingItemSystem::CanUseItem(std::uint16_t itemKind, int a3, char* a4, char* a5, cltPartySystem* a6, int a7, unsigned int a8, unsigned int a9, unsigned int a10, unsigned int a11) {
    return IsUseItem(itemKind) ? 0 : 1;
}

void cltUsingItemSystem::UseItem(std::uint16_t itemKind, unsigned int itemValue) {
    strUsingItemInfo* usingItemInfo = GetEmptyUsingItemInfo();
    if (!usingItemInfo) {
        return;
    }
    usingItemInfo->itemKind = itemKind;
    usingItemInfo->value0 = itemValue;
}

std::uint16_t cltUsingItemSystem::UpdateCoinItem(int a2, int a3, std::uint16_t a4, std::uint8_t* a5) { return a4; }
std::uint16_t cltUsingItemSystem::ChangeCoinItem(int a2, std::uint8_t* a3) { return 0; }

int cltUsingItemSystem::UseItem(std::uint16_t itemKind, int a3, unsigned int a4, char* a5, char* a6, char* a7, std::int16_t a8, char a9, std::uint16_t a10, std::uint16_t a11, std::uint16_t a12, std::uint16_t* a13, int* a14, std::uint16_t* a15, int* a16, std::uint16_t* a17, cltPartySystem* a18, unsigned int Seed, int a20, std::uint16_t* a21, std::uint16_t* a22, int* a23, int* a24, std::uint8_t* a25) {
    UseItem(itemKind, a4);
    return 1;
}

int cltUsingItemSystem::CanUseSustainedTypeItemWithoutItem(std::uint16_t itemKind) { return IsUseItem(itemKind); }
void cltUsingItemSystem::UseSustainedTypeItemWithoutItem(std::uint16_t itemKind, unsigned int itemValue) { UseItem(itemKind, itemValue); }
strUsingItemInfo* cltUsingItemSystem::GetUsingItemInfo() { return m_usingItems; }
strUsingItemInfo* cltUsingItemSystem::GetUsingItemInfo(unsigned int index) { return index < kUsingItemSlotCount ? &m_usingItems[index] : nullptr; }

std::int16_t cltUsingItemSystem::GetTotalStrOfUsingItem() {
    int totalStr = 0;
    for (const auto& usingItemInfo : m_usingItems) totalStr += usingItemInfo.addStr;
    return static_cast<std::int16_t>(totalStr);
}

std::int16_t cltUsingItemSystem::GetTotalDexOfUsingItem() {
    int totalDex = 0;
    for (const auto& usingItemInfo : m_usingItems) totalDex += usingItemInfo.addDex;
    return static_cast<std::int16_t>(totalDex);
}

std::int16_t cltUsingItemSystem::GetTotalIntOfUsingItem() {
    int totalInt = 0;
    for (const auto& usingItemInfo : m_usingItems) totalInt += usingItemInfo.addInt;
    return static_cast<std::int16_t>(totalInt);
}

std::int16_t cltUsingItemSystem::GetTotalVitOfUsingItem() {
    int totalVit = 0;
    for (const auto& usingItemInfo : m_usingItems) totalVit += usingItemInfo.addVit;
    return static_cast<std::int16_t>(totalVit);
}

void cltUsingItemSystem::OnUsingItemInitialize(unsigned int a1, cltUsingItemSystem* a2) {}
void cltUsingItemSystem::OnUsingItemPoll(unsigned int a1, cltUsingItemSystem* a2) {}
void cltUsingItemSystem::OnUsingItemCustom(unsigned int a1, cltUsingItemSystem* a2) {}
void cltUsingItemSystem::OnUsingItemTimeOuted(unsigned int a1, cltUsingItemSystem* a2) {}

int cltUsingItemSystem::IsAlreadyUsedItem(std::uint16_t itemKind) {
    for (const auto& usingItemInfo : m_usingItems) {
        if (usingItemInfo.itemKind == itemKind) {
            return 1;
        }
    }
    return 0;
}

strUsingItemInfo* cltUsingItemSystem::GetEmptyUsingItemInfo() {
    for (auto& usingItemInfo : m_usingItems) {
        if (!usingItemInfo.itemKind) {
            return &usingItemInfo;
        }
    }
    return nullptr;
}

int cltUsingItemSystem::GetUsingItemNum() {
    int usingItemCount = 0;
    for (const auto& usingItemInfo : m_usingItems) {
        if (usingItemInfo.itemKind) {
            ++usingItemCount;
        }
    }
    return usingItemCount;
}

int cltUsingItemSystem::IsActiveFastRun() { return 0; }
int cltUsingItemSystem::GetMaxHPAdvantage() { return 0; }
int cltUsingItemSystem::GetMaxManaAdvantage() { return 0; }
int cltUsingItemSystem::GetHitRateAdvantage() { return 0; }
int cltUsingItemSystem::GetCriticalHitRateAdvantage() { return 0; }
int cltUsingItemSystem::GetMissRateAdvantage() { return 0; }
int cltUsingItemSystem::GetAPowerAdvantage() { return 0; }
int cltUsingItemSystem::GetDPowerAdvantage() { return 0; }

stItemKindInfo* cltUsingItemSystem::IsCosmeticItem(std::uint16_t itemKind) {
    return m_pclItemKindInfo ? m_pclItemKindInfo->GetItemKindInfo(itemKind) : nullptr;
}

void cltUsingItemSystem::OnUsingItemTimeOuted(unsigned int timerId) {
    for (auto& usingItemInfo : m_usingItems) {
        if (usingItemInfo.timerId == timerId) {
            usingItemInfo = {};
        }
    }
}

void cltUsingItemSystem::SetChangeCoinSucRate(int successRate) { m_changeCoinSuccessRate = successRate; }
void cltUsingItemSystem::SetChangeCoinRate(int rateA, int rateB) { m_changeCoinRateA = rateA; m_changeCoinRateB = rateB; }
