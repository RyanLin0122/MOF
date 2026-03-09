#include "Logic/cltBaseInventory.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "Logic/cltItemList.h"
#include "System/cltQuickSlotSystem.h"
#include "Info/cltItemKindInfo.h"
#include "Network/CMofMsg.h"

cltItemKindInfo* cltBaseInventory::m_pclItemKindInfo = nullptr;
DCTTextManager* cltBaseInventory::m_pclTextManager = nullptr;
void (*cltBaseInventory::m_pExternCriticalErrorFuncPtr)(char*, char*, unsigned int) = nullptr;

namespace {
constexpr int kInventorySlotCount = 255;
constexpr int kNormalBagSlotsPerPage = 24;
constexpr int kMaxNormalBagSlots = 72;
constexpr int kFashionBagStart = 72;
constexpr int kFashionBagSlots = 24;

bool IsSlotIndexInRange(int slotIndex, int beginSlotIndex, int endSlotIndex) {
    return slotIndex >= beginSlotIndex && slotIndex <= endSlotIndex;
}

strInventoryItem MakeInventoryItem(std::uint16_t itemKind, std::uint16_t itemQuantity, std::uint32_t value0, std::uint32_t sealedStatus) {
    strInventoryItem inventoryItem;
    inventoryItem.itemKind = itemKind;
    inventoryItem.itemQty = itemQuantity;
    inventoryItem.value0 = value0;
    inventoryItem.value1 = sealedStatus;
    return inventoryItem;
}
}

void cltBaseInventory::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, DCTTextManager* textManager, void (*criticalErrorCallback)(char*, char*, unsigned int)) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclTextManager = textManager;
    m_pExternCriticalErrorFuncPtr = criticalErrorCallback;
}

cltBaseInventory::cltBaseInventory() {
    Free();
    m_genericBagNum = 1;
}

cltBaseInventory::~cltBaseInventory() = default;

void cltBaseInventory::Initialize(cltQuickSlotSystem* quickSlotSystem, cltTitleSystem* titleSystem, cltPetSystem* petSystem, int genericBagNum, cltItemList* itemList) {
    m_pQuickSlotSystem = quickSlotSystem;
    m_pTitleSystem = titleSystem;
    m_pPetSystem = petSystem;
    m_genericBagNum = static_cast<std::uint8_t>(std::clamp(genericBagNum, 1, 3));
    std::fill(m_inventoryItems.begin(), m_inventoryItems.end(), strInventoryItem{});
    m_lockToken = 0;
    m_isLocked = 0;
    m_lockTick = 0;
    std::memset(m_lockReason, 0, sizeof(m_lockReason));

    if (!itemList) {
        return;
    }

    for (unsigned int itemIndex = 0; itemIndex < itemList->GetItemsNum(); ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        unsigned int value0 = 0;
        std::uint16_t itemPosition = 0;
        itemList->GetItem(itemIndex, &itemKind, &itemQuantity, &value0, &itemPosition, nullptr);
        if (itemPosition < m_inventoryItems.size()) {
            m_inventoryItems[itemPosition] = MakeInventoryItem(itemKind, itemQuantity, value0, static_cast<std::uint32_t>(itemList->GetSealedStatus(itemIndex)));
        }
    }
}

unsigned int cltBaseInventory::CanAddInventoryItem(int slotIndex, std::uint16_t itemKind, std::uint16_t itemQuantity) {
    if (IsLock() == 1) {
        return 108;
    }
    if (!itemKind || !itemQuantity) {
        return 1;
    }

    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    GetItemSlotIndexRangeByItemType(m_pclItemKindInfo && m_pclItemKindInfo->IsFashionItem(itemKind) ? 0 : 1, &beginSlotIndex, &endSlotIndex);
    if (!IsSlotIndexInRange(slotIndex, beginSlotIndex, endSlotIndex)) {
        return 1;
    }

    strInventoryItem* inventoryItem = GetInventoryItem(static_cast<std::uint16_t>(slotIndex));
    if (!inventoryItem || !inventoryItem->itemKind) {
        return 0;
    }
    if (inventoryItem->itemKind != itemKind) {
        return 1;
    }
    return inventoryItem->itemQty + itemQuantity > GetPileUpNum(itemKind);
}

