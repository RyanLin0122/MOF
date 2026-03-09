#include "System/cltQuickSlotSystem.h"

#include <algorithm>
#include <cstdio>

#include "Network/CMofMsg.h"
#include "Logic/cltBaseInventory.h"
#include "Info/cltItemKindInfo.h"

cltItemKindInfo* cltQuickSlotSystem::m_pclItemKindInfo = nullptr;
cltSkillKindInfo* cltQuickSlotSystem::m_pclSkillKindInfo = nullptr;

namespace {
constexpr std::uint32_t kQuickSlotItemTag = 0x10000;
constexpr std::uint32_t kQuickSlotSkillTag = 0x20000;
constexpr std::uint32_t kQuickSlotEmblemTag = 0x40000;
constexpr std::uint32_t kQuickSlotTypeMask = 0x70000;
constexpr int kQuickSlotPageCount = 3;
constexpr int kQuickSlotEntriesPerPage = 12;

bool IsItemSlot(std::uint32_t rawSlotValue) {
    return (rawSlotValue & kQuickSlotTypeMask) == kQuickSlotItemTag;
}

std::uint16_t GetPayload(std::uint32_t rawSlotValue) {
    return static_cast<std::uint16_t>(rawSlotValue);
}
}

void cltQuickSlotSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltSkillKindInfo* skillKindInfo) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclSkillKindInfo = skillKindInfo;
}

cltQuickSlotSystem::cltQuickSlotSystem() = default;

void cltQuickSlotSystem::Initialize(cltBaseInventory* baseInventory, cltMyItemSystem* myItemSystem, cltQuestSystem* questSystem, CMofMsg* message) {
    m_pBaseInventory = baseInventory;
    m_pMyItemSystem = myItemSystem;
    m_pQuestSystem = questSystem;
    m_slotValues = {};
    m_dirtyFlags = {};
    m_hasPendingChange = 0;

    if (!message) {
        return;
    }

    std::uint8_t slotCount = 0;
    message->Get_BYTE(&slotCount);
    for (int absoluteSlotIndex = 0; absoluteSlotIndex < slotCount; ++absoluteSlotIndex) {
        std::uint8_t slotIndex = 0;
        std::uint32_t rawSlotValue = 0;
        message->Get_BYTE(&slotIndex);
        message->Get_DWORD(&rawSlotValue);
        const int pageIndex = slotIndex / kQuickSlotEntriesPerPage;
        const int indexInsidePage = slotIndex % kQuickSlotEntriesPerPage;
        if (pageIndex >= kQuickSlotPageCount) {
            continue;
        }
        m_slotValues[pageIndex][indexInsidePage] = rawSlotValue;
        if (IsItemSlot(rawSlotValue) && m_pBaseInventory) {
            auto* inventoryItem = m_pBaseInventory->GetInventoryItem(GetPayload(rawSlotValue));
            if (!inventoryItem || !inventoryItem->itemKind) {
                OnItemOuted(static_cast<std::uint8_t>(GetPayload(rawSlotValue)), pageIndex);
            }
        }
    }
}

std::uint16_t cltQuickSlotSystem::GetTransportSlot() {
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            const std::uint32_t rawSlotValue = m_slotValues[pageIndex][slotIndex];
            if (!IsItemSlot(rawSlotValue) || !m_pBaseInventory) {
                continue;
            }
            auto* inventoryItem = m_pBaseInventory->GetInventoryItem(GetPayload(rawSlotValue));
            if (inventoryItem && inventoryItem->itemKind) {
                return GetPayload(rawSlotValue);
            }
        }
    }
    return 0;
}

void cltQuickSlotSystem::Free() {
    m_pBaseInventory = nullptr;
    m_pMyItemSystem = nullptr;
    m_pQuestSystem = nullptr;
    m_slotValues = {};
    m_dirtyFlags = {};
    m_hasPendingChange = 0;
}

stItemKindInfo* cltQuickSlotSystem::AddForItem(unsigned int pageIndex, unsigned int slotIndex, std::uint8_t inventorySlotIndex) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage || !m_pBaseInventory || !m_pclItemKindInfo) {
        return nullptr;
    }

    auto* inventoryItem = m_pBaseInventory->GetInventoryItem(inventorySlotIndex);
    if (!inventoryItem || !inventoryItem->itemKind) {
        return nullptr;
    }

    stItemKindInfo* itemInfo = m_pclItemKindInfo->GetItemKindInfo(inventoryItem->itemKind);
    if (!itemInfo) {
        return nullptr;
    }

    m_slotValues[pageIndex][slotIndex] = kQuickSlotItemTag | inventorySlotIndex;
    SetChange(static_cast<int>(pageIndex), static_cast<int>(slotIndex));
    return itemInfo;
}

