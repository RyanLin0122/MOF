#include "System/cltEmoticonSystem.h"

#include <cstring>

#include "Info/cltEmoticonKindInfo.h"
#include "Logic/cltBaseInventory.h"

cltBaseInventory* cltEmoticonSystem::m_pcltBaseInventory = nullptr;
cltEmoticonKindInfo* cltEmoticonSystem::m_pcltEmoticonKindInfo = nullptr;

cltEmoticonSystem::cltEmoticonSystem() = default;
cltEmoticonSystem::~cltEmoticonSystem() = default;

void cltEmoticonSystem::InitializeStaticVariable(cltBaseInventory* baseInventory, cltEmoticonKindInfo* emoticonKindInfo) {
    m_pcltBaseInventory = baseInventory;
    m_pcltEmoticonKindInfo = emoticonKindInfo;
}

int cltEmoticonSystem::Initialize(std::uint8_t* shortcutData) {
    std::memset(m_shortCut.data(), 0, sizeof(m_shortCut));
    std::memset(m_kindSlots.data(), 0, sizeof(m_kindSlots));

    m_ownedSetCount = 0;
    for (int i = 0; i < 5; ++i) {
        m_kindSlots[i] = static_cast<std::uint16_t>(2 * i + 1);
    }

    ++m_ownedSetCount;
    for (int slot = 168; slot <= 191; ++slot) {
        auto* inv = m_pcltBaseInventory ? m_pcltBaseInventory->GetInventoryItem(static_cast<std::uint16_t>(slot)) : nullptr;
        if (!inv || !inv->itemKind || !inv->itemQty) continue;
        if (!m_pcltEmoticonKindInfo || !m_pcltEmoticonKindInfo->IsEmoticonItem(inv->itemKind)) continue;

        auto* info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(inv->itemKind);
        if (!info) continue;

        for (int i = 0; i < 5; ++i) {
            const int idx = 5 * m_ownedSetCount + i;
            m_kindSlots[idx] = static_cast<std::uint16_t>(info->slot[i].kind);
        }
        ++m_ownedSetCount;
    }

    for (int i = 0; i < 5; ++i) {
        m_shortCut[i] = shortcutData ? shortcutData[i] : 0;
    }
    return 1;
}

int cltEmoticonSystem::AutoReload() {
    std::memset(m_kindSlots.data(), 0, sizeof(m_kindSlots));
    m_ownedSetCount = 0;

    for (int i = 0; i < 5; ++i) {
        m_kindSlots[i] = static_cast<std::uint16_t>(2 * i + 1);
    }

    ++m_ownedSetCount;
    for (int slot = 168; slot <= 191; ++slot) {
        auto* inv = m_pcltBaseInventory ? m_pcltBaseInventory->GetInventoryItem(static_cast<std::uint16_t>(slot)) : nullptr;
        if (!inv || !inv->itemKind || !inv->itemQty) continue;
        if (!m_pcltEmoticonKindInfo || !m_pcltEmoticonKindInfo->IsEmoticonItem(inv->itemKind)) continue;

        auto* info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(inv->itemKind);
        if (!info) continue;

        for (int i = 0; i < 5; ++i) {
            const int idx = 5 * m_ownedSetCount + i;
            m_kindSlots[idx] = static_cast<std::uint16_t>(info->slot[i].kind);
        }
        ++m_ownedSetCount;
    }

    return 1;
}

stEmoticonItemInfo* cltEmoticonSystem::GetBuyEmoticonItemInfoByIndex(std::uint8_t index) {
    if (m_ownedSetCount > index) {
        return m_pcltEmoticonKindInfo->GetEmoticonItemInfoByKind(m_kindSlots[5 * index]);
    }
    return nullptr;
}

int cltEmoticonSystem::AddShortCutData(std::uint8_t index, int data) {
    if (index >= 5) return 0;
    m_shortCut[index] = data;
    return 1;
}

int cltEmoticonSystem::DelShortCutData(std::uint8_t index) {
    if (index >= 5) return 0;
    if (!m_shortCut[index]) return 0;
    m_shortCut[index] = 0;
    return 1;
}

int cltEmoticonSystem::MoveShortCutData(std::uint8_t from, std::uint8_t to) {
    if (from >= 5 || to >= 5) return 0;
    const int src = m_shortCut[from];
    if (!src) return 0;
    m_shortCut[from] = m_shortCut[to];
    m_shortCut[to] = src;
    return 1;
}

int cltEmoticonSystem::GetData(std::uint8_t index) {
    if (index >= 5) return 0;
    return m_shortCut[index];
}

void cltEmoticonSystem::AddEmoticonItem(std::uint16_t kind) {
    if (!m_pcltEmoticonKindInfo || !m_pcltEmoticonKindInfo->IsEmoticonItem(kind)) return;
    auto* info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(kind);
    if (!info) return;

    if (m_ownedSetCount >= 24) return;
    for (int i = 0; i < 5; ++i) {
        const int idx = 5 * m_ownedSetCount + i;
        m_kindSlots[idx] = static_cast<std::uint16_t>(info->slot[i].kind);
    }
    ++m_ownedSetCount;
}

void cltEmoticonSystem::DelEmoticonItem(std::uint16_t kind) {
    if (!m_pcltEmoticonKindInfo || !m_pcltEmoticonKindInfo->IsEmoticonItem(kind)) return;
    if (!m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(kind)) return;

    for (int group = 0; group < m_ownedSetCount; ++group) {
        for (int i = 0; i < 5; ++i) {
            if (m_kindSlots[5 * group + i] == kind) {
                std::memmove(&m_kindSlots[5 * group], &m_kindSlots[5 * group + 5],
                             sizeof(std::uint16_t) * (5 * m_ownedSetCount - 5 * group));
                --m_ownedSetCount;
                return;
            }
        }
    }
}
