#include "System/cltExStorageSystem.h"

#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "Network/CMofMsg.h"
#include "System/cltMoneySystem.h"

cltItemKindInfo* cltExStorageSystem::m_pclItemKindInfo = nullptr;

cltExStorageSystem g_clExStorageSystem;

void cltExStorageSystem::InitializeStaticVariable(cltItemKindInfo* pItemKindInfo) {
    m_pclItemKindInfo = pItemKindInfo;
}

cltExStorageSystem::cltExStorageSystem() {
    std::memset(m_items, 0, sizeof(m_items));
    m_depositMoney = 0;
    m_genericBagNum = 0;
    m_pInventory = nullptr;
    m_pMoney = nullptr;
}

cltExStorageSystem::~cltExStorageSystem() {
}

void cltExStorageSystem::Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, int depositMoney, char bagNum, cltItemList* pItemList) {
    m_pInventory = pInventory;
    m_pMoney = pMoney;
    m_depositMoney = depositMoney;
    m_genericBagNum = static_cast<std::uint8_t>(bagNum);

    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
    std::uint16_t itemPos = 0;
    unsigned int itemValue = 0;

    std::uint16_t itemCount = pItemList->GetItemsNum();
    std::memset(m_items, 0, sizeof(m_items));

    for (int i = 0; i < itemCount; ++i) {
        pItemList->GetItem(i, &itemKind, &itemQty, &itemValue, &itemPos, nullptr);
        m_items[itemPos].itemKind = itemKind;
        m_items[itemPos].itemQty = itemQty;
        m_items[itemPos].value = itemValue;
    }
}

void cltExStorageSystem::Initialize(cltBaseInventory* pInventory, cltMoneySystem* pMoney, CMofMsg* pMsg) {
    m_pMoney = pMoney;
    m_pInventory = pInventory;

    std::uint8_t bagNum = 0;
    pMsg->Get_BYTE(&bagNum);
    m_genericBagNum = bagNum;

    int depositMoney = 0;
    pMsg->Get_LONG(&depositMoney);
    m_depositMoney = depositMoney;

    std::uint16_t itemCount = 0;
    pMsg->Get_WORD(&itemCount);

    for (int i = 0; i < itemCount; ++i) {
        std::uint8_t slot = 0;
        std::uint16_t kind = 0;
        std::uint16_t qty = 0;
        pMsg->Get_BYTE(&slot);
        pMsg->Get_WORD(&kind);
        pMsg->Get_WORD(&qty);
        m_items[slot].itemKind = kind;
        m_items[slot].itemQty = qty;
    }

    std::uint16_t valueCount = 0;
    pMsg->Get_WORD(&valueCount);

    for (int i = 0; i < valueCount; ++i) {
        std::uint8_t slot = 0;
        unsigned int value = 0;
        pMsg->Get_BYTE(&slot);
        pMsg->Get_DWORD(&value);
        m_items[slot].value = value;
    }
}

void cltExStorageSystem::Free() {
    m_genericBagNum = 0;
    m_depositMoney = 0;
    m_pInventory = nullptr;
    m_pMoney = nullptr;
    std::memset(m_items, 0, sizeof(m_items));
}

int cltExStorageSystem::GetEmptySlotNum() {
    int count = 0;
    int totalSlots = SLOTS_PER_BAG * m_genericBagNum;
    for (int i = 0; i < totalSlots; ++i) {
        if (!m_items[i].itemKind)
            ++count;
    }
    return count;
}

int cltExStorageSystem::GetMaxAddAbleItemQty(std::uint16_t itemKind) {
    stItemKindInfo* info = m_pclItemKindInfo->GetItemKindInfo(itemKind);
    if (!info)
        return 0;

    int pileUp = m_pclItemKindInfo->GetMaxPileUpNum(itemKind);
    int result = GetEmptySlotNum() * pileUp;

    int totalSlots = SLOTS_PER_BAG * m_genericBagNum;
    for (int i = 0; i < totalSlots; ++i) {
        if (m_items[i].itemKind == itemKind)
            result += pileUp - m_items[i].itemQty;
    }
    return result;
}

int cltExStorageSystem::CanAddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty) {
    int maxQty = GetMaxAddAbleItemQty(itemKind);
    if (!maxQty)
        return 0;
    return maxQty >= qty;
}