int cltQuickSlotSystem::AddForSkill(unsigned int pageIndex, unsigned int slotIndex, std::uint16_t skillKind) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage) {
        return 0;
    }
    m_slotValues[pageIndex][slotIndex] = kQuickSlotSkillTag | skillKind;
    SetChange(static_cast<int>(pageIndex), static_cast<int>(slotIndex));
    return 1;
}

int cltQuickSlotSystem::AddForEmblem(unsigned int pageIndex, unsigned int slotIndex, std::uint16_t emblemKind, std::uint16_t* outEmblemKind, unsigned int* outRawSlotValue) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage) {
        return 0;
    }
    m_slotValues[pageIndex][slotIndex] = kQuickSlotEmblemTag | emblemKind;
    if (outEmblemKind) {
        *outEmblemKind = emblemKind;
    }
    if (outRawSlotValue) {
        *outRawSlotValue = m_slotValues[pageIndex][slotIndex];
    }
    SetChange(static_cast<int>(pageIndex), static_cast<int>(slotIndex));
    return 1;
}

int cltQuickSlotSystem::Del(unsigned int pageIndex, unsigned int slotIndex) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage) {
        return 0;
    }
    m_slotValues[pageIndex][slotIndex] = 0;
    SetChange(static_cast<int>(pageIndex), static_cast<int>(slotIndex));
    return 1;
}

int cltQuickSlotSystem::Move(unsigned int pageIndex, unsigned int slotIndex, unsigned int targetAbsoluteSlotIndex) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage || targetAbsoluteSlotIndex >= kQuickSlotPageCount * kQuickSlotEntriesPerPage) {
        return 0;
    }

    const unsigned int targetPageIndex = targetAbsoluteSlotIndex / kQuickSlotEntriesPerPage;
    const unsigned int targetSlotIndex = targetAbsoluteSlotIndex % kQuickSlotEntriesPerPage;
    std::swap(m_slotValues[pageIndex][slotIndex], m_slotValues[targetPageIndex][targetSlotIndex]);
    SetChange(static_cast<int>(pageIndex), static_cast<int>(slotIndex));
    SetChange(static_cast<int>(targetPageIndex), static_cast<int>(targetSlotIndex));
    return 1;
}

unsigned int cltQuickSlotSystem::Get(unsigned int pageIndex, unsigned int slotIndex) {
    if (pageIndex >= kQuickSlotPageCount || slotIndex >= kQuickSlotEntriesPerPage) {
        return 0;
    }
    return m_slotValues[pageIndex][slotIndex];
}

void cltQuickSlotSystem::OnItemMerged(std::uint8_t sourceInventorySlotIndex, std::uint8_t mergedInventorySlotIndex) {
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (m_slotValues[pageIndex][slotIndex] == (kQuickSlotItemTag | sourceInventorySlotIndex)) {
                m_slotValues[pageIndex][slotIndex] = kQuickSlotItemTag | mergedInventorySlotIndex;
                SetChange(pageIndex, slotIndex);
            }
        }
    }
}

void cltQuickSlotSystem::OnItemMoved(std::uint8_t sourceInventorySlotIndex, std::uint8_t targetInventorySlotIndex) {
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (m_slotValues[pageIndex][slotIndex] == (kQuickSlotItemTag | sourceInventorySlotIndex)) {
                m_slotValues[pageIndex][slotIndex] = kQuickSlotItemTag | targetInventorySlotIndex;
                SetChange(pageIndex, slotIndex);
            } else if (m_slotValues[pageIndex][slotIndex] == (kQuickSlotItemTag | targetInventorySlotIndex)) {
                m_slotValues[pageIndex][slotIndex] = kQuickSlotItemTag | sourceInventorySlotIndex;
                SetChange(pageIndex, slotIndex);
            }
        }
    }
}

void cltQuickSlotSystem::OnItemOuted(std::uint8_t inventorySlotIndex, int pageIndex) {
    const std::uint32_t targetRawSlotValue = kQuickSlotItemTag | inventorySlotIndex;
    if (pageIndex >= 0 && pageIndex < kQuickSlotPageCount) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (m_slotValues[pageIndex][slotIndex] == targetRawSlotValue) {
                m_slotValues[pageIndex][slotIndex] = 0;
                SetChange(pageIndex, slotIndex);
            }
        }
        return;
    }

    for (int currentPageIndex = 0; currentPageIndex < kQuickSlotPageCount; ++currentPageIndex) {
        OnItemOuted(inventorySlotIndex, currentPageIndex);
    }
}

