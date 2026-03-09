#include "System/cltEquipmentSystem.h"

#include <algorithm>
#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include "System/cltClassSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltSexSystem.h"
#include "System/cltBasicAppearSystem.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"

cltItemKindInfo* cltEquipmentSystem::m_pclItemKindInfo = nullptr;
cltClassKindInfo* cltEquipmentSystem::m_pclClassKindInfo = nullptr;
unsigned int* cltEquipmentSystem::m_dwEquipAtbForFashion = nullptr;
unsigned int* cltEquipmentSystem::m_dwEquipAtbForBattle = nullptr;
unsigned __int16* cltEquipmentSystem::m_wFashionFullSetEffectedKinds = nullptr;

namespace {
constexpr int kEquipSlotCount = 11;
constexpr unsigned int kFashionEquipType = 0;
constexpr unsigned int kBattleEquipType = 1;

const std::array<unsigned int, kEquipSlotCount>& GetValidityArray(const cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionValidity : self->m_battleValidity;
}

std::array<unsigned int, kEquipSlotCount>& GetValidityArray(cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionValidity : self->m_battleValidity;
}

const std::array<std::uint16_t, kEquipSlotCount>& GetItemKindArray(const cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionItemKinds : self->m_battleItemKinds;
}

std::array<std::uint16_t, kEquipSlotCount>& GetItemKindArray(cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionItemKinds : self->m_battleItemKinds;
}

const std::array<std::uint32_t, kEquipSlotCount>& GetItemTimeArray(const cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionItemTimes : self->m_battleItemTimes;
}

std::array<std::uint32_t, kEquipSlotCount>& GetItemTimeArray(cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionItemTimes : self->m_battleItemTimes;
}

const std::array<std::uint32_t, kEquipSlotCount>& GetSealStateArray(const cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionSealStates : self->m_battleSealStates;
}

std::array<std::uint32_t, kEquipSlotCount>& GetSealStateArray(cltEquipmentSystem* self, unsigned int equipType) {
    return equipType == kFashionEquipType ? self->m_fashionSealStates : self->m_battleSealStates;
}
}

void cltEquipmentSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltClassKindInfo* classKindInfo) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclClassKindInfo = classKindInfo;
}

cltEquipmentSystem::cltEquipmentSystem() = default;
cltEquipmentSystem::~cltEquipmentSystem() = default;

void cltEquipmentSystem::Initialize(
    cltSexSystem* sexSystem,
    cltLevelSystem* levelSystem,
    cltClassSystem* classSystem,
    cltPlayerAbility* playerAbility,
    cltBaseInventory* baseInventory,
    cltSkillSystem* skillSystem,
    CPlayerSpirit* playerSpirit,
    cltBasicAppearSystem* basicAppearSystem,
    std::uint16_t* const fashionItemKinds,
    std::uint16_t* const battleItemKinds,
    unsigned int* const fashionItemTimes,
    unsigned int* const battleItemTimes) {

    m_pSexSystem = sexSystem;
    m_pLevelSystem = levelSystem;
    m_pClassSystem = classSystem;
    m_pPlayerAbility = playerAbility;
    m_pBaseInventory = baseInventory;
    m_pSkillSystem = skillSystem;
    m_pPlayerSpirit = playerSpirit;
    m_pBasicAppearSystem = basicAppearSystem;

    if (fashionItemKinds) {
        std::copy_n(fashionItemKinds, kEquipSlotCount, m_fashionItemKinds.begin());
    }
    if (battleItemKinds) {
        std::copy_n(battleItemKinds, kEquipSlotCount, m_battleItemKinds.begin());
    }
    if (fashionItemTimes) {
        std::copy_n(fashionItemTimes, kEquipSlotCount, m_fashionItemTimes.begin());
    }
    if (battleItemTimes) {
        std::copy_n(battleItemTimes, kEquipSlotCount, m_battleItemTimes.begin());
    }

    UpdateValidity();
}

