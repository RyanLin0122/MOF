#include "System/cltTradeSystem.h"

#include <cstring>

#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "System/cltMoneySystem.h"

// ====== cltTradeBasket ======

cltTradeBasket::cltTradeBasket() {
}

cltTradeBasket::~cltTradeBasket() {
}

void cltTradeBasket::Free() {
    m_account = 0;
    m_pInventory = nullptr;
    m_pMoney = nullptr;
    m_changeMoney = 0;
    m_tradeOK = 0;
    std::memset(m_items, 0, sizeof(m_items));
}

void cltTradeBasket::Initialize(unsigned int account, cltBaseInventory* pInventory, cltMoneySystem* pMoney) {
    std::memset(m_items, 0, sizeof(m_items));
    m_account = account;
    m_pInventory = pInventory;
    m_pMoney = pMoney;
    m_tradeOK = 0;
}

unsigned int cltTradeBasket::AddItemByInventoryPos(std::uint16_t invenPos, std::uint16_t qty) {
    if (!GetAccount())
        return 0;

    std::uint16_t emptyPos = GetEmptyPos();
    auto* invItem = m_pInventory->GetInventoryItem(invenPos);

    stTradeItem& slot = m_items[emptyPos];
    slot.itemKind = invItem->itemKind;
    slot.inventoryPos = invenPos;
    slot.itemQty = qty;
    slot.itemTime = invItem->value0;

    SetTradeOK(0);
    return 1;
}

unsigned int cltTradeBasket::AddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty, unsigned int time) {
    if (!GetAccount())
        return 0;

    std::uint16_t emptyPos = GetEmptyPos();
    stTradeItem& slot = m_items[emptyPos];
    slot.itemKind = itemKind;
    slot.itemQty = qty;
    slot.inventoryPos = 0;
    slot.itemTime = time;

    SetTradeOK(0);
    return 1;
}

void cltTradeBasket::DeleteItem(std::uint8_t slot, std::uint16_t qty) {
    if (!GetAccount())
        return;

    stTradeItem& item = m_items[slot];
    if (!item.itemKind)
        return;

    if (item.itemQty == 1 || item.itemQty <= qty) {
        item.itemKind = 0;
        item.inventoryPos = 0;
        item.itemQty = 0;
        SetTradeOK(0);
    } else {
        item.itemQty -= qty;
        SetTradeOK(0);
    }
}

std::uint16_t cltTradeBasket::GetItemKind(std::uint16_t slot) {
    return m_items[slot].itemKind;
}

std::uint16_t cltTradeBasket::GetInventoryPos(std::uint16_t slot) {
    return m_items[slot].inventoryPos;
}

std::uint16_t cltTradeBasket::GetItemQuanity(std::uint16_t slot) {
    return m_items[slot].itemQty;
}

unsigned int cltTradeBasket::GetItemTime(std::uint16_t slot) {
    return m_items[slot].itemTime;
}

int cltTradeBasket::CanAddTradeBasket() {
    for (int i = 0; i < MAX_TRADE_ITEMS; ++i) {
        if (!m_items[i].itemKind)
            return 1;
    }
    return 0;
}

unsigned int cltTradeBasket::GetAccount() {
    return m_account;
}

std::uint16_t cltTradeBasket::GetEmptyPos() {
    if (!GetAccount())
        return MAX_TRADE_ITEMS;

    for (int i = 0; i < MAX_TRADE_ITEMS; ++i) {
        if (!m_items[i].itemKind)
            return static_cast<std::uint16_t>(i);
    }
    return MAX_TRADE_ITEMS;
}

void cltTradeBasket::SetTradeOK(int value) {
    m_tradeOK = value;
}

int cltTradeBasket::GetTradeOK() {
    return m_tradeOK;
}

void cltTradeBasket::SetItemList(cltItemList* pItemList) {
    for (int i = 0; i < MAX_TRADE_ITEMS; ++i) {
        if (GetItemKind(i)) {
            std::uint16_t pos = GetInventoryPos(i);
            unsigned int time = GetItemTime(i);
            std::int16_t qty = GetItemQuanity(i);
            std::int16_t kind = GetItemKind(i);
            pItemList->AddItem(kind, qty, time, 0, pos, nullptr);
        }
    }
}

