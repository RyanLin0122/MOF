#include "System/cltCashshopSystem.h"

#include "Logic/cltCashShopItem.h"

namespace {
constexpr std::uint16_t kMaxBoughtItem = 1000;
constexpr std::uint8_t kMaxMsgBoughtItem = 100;

static int FindBoughtIndex(const std::array<strBoughtCashshopItemInfo, kMaxBoughtItem>& arr,
                           std::uint16_t n, std::int64_t id) {
    for (std::uint16_t i = 0; i < n; ++i) {
        if (arr[i].id == id) return i;
    }
    return -1;
}

static void RemoveBoughtAt(std::array<strBoughtCashshopItemInfo, kMaxBoughtItem>& arr,
                           std::uint16_t& n, int idx) {
    if (idx < 0 || idx >= n) return;
    for (int i = idx; i + 1 < n; ++i) arr[i] = arr[i + 1];
    arr[n - 1] = {};
    --n;
}

static bool AppendBought(std::array<strBoughtCashshopItemInfo, kMaxBoughtItem>& arr,
                         std::uint16_t& n,
                         std::int64_t id, std::uint16_t itemKind, std::uint16_t qty) {
    if (n >= kMaxBoughtItem) return false;
    arr[n].id = id;
    arr[n].itemKind = itemKind;
    arr[n].qty = qty;
    arr[n].selected = 0;
    ++n;
    return true;
}

static bool HasName(const strVerifiedCharInfo& v) {
    return v.name[0] != '\0';
}

static unsigned int ComputeExtraValue(std::uint16_t itemKind, unsigned int extraArg) {
    auto* info = g_clItemKindInfo.GetItemKindInfo(itemKind);
    if (!info) return 0;
    // Ground-truth (mofclient.c) gates this by the WORD at offset +0x3C in stItemKindInfo,
    // which maps to m_wUseTerm in our reconstructed struct.
    if (info->m_wUseTerm != 0) {
        return extraArg;
    }
    return 0;
}
} // namespace

cltItemKindInfo* cltCashshopSystem::m_pclItemKindInfo = nullptr;
cltCashShopItem* cltCashshopSystem::m_pclCashShopItem = nullptr;

void cltCashshopSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltCashShopItem* cashShopItem) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclCashShopItem = cashShopItem;
}

cltCashshopSystem::cltCashshopSystem() { Free(); }

int cltCashshopSystem::Initialize(cltBaseInventory* inventory, int cashMoney, CMofMsg* msg) {
    Free();

    inventory_ = inventory;
    cashMoney_ = cashMoney;

    if (!msg) return 1;

    std::uint16_t count = 0;
    if (!msg->Get_WORD(&count) || count >= kMaxBoughtItem) {
        Free();
        return 0;
    }

    for (std::uint16_t i = 0; i < count; ++i) {
        std::int64_t id = 0;
        std::uint16_t itemKind = 0;
        std::uint16_t qty = 0;

        if (!msg->Get_INT64(&id) || !msg->Get_WORD(&itemKind) || !msg->Get_WORD(&qty)) {
            Free();
            return 0;
        }

        if (!AppendBought(bought_, boughtCount_, id, itemKind, qty)) {
            Free();
            return 0;
        }
    }

    return 1;
}

void cltCashshopSystem::Free() {
    inventory_ = nullptr;
    cashMoney_ = 0;
    verified_ = {};
    buying_ = {};
    bought_.fill({});
    boughtCount_ = 0;
}

cltBaseInventory* cltCashshopSystem::GetInventorySystem() { return inventory_; }

int cltCashshopSystem::CanMoveBoughtCashItemToInventory(std::int64_t itemId) {
    if (!inventory_) return 0;

    const int idx = FindBoughtIndex(bought_, boughtCount_, itemId);
    if (idx < 0) return 0;

    const auto& b = bought_[idx];
    return inventory_->CanAddInventoryItem(b.itemKind, b.qty) == 0;
}