strExStorageItem* cltExStorageSystem::CanAddItem(std::uint8_t invenSlot, std::uint16_t qty) {
    if (!qty)
        return nullptr;
    if (!m_pInventory->CanDelInventoryItem(invenSlot, qty))
        return nullptr;

    auto* invItem = m_pInventory->GetInventoryItem(invenSlot);
    if (!invItem)
        return nullptr;

    if (invItem->itemQty < qty)
        return nullptr;

    if (m_pInventory->GetItemSealed(invenSlot))
        return nullptr;

    if (!m_pclItemKindInfo->IsExStorageItem(invItem->itemKind))
        return nullptr;

    return reinterpret_cast<strExStorageItem*>(static_cast<std::uintptr_t>(CanAddItemByItemKind(invItem->itemKind, qty) != 0 ? 1 : 0));
}

void cltExStorageSystem::AddItem(char invenSlot, std::uint16_t qty, std::uint8_t* changeSlots) {
    int remaining = qty;
    auto* invItem = m_pInventory->GetInventoryItem(static_cast<std::uint8_t>(invenSlot));
    stItemKindInfo* kindInfo = m_pclItemKindInfo->GetItemKindInfo(invItem->itemKind);
    int pileUp = m_pclItemKindInfo->GetMaxPileUpNum(invItem->itemKind);

    int totalSlots = SLOTS_PER_BAG * m_genericBagNum;

    // First, fill existing stacks
    for (int i = 0; i < totalSlots && remaining > 0; ++i) {
        if (m_items[i].itemKind == invItem->itemKind) {
            if (m_items[i].itemQty < pileUp) {
                int canAdd = pileUp - m_items[i].itemQty;
                if (canAdd > remaining)
                    canAdd = remaining;
                remaining -= canAdd;
                m_items[i].itemQty += canAdd;
                if (changeSlots)
                    changeSlots[i] = 1;
                if (!remaining)
                    goto done;
            }
        }
    }

    // Then fill empty slots
    while (remaining > 0) {
        int emptyIdx = GetEmptySlotIndex();
        if (emptyIdx == -1)
            break;
        m_items[emptyIdx].itemKind = invItem->itemKind;
        m_items[emptyIdx].value = invItem->value0;
        int toAdd = pileUp;
        if (toAdd > remaining)
            toAdd = remaining;
        m_items[emptyIdx].itemQty = toAdd;
        remaining -= toAdd;
        if (changeSlots)
            changeSlots[emptyIdx] = 1;
    }

done:
    m_pInventory->DelInventoryItem(static_cast<std::uint8_t>(invenSlot), qty, nullptr);
}

int cltExStorageSystem::CanDelItem(std::uint8_t slot, std::uint16_t qty) {
    if (!qty)
        return 0;

    std::uint16_t kind = 0;
    std::uint16_t storedQty = 0;
    unsigned int value = 0;

    if (!GetStorageItem(slot, &kind, &storedQty, &value))
        return 0;

    if (!kind)
        return 0;

    if (storedQty < qty)
        return 0;

    return m_pInventory->CanAddInventoryItem(kind, qty) == 0;
}

void cltExStorageSystem::DelItem(std::uint8_t slot, std::uint16_t qty, std::uint8_t* changeSlots) {
    m_items[slot].itemQty -= qty;

    strInventoryItem newItem;
    newItem.itemKind = m_items[slot].itemKind;
    newItem.itemQty = qty;
    newItem.value0 = m_items[slot].value;
    newItem.value1 = 0;

    m_pInventory->AddInventoryItem(&newItem, changeSlots, nullptr);

    if (!m_items[slot].itemQty) {
        m_items[slot].itemKind = 0;
        m_items[slot].value = 0;
    }
}

stItemKindInfo* cltExStorageSystem::CanMoveItem(std::uint8_t from, std::uint8_t to) {
    strExStorageItem* fromItem = GetStorageItem(from);
    if (!fromItem)
        return nullptr;

    strExStorageItem* toItem = GetStorageItem(to);
    if (!toItem)
        return nullptr;

    if (!fromItem->itemKind)
        return nullptr;

    stItemKindInfo* info = m_pclItemKindInfo->GetItemKindInfo(fromItem->itemKind);
    if (!info)
        return nullptr;

    int totalSlots = SLOTS_PER_BAG * m_genericBagNum;
    if (from < totalSlots && to < totalSlots)
        return info;

    return nullptr;
}