int cltBaseInventory::CanAddInventoryItem(std::uint16_t itemKind, std::uint16_t itemQuantity) {
    if (IsLock() == 1 || !itemKind || !itemQuantity) {
        return 1;
    }
    if (m_pclItemKindInfo && !m_pclItemKindInfo->GetItemKindInfo(itemKind)) {
        return 5;
    }

    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    const int itemType = m_pclItemKindInfo && m_pclItemKindInfo->IsFashionItem(itemKind) ? 0 : 1;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);

    int remainingQuantity = itemQuantity;
    for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex; ++slotIndex) {
        strInventoryItem* inventoryItem = GetInventoryItem(static_cast<std::uint16_t>(slotIndex));
        if (!inventoryItem || inventoryItem->itemKind != itemKind) {
            continue;
        }
        const int freePileSpace = GetPileUpNum(itemKind) - inventoryItem->itemQty;
        if (freePileSpace > 0) {
            remainingQuantity -= freePileSpace;
            if (remainingQuantity <= 0) {
                return 0;
            }
        }
    }

    const int emptySlotCount = GetEmptyItemSlotNum(itemType);
    if (!emptySlotCount) {
        return 103;
    }
    return remainingQuantity <= emptySlotCount * GetPileUpNum(itemKind) ? 0 : 103;
}

int cltBaseInventory::CanAddInventoryItems(cltItemList* itemList) {
    if (!itemList) {
        return 0;
    }

    std::array<strInventoryItem, kInventorySlotCount> simulatedInventory = m_inventoryItems;
    for (unsigned int itemIndex = 0; itemIndex < itemList->GetItemsNum(); ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        unsigned int value0 = 0;
        std::uint16_t itemPosition = 0;
        itemList->GetItem(itemIndex, &itemKind, &itemQuantity, &value0, &itemPosition, nullptr);

        if (CanAddInventoryItem(itemKind, itemQuantity)) {
            return 103;
        }

        int beginSlotIndex = 0;
        int endSlotIndex = 0;
        const int itemType = m_pclItemKindInfo && m_pclItemKindInfo->IsFashionItem(itemKind) ? 0 : 1;
        GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);

        int remainingQuantity = itemQuantity;
        for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex && remainingQuantity > 0; ++slotIndex) {
            strInventoryItem& inventoryItem = simulatedInventory[slotIndex];
            if (inventoryItem.itemKind == itemKind && inventoryItem.itemQty < GetPileUpNum(itemKind)) {
                const int addableQuantity = std::min<int>(remainingQuantity, GetPileUpNum(itemKind) - inventoryItem.itemQty);
                inventoryItem.itemQty = static_cast<std::uint16_t>(inventoryItem.itemQty + addableQuantity);
                remainingQuantity -= addableQuantity;
            }
        }
        for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex && remainingQuantity > 0; ++slotIndex) {
            strInventoryItem& inventoryItem = simulatedInventory[slotIndex];
            if (!inventoryItem.itemKind) {
                const int addableQuantity = std::min<int>(remainingQuantity, GetPileUpNum(itemKind));
                inventoryItem = MakeInventoryItem(itemKind, static_cast<std::uint16_t>(addableQuantity), value0, static_cast<std::uint32_t>(itemList->GetSealedStatus(itemIndex)));
                remainingQuantity -= addableQuantity;
            }
        }
        if (remainingQuantity > 0) {
            return 103;
        }
    }
    return 0;
}

int cltBaseInventory::CanDelInventoryItems(cltItemList* itemList) {
    if (!itemList) {
        return 0;
    }
    for (unsigned int itemIndex = 0; itemIndex < itemList->GetItemsNum(); ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        std::uint16_t itemPosition = 0;
        unsigned int value0 = 0;
        itemList->GetItem(itemIndex, &itemKind, &itemQuantity, &value0, &itemPosition, nullptr);
        if (!CanDelInventoryItem(itemKind, itemQuantity)) {
            return 0;
        }
    }
    return 1;
}

strInventoryItem* cltBaseInventory::CanDelInventoryItem(std::uint16_t itemKind, std::uint16_t itemQuantity) {
    return IsThereInventoryItem(itemKind, itemQuantity);
}

strInventoryItem* cltBaseInventory::CanDelInventoryItem(std::uint16_t itemKind, std::uint16_t itemPosition, std::uint16_t itemQuantity) {
    return IsThereInventoryItem(itemKind, itemPosition, itemQuantity);
}

