#include "Logic/cltFieldItem.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "Info/cltItemKindInfo.h"

cltItemKindInfo* cltFieldItem::m_pclItemKindInfo = nullptr;
Map* cltFieldItem::m_pMap = nullptr;

namespace {
struct FieldItemState {
    std::uint16_t fieldItemID;
    std::uint16_t reserved0;
    int x;
    int y;
    int reserved1;
    int itemCount;
    int reserved2;
    int reserved3;
    int reserved4;
    std::uint16_t itemID;
    std::uint16_t itemInfo;
    std::uint8_t moveType;
    std::uint8_t inUse;
    std::uint8_t reserved5[22];
};

static_assert(sizeof(FieldItemState) <= 0x60, "FieldItemState must fit into cltFieldItem::m_raw");

FieldItemState& GetFieldItemState(cltFieldItem* fieldItem) {
    return *reinterpret_cast<FieldItemState*>(fieldItem);
}

const FieldItemState& GetFieldItemState(const cltFieldItem* fieldItem) {
    return *reinterpret_cast<const FieldItemState*>(fieldItem);
}
}

void cltFieldItem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, Map* map) {
    m_pclItemKindInfo = itemKindInfo;
    m_pMap = map;
}

cltFieldItem::cltFieldItem() {
    std::memset(m_raw, 0, sizeof(m_raw));
}

cltFieldItem::~cltFieldItem() = default;

int cltFieldItem::Init(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
    FieldItemState& state = GetFieldItemState(this);
    std::memset(&state, 0, sizeof(state));
    state.fieldItemID = fieldItemID;
    state.x = x;
    state.y = y;
    state.itemCount = itemCount;
    state.itemID = itemID;
    state.itemInfo = itemInfo;
    state.moveType = moveType;
    state.inUse = 1;
    return 1;
}

void cltFieldItem::Free() {
    std::memset(m_raw, 0, sizeof(m_raw));
}

void cltFieldItem::Poll() {}
void cltFieldItem::PrepareDrawing() {}
void cltFieldItem::Draw() {}

bool cltFieldItem::IsUsing() {
    return GetFieldItemState(this).inUse != 0;
}

void cltFieldItem::GetItemInfo(uint16_t* fieldItemID, int* x, int* y, int* itemCount, uint16_t* itemID, uint16_t* itemInfo) {
    const FieldItemState& state = GetFieldItemState(this);
    if (fieldItemID) *fieldItemID = state.fieldItemID;
    if (x) *x = state.x;
    if (y) *y = state.y;
    if (itemCount) *itemCount = state.itemCount;
    if (itemID) *itemID = state.itemID;
    if (itemInfo) *itemInfo = state.itemInfo;
}

void cltFieldItem::GetItemInfo(int* itemCount, uint16_t* itemID, uint16_t* itemInfo) {
    const FieldItemState& state = GetFieldItemState(this);
    if (itemCount) *itemCount = state.itemCount;
    if (itemID) *itemID = state.itemID;
    if (itemInfo) *itemInfo = state.itemInfo;
}

uint16_t cltFieldItem::GetFieldItemID() {
    return GetFieldItemState(this).fieldItemID;
}

cltFieldItemManager::cltFieldItemManager() {
    Free();
}

cltFieldItemManager::~cltFieldItemManager() = default;

void cltFieldItemManager::Free() {
    for (auto& fieldItem : m_items) {
        fieldItem.Free();
    }
    std::memset(m_tempBuffer, 0, sizeof(m_tempBuffer));
    m_tempBufferCount = 0;
    m_pad100001 = 0;
    m_activeItemCount = 0;
    m_cachedVisibleCount = 0;
    m_pad100006 = 0;
    m_lastSysMsgTick = 0;
    std::memset(m_cachedVisibleItems, 0, sizeof(m_cachedVisibleItems));
}

void cltFieldItemManager::AddItem(uint16_t fieldItemID, int x, int y, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
    for (auto& fieldItem : m_items) {
        if (!fieldItem.IsUsing()) {
            fieldItem.Init(fieldItemID, x, y, itemCount, itemID, itemInfo, moveType);
            ++m_activeItemCount;
            return;
        }
    }
}

void cltFieldItemManager::DelItem(uint32_t accountID, uint16_t fieldItemID, uint16_t itemInfo, int isPetPickup) {
    DelItem(fieldItemID, itemInfo);
    DelTempBufferItem(fieldItemID);
}

void cltFieldItemManager::DelItem(uint16_t fieldItemID, uint16_t itemInfo) {
    for (auto& fieldItem : m_items) {
        if (!fieldItem.IsUsing()) {
            continue;
        }
        if (fieldItem.GetFieldItemID() == fieldItemID) {
            uint16_t currentItemInfo = 0;
            fieldItem.GetItemInfo(nullptr, &currentItemInfo, nullptr);
            if (!itemInfo || currentItemInfo == itemInfo) {
                fieldItem.Free();
                if (m_activeItemCount > 0) {
                    --m_activeItemCount;
                }
                return;
            }
        }
    }
}

