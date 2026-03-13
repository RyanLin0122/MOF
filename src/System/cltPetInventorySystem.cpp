#include "System/cltPetInventorySystem.h"

#include <algorithm>
#include <cstring>

#include "Info/cltItemKindInfo.h"
#include "Network/CMofMsg.h"

cltItemKindInfo* cltPetInventorySystem::m_pclItemKindInfo = nullptr;

void cltPetInventorySystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo) { m_pclItemKindInfo = itemKindInfo; }

cltPetInventorySystem::cltPetInventorySystem() {
    std::memset(m_items.data(), 0, sizeof(m_items));
}

void cltPetInventorySystem::Initialize(CMofMsg* msg) {
    std::memset(m_items.data(), 0, sizeof(m_items));
    m_bagNum = 0;
    if (!msg) return;

    msg->Get_BYTE(&m_bagNum);
    int itemCount = 0;
    msg->Get_LONG(&itemCount);

    for (int i = 0; i < itemCount; ++i) {
        std::uint8_t slot = 0xFF;
        msg->Get_BYTE(&slot);
        if (slot >= kMaxSlots) {
            std::uint16_t dummy{};
            msg->Get_WORD(&dummy);
            msg->Get_WORD(&dummy);
            continue;
        }
        msg->Get_WORD(&m_items[slot].itemKind);
        msg->Get_WORD(&m_items[slot].itemQty);
    }
}

void cltPetInventorySystem::Free() {
    std::memset(m_items.data(), 0, sizeof(m_items));
    m_bagNum = 0;
}

int cltPetInventorySystem::CanAddItem(std::uint16_t itemKind, std::uint16_t itemQty) {
    if (!itemKind || !itemQty) return 1;
    if (!m_pclItemKindInfo || !m_pclItemKindInfo->IsPetInventoryItem(itemKind)) return 1904;

    int remained = itemQty;
    const int maxSlot = kSlotsPerBag * static_cast<int>(m_bagNum);
    for (int i = 0; i < maxSlot; ++i) {
        const strPetInventoryItem& item = m_items[i];
        if (item.itemKind != 0 && item.itemKind == itemKind) {
            const int maxPile = m_pclItemKindInfo->GetMaxPileUpNum(itemKind);
            if (remained + item.itemQty <= maxPile) return 0;
            remained += item.itemQty - maxPile;
            if (remained < 0) return 0;
        }
    }

    const int emptyCount = GetEmptyItemSlotNum();
    if (!emptyCount) return 103;
    const int maxPile = m_pclItemKindInfo->GetMaxPileUpNum(itemKind);
    return remained <= emptyCount * maxPile ? 0 : 103;
}

void cltPetInventorySystem::AddItem(std::uint16_t itemKind, int itemQty, std::uint8_t* outChangedSlots) {
    int remained = static_cast<std::uint16_t>(itemQty);
    const int maxSlot = kSlotsPerBag * static_cast<int>(m_bagNum);

    for (int i = 0; i < maxSlot; ++i) {
        if (m_items[i].itemKind && m_items[i].itemKind == itemKind) {
            const int maxPile = m_pclItemKindInfo ? m_pclItemKindInfo->GetMaxPileUpNum(itemKind) : 1;
            if (maxPile - m_items[i].itemQty >= static_cast<std::uint16_t>(remained)) {
                AddItem(static_cast<std::uint8_t>(i), itemKind, static_cast<std::uint16_t>(remained), outChangedSlots);
                return;
            }

            const std::uint16_t addable = static_cast<std::uint16_t>(maxPile - m_items[i].itemQty);
            AddItem(static_cast<std::uint8_t>(i), itemKind, addable, outChangedSlots);
            remained -= addable;
            if (static_cast<std::uint16_t>(remained) == 0) return;
        }
    }

    const int maxPile = m_pclItemKindInfo ? m_pclItemKindInfo->GetMaxPileUpNum(itemKind) : 1;
    std::uint16_t neededSlots = static_cast<std::uint16_t>(remained / maxPile);
    if (static_cast<std::uint16_t>(remained) % maxPile > 0) ++neededSlots;

    while (neededSlots--) {
        const std::uint8_t slot = static_cast<std::uint8_t>(GetFirstEmptyItemSlot());
        std::uint16_t chunk = 0;
        if (static_cast<std::uint16_t>(remained) <= maxPile) {
            chunk = static_cast<std::uint16_t>(remained);
        } else {
            chunk = static_cast<std::uint16_t>(maxPile);
            remained -= maxPile;
        }
        AddItem(slot, itemKind, chunk, outChangedSlots);
    }
}

