#include "System/cltEnchantSystem.h"

#include <cstdlib>
#include <cstring>

#include "Info/cltEnchantKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltMoneySystem.h"
#include "System/cltEmblemSystem.h"

cltEnchantKindInfo* cltEnchantSystem::m_pclEnchantKindInfo = nullptr;

void cltEnchantSystem::InitializeStaticVariable(cltEnchantKindInfo* pEnchantKindInfo) {
    m_pclEnchantKindInfo = pEnchantKindInfo;
}

cltEnchantSystem::cltEnchantSystem() {
    m_pInventory = nullptr;
    m_pMoney = nullptr;
    m_pEmblem = nullptr;
}

cltEnchantSystem::~cltEnchantSystem() {
}

void cltEnchantSystem::Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, cltEmblemSystem* pEmblem) {
    m_pInventory = pInventory;
    m_pMoney = pMoney;
    m_pEmblem = pEmblem;
}

void cltEnchantSystem::Free() {
    m_pInventory = nullptr;
    m_pMoney = nullptr;
    m_pEmblem = nullptr;
}

int cltEnchantSystem::CanEnchantItem(std::uint8_t weaponSlot, std::uint8_t stoneSlot) {
    int damageFlag = 0;

    if (!m_pInventory->CanDelInventoryItem(weaponSlot, 1))
        return 1;
    if (!m_pInventory->CanDelInventoryItem(stoneSlot, 1))
        return 1;

    auto* weaponItem = m_pInventory->GetInventoryItem(weaponSlot);
    if (!weaponItem)
        return 1207;

    std::uint16_t weaponKind = weaponItem->itemKind;
    if (!weaponKind)
        return 1;

    strEnchantKindInfo* enchantInfo = m_pclEnchantKindInfo->GetEnchantKindInfoByReqHuntItemKind(weaponKind);
    if (!enchantInfo)
        return 1204;

    auto* stoneItem = m_pInventory->GetInventoryItem(stoneSlot);
    if (!stoneItem)
        return 1208;

    std::int16_t stoneKind = stoneItem->itemKind;
    int stoneIdx = 0;
    for (; stoneIdx < 5; ++stoneIdx) {
        if (stoneKind == enchantInfo->StoneKind[stoneIdx])
            break;
    }

    if (stoneIdx < 5) {
        damageFlag = enchantInfo->DamagePermille[stoneIdx];
    }

    if (stoneIdx == 5)
        return 1205;

    if (damageFlag) {
        std::uint16_t damagedItem = m_pclEnchantKindInfo->GetDamagedItem(weaponKind);
        if (damagedItem) {
            strEnchantKindInfo* damagedInfo = m_pclEnchantKindInfo->GetEnchantKindInfoByReqHuntItemKind(damagedItem);
            if (damagedInfo) {
                if (m_pInventory->CanAddInventoryItem(damagedInfo->StoneKind[0], 1))
                    return 1209;
            }
        }
    }

    if (m_pMoney->CanDecreaseMoney(enchantInfo->Cost))
        return (weaponItem->value0 != 0) ? 0x4B4 : 0;

    return 1206;
}