BOOL cltBaseInventory::CanDelInventoryItemByKindNQty(int itemKind, std::uint16_t itemQuantity) {
    return GetAllItemCount(itemKind) >= itemQuantity;
}

int cltBaseInventory::AddInventoryItem(cltItemList* itemList, std::uint8_t* inventoryFlags) {
    if (!itemList) {
        return 0;
    }
    int result = 1;
    for (unsigned int itemIndex = 0; itemIndex < itemList->GetItemsNum(); ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        unsigned int value0 = 0;
        std::uint16_t itemPosition = 0;
        itemList->GetItem(itemIndex, &itemKind, &itemQuantity, &value0, &itemPosition, nullptr);
        strInventoryItem inventoryItem = MakeInventoryItem(itemKind, itemQuantity, value0, static_cast<std::uint32_t>(itemList->GetSealedStatus(itemIndex)));
        if (!AddInventoryItem(&inventoryItem, inventoryFlags, nullptr)) {
            result = 0;
        }
    }
    return result;
}

int cltBaseInventory::AddInventoryItem(strInventoryItem* inventoryItem, std::uint8_t* inventoryFlags, std::uint16_t* outSlotIndex) {
    if (!inventoryItem) {
        return 0;
    }
    if (CanAddInventoryItem(inventoryItem->itemKind, inventoryItem->itemQty)) {
        return 0;
    }

    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    const int itemType = m_pclItemKindInfo && m_pclItemKindInfo->IsFashionItem(inventoryItem->itemKind) ? 0 : 1;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);

    for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex; ++slotIndex) {
        strInventoryItem* existingItem = GetInventoryItem(static_cast<std::uint16_t>(slotIndex));
        if (!existingItem || existingItem->itemKind != inventoryItem->itemKind || existingItem->itemQty >= GetPileUpNum(inventoryItem->itemKind)) {
            continue;
        }
        const int addableQuantity = std::min<int>(inventoryItem->itemQty, GetPileUpNum(inventoryItem->itemKind) - existingItem->itemQty);
        existingItem->itemQty = static_cast<std::uint16_t>(existingItem->itemQty + addableQuantity);
        inventoryItem->itemQty = static_cast<std::uint16_t>(inventoryItem->itemQty - addableQuantity);
        existingItem->value0 = inventoryItem->value0;
        existingItem->value1 = inventoryItem->value1;
        if (inventoryFlags) {
            inventoryFlags[slotIndex] = 1;
        }
        if (!inventoryItem->itemQty) {
            if (outSlotIndex) {
                *outSlotIndex = static_cast<std::uint16_t>(slotIndex);
            }
            return 1;
        }
    }

    while (inventoryItem->itemQty > 0) {
        const int emptySlotIndex = GetEmptyItemSlotPos(inventoryItem->itemKind, beginSlotIndex, endSlotIndex);
        if (emptySlotIndex < 0) {
            return 0;
        }
        strInventoryItem splitItem = *inventoryItem;
        splitItem.itemQty = static_cast<std::uint16_t>(std::min<int>(inventoryItem->itemQty, GetPileUpNum(inventoryItem->itemKind)));
        inventoryItem->itemQty = static_cast<std::uint16_t>(inventoryItem->itemQty - splitItem.itemQty);
        AddInventoryItem(static_cast<std::uint16_t>(emptySlotIndex), &splitItem, inventoryFlags);
        if (outSlotIndex) {
            *outSlotIndex = static_cast<std::uint16_t>(emptySlotIndex);
        }
    }
    return 1;
}

int cltBaseInventory::AddInventoryItem(std::uint16_t slotIndex, strInventoryItem* inventoryItem, std::uint8_t* inventoryFlags) {
    strInventoryItem* targetSlot = GetInventoryItem(slotIndex);
    if (!targetSlot || !inventoryItem || !inventoryItem->itemQty) {
        return 0;
    }
    if (!targetSlot->itemKind) {
        targetSlot->itemKind = inventoryItem->itemKind;
        targetSlot->itemQty = 0;
    }
    targetSlot->itemQty = static_cast<std::uint16_t>(targetSlot->itemQty + inventoryItem->itemQty);
    targetSlot->value0 = inventoryItem->value0;
    targetSlot->value1 = inventoryItem->value1;
    if (inventoryFlags) {
        inventoryFlags[slotIndex] = 1;
    }
    return 1;
}