void cltEquipmentSystem::Initialize(
    cltSexSystem* sexSystem,
    cltLevelSystem* levelSystem,
    cltClassSystem* classSystem,
    cltPlayerAbility* playerAbility,
    cltBaseInventory* baseInventory,
    cltSkillSystem* skillSystem,
    CPlayerSpirit* playerSpirit,
    cltBasicAppearSystem* basicAppearSystem,
    const stEquipItemInfo* fashionEquipItems,
    const stEquipItemInfo* battleEquipItems) {

    m_pSexSystem = sexSystem;
    m_pLevelSystem = levelSystem;
    m_pClassSystem = classSystem;
    m_pPlayerAbility = playerAbility;
    m_pBaseInventory = baseInventory;
    m_pSkillSystem = skillSystem;
    m_pPlayerSpirit = playerSpirit;
    m_pBasicAppearSystem = basicAppearSystem;

    for (int slotIndex = 0; slotIndex < kEquipSlotCount; ++slotIndex) {
        if (fashionEquipItems) {
            m_fashionItemKinds[slotIndex] = fashionEquipItems[slotIndex].itemKind;
            m_fashionItemTimes[slotIndex] = fashionEquipItems[slotIndex].itemTime;
            m_fashionSealStates[slotIndex] = fashionEquipItems[slotIndex].sealedStatus;
        }
        if (battleEquipItems) {
            m_battleItemKinds[slotIndex] = battleEquipItems[slotIndex].itemKind;
            m_battleItemTimes[slotIndex] = battleEquipItems[slotIndex].itemTime;
            m_battleSealStates[slotIndex] = battleEquipItems[slotIndex].sealedStatus;
        }
    }

    UpdateValidity();
}

void cltEquipmentSystem::Free() {
    m_pLevelSystem = nullptr;
    m_pClassSystem = nullptr;
    m_pSexSystem = nullptr;
    m_pPlayerAbility = nullptr;
    m_pBaseInventory = nullptr;
    m_pSkillSystem = nullptr;
    m_pPlayerSpirit = nullptr;
    m_pBasicAppearSystem = nullptr;
    m_fashionItemKinds.fill(0);
    m_battleItemKinds.fill(0);
    m_fashionValidity.fill(0);
    m_battleValidity.fill(0);
    m_fashionItemTimes.fill(0);
    m_battleItemTimes.fill(0);
    m_fashionSealStates.fill(0);
    m_battleSealStates.fill(0);
}

strInventoryItem* cltEquipmentSystem::CanEquipItem(int equipType, unsigned __int16 inventorySlotIndex) {
    if (!m_pBaseInventory) {
        return nullptr;
    }
    return m_pBaseInventory->GetInventoryItem(inventorySlotIndex);
}

unsigned int cltEquipmentSystem::CanEquipItem(unsigned int equipType, unsigned int equipSlotIndex, unsigned __int16 inventorySlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount || !m_pBaseInventory) {
        return 1;
    }
    auto* inventoryItem = m_pBaseInventory->GetInventoryItem(inventorySlotIndex);
    return inventoryItem && inventoryItem->itemKind ? 0u : 1u;
}

stItemKindInfo* cltEquipmentSystem::CanEquipItemByItemKind(int equipType, unsigned __int16 itemKind) {
    if (!m_pclItemKindInfo) {
        return nullptr;
    }
    return m_pclItemKindInfo->GetItemKindInfo(itemKind);
}

__int16 cltEquipmentSystem::EquipItem(unsigned int equipType, unsigned int equipSlotIndex, unsigned __int16 inventorySlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount || !m_pBaseInventory) {
        return 0;
    }

    auto* inventoryItem = m_pBaseInventory->GetInventoryItem(inventorySlotIndex);
    if (!inventoryItem || !inventoryItem->itemKind) {
        return 0;
    }

    GetItemKindArray(this, equipType)[equipSlotIndex] = inventoryItem->itemKind;
    GetItemTimeArray(this, equipType)[equipSlotIndex] = inventoryItem->value0;
    GetSealStateArray(this, equipType)[equipSlotIndex] = inventoryItem->value1;
    UpdateValidity();
    return static_cast<__int16>(inventoryItem->itemKind);
}

strInventoryItem* cltEquipmentSystem::CanUnEquipItem(unsigned int equipType, unsigned int equipSlotIndex, unsigned __int16 inventorySlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount || !m_pBaseInventory) {
        return nullptr;
    }
    return m_pBaseInventory->GetInventoryItem(inventorySlotIndex);
}

