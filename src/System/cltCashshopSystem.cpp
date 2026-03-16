#include "System/cltCashshopSystem.h"

#include "Logic/cltCashShopItem.h"
#include "Logic/cltItemList.h"

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
} // namespace

cltItemKindInfo* cltCashshopSystem::m_pclItemKindInfo = nullptr;
cltCashShopItem* cltCashshopSystem::m_pclCashShopItem = nullptr;

void cltCashshopSystem::InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltCashShopItem* cashShopItem) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclCashShopItem = cashShopItem;
}

// GT constructor explicitly zeroes each region; does NOT call Free() (which omits boughtCount_)
cltCashshopSystem::cltCashshopSystem() {
    inventory_ = nullptr;
    cashMoney_ = 0;
    boughtCount_ = 0;
    bought_.fill({});
    verified_ = {};
    buying_ = {};
}

// GT: clears only verified_ and buying_ (preserves bought_/boughtCount_);
// sets inventory_/cashMoney_ first, then reads boughtCount_ directly from msg.
// On failure: calls Free() (which does NOT clear boughtCount_) then returns 0.
int cltCashshopSystem::Initialize(cltBaseInventory* inventory, int cashMoney, CMofMsg* msg) {
    verified_ = {};
    buying_ = {};

    inventory_ = inventory;
    cashMoney_ = cashMoney;

    if (!msg->Get_WORD(&boughtCount_) || boughtCount_ >= kMaxBoughtItem) {
        Free();
        return 0;
    }

    if (boughtCount_ == 0) return 1;

    for (std::uint16_t i = 0; i < boughtCount_; ++i) {
        if (!msg->Get_INT64(&bought_[i].id) ||
            !msg->Get_WORD(&bought_[i].itemKind) ||
            !msg->Get_WORD(&bought_[i].qty)) {
            Free();
            return 0;
        }
    }

    return 1;
}

// GT Free() clears inventory_, cashMoney_, bought_, verified_, buying_
// but does NOT clear boughtCount_
void cltCashshopSystem::Free() {
    inventory_ = nullptr;
    cashMoney_ = 0;
    bought_.fill({});
    verified_ = {};
    buying_ = {};
}

cltBaseInventory* cltCashshopSystem::GetInventorySystem() { return inventory_; }

// GT: no null check on inventory_
int cltCashshopSystem::CanMoveBoughtCashItemToInventory(std::int64_t itemId) {
    const int idx = FindBoughtIndex(bought_, boughtCount_, itemId);
    if (idx < 0) return 0;

    const auto& b = bought_[idx];
    return inventory_->CanAddInventoryItem(b.itemKind, b.qty) == 0;
}

// GT: checks only itemCount == 0 (not <= 0); no null checks on itemIds or inventory_
int cltCashshopSystem::CanMoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds) {
    if (!itemCount) return 0;

    cltItemList itemList;
    itemList.Initialize(1000);

    if (itemCount > 0) {
        for (int i = 0; i < itemCount; ++i) {
            const int idx = FindBoughtIndex(bought_, boughtCount_, itemIds[i]);
            if (idx < 0) return 0;
            itemList.AddItem(bought_[idx].itemKind, bought_[idx].qty, 0, 0, 0xFFFF, 0);
        }
    }
    return inventory_->CanAddInventoryItems(&itemList) == 0;
}

// GT: uses m_pclItemKindInfo; sets *outHasExtra=1 when useTerm!=0 without null check;
// no outHasExtra initialisation to 0; no null check on inventory_.
// GT does not early-return when item not found; idx may equal boughtCount_ (OOB in original).
void cltCashshopSystem::MoveBoughtCashItemToInventory(std::int64_t itemId, unsigned int extraArg,
                                                       int* outHasExtra, std::uint8_t* changedSlots) {
    const int idx = FindBoughtIndex(bought_, boughtCount_, itemId);

    const auto b = bought_[idx];

    unsigned int extraVal = 0;
    auto* info = m_pclItemKindInfo->GetItemKindInfo(b.itemKind);
    if (info->m_wUseTerm != 0) {
        extraVal = extraArg;
        *outHasExtra = 1;
    }

    strInventoryItem inv{};
    inv.itemKind = b.itemKind;
    inv.itemQty = b.qty;
    inv.value0 = extraVal;
    inv.value1 = 0;

    inventory_->AddInventoryItem(&inv, changedSlots, nullptr);
    RemoveBoughtAt(bought_, boughtCount_, idx);
}