int cltBaseInventory::DelInventoryItem(cltItemList* itemList, std::uint8_t* inventoryFlags) {
    if (!itemList) {
        return 0;
    }
    int result = 1;
    for (unsigned int itemIndex = 0; itemIndex < itemList->GetItemsNum(); ++itemIndex) {
        std::uint16_t itemKind = 0;
        std::uint16_t itemQuantity = 0;
        unsigned int value0 = 0;
        std::uint16_t itemPosition = 0;
        itemList->GetItem(itemIndex, &itemKind, &itemQuantity, &value0, &itemPosition, nullptr);
        if (!DelInventoryItem(itemKind, itemQuantity, inventoryFlags)) {
            result = 0;
        }
    }
    return result;
}

BOOL cltBaseInventory::DelInventoryItemKind(std::uint16_t itemKind, std::uint16_t itemQuantity, cltItemList* outItemList, std::uint8_t* inventoryFlags) {
    if (outItemList) {
        outItemList->AddItem(static_cast<std::int16_t>(itemKind), static_cast<std::int16_t>(itemQuantity), 0, 0, 0xFFFFu, nullptr);
    }
    return DelInventoryItem(itemKind, itemQuantity, inventoryFlags) != 0;
}

int cltBaseInventory::DelInventoryItem(std::uint16_t itemKind, std::uint16_t itemQuantity, std::uint8_t* inventoryFlags) {
    std::uint16_t remainingQuantity = itemQuantity;
    for (std::size_t slotIndex = 0; slotIndex < m_inventoryItems.size() && remainingQuantity > 0; ++slotIndex) {
        strInventoryItem& inventoryItem = m_inventoryItems[slotIndex];
        if (inventoryItem.itemKind != itemKind) {
            continue;
        }
        const std::uint16_t removableQuantity = (remainingQuantity < inventoryItem.itemQty) ? remainingQuantity : inventoryItem.itemQty;
        inventoryItem.itemQty = static_cast<std::uint16_t>(inventoryItem.itemQty - removableQuantity);
        remainingQuantity = static_cast<std::uint16_t>(remainingQuantity - removableQuantity);
        if (!inventoryItem.itemQty) {
            inventoryItem = {};
        }
        if (inventoryFlags) {
            inventoryFlags[slotIndex] = 1;
        }
    }
    return remainingQuantity == 0;
}

int cltBaseInventory::DelInventoryItem(int slotIndex, std::uint16_t itemQuantity, std::uint8_t* inventoryFlags) {
    strInventoryItem* inventoryItem = GetInventoryItem(static_cast<std::uint16_t>(slotIndex));
    if (!inventoryItem || !inventoryItem->itemKind || inventoryItem->itemQty < itemQuantity) {
        return 0;
    }
    inventoryItem->itemQty = static_cast<std::uint16_t>(inventoryItem->itemQty - itemQuantity);
    if (!inventoryItem->itemQty) {
        *inventoryItem = {};
    }
    if (inventoryFlags) {
        inventoryFlags[slotIndex] = 1;
    }
    return 1;
}

void cltBaseInventory::DelInventoryItem(char slotIndex, std::uint8_t* inventoryFlags) {
    strInventoryItem* inventoryItem = GetInventoryItem(static_cast<unsigned char>(slotIndex));
    if (inventoryItem) {
        DelInventoryItem(static_cast<unsigned char>(slotIndex), inventoryItem->itemQty, inventoryFlags);
    }
}

strInventoryItem* cltBaseInventory::GetInventoryItem(std::uint16_t slotIndex) {
    if (!IsValidItemSlotIndex(slotIndex)) {
        return nullptr;
    }
    return &m_inventoryItems[slotIndex];
}

int cltBaseInventory::EmptyInventoryItem(std::uint16_t slotIndex) {
    strInventoryItem* inventoryItem = GetInventoryItem(slotIndex);
    if (!inventoryItem) {
        return 0;
    }
    *inventoryItem = {};
    return 1;
}

void cltBaseInventory::Free() {
    m_lockToken = 0;
    m_isLocked = 0;
    std::memset(m_lockReason, 0, sizeof(m_lockReason));
    m_lockTick = 0;
    m_genericBagNum = 1;
    m_pQuickSlotSystem = nullptr;
    m_pTitleSystem = nullptr;
    m_pPetSystem = nullptr;
    std::fill(m_inventoryItems.begin(), m_inventoryItems.end(), strInventoryItem{});
}