int cltCashshopSystem::CanMoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds) {
    if (!itemIds || itemCount <= 0) return 0;
    if (!inventory_) return 0;

    std::array<strInventoryItem, kMaxMsgBoughtItem> toAdd{};
    int addN = 0;

    for (int i = 0; i < itemCount; ++i) {
        const int idx = FindBoughtIndex(bought_, boughtCount_, itemIds[i]);
        if (idx < 0) return 0;

        if (addN >= static_cast<int>(toAdd.size())) return 0;

        toAdd[addN].itemKind = bought_[idx].itemKind;
        toAdd[addN].itemQty = bought_[idx].qty;
        ++addN;
    }

    for (int i = 0; i < addN; ++i) {
        if (inventory_->CanAddInventoryItem(toAdd[i].itemKind, toAdd[i].itemQty) != 0) return 0;
    }

    return 1;
}

void cltCashshopSystem::MoveBoughtCashItemToInventory(std::int64_t itemId, unsigned int extraArg,
                                                       int* outHasExtra, std::uint8_t* changedSlots) {
    if (outHasExtra) *outHasExtra = 0;

    const int idx = FindBoughtIndex(bought_, boughtCount_, itemId);
    if (idx < 0 || !inventory_) return;

    const auto b = bought_[idx];

    strInventoryItem inv{};
    inv.itemKind = b.itemKind;
    inv.itemQty = b.qty;
    inv.value0 = ComputeExtraValue(b.itemKind, extraArg);
    inv.value1 = 0;

    if (inv.value0 && outHasExtra) *outHasExtra = 1;

    inventory_->AddInventoryItem(&inv, changedSlots, nullptr);
    RemoveBoughtAt(bought_, boughtCount_, idx);
}

void cltCashshopSystem::MoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds,
                                                       unsigned int extraArg, std::uint8_t* changedSlots,
                                                       char* buffer) {
    if (!itemIds || itemCount <= 0) return;

    std::uint16_t pos = 0;
    if (buffer) buffer[0] = '\0';

    for (int i = 0; i < itemCount; ++i) {
        const int idx = FindBoughtIndex(bought_, boughtCount_, itemIds[i]);
        if (idx < 0 || !inventory_) continue;

        const auto b = bought_[idx];

        strInventoryItem inv{};
        inv.itemKind = b.itemKind;
        inv.itemQty = b.qty;
        inv.value0 = ComputeExtraValue(b.itemKind, extraArg);
        inv.value1 = 0;

        inventory_->AddInventoryItem(&inv, changedSlots, &pos);

        if (buffer && inv.value0) {
            char tmp[128]{};
            std::snprintf(tmp, sizeof(tmp), "%s%lld,%u,", buffer, static_cast<long long>(b.id), pos);
            std::strncpy(buffer, tmp, 1023);
            buffer[1023] = '\0';
        }

        RemoveBoughtAt(bought_, boughtCount_, idx);
    }
}

void cltCashshopSystem::SetCashMoney(int money) { cashMoney_ = money; }
int cltCashshopSystem::GetCashMoney() { return cashMoney_; }

void cltCashshopSystem::SetVerifiedCharInfo(char* name, char age, std::uint16_t unk,
                                            char gender, char nation, char* account,
                                            int value0, int value1) {
    ResetVerifiedCharInfo();

    if (name) std::strncpy(verified_.name.data(), name, verified_.name.size() - 1);
    verified_.age = static_cast<std::uint8_t>(age);
    verified_.unk = unk;
    verified_.gender = static_cast<std::uint8_t>(gender);
    verified_.nation = static_cast<std::uint8_t>(nation);

    if (account) std::strncpy(verified_.account.data(), account, verified_.account.size() - 1);

    verified_.value0 = value0;
    verified_.value1 = value1;
}

void cltCashshopSystem::ResetVerifiedCharInfo() {
    verified_ = {};
}

int cltCashshopSystem::GetVerifiedCharInfo(char* name, std::uint8_t* age, char* account, int* value0, int* value1) {
    if (!HasName(verified_)) return 0;

    if (name) std::strcpy(name, verified_.name.data());
    if (age) *age = verified_.age;
    if (account) std::strcpy(account, verified_.account.data());
    if (value0) *value0 = verified_.value0;
    if (value1) *value1 = verified_.value1;
    return 1;
}

strVerifiedCharInfo* cltCashshopSystem::GetVerifiedCharInfo() {
    return HasName(verified_) ? &verified_ : nullptr;
}

int cltCashshopSystem::IsVerifiedCharInfo(char* name) {
    return HasName(verified_) && name && std::strcmp(verified_.name.data(), name) == 0;
}

int cltCashshopSystem::IsThereVerifiedChar() {
    return HasName(verified_);
}