void cltFieldItemManager::Poll() {
    for (auto& fieldItem : m_items) {
        if (fieldItem.IsUsing()) {
            fieldItem.Poll();
        }
    }
}

void cltFieldItemManager::PrepareDrawing() {
    m_cachedVisibleCount = 0;
    for (auto& fieldItem : m_items) {
        if (!fieldItem.IsUsing()) {
            continue;
        }
        fieldItem.PrepareDrawing();
        if (m_cachedVisibleCount < 1000) {
            m_cachedVisibleItems[m_cachedVisibleCount++] = &fieldItem;
        }
    }
}

void cltFieldItemManager::Draw() {
    for (std::uint16_t visibleIndex = 0; visibleIndex < m_cachedVisibleCount; ++visibleIndex) {
        if (m_cachedVisibleItems[visibleIndex]) {
            m_cachedVisibleItems[visibleIndex]->Draw();
        }
    }
}

int cltFieldItemManager::GetNearItemInfo(float x, float y, uint16_t* outFieldItemID, float* outX, float* outY, int checkPet, int rangeX, int rangeY) {
    float nearestDistance = 0.0f;
    bool foundItem = false;

    for (auto& fieldItem : m_items) {
        if (!fieldItem.IsUsing()) {
            continue;
        }
        uint16_t fieldItemID = 0;
        int itemX = 0;
        int itemY = 0;
        int itemCount = 0;
        uint16_t itemID = 0;
        uint16_t itemInfo = 0;
        fieldItem.GetItemInfo(&fieldItemID, &itemX, &itemY, &itemCount, &itemID, &itemInfo);

        const float deltaX = std::fabs(x - static_cast<float>(itemX));
        const float deltaY = std::fabs(y - static_cast<float>(itemY));
        if ((rangeX > 0 && deltaX > rangeX) || (rangeY > 0 && deltaY > rangeY)) {
            continue;
        }

        const float distance = deltaX + deltaY;
        if (!foundItem || distance < nearestDistance) {
            nearestDistance = distance;
            foundItem = true;
            if (outFieldItemID) *outFieldItemID = fieldItemID;
            if (outX) *outX = static_cast<float>(itemX);
            if (outY) *outY = static_cast<float>(itemY);
        }
    }

    return foundItem ? 1 : 0;
}

void cltFieldItemManager::SetSysMsg(int textID) {
    m_lastSysMsgTick = static_cast<std::uint32_t>(textID);
}

cltFieldItem* cltFieldItemManager::GetFieldItem(uint16_t fieldItemID) {
    for (auto& fieldItem : m_items) {
        if (fieldItem.IsUsing() && fieldItem.GetFieldItemID() == fieldItemID) {
            return &fieldItem;
        }
    }
    return nullptr;
}

int cltFieldItemManager::DelTempBufferItem(uint16_t fieldItemID) {
    for (std::uint8_t bufferIndex = 0; bufferIndex < m_tempBufferCount; ++bufferIndex) {
        if (m_tempBuffer[bufferIndex].fieldItemID == fieldItemID) {
            std::memmove(&m_tempBuffer[bufferIndex], &m_tempBuffer[bufferIndex + 1], (m_tempBufferCount - bufferIndex - 1) * sizeof(cltFieldItemBufferEntry));
            --m_tempBufferCount;
            return 1;
        }
    }
    return 0;
}

void cltFieldItemManager::PushBuffer(uint32_t accountID, uint16_t fieldItemID, int itemCount, uint16_t itemID, uint16_t itemInfo, uint8_t moveType) {
    if (m_tempBufferCount >= 200) {
        return;
    }
    cltFieldItemBufferEntry& bufferEntry = m_tempBuffer[m_tempBufferCount++];
    bufferEntry.accountID = accountID;
    bufferEntry.fieldItemID = fieldItemID;
    bufferEntry.itemCount = itemCount;
    bufferEntry.itemID = itemID;
    bufferEntry.itemInfo = itemInfo;
    bufferEntry.moveType = moveType;
}

void cltFieldItemManager::PopBuffer(uint32_t accountID) {
    std::uint8_t writeIndex = 0;
    for (std::uint8_t readIndex = 0; readIndex < m_tempBufferCount; ++readIndex) {
        if (m_tempBuffer[readIndex].accountID != accountID) {
            if (writeIndex != readIndex) {
                m_tempBuffer[writeIndex] = m_tempBuffer[readIndex];
            }
            ++writeIndex;
        }
    }
    m_tempBufferCount = writeIndex;
}