void cltPetInventorySystem::AddItem(std::uint8_t slot, std::uint16_t itemKind, std::uint16_t itemQty, std::uint8_t* outChangedSlots) {
    if (slot >= kMaxSlots) return;
    m_items[slot].itemKind = itemKind;
    m_items[slot].itemQty = static_cast<std::uint16_t>(m_items[slot].itemQty + itemQty);
    if (outChangedSlots) outChangedSlots[slot] = 1;
}

int cltPetInventorySystem::CanMoveItem(std::uint8_t from, std::uint8_t to) {
    if (!IsValidSlot(from)) return 0;
    if (!IsValidSlot(to)) return 0;
    if (from == to) return 0;
    strPetInventoryItem* src = GetPetInventoryItem(from);
    return src && src->itemKind != 0;
}

void cltPetInventorySystem::MoveItem(std::uint8_t from, std::uint8_t to, std::uint8_t* outChangedSlots) {
    strPetInventoryItem* src = GetPetInventoryItem(from);
    strPetInventoryItem* dst = GetPetInventoryItem(to);
    if (!src || !dst) return;

    std::uint8_t changedFrom = from;
    if (src->itemKind == dst->itemKind) {
        const auto* info = m_pclItemKindInfo ? m_pclItemKindInfo->GetItemKindInfo(src->itemKind) : nullptr;
        if (info) {
            const std::uint8_t maxPile = info->m_byMaxPileUpNum;
            if (maxPile > dst->itemQty) {
                const std::uint16_t srcQty = src->itemQty;
                const std::uint16_t dstQty = dst->itemQty;
                if (maxPile >= dstQty + srcQty) {
                    dst->itemQty = static_cast<std::uint16_t>(dstQty + srcQty);
                    EmptyPetInventoryItem(from);
                } else {
                    src->itemQty = static_cast<std::uint16_t>(dstQty + srcQty - maxPile);
                    dst->itemQty = maxPile;
                }
                goto changed;
            }
        }
    }

    std::swap(*src, *dst);

changed:
    if (outChangedSlots) {
        outChangedSlots[changedFrom] = 1;
        outChangedSlots[to] = 1;
    }
}

int cltPetInventorySystem::CanDelItemBySlot(std::uint8_t slot, std::uint16_t qty) {
    if (!IsValidSlot(slot)) return 0;
    if (!qty) return 0;
    const strPetInventoryItem& item = m_items[slot];
    if (!item.itemKind) return 0;
    return item.itemQty >= qty;
}

int cltPetInventorySystem::CanDelItemBySlot(std::uint8_t slot) {
    const strPetInventoryItem* item = GetPetInventoryItem(slot);
    return item ? CanDelItemBySlot(slot, item->itemQty) : 0;
}

int cltPetInventorySystem::CanDelItemByItemKind(std::uint16_t itemKind, std::uint16_t qty) {
    return GetInventoryItemQuantity(itemKind) >= qty;
}

void cltPetInventorySystem::DelItemBySlot(std::uint8_t slot, std::uint16_t qty, std::uint8_t* outChangedSlots) {
    if (slot >= kMaxSlots) return;
    strPetInventoryItem& item = m_items[slot];
    item.itemQty = static_cast<std::uint16_t>(item.itemQty - qty);
    if (!item.itemQty) {
        item.itemKind = 0;
    }
    if (outChangedSlots) outChangedSlots[slot] = 1;
}