void cltEquipmentSystem::UnEquipItem(unsigned int equipType, unsigned int equipSlotIndex, unsigned __int16 inventorySlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return;
    }
    GetItemKindArray(this, equipType)[equipSlotIndex] = 0;
    GetItemTimeArray(this, equipType)[equipSlotIndex] = 0;
    GetSealStateArray(this, equipType)[equipSlotIndex] = 0;
    UpdateValidity();
}

unsigned __int16 cltEquipmentSystem::GetEquipItem(unsigned int equipType, unsigned int equipSlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    return GetItemKindArray(this, equipType)[equipSlotIndex];
}

int cltEquipmentSystem::GetSealedStatus(int equipType, int equipSlotIndex) {
    if (equipSlotIndex < 0 || equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    return static_cast<int>(GetSealStateArray(this, static_cast<unsigned int>(equipType))[equipSlotIndex]);
}

unsigned int cltEquipmentSystem::GetEquipItemTime(unsigned int equipType, unsigned int equipSlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    return GetItemTimeArray(this, equipType)[equipSlotIndex];
}

unsigned __int16* cltEquipmentSystem::GetEquipItemList(unsigned int equipType) {
    return GetItemKindArray(this, equipType).data();
}

int cltEquipmentSystem::CanUnEquipAllItem(unsigned int equipType) {
    return 1;
}

void cltEquipmentSystem::UnEquipAllItem(cltItemList* itemList, unsigned int equipType, unsigned __int8* inventoryFlags, unsigned __int8* equipFlags) {
    for (int slotIndex = 0; slotIndex < kEquipSlotCount; ++slotIndex) {
        const std::uint16_t equippedItemKind = GetItemKindArray(this, equipType)[slotIndex];
        if (!equippedItemKind) {
            continue;
        }
        if (itemList) {
            itemList->AddItem(static_cast<std::int16_t>(equippedItemKind), 1, 0, static_cast<int>(GetSealStateArray(this, equipType)[slotIndex]), 0xFFFFu, nullptr);
        }
        GetItemKindArray(this, equipType)[slotIndex] = 0;
        GetItemTimeArray(this, equipType)[slotIndex] = 0;
        GetSealStateArray(this, equipType)[slotIndex] = 0;
        if (equipFlags) {
            equipFlags[slotIndex] = 1;
        }
    }
    UpdateValidity();
}

void cltEquipmentSystem::UnEquipAllItem(unsigned int equipType, unsigned __int8* equipFlags) {
    UnEquipAllItem(nullptr, equipType, nullptr, equipFlags);
}

stItemKindInfo* cltEquipmentSystem::CanMoveEquipItem(unsigned int equipType, unsigned int sourceSlotIndex, unsigned int targetSlotIndex) {
    if (sourceSlotIndex >= kEquipSlotCount || targetSlotIndex >= kEquipSlotCount || !m_pclItemKindInfo) {
        return nullptr;
    }
    const std::uint16_t itemKind = GetItemKindArray(this, equipType)[sourceSlotIndex];
    return itemKind ? m_pclItemKindInfo->GetItemKindInfo(itemKind) : nullptr;
}

void cltEquipmentSystem::MoveEquipItem(unsigned int equipType, unsigned int sourceSlotIndex, unsigned int targetSlotIndex) {
    if (sourceSlotIndex >= kEquipSlotCount || targetSlotIndex >= kEquipSlotCount) {
        return;
    }
    std::swap(GetItemKindArray(this, equipType)[sourceSlotIndex], GetItemKindArray(this, equipType)[targetSlotIndex]);
    std::swap(GetItemTimeArray(this, equipType)[sourceSlotIndex], GetItemTimeArray(this, equipType)[targetSlotIndex]);
    std::swap(GetSealStateArray(this, equipType)[sourceSlotIndex], GetSealStateArray(this, equipType)[targetSlotIndex]);
    UpdateValidity();
}

unsigned __int16 cltEquipmentSystem::GetTotalStr() { return 0; }
unsigned __int16 cltEquipmentSystem::GetTotalDex() { return 0; }
unsigned __int16 cltEquipmentSystem::GetTotalInt() { return 0; }
unsigned __int16 cltEquipmentSystem::GetTotalVit() { return 0; }

void cltEquipmentSystem::GetAPower(unsigned __int16* outMinPower, unsigned __int16* outMaxPower) {
    if (outMinPower) *outMinPower = 0;
    if (outMaxPower) *outMaxPower = 0;
}

int cltEquipmentSystem::GetAPowerAdvantage(int a2) { return 0; }
__int16 cltEquipmentSystem::GetDPower() { return 0; }
int cltEquipmentSystem::GetDPowerAdvatnage(int a2) { return 0; }
int cltEquipmentSystem::GetMaxHPAdvantage() { return 0; }
int cltEquipmentSystem::GetMaxManaAdvantage() { return 0; }
int cltEquipmentSystem::GetSkillAPowerAdvantage() { return 0; }
int cltEquipmentSystem::GetHitRate() { return 0; }
int cltEquipmentSystem::GetCriticalHitRate() { return 0; }
int cltEquipmentSystem::GetMissRate() { return 0; }

stItemKindInfo* cltEquipmentSystem::GetEquipablePos(unsigned __int16 itemKind, unsigned int* outEquipType, unsigned int* outEquipSlotIndex, unsigned int* outReserved) {
    if (outEquipType) *outEquipType = kBattleEquipType;
    if (outEquipSlotIndex) *outEquipSlotIndex = 0;
    if (outReserved) *outReserved = 0;
    return m_pclItemKindInfo ? m_pclItemKindInfo->GetItemKindInfo(itemKind) : nullptr;
}

stItemKindInfo* cltEquipmentSystem::GetEquipablePosByItemKind(unsigned __int16 itemKind, unsigned int* outEquipType, unsigned int* outEquipSlotIndex, unsigned int* outReserved) {
    return GetEquipablePos(itemKind, outEquipType, outEquipSlotIndex, outReserved);
}

int cltEquipmentSystem::GetUnEquipablePos(unsigned int equipType, unsigned int equipSlotIndex, unsigned __int16* outInventorySlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    if (outInventorySlotIndex) {
        *outInventorySlotIndex = static_cast<unsigned __int16>(equipSlotIndex);
    }
    return 1;
}

int cltEquipmentSystem::GetExpAdvantageByEquipItem() { return 0; }

void cltEquipmentSystem::UpdateValidity() {
    m_fashionValidity.fill(0);
    m_battleValidity.fill(0);

    for (int slotIndex = 0; slotIndex < kEquipSlotCount; ++slotIndex) {
        if (m_fashionItemKinds[slotIndex]) {
            m_fashionValidity[slotIndex] = 1;
        }
        if (m_battleItemKinds[slotIndex]) {
            m_battleValidity[slotIndex] = 1;
        }
    }
}

int cltEquipmentSystem::IsEquipItemValidity(unsigned int equipType, unsigned int equipSlotIndex) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    return static_cast<int>(GetValidityArray(this, equipType)[equipSlotIndex]);
}