int cltTradeBasket::IsAlreadyTradingItem(std::uint16_t invenPos) {
    for (int i = 0; i < MAX_TRADE_ITEMS; ++i) {
        if (m_items[i].itemKind && m_items[i].inventoryPos == invenPos)
            return 1;
    }
    return 0;
}

// ====== cltTradeSystem ======

cltTradeSystem::TradeCompletedFunc cltTradeSystem::m_pOnTradeCompletedFuncPtr = nullptr;
cltTradeSystem::TradeFailedFunc cltTradeSystem::m_pOnTradeFailedFuncPtr = nullptr;

cltTradeSystem g_clTradeSystem;

void cltTradeSystem::InitializeStaticVariable(TradeCompletedFunc onCompleted, TradeFailedFunc onFailed) {
    m_pOnTradeCompletedFuncPtr = onCompleted;
    m_pOnTradeFailedFuncPtr = onFailed;
}

cltTradeSystem::cltTradeSystem() {
}

cltTradeSystem::~cltTradeSystem() {
}

void cltTradeSystem::Free() {
    m_myBasket.Free();
    m_otherBasket.Free();
}

void cltTradeSystem::Initialize(unsigned int myAccount, cltBaseInventory* myInventory, cltMoneySystem* myMoney,
                                 unsigned int otherAccount, cltBaseInventory* otherInventory, cltMoneySystem* otherMoney) {
    m_myBasket.Free();
    m_otherBasket.Free();
    m_myBasket.Initialize(myAccount, myInventory, myMoney);
    m_otherBasket.Initialize(otherAccount, otherInventory, otherMoney);
    m_lastChangedTime = 0;
}

cltTradeBasket* cltTradeSystem::AddTadeBasketByInventoryPos(unsigned int account, std::uint16_t invenPos, std::uint16_t qty) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return nullptr;

    if (!basket->CanAddTradeBasket())
        return nullptr;

    if (basket->IsAlreadyTradingItem(invenPos) == 1)
        return nullptr;

    if (!basket->GetInventory()->IsThereInventoryItem(invenPos, qty))
        return nullptr;

    if (!basket->AddItemByInventoryPos(invenPos, qty))
        return nullptr;

    m_myBasket.SetTradeOK(0);
    m_otherBasket.SetTradeOK(0);
    UpdateLastChangedTime();
    return reinterpret_cast<cltTradeBasket*>(1);
}

unsigned int cltTradeSystem::AddTadeBasketByItemKind(unsigned int account, std::uint16_t itemKind, std::uint16_t qty, unsigned int time) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return 0;

    if (!basket->CanAddTradeBasket())
        return 0;

    if (!basket->AddItemByItemKind(itemKind, qty, time))
        return 0;

    m_myBasket.SetTradeOK(0);
    m_otherBasket.SetTradeOK(0);
    UpdateLastChangedTime();
    return 1;
}

int cltTradeSystem::DeleteTadeBasket(unsigned int account, std::uint8_t slot, std::uint16_t qty) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return 0;

    if (!basket->GetItemKind(slot))
        return 0;

    basket->DeleteItem(slot, qty);
    m_myBasket.SetTradeOK(0);
    m_otherBasket.SetTradeOK(0);
    UpdateLastChangedTime();
    return 1;
}

cltTradeBasket* cltTradeSystem::CanAddTradeBasket(unsigned int account) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (basket)
        return basket->CanAddTradeBasket() ? basket : nullptr;
    return nullptr;
}

cltTradeBasket* cltTradeSystem::GetTradeBasket(unsigned int account) {
    if (m_myBasket.GetAccount() == account)
        return &m_myBasket;
    if (m_otherBasket.GetAccount() == account)
        return &m_otherBasket;
    return nullptr;
}

void cltTradeSystem::GetAccounts(unsigned int* outMyAccount, unsigned int* outOtherAccount) {
    *outMyAccount = m_myBasket.GetAccount();
    *outOtherAccount = m_otherBasket.GetAccount();
}