void cltPetInventorySystem::DelItemBySlot(std::uint8_t slot, std::uint8_t* outChangedSlots) {
    strPetInventoryItem* item = GetPetInventoryItem(slot);
    if (!item) return;
    DelItemBySlot(slot, item->itemQty, outChangedSlots);
}

void cltPetInventorySystem::DelItemByItemKind(std::uint16_t itemKind, std::uint16_t qty, std::uint8_t* outChangedSlots) {
    const int maxSlot = kSlotsPerBag * static_cast<int>(m_bagNum);
    for (int i = 0; i < maxSlot; ++i) {
        if (m_items[i].itemKind == itemKind) {
            const std::uint16_t has = m_items[i].itemQty;
            if (has > qty) {
                DelItemBySlot(static_cast<std::uint8_t>(i), qty, outChangedSlots);
                return;
            }
            qty = static_cast<std::uint16_t>(qty - has);
            DelItemBySlot(static_cast<std::uint8_t>(i), has, outChangedSlots);
            if (!qty) return;
        }
    }
}

int cltPetInventorySystem::IsValidSlot(std::uint8_t slot) {
    return slot != 0xFF;
}

strPetInventoryItem* cltPetInventorySystem::GetPetInventoryItem(std::uint8_t slot) {
    if (!IsValidSlot(slot) || slot >= kMaxSlots) return nullptr;
    return &m_items[slot];
}

std::uint8_t cltPetInventorySystem::GetPetBagNum() { return m_bagNum; }

void cltPetInventorySystem::EmptyPetInventoryItem(std::uint8_t slot) {
    strPetInventoryItem* item = GetPetInventoryItem(slot);
    if (item) {
        item->itemKind = 0;
        item->itemQty = 0;
    }
}

int cltPetInventorySystem::CanIncreasePetBagNum() {
    return m_bagNum < kMaxBags;
}

void cltPetInventorySystem::IncreasePetBagNum() {
    ++m_bagNum;
}

void cltPetInventorySystem::OnPetCreated(std::uint8_t bagInitCount) {
    std::memset(m_items.data(), 0, sizeof(m_items));
    m_bagNum = bagInitCount;
}

void cltPetInventorySystem::OnPetDeleted() {
    std::memset(m_items.data(), 0, sizeof(m_items));
    m_bagNum = 0;
}

int cltPetInventorySystem::GetEmptyItemSlotNum() {
    int result = 0;
    int count = kSlotsPerBag * static_cast<int>(m_bagNum);
    for (int i = 0; i < count; ++i) {
        if (!m_items[i].itemKind) ++result;
    }
    return result;
}

int cltPetInventorySystem::GetFirstEmptyItemSlot() {
    const int count = kSlotsPerBag * static_cast<int>(m_bagNum);
    if (count <= 0) return -1;
    for (int i = 0; i < count; ++i) {
        if (!m_items[i].itemKind) return i;
    }
    return -1;
}

int cltPetInventorySystem::IsPetInventoryEmpty() {
    const int count = kSlotsPerBag * static_cast<int>(m_bagNum);
    if (count <= 0) return 1;
    for (int i = 0; i < count; ++i) {
        if (m_items[i].itemKind) return 0;
    }
    return 1;
}

std::uint16_t cltPetInventorySystem::GetInventoryItemQuantity(std::uint16_t itemKind) {
    if (!itemKind) return 0;
    std::uint16_t result = 0;
    int count = kSlotsPerBag * static_cast<int>(m_bagNum);
    for (int i = 0; i < count; ++i) {
        if (m_items[i].itemKind == itemKind) {
            result = static_cast<std::uint16_t>(result + m_items[i].itemQty);
        }
    }
    return result;
}

int cltPetInventorySystem::GetSlotNumByItemID(std::uint16_t itemKind) {
    if (!itemKind) return 0;
    const int count = kSlotsPerBag * static_cast<int>(m_bagNum);
    if (count <= 0) return 255;
    for (int i = 0; i < count; ++i) {
        if (m_items[i].itemKind == itemKind) return i;
    }
    return 255;
}