int cltBaseInventory::GetEmptyItemSlotPos(int itemType) {
    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);
    return GetEmptyItemSlotPos(0, beginSlotIndex, endSlotIndex);
}

int cltBaseInventory::GetEmptyItemSlotPos(std::uint16_t itemKind, int beginSlotIndex, int endSlotIndex) {
    for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex; ++slotIndex) {
        if (!m_inventoryItems[slotIndex].itemKind) {
            return slotIndex;
        }
    }
    return -1;
}

int cltBaseInventory::GetUseItemSlotPos(int a2) { return -1; }
stItemKindInfo* cltBaseInventory::GetUseItemSlotPos(int a2, int* a3, std::uint16_t* a4) { return nullptr; }

strInventoryItem* cltBaseInventory::IsThereInventoryItem(std::uint16_t itemKind, std::uint16_t itemQuantity) {
    for (strInventoryItem& inventoryItem : m_inventoryItems) {
        if (inventoryItem.itemKind == itemKind && inventoryItem.itemQty >= itemQuantity) {
            return &inventoryItem;
        }
    }
    return nullptr;
}

strInventoryItem* cltBaseInventory::IsThereInventoryItem(std::uint16_t itemKind, std::uint16_t slotIndex, std::uint16_t itemQuantity) {
    strInventoryItem* inventoryItem = GetInventoryItem(slotIndex);
    if (inventoryItem && inventoryItem->itemKind == itemKind && inventoryItem->itemQty >= itemQuantity) {
        return inventoryItem;
    }
    return nullptr;
}

int cltBaseInventory::IsThereInventoryItem(cltItemList* itemList) {
    return itemList ? !CanDelInventoryItems(itemList) : 0;
}

BOOL cltBaseInventory::IsThereInventoryItemByKindNQty(int itemKind, std::uint16_t itemQuantity) {
    return GetAllItemCount(itemKind) >= itemQuantity;
}

int cltBaseInventory::FindFirstItem(int itemKind) {
    for (int slotIndex = 0; slotIndex < static_cast<int>(m_inventoryItems.size()); ++slotIndex) {
        if (m_inventoryItems[slotIndex].itemKind == itemKind) {
            return slotIndex;
        }
    }
    return -1;
}

std::uint8_t cltBaseInventory::GetGenericBagNum() { return m_genericBagNum; }

std::uint8_t cltBaseInventory::GetPileUpNum(std::uint16_t itemKind) {
    return m_pclItemKindInfo ? m_pclItemKindInfo->GetMaxPileUpNum(itemKind) : 1;
}

std::int16_t cltBaseInventory::GetInventoryItemQuantity(int slotIndex) {
    strInventoryItem* inventoryItem = GetInventoryItem(static_cast<std::uint16_t>(slotIndex));
    return inventoryItem ? inventoryItem->itemQty : 0;
}

int cltBaseInventory::CanMoveItem(std::uint16_t sourceSlotIndex, std::uint16_t targetSlotIndex) {
    return IsValidItemSlotIndex(sourceSlotIndex) && IsValidItemSlotIndex(targetSlotIndex);
}

int cltBaseInventory::MoveItem(std::uint16_t sourceSlotIndex, std::uint16_t targetSlotIndex) {
    if (!CanMoveItem(sourceSlotIndex, targetSlotIndex)) {
        return 0;
    }
    std::swap(m_inventoryItems[sourceSlotIndex], m_inventoryItems[targetSlotIndex]);
    return 1;
}

std::uint16_t* cltBaseInventory::IsEquipTypeItem(std::uint16_t a2) { return nullptr; }

BOOL cltBaseInventory::GetItemSlotIndexRangeByItemKind(std::uint16_t itemKind, int* outBeginSlotIndex, int* outEndSlotIndex) {
    GetItemSlotIndexRangeByItemType(m_pclItemKindInfo && m_pclItemKindInfo->IsFashionItem(itemKind) ? 0 : 1, outBeginSlotIndex, outEndSlotIndex);
    return 1;
}

void cltBaseInventory::GetItemSlotIndexRangeByItemType(int itemType, int* outBeginSlotIndex, int* outEndSlotIndex) {
    if (!outBeginSlotIndex || !outEndSlotIndex) {
        return;
    }
    if (itemType == 0) {
        *outBeginSlotIndex = kFashionBagStart;
        *outEndSlotIndex = kFashionBagStart + kFashionBagSlots - 1;
    } else {
        *outBeginSlotIndex = 0;
        *outEndSlotIndex = std::min<int>(kNormalBagSlotsPerPage * m_genericBagNum - 1, kMaxNormalBagSlots - 1);
    }
}