// GT: itemCount > 0 gates loop (no null checks on itemIds/inventory_); uses m_pclItemKindInfo;
// passes outPos=nullptr when useTerm==0, &pos when useTerm!=0;
// sprintf directly into buffer (no tmp, no size limit) only when useTerm!=0.
void cltCashshopSystem::MoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds,
                                                       unsigned int extraArg, std::uint8_t* changedSlots,
                                                       char* buffer) {
    if (itemCount > 0) {
        for (int i = 0; i < itemCount; ++i) {
            const int idx = FindBoughtIndex(bought_, boughtCount_, itemIds[i]);
            if (idx < 0) continue;

            const auto b = bought_[idx];

            auto* info = m_pclItemKindInfo->GetItemKindInfo(b.itemKind);
            const bool hasExtra = info->m_wUseTerm != 0;

            strInventoryItem inv{};
            inv.itemKind = b.itemKind;
            inv.itemQty = b.qty;
            inv.value0 = hasExtra ? extraArg : 0u;
            inv.value1 = 0;

            if (!hasExtra) {
                inventory_->AddInventoryItem(&inv, changedSlots, nullptr);
            } else {
                std::uint16_t pos = 0;
                inventory_->AddInventoryItem(&inv, changedSlots, &pos);
                if (buffer)
                    std::sprintf(buffer, "%s%lld, %d, ", buffer, static_cast<long long>(b.id), static_cast<int>(pos));
            }

            RemoveBoughtAt(bought_, boughtCount_, idx);
        }
    }
}

void cltCashshopSystem::SetCashMoney(int money) { cashMoney_ = money; }
int cltCashshopSystem::GetCashMoney() { return cashMoney_; }

// GT: no null check on name, uses strcpy (no size limit)
void cltCashshopSystem::SetVerifiedCharInfo(char* name, char age, std::uint16_t unk,
                                            char gender, char nation, char* account,
                                            int value0, int value1) {
    ResetVerifiedCharInfo();

    std::strcpy(verified_.name.data(), name);
    verified_.age = static_cast<std::uint8_t>(age);
    verified_.unk = unk;
    verified_.gender = static_cast<std::uint8_t>(gender);
    verified_.nation = static_cast<std::uint8_t>(nation);

    if (account) {
        std::strcpy(verified_.account.data(), account);
        verified_.value0 = value0;
        verified_.value1 = value1;
    }
}

void cltCashshopSystem::ResetVerifiedCharInfo() {
    verified_ = {};
}

// GT: no null check on name or age; direct access (callers must pass valid pointers)
int cltCashshopSystem::GetVerifiedCharInfo(char* name, std::uint8_t* age, char* account, int* value0, int* value1) {
    if (!HasName(verified_)) return 0;

    std::strcpy(name, verified_.name.data());
    *age = verified_.age;
    if (account) std::strcpy(account, verified_.account.data());
    if (value0) *value0 = verified_.value0;
    if (value1) *value1 = verified_.value1;
    return 1;
}

strVerifiedCharInfo* cltCashshopSystem::GetVerifiedCharInfo() {
    return HasName(verified_) ? &verified_ : nullptr;
}

int cltCashshopSystem::IsVerifiedCharInfo(char* name) {
    return HasName(verified_) && std::strcmp(verified_.name.data(), name) == 0;
}

int cltCashshopSystem::IsThereVerifiedChar() {
    return HasName(verified_);
}

// GT: no null check on ids; direct qmemcpy
int cltCashshopSystem::SetBuyingCashItemsInfo(std::uint8_t shopType, std::uint8_t buyCount, int* ids) {
    if (!buyCount) return 0;
    if (buyCount + boughtCount_ >= kMaxBoughtItem) return 0;

    buying_.shopType = shopType;
    buying_.buyCount = buyCount;
    std::memcpy(buying_.itemIds.data(), ids, 4 * buyCount);

    return 1;
}

void cltCashshopSystem::ResetBuyingCashItemsInfo() {
    buying_ = {};
}

// GT: if allPrice != 0, return allPrice <= cashMoney_ (negative prices pass through)
int cltCashshopSystem::CanBuyCashItems() {
    const int allPrice = GetBuyingItemAllPrices();
    if (!allPrice) return 0;
    return allPrice <= cashMoney_;
}

// GT: no null check on m_pclCashShopItem
int cltCashshopSystem::GetBuyingItemAllPrices() {
    return m_pclCashShopItem->GetTotalPrice(buying_.itemIds.data(), buying_.buyCount, buying_.shopType);
}

// GT: no null checks
void cltCashshopSystem::GetBuyingItemStringForWeb(char* out, int outSize) {
    m_pclCashShopItem->GetCompositionItemData(out, buying_.itemIds.data(), buying_.buyCount, buying_.shopType, outSize);
}

// GT: when verified char present and outVerifiedValue set, strcpy outVerifiedName without null check;
// no null checks on itemKinds/itemQtys/itemIds arrays; loop condition is itemCount > 0
void cltCashshopSystem::BoughtCashItems(int cashMoney, int itemCount,
                                        std::uint16_t* itemKinds, std::uint16_t* itemQtys,
                                        std::int64_t* itemIds,
                                        int* outVerifiedValue, char* outVerifiedName) {
    SetCashMoney(cashMoney);

    if (IsThereVerifiedChar()) {
        if (outVerifiedValue) {
            *outVerifiedValue = verified_.value1;
            std::strcpy(outVerifiedName, verified_.name.data());
        }
    } else {
        if (outVerifiedValue) *outVerifiedValue = 0;

        if (itemCount > 0) {
            for (int i = 0; i < itemCount; ++i) {
                if (!AppendBought(bought_, boughtCount_, itemIds[i], itemKinds[i], itemQtys[i])) break;
            }
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

    // GT does not clamp count; raw packet value used directly (may exceed array bounds).
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