cltTradeBasket* cltTradeSystem::CanSetChangeMoney(unsigned int account) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return nullptr;
    return reinterpret_cast<cltTradeBasket*>(basket->GetChangeMoney() == 0 ? 1 : 0);
}

void cltTradeSystem::SetChangeMoney(unsigned int account, int money) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return;

    basket->SetChangeMoney(money);
    m_myBasket.SetTradeOK(0);
    m_otherBasket.SetTradeOK(0);
    UpdateLastChangedTime();
}

int cltTradeSystem::GetChangeMoney(unsigned int account) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return 0;
    return basket->GetChangeMoney();
}

void cltTradeSystem::SuccedTradeMoney(unsigned int account) {
    unsigned int myAcc = m_myBasket.GetAccount();
    unsigned int otherAcc = m_otherBasket.GetAccount();

    if (account == myAcc) {
        int myMoney = m_myBasket.GetChangeMoney();
        int otherMoney = m_otherBasket.GetChangeMoney();
        m_myBasket.GetMoneySystem()->DecreaseMoney(myMoney);
        m_myBasket.GetMoneySystem()->IncreaseMoney(otherMoney);
    } else {
        int otherMoney = m_otherBasket.GetChangeMoney();
        int myMoney = m_myBasket.GetChangeMoney();
        m_otherBasket.GetMoneySystem()->DecreaseMoney(otherMoney);
        m_otherBasket.GetMoneySystem()->IncreaseMoney(myMoney);
    }
}

cltTradeBasket* cltTradeSystem::GetTradeOK(unsigned int account) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (basket)
        return reinterpret_cast<cltTradeBasket*>(basket->GetTradeOK());
    return nullptr;
}

int cltTradeSystem::CanSetTradeOK() {
    unsigned int now = timeGetTime();
    if (now < m_lastChangedTime)
        return (now - m_lastChangedTime - 1) >= 0x7D0;
    return (now - m_lastChangedTime) >= 0x7D0;
}

unsigned int cltTradeSystem::SetTradeOK(unsigned int account, int value) {
    cltTradeBasket* basket = GetTradeBasket(account);
    if (!basket)
        return 1;

    basket->SetTradeOK(value);

    if (!m_myBasket.GetTradeOK())
        return 0;
    if (!m_otherBasket.GetTradeOK())
        return 0;

    unsigned int result = SucceedTrade();
    if (!result)
        return 0;

    basket->SetTradeOK(value == 0 ? 1 : 0);
    return result;
}