void cltExStorageSystem::MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* changeSlots) {
    strExStorageItem* fromItem = GetStorageItem(from);
    strExStorageItem* toItem = GetStorageItem(to);

    if (toItem->itemKind) {
        if (fromItem->itemKind == toItem->itemKind) {
            // Stack merge
            int pileUp = m_pclItemKindInfo->GetMaxPileUpNum(fromItem->itemKind);
            int canMove = pileUp - toItem->itemQty;
            std::uint16_t moveQty = fromItem->itemQty;
            if (canMove < moveQty)
                moveQty = canMove;
            toItem->itemQty += moveQty;
            fromItem->itemQty -= moveQty;
            if (!fromItem->itemQty) {
                fromItem->itemKind = 0;
                fromItem->value = 0;
            }
        } else {
            // Swap
            strExStorageItem temp = *fromItem;
            *fromItem = *toItem;
            *toItem = temp;
        }
    } else {
        // Move to empty slot
        *toItem = *fromItem;
        fromItem->itemKind = 0;
        fromItem->itemQty = 0;
        fromItem->value = 0;
    }

    if (changeSlots) {
        changeSlots[from] = 1;
        changeSlots[to] = 1;
    }
}

int cltExStorageSystem::CanDepositMoney(int amount) {
    if (!m_pMoney->CanDecreaseMoney(amount))
        return 0;

    std::int64_t charge = GetChargeMoney(amount);
    if (!charge)
        return 0;

    std::int64_t total = static_cast<std::int64_t>(m_depositMoney) + amount;
    return total <= MAX_DEPOSIT;
}

int cltExStorageSystem::DepositMoney(int amount) {
    int charge = static_cast<int>(GetChargeMoney(amount));
    m_depositMoney += amount - charge;
    m_pMoney->DecreaseMoney(amount);
    return charge;
}

int cltExStorageSystem::CanWithdrawingMoney(int amount) {
    if (amount <= 0)
        return 0;
    if (m_depositMoney < amount)
        return 0;
    return m_pMoney->CanIncreaseMoney(amount);
}

void cltExStorageSystem::WithdrawingMoney(int amount) {
    m_depositMoney -= amount;
    m_pMoney->IncreaseMoney(amount);
}

int cltExStorageSystem::GetStorageItem(std::uint8_t slot, std::uint16_t* outKind, std::uint16_t* outQty, unsigned int* outValue) {
    if (slot >= 0x60)
        return 0;
    *outKind = m_items[slot].itemKind;
    *outQty = m_items[slot].itemQty;
    *outValue = m_items[slot].value;
    return 1;
}

strExStorageItem* cltExStorageSystem::GetStorageItem(std::uint8_t slot) {
    if (slot < 0x60)
        return &m_items[slot];
    return nullptr;
}

int cltExStorageSystem::GetDepositMoney() {
    return m_depositMoney;
}

int cltExStorageSystem::GetMaxDepositAbleMoney() {
    int maxDepositable = MAX_DEPOSIT - m_depositMoney;
    int currentMoney = m_pMoney->GetGameMoney();
    return maxDepositable < currentMoney ? maxDepositable : currentMoney;
}

unsigned int cltExStorageSystem::CanExpandGenericBag(int expandType) {
    if (m_genericBagNum >= MAX_BAG_NUM)
        return 700;

    if (expandType == 1) {
        int cost = GetMoneyToExpandGenericBag();
        if (cost == -1)
            return 703;
        if (!m_pMoney->CanDecreaseMoney(cost))
            return 701;
    } else {
        if (!m_pInventory->GetExpandExStorageItemQty())
            return 702;
    }

    return 0;
}

int cltExStorageSystem::ExpandGenericBag(int expandType) {
    if (expandType == 1) {
        int cost = GetMoneyToExpandGenericBag();
        m_pMoney->DecreaseMoney(cost);
        ++m_genericBagNum;
        return cost;
    } else {
        int result = m_pInventory->DecreaseExpandExStorageItemQty();
        ++m_genericBagNum;
        return result;
    }
}

int cltExStorageSystem::GetMoneyToExpandGenericBag() {
    return m_genericBagNum != 0 ? -1 : 300000;
}

std::uint8_t cltExStorageSystem::GetGenericBagNum() {
    return m_genericBagNum;
}

int cltExStorageSystem::GetEmptySlotIndex() {
    int totalSlots = SLOTS_PER_BAG * m_genericBagNum;
    if (totalSlots <= 0)
        return -1;

    for (int i = 0; i < totalSlots; ++i) {
        if (!m_items[i].itemKind)
            return i;
    }
    return -1;
}

cltBaseInventory* cltExStorageSystem::GetInventorySystem() {
    return m_pInventory;
}

cltMoneySystem* cltExStorageSystem::GetMoneySystem() {
    return m_pMoney;
}

std::int64_t cltExStorageSystem::GetChargeMoney(int amount) {
    return static_cast<std::int64_t>(amount) * GetDepostitChargeRate() / 100;
}

int cltExStorageSystem::GetDepostitChargeRate() {
    switch (m_genericBagNum) {
        case 0:
        case 1: return 10;
        case 2: return 5;
        case 3: return 3;
        default: return 2;
    }
}
