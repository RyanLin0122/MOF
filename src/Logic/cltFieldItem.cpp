#include "Logic/cltFieldItem.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <windows.h>

#include "Info/cltItemKindInfo.h"
#include "global.h"

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