void cltQuickSlotSystem::OnSkillDeleted(std::uint16_t skillKind) {
    const std::uint32_t targetRawSlotValue = kQuickSlotSkillTag | skillKind;
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (m_slotValues[pageIndex][slotIndex] == targetRawSlotValue) {
                m_slotValues[pageIndex][slotIndex] = 0;
                SetChange(pageIndex, slotIndex);
            }
        }
    }
}

void cltQuickSlotSystem::OnSkillAdded(std::uint16_t skillKind) {}

void cltQuickSlotSystem::OnClassReseted() {
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (!IsItemSlot(m_slotValues[pageIndex][slotIndex])) {
                m_slotValues[pageIndex][slotIndex] = 0;
                SetChange(pageIndex, slotIndex);
            }
        }
    }
}

void cltQuickSlotSystem::OnItemSorted(strInventoryItemForSort* sortedItems) {
    if (!sortedItems) {
        return;
    }

    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            std::uint32_t& rawSlotValue = m_slotValues[pageIndex][slotIndex];
            if (!IsItemSlot(rawSlotValue)) {
                continue;
            }
            for (int sortedIndex = 0; sortedIndex <= 71; ++sortedIndex) {
                if (GetPayload(rawSlotValue) == sortedItems[sortedIndex].sortKey) {
                    rawSlotValue = kQuickSlotItemTag | static_cast<std::uint16_t>(sortedIndex);
                    SetChange(pageIndex, slotIndex);
                    break;
                }
            }
        }
    }
}

void cltQuickSlotSystem::GetQuickSlotInfoForDBQuery(char* buffer) {
    if (!buffer) {
        return;
    }
    *buffer = 0;
    if (!IsChange()) {
        return;
    }

    for (int absoluteSlotIndex = 0; absoluteSlotIndex < kQuickSlotPageCount * kQuickSlotEntriesPerPage; ++absoluteSlotIndex) {
        const int pageIndex = absoluteSlotIndex / kQuickSlotEntriesPerPage;
        const int slotIndex = absoluteSlotIndex % kQuickSlotEntriesPerPage;
        if (m_dirtyFlags[pageIndex][slotIndex]) {
            std::sprintf(buffer, "%s%d, %d, ", buffer, absoluteSlotIndex, m_slotValues[pageIndex][slotIndex]);
            m_dirtyFlags[pageIndex][slotIndex] = 0;
        }
    }
    m_hasPendingChange = 0;
}

void cltQuickSlotSystem::OnPremiumQuickSlotEnabled() {}

void cltQuickSlotSystem::OnPremiumQuickSlotDisabled() {}

int cltQuickSlotSystem::OnRelinkItem(unsigned int pageIndex, int inventorySlotIndex) {
    if (pageIndex >= kQuickSlotPageCount || inventorySlotIndex < 0) {
        return 0;
    }
    for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
        if (IsItemSlot(m_slotValues[pageIndex][slotIndex])) {
            m_slotValues[pageIndex][slotIndex] = kQuickSlotItemTag | static_cast<std::uint16_t>(inventorySlotIndex);
            SetChange(static_cast<int>(pageIndex), slotIndex);
            return 1;
        }
    }
    return 0;
}

void cltQuickSlotSystem::SetChange(int pageIndex, int slotIndex) {
    if (pageIndex < 0 || pageIndex >= kQuickSlotPageCount || slotIndex < 0 || slotIndex >= kQuickSlotEntriesPerPage) {
        return;
    }
    m_dirtyFlags[pageIndex][slotIndex] = 1;
    m_hasPendingChange = 1;
}

int cltQuickSlotSystem::IsChange() {
    return static_cast<int>(m_hasPendingChange);
}

void cltQuickSlotSystem::CheckQSLItem() {
    for (int pageIndex = 0; pageIndex < kQuickSlotPageCount; ++pageIndex) {
        for (int slotIndex = 0; slotIndex < kQuickSlotEntriesPerPage; ++slotIndex) {
            if (!IsItemSlot(m_slotValues[pageIndex][slotIndex]) || !m_pBaseInventory) {
                continue;
            }
            auto* inventoryItem = m_pBaseInventory->GetInventoryItem(GetPayload(m_slotValues[pageIndex][slotIndex]));
            if (!inventoryItem || !inventoryItem->itemKind) {
                m_slotValues[pageIndex][slotIndex] = 0;
                SetChange(pageIndex, slotIndex);
            }
        }
    }
}