int cltBaseInventory::GetEmptyItemSlotNum(int itemType) {
    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);
    int emptySlotCount = 0;
    for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex; ++slotIndex) {
        if (!m_inventoryItems[slotIndex].itemKind) {
            ++emptySlotCount;
        }
    }
    return emptySlotCount;
}

BOOL cltBaseInventory::IsValidItemSlotIndex(int slotIndex) {
    return slotIndex >= 0 && slotIndex < static_cast<int>(m_inventoryItems.size());
}

BOOL cltBaseInventory::IsValidItemSlotIndex(int slotIndex, int itemType) {
    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);
    return IsSlotIndexInRange(slotIndex, beginSlotIndex, endSlotIndex);
}

BOOL cltBaseInventory::IsIdenticalItemTypeSlotIndex(int sourceSlotIndex, int targetSlotIndex) {
    const bool sourceIsFashionSlot = sourceSlotIndex >= kFashionBagStart;
    const bool targetIsFashionSlot = targetSlotIndex >= kFashionBagStart;
    return sourceIsFashionSlot == targetIsFashionSlot;
}

void cltBaseInventory::FillOutInventoryInfo(CMofMsg* message) {
    if (!message) {
        return;
    }
    cltItemList itemList;
    itemList.Initialize(7u);
    for (std::uint16_t slotIndex = 0; slotIndex < m_inventoryItems.size(); ++slotIndex) {
        const strInventoryItem& inventoryItem = m_inventoryItems[slotIndex];
        if (!inventoryItem.itemKind) {
            continue;
        }
        itemList.AddItem(static_cast<std::int16_t>(inventoryItem.itemKind), static_cast<std::int16_t>(inventoryItem.itemQty), static_cast<int>(inventoryItem.value0), static_cast<int>(inventoryItem.value1), slotIndex, nullptr);
    }
    itemList.FillOutItemListInfo(message);
}

strInventoryItem* cltBaseInventory::EquipedItem(std::uint16_t slotIndex, std::int16_t itemKind, int value0, int sealedStatus) {
    strInventoryItem* inventoryItem = GetInventoryItem(slotIndex);
    if (!inventoryItem || inventoryItem->itemQty < 1) {
        return nullptr;
    }
    if (inventoryItem->itemQty == 1) {
        if (itemKind) {
            *inventoryItem = MakeInventoryItem(static_cast<std::uint16_t>(itemKind), 1, static_cast<std::uint32_t>(value0), static_cast<std::uint32_t>(sealedStatus));
        } else {
            EmptyInventoryItem(slotIndex);
        }
        return reinterpret_cast<strInventoryItem*>(1);
    }
    return nullptr;
}

BOOL cltBaseInventory::CanIncreaseGenericBagNum() { return m_genericBagNum < 3; }
void cltBaseInventory::IncreaseGenericBagNum() { if (m_genericBagNum < 3) ++m_genericBagNum; }
int cltBaseInventory::GetExpandExStorageItemQty() { return 0; }
int cltBaseInventory::DecreaseExpandExStorageItemQty() { return 0; }
unsigned int cltBaseInventory::GetTownPortalItemQty(int* a2) { if (a2) *a2 = 0; return 0; }
unsigned int cltBaseInventory::GetPostItItemQty(int* a2) { if (a2) *a2 = 0; return 0; }
std::uint16_t cltBaseInventory::GetTransportItem() { return 0; }

int cltBaseInventory::GetAllItemCount(std::uint16_t itemKind) {
    int totalQuantity = 0;
    for (const strInventoryItem& inventoryItem : m_inventoryItems) {
        if (inventoryItem.itemKind == itemKind) {
            totalQuantity += inventoryItem.itemQty;
        }
    }
    return totalQuantity;
}

int cltBaseInventory::GetAllItemCount(int itemKind) { return GetAllItemCount(static_cast<std::uint16_t>(itemKind)); }
int cltBaseInventory::IsLock() { return m_isLocked; }

int cltBaseInventory::Lock(char* lockReason) {
    ++m_lockToken;
    if (!m_lockToken) {
        m_lockToken = 1;
    }
    m_isLocked = 1;
    std::strncpy(m_lockReason, lockReason ? lockReason : "", sizeof(m_lockReason) - 1);
    m_lockReason[sizeof(m_lockReason) - 1] = 0;
    return m_lockToken;
}