int cltEnchantSystem::EnchantItem(unsigned int seed, std::uint8_t weaponSlot, char stoneSlot,
                                   std::uint8_t* changeSlots, std::uint16_t* outEnchantID,
                                   int* outCost, std::uint16_t* outResultItemKind, std::uint16_t* outStoneKind) {
    std::uint16_t damagedItemKind = 0;
    std::int16_t damagedStoneKind = 0;
    int damageFlag = 0;
    int evaporateFlag = 0;

    std::srand(seed);

    auto* weaponItem = m_pInventory->GetInventoryItem(weaponSlot);
    auto* stoneItem = m_pInventory->GetInventoryItem(static_cast<std::uint8_t>(stoneSlot));

    if (outStoneKind)
        *outStoneKind = stoneItem->itemKind;

    strEnchantKindInfo* enchantInfo = m_pclEnchantKindInfo->GetEnchantKindInfoByReqHuntItemKind(weaponItem->itemKind);
    std::uint16_t stoneKind = stoneItem->itemKind;

    int stoneIdx = 0;
    int successRate = 0;
    for (; stoneIdx < 5; ++stoneIdx) {
        if (stoneKind == enchantInfo->StoneKind[stoneIdx])
            break;
    }

    if (stoneIdx < 5) {
        successRate = enchantInfo->SuccessPermille[stoneIdx];
        evaporateFlag = enchantInfo->EvaporatePermille[stoneIdx];
        damageFlag = enchantInfo->DamagePermille[stoneIdx];
    }

    int enchantAdvantage = m_pEmblem->GetEnchantAdvantage();
    int adjustedRate = static_cast<int>(
        (static_cast<std::int64_t>(274877907LL) * successRate * enchantAdvantage) >> 32
    );
    int finalRate = (static_cast<unsigned int>(adjustedRate) >> 31) + (adjustedRate >> 6) + successRate;

    if (outEnchantID)
        *outEnchantID = enchantInfo->EnchantID;

    int result;
    if (std::rand() % 1000 < finalRate) {
        result = 1200; // success
    } else {
        int roll = std::rand() % 1000;
        if (roll < evaporateFlag) {
            result = 1202; // evaporate
        } else if (roll < damageFlag) {
            damagedItemKind = m_pclEnchantKindInfo->GetDamagedItem(weaponItem->itemKind);
            if (damagedItemKind) {
                result = 1203; // damage
            } else {
                result = 1201; // fail
            }
        } else {
            result = 1201; // fail
        }
    }

    switch (result) {
    case 1200: { // success
        auto* sealedState = m_pInventory->GetItemSealed(weaponSlot);
        m_pInventory->DelInventoryItem(weaponSlot, 1, changeSlots);
        strInventoryItem newItem;
        newItem.itemKind = enchantInfo->ResultItem;
        newItem.itemQty = 1;
        newItem.value0 = 0;
        newItem.value1 = reinterpret_cast<std::uint32_t>(sealedState);
        m_pInventory->AddInventoryItem(&newItem, changeSlots, nullptr);
        if (outResultItemKind)
            *outResultItemKind = enchantInfo->ResultItem;
        break;
    }
    case 1202: { // evaporate
        m_pInventory->DelInventoryItem(weaponSlot, 1, changeSlots);
        if (outResultItemKind)
            *outResultItemKind = 0;
        break;
    }
    case 1203: { // damage
        auto* sealedState = m_pInventory->GetItemSealed(weaponSlot);
        m_pInventory->DelInventoryItem(weaponSlot, 1, changeSlots);
        strInventoryItem newItem;
        newItem.itemKind = damagedItemKind;
        newItem.itemQty = 1;
        newItem.value0 = 0;
        newItem.value1 = reinterpret_cast<std::uint32_t>(sealedState);
        m_pInventory->AddInventoryItem(&newItem, changeSlots, nullptr);
        if (outResultItemKind)
            *outResultItemKind = damagedItemKind;
        strEnchantKindInfo* damagedEnchant = m_pclEnchantKindInfo->GetEnchantKindInfoByReqHuntItemKind(damagedItemKind);
        if (damagedEnchant)
            damagedStoneKind = damagedEnchant->StoneKind[0];
        break;
    }
    default: { // 1201 = fail
        if (outResultItemKind)
            *outResultItemKind = weaponItem->itemKind;
        break;
    }
    }

    m_pInventory->DelInventoryItem(static_cast<std::uint8_t>(stoneSlot), 1, changeSlots);

    if (damagedStoneKind) {
        strInventoryItem stoneReturn;
        stoneReturn.itemKind = damagedStoneKind;
        stoneReturn.itemQty = 1;
        stoneReturn.value0 = 0;
        stoneReturn.value1 = 0;
        m_pInventory->AddInventoryItem(&stoneReturn, changeSlots, nullptr);
    }

    m_pMoney->DecreaseMoney(enchantInfo->Cost);

    if (outCost)
        *outCost = enchantInfo->Cost;

    m_pEmblem->OnEvent_EnchantItem();

    return result;
}

std::uint16_t cltEnchantSystem::GetEnchantItemKind(std::uint8_t weaponSlot) {
    auto* item = m_pInventory->GetInventoryItem(weaponSlot);
    if (!item)
        return 0;

    strEnchantKindInfo* info = m_pclEnchantKindInfo->GetEnchantKindInfoByReqHuntItemKind(item->itemKind);
    if (info)
        return info->ResultItem;

    return 0;
}