unsigned int cltTradeSystem::SucceedTrade() {
    cltItemList myItemList;
    cltItemList otherItemList;
    std::uint8_t myChangeSlots[255] = {};
    std::uint8_t otherChangeSlots[255] = {};

    if (!m_myBasket.GetTradeOK() || !m_otherBasket.GetTradeOK())
        return 1;

    unsigned int myAccount = m_myBasket.GetAccount();
    unsigned int otherAccount = m_otherBasket.GetAccount();

    m_myBasket.SetItemList(&myItemList);
    m_otherBasket.SetItemList(&otherItemList);

    cltBaseInventory* myInv = m_myBasket.GetInventory();
    cltBaseInventory* otherInv = m_otherBasket.GetInventory();
    cltMoneySystem* myMoney = m_myBasket.GetMoneySystem();
    cltMoneySystem* otherMoney = m_otherBasket.GetMoneySystem();
    int myChangeMoney = m_myBasket.GetChangeMoney();
    int otherChangeMoney = m_otherBasket.GetChangeMoney();

    // Validate: my inventory can handle other's items
    if (myInv) {
        if (otherInv && !otherInv->IsThereInventoryItem(&otherItemList)) {
            unsigned int errCode = 500;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
        if (myInv->CanAddInventoryItems(&otherItemList)) {
            unsigned int errCode = 500;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
    }

    if (otherInv) {
        if (myInv && !myInv->IsThereInventoryItem(&myItemList)) {
            unsigned int errCode = 500;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
        if (otherInv->CanAddInventoryItems(&myItemList)) {
            unsigned int errCode = 500;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
    }

    // Validate money transfers
    if (myChangeMoney) {
        if ((otherMoney && !otherMoney->CanDecreaseMoney(otherChangeMoney))
            || !myMoney->CanIncreaseMoney(otherChangeMoney)) {
            unsigned int errCode = 501;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
    }
    if (otherChangeMoney) {
        if ((myMoney && !myMoney->CanDecreaseMoney(myChangeMoney))
            || !otherMoney->CanIncreaseMoney(myChangeMoney)) {
            unsigned int errCode = 501;
            if (m_pOnTradeFailedFuncPtr)
                m_pOnTradeFailedFuncPtr(myAccount, otherAccount, errCode);
            return errCode;
        }
    }

    // Execute item trade
    if (myInv)
        myInv->DelInventoryItem(&myItemList, myChangeSlots);
    if (otherInv)
        otherInv->DelInventoryItem(&otherItemList, otherChangeSlots);
    if (myInv)
        myInv->AddInventoryItem(&otherItemList, myChangeSlots);
    if (otherInv)
        otherInv->AddInventoryItem(&myItemList, otherChangeSlots);

    // Execute money trade
    if (myChangeMoney)
        SuccedTradeMoney(myAccount);
    if (otherChangeMoney)
        SuccedTradeMoney(otherAccount);

    if (m_pOnTradeCompletedFuncPtr) {
        m_pOnTradeCompletedFuncPtr(
            myAccount, myChangeMoney, myInv, myChangeSlots, &myItemList,
            otherAccount, otherChangeMoney, otherInv, otherChangeSlots, &otherItemList);
    }

    Free();
    return 0;
}

std::uint16_t cltTradeSystem::GetMyItemQuantityByInventoryPos(std::uint16_t invenPos) {
    for (int i = 0; i < cltTradeBasket::MAX_TRADE_ITEMS; ++i) {
        if (m_myBasket.GetInventoryPos(i) == invenPos)
            return m_myBasket.GetItemQuanity(i);
    }
    return 0;
}

std::uint16_t cltTradeSystem::GetMyTradeBasketItemKind(std::uint16_t slot) {
    return m_myBasket.GetItemKind(slot);
}

std::uint16_t cltTradeSystem::GetMyTradeBasketItemQuantity(std::uint16_t slot) {
    return m_myBasket.GetItemQuanity(slot);
}

std::uint16_t cltTradeSystem::GetOtherTradeBasketItemKind(std::uint16_t slot) {
    return m_otherBasket.GetItemKind(slot);
}

std::uint16_t cltTradeSystem::GetOtherTradeBasketItemQuantity(std::uint16_t slot) {
    return m_otherBasket.GetItemQuanity(slot);
}

int cltTradeSystem::GetMyTradeMoney() {
    return m_myBasket.GetChangeMoney();
}

int cltTradeSystem::GetOtherTradeMoney() {
    return m_otherBasket.GetChangeMoney();
}

int cltTradeSystem::GetMyTradeOK() {
    return m_myBasket.GetTradeOK();
}

int cltTradeSystem::GetOtherTradeOK() {
    return m_otherBasket.GetTradeOK();
}

unsigned int cltTradeSystem::IsUsingTradeSystem() {
    if (m_myBasket.GetAccount() || m_otherBasket.GetAccount())
        return 1;
    return 0;
}

void cltTradeSystem::UpdateLastChangedTime() {
    m_lastChangedTime = timeGetTime();
}

int cltTradeSystem::IsOtherBasketEmpty() {
    for (int i = 0; i < cltTradeBasket::MAX_TRADE_ITEMS; ++i) {
        if (GetOtherTradeBasketItemKind(i))
            return 0;
    }
    return GetOtherTradeMoney() == 0;
}

int cltTradeSystem::IsMyBasketEmpty() {
    for (int i = 0; i < cltTradeBasket::MAX_TRADE_ITEMS; ++i) {
        if (GetMyTradeBasketItemKind(i))
            return 0;
    }
    return GetMyTradeMoney() == 0;
}