int cltBaseInventory::Unlock(int lockToken) {
    if (m_lockToken != lockToken) {
        return 0;
    }
    m_isLocked = 0;
    std::memset(m_lockReason, 0, sizeof(m_lockReason));
    return 1;
}

int cltBaseInventory::SortInventoryItems(int itemType) {
    int beginSlotIndex = 0;
    int endSlotIndex = 0;
    GetItemSlotIndexRangeByItemType(itemType, &beginSlotIndex, &endSlotIndex);

    std::array<strInventoryItemForSort, 72> sortedItems{};
    int sortedCount = 0;
    for (int slotIndex = beginSlotIndex; slotIndex <= endSlotIndex && sortedCount < static_cast<int>(sortedItems.size()); ++slotIndex) {
        const strInventoryItem& inventoryItem = m_inventoryItems[slotIndex];
        sortedItems[sortedCount].sortKey = static_cast<std::uint16_t>(slotIndex);
        sortedItems[sortedCount].itemKind = inventoryItem.itemKind;
        sortedItems[sortedCount].value0 = inventoryItem.value0;
        sortedItems[sortedCount].value1 = inventoryItem.value1;
        sortedItems[sortedCount].itemPos = static_cast<std::uint16_t>(slotIndex);
        ++sortedCount;
    }

    std::sort(sortedItems.begin(), sortedItems.begin() + sortedCount, [](const strInventoryItemForSort& leftItem, const strInventoryItemForSort& rightItem) {
        if (!leftItem.itemKind) return false;
        if (!rightItem.itemKind) return true;
        return leftItem.itemKind < rightItem.itemKind;
    });

    for (int sortedIndex = 0; sortedIndex < sortedCount; ++sortedIndex) {
        const auto& sortedItem = sortedItems[sortedIndex];
        m_inventoryItems[beginSlotIndex + sortedIndex] = MakeInventoryItem(sortedItem.itemKind, 0, sortedItem.value0, sortedItem.value1);
    }

    if (m_pQuickSlotSystem) {
        m_pQuickSlotSystem->OnItemSorted(sortedItems.data());
    }
    return 1;
}

BOOL cltBaseInventory::ISGetItemID(unsigned int a2, unsigned int* a3, unsigned int* a4) { if (a3) *a3 = 0; if (a4) *a4 = 0; return 0; }
cltPetInventorySystem* cltBaseInventory::CanMoveItemToPetInventory(std::uint8_t a2, std::uint16_t a3) { return nullptr; }
void cltBaseInventory::MoveItemToPetInventory(char a2, int a3, std::uint8_t* a4, std::uint8_t* a5) {}
cltPetInventorySystem* cltBaseInventory::CanMoveItemFromPetInventory(std::uint8_t a2, std::uint16_t a3) { return nullptr; }
void cltBaseInventory::MoveItemFromPetInventory(std::uint8_t a2, std::uint16_t a3, std::uint8_t* a4, std::uint8_t* a5) {}
int cltBaseInventory::IsExistMoveServer() { return 0; }

strInventoryItem* cltBaseInventory::GetItemSealed(std::uint16_t slotIndex) {
    strInventoryItem* inventoryItem = GetInventoryItem(slotIndex);
    return inventoryItem && inventoryItem->value1 ? inventoryItem : nullptr;
}

int cltBaseInventory::SetItemSealed(std::uint16_t slotIndex, int sealedStatus, int a4) {
    strInventoryItem* inventoryItem = GetInventoryItem(slotIndex);
    if (!inventoryItem) {
        return 0;
    }
    inventoryItem->value1 = static_cast<std::uint32_t>(sealedStatus);
    return 1;
}

int cltBaseInventory::IsSaveChangeCoinItem(int a2) { return 0; }

int comp_arry_kind_up(const void* a1, const void* a2) { return 0; }
int comp_arry_kind_down(const void* a1, const void* a2) { return 0; }
int comp_arry_string_up(const void* a1, const void* a2) { return 0; }
int comp_arry_string_down(const void* a1, const void* a2) { return 0; }
int comp_arry_rare_up(const void* a1, const void* a2) { return 0; }
int comp_arry_rare_down(const void* a1, const void* a2) { return 0; }
