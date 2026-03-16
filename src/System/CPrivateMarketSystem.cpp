#include "System/CPrivateMarketSystem.h"

#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Logic/cltBaseInventory.h"

cltItemKindInfo* CPrivateMarketSystem::m_pclItemKindInfo = nullptr;

CPrivateMarketSystem g_clPrivateMarketSystem;

void CPrivateMarketSystem::InitializeStaticVariable(cltItemKindInfo* pItemKindInfo) {
    m_pclItemKindInfo = pItemKindInfo;
}

CPrivateMarketSystem::CPrivateMarketSystem() {
}

CPrivateMarketSystem::~CPrivateMarketSystem() {
}

int CPrivateMarketSystem::Initialize(cltBaseInventory* pInventory) {
    std::memset(m_sellingItems, 0, sizeof(m_sellingItems));
    m_sellingCount = 0;
    m_pInventory = pInventory;
    return 1;
}

int CPrivateMarketSystem::Initialize() {
    std::memset(m_sellingItems, 0, sizeof(m_sellingItems));
    m_sellingCount = 0;
    return 1;
}

int CPrivateMarketSystem::CanAddPrivateMarketList(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price) {
    if (m_sellingCount > MAX_SELLING_ITEMS)
        return 2;

    auto* item = m_pInventory->GetInventoryItem(invenSlot);
    if (!item)
        return 3;

    if (!m_pclItemKindInfo->IsTradeItem(item->itemKind))
        return 4;

    if (qty > item->itemQty)
        return 5;

    if (!IsEmptySlot(slot))
        return 6;

    if (IsExistInvenSlot(invenSlot) == 1)
        return 3;

    auto* sealed = m_pInventory->GetItemSealed(invenSlot);
    if (sealed == reinterpret_cast<strInventoryItem*>(1))
        return 10;

    int v10 = -(sealed != reinterpret_cast<strInventoryItem*>(2));
    v10 = v10 & 0xF6;
    return v10 + 11;
}

int CPrivateMarketSystem::AddPrivateMarketList(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price, unsigned int param) {
    AddItem(slot, itemKind, invenSlot, qty, price, param);
    IncreaseSellingItemListCount();
    return 1;
}

int CPrivateMarketSystem::AddItem(std::uint16_t slot, std::uint16_t itemKind, std::uint16_t invenSlot, std::uint16_t qty, unsigned int price, unsigned int param) {
    stSellingItem& item = m_sellingItems[slot];
    item.itemKind = itemKind;
    item.invenSlot = invenSlot;
    item.qty = qty;
    item.price = price;
    item.param = param;
    return 1;
}

int CPrivateMarketSystem::SetItemQty(std::uint16_t slot, std::uint16_t qty) {
    m_sellingItems[slot].qty = qty;
    return 1;
}

int CPrivateMarketSystem::CanDelPrivateMarketList(std::uint16_t slot) {
    return 2 - !IsEmptySlot(slot);
}

std::uint16_t CPrivateMarketSystem::DelPrivateMarketList(std::uint16_t slot) {
    DelItem(slot);
    DecreaseSellingItemListCount();
    return 1;
}

std::uint16_t CPrivateMarketSystem::DelItem(std::uint16_t slot) {
    std::memset(&m_sellingItems[slot], 0, sizeof(stSellingItem));
    return 1;
}

void CPrivateMarketSystem::PrivateMarketItemToInventory() {
    for (std::uint16_t i = 0; i < MAX_SELLING_ITEMS; ++i) {
        if (!IsEmptySlot(i))
            DelPrivateMarketList(i);
    }
}

std::uint16_t CPrivateMarketSystem::CanSellPrivateMarketItem(std::uint16_t slot, std::uint16_t qty) {
    if (IsEmptySlot(slot))
        return 2;

    stSellingItem& item = m_sellingItems[slot];
    if (m_pInventory->CanDelInventoryItem(item.invenSlot, qty)) {
        auto* invItem = m_pInventory->GetInventoryItem(item.invenSlot);
        if (invItem) {
            if (invItem->itemKind == item.itemKind) {
                return (invItem->itemQty < qty) ? 4 : 1;
            } else {
                return 7;
            }
        } else {
            return 7;
        }
    } else {
        return 7;
    }
}

void CPrivateMarketSystem::SellPrivateMarketItem(std::uint16_t slot, std::uint16_t qty) {
    stSellingItem& item = m_sellingItems[slot];
    m_pInventory->DelInventoryItem(item.invenSlot, qty, nullptr);

    if (item.qty == qty) {
        std::memset(&item, 0, sizeof(stSellingItem));
        DecreaseSellingItemListCount();
    } else {
        item.qty -= qty;
    }
}

int CPrivateMarketSystem::IsEmptySlot(std::uint16_t slot) {
    return m_sellingItems[slot].itemKind == 0;
}

std::int16_t CPrivateMarketSystem::GetEmptySlot() {
    for (int i = 0; i < MAX_SELLING_ITEMS; ++i) {
        if (m_sellingItems[i].itemKind != 0)
            return static_cast<std::int16_t>(i);
    }
    return -1;
}

int CPrivateMarketSystem::CanGetSellingItemList() {
    return m_sellingCount != 0;
}

stSellingItem* CPrivateMarketSystem::GetSellingItemList() {
    return m_sellingItems;
}

stSellingItem* CPrivateMarketSystem::GetSellingItemInfo(std::uint16_t slot) {
    if (slot >= MAX_SELLING_ITEMS)
        return nullptr;
    return &m_sellingItems[slot];
}

int CPrivateMarketSystem::IncreaseSellingItemListCount() {
    ++m_sellingCount;
    return 1;
}

int CPrivateMarketSystem::DecreaseSellingItemListCount() {
    --m_sellingCount;
    return 1;
}

int CPrivateMarketSystem::IsExistInvenSlot(std::uint16_t invenSlot) {
    for (int i = 0; i < MAX_SELLING_ITEMS; ++i) {
        if (m_sellingItems[i].itemKind != 0 && m_sellingItems[i].invenSlot == invenSlot)
            return 1;
    }
    return 0;
}