int cltEquipmentSystem::GetWeaponAttackAtb() { return 0; }
int cltEquipmentSystem::IsEquipedFashionFullSet() { return 0; }

stItemKindInfo* cltEquipmentSystem::IsEquipedMultiTargetWeapon() {
    return nullptr;
}

int cltEquipmentSystem::IsEquipedBattleItem() {
    for (std::uint16_t equippedItemKind : m_battleItemKinds) {
        if (equippedItemKind) {
            return 1;
        }
    }
    return 0;
}

int cltEquipmentSystem::IsEffectedEquipKindByFashionFullSet(unsigned int a2) { return 0; }
int cltEquipmentSystem::GetMagicResist() { return 0; }
int cltEquipmentSystem::GetAutoRecoverHPAdvantage() { return 0; }
int cltEquipmentSystem::GetAutoRecoverManaAdavntage() { return 0; }

int cltEquipmentSystem::IsFashionAllUnequiped() {
    for (std::uint16_t equippedItemKind : m_fashionItemKinds) {
        if (equippedItemKind) {
            return 0;
        }
    }
    return 1;
}

void cltEquipmentSystem::SetEquipItemsByChangeSex(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4) {
    m_fashionItemKinds[0] = a2;
    m_fashionItemKinds[1] = a3;
    m_fashionItemKinds[2] = a4;
    UpdateValidity();
}

char cltEquipmentSystem::SetItemSealed(char equipType, unsigned __int16 equipSlotIndex, int sealedStatus) {
    if (equipSlotIndex >= kEquipSlotCount) {
        return 0;
    }
    GetSealStateArray(this, static_cast<unsigned int>(equipType))[equipSlotIndex] = static_cast<std::uint32_t>(sealedStatus);
    return 1;
}