int cltCashshopSystem::SetBuyingCashItemsInfo(std::uint8_t shopType, std::uint8_t buyCount, int* ids) {
    if (!buyCount) return 0;
    if (buyCount + boughtCount_ >= kMaxBoughtItem) return 0;

    buying_.shopType = shopType;
    buying_.buyCount = buyCount;

    for (std::uint8_t i = 0; i < buyCount; ++i) {
        buying_.itemIds[i] = ids ? ids[i] : 0;
    }

    return 1;
}

void cltCashshopSystem::ResetBuyingCashItemsInfo() {
    buying_ = {};
}

int cltCashshopSystem::CanBuyCashItems() {
    const int allPrice = GetBuyingItemAllPrices();
    if (allPrice <= 0) return 0;
    return allPrice <= cashMoney_;
}

int cltCashshopSystem::GetBuyingItemAllPrices() {
    if (!m_pclCashShopItem) return 0;
    return m_pclCashShopItem->GetTotalPrice(buying_.itemIds.data(), buying_.buyCount, buying_.shopType);
}

void cltCashshopSystem::GetBuyingItemStringForWeb(char* out, int outSize) {
    if (!out || outSize <= 0 || !m_pclCashShopItem) return;
    m_pclCashShopItem->GetCompositionItemData(out, buying_.itemIds.data(), buying_.buyCount, buying_.shopType, outSize);
}

void cltCashshopSystem::BoughtCashItems(int cashMoney, int itemCount,
                                        std::uint16_t* itemKinds, std::uint16_t* itemQtys,
                                        std::int64_t* itemIds,
                                        int* outVerifiedValue, char* outVerifiedName) {
    SetCashMoney(cashMoney);

    if (IsThereVerifiedChar()) {
        if (outVerifiedValue) *outVerifiedValue = verified_.value1;
        if (outVerifiedName) std::strcpy(outVerifiedName, verified_.name.data());
    } else {
        if (outVerifiedValue) *outVerifiedValue = 0;

        for (int i = 0; i < itemCount; ++i) {
            const std::int64_t id = itemIds ? itemIds[i] : 0;
            const std::uint16_t kind = itemKinds ? itemKinds[i] : 0;
            const std::uint16_t qty = itemQtys ? itemQtys[i] : 0;

            if (!AppendBought(bought_, boughtCount_, id, kind, qty)) break;
        }
    }

    ResetVerifiedCharInfo();
    ResetBuyingCashItemsInfo();
}

void cltCashshopSystem::BoughtCashItems(CMofMsg* msg) {
    if (!msg) return;

    int cashMoney = 0;
    std::uint8_t count = 0;

    std::array<std::uint16_t, kMaxMsgBoughtItem> kinds{};
    std::array<std::uint16_t, kMaxMsgBoughtItem> qtys{};
    std::array<std::int64_t, kMaxMsgBoughtItem> ids{};

    msg->Get_LONG(&cashMoney);
    msg->Get_BYTE(&count);

    count = std::min<std::uint8_t>(count, kMaxMsgBoughtItem);
    for (std::uint8_t i = 0; i < count; ++i) {
        msg->Get_INT64(&ids[i]);
        msg->Get_WORD(&kinds[i]);
        msg->Get_WORD(&qtys[i]);
    }

    BoughtCashItems(cashMoney, count, kinds.data(), qtys.data(), ids.data(), nullptr, nullptr);
}

strBoughtCashshopItemInfo* cltCashshopSystem::GetBoughtCashItem(std::uint16_t* outCount) {
    if (outCount) *outCount = boughtCount_;
    return bought_.data();
}

void cltCashshopSystem::SetSelectBoughtCashItem(std::uint16_t index) {
    if (index >= boughtCount_) return;
    bought_[index].selected = bought_[index].selected == 0;
}

void cltCashshopSystem::SetSelectBoughtCashItem(std::uint16_t index, std::uint8_t onoff) {
    if (index >= boughtCount_) return;
    bought_[index].selected = onoff;
}

void cltCashshopSystem::SetSelectAllBoughtCashItem() {
    for (std::uint16_t i = 0; i < boughtCount_; ++i) {
        bought_[i].selected = 1;
    }
}

strBuyingCashItemsInfo* cltCashshopSystem::GetBuyingCashItemInfo() {
    return &buying_;
}
