#pragma once

#include <cstdint>
#include <cstring>
#include <windows.h>

class cltBaseInventory;
class cltMoneySystem;
class cltItemList;

#pragma pack(push, 1)
struct stTradeItem {
    std::uint16_t inventoryPos = 0;
    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
    std::uint16_t _pad = 0;
    std::uint32_t itemTime = 0;
};
#pragma pack(pop)
static_assert(sizeof(stTradeItem) == 12, "stTradeItem size must be 12 bytes");

class cltTradeBasket {
public:
    cltTradeBasket();
    virtual ~cltTradeBasket();

    void Initialize(unsigned int account, cltBaseInventory* pInventory, cltMoneySystem* pMoney);
    void Free();

    unsigned int AddItemByInventoryPos(std::uint16_t invenPos, std::uint16_t qty);
    unsigned int AddItemByItemKind(std::uint16_t itemKind, std::uint16_t qty, unsigned int time);
    void DeleteItem(std::uint8_t slot, std::uint16_t qty);

    std::uint16_t GetItemKind(std::uint16_t slot);
    std::uint16_t GetInventoryPos(std::uint16_t slot);
    std::uint16_t GetItemQuanity(std::uint16_t slot);
    unsigned int GetItemTime(std::uint16_t slot);

    int CanAddTradeBasket();
    unsigned int GetAccount();
    std::uint16_t GetEmptyPos();

    void SetTradeOK(int value);
    int GetTradeOK();

    void SetItemList(cltItemList* pItemList);
    int IsAlreadyTradingItem(std::uint16_t invenPos);

    // Accessors for trade system internals
    void SetChangeMoney(int money) { m_changeMoney = money; }
    int GetChangeMoney() { return m_changeMoney; }
    cltBaseInventory* GetInventory() { return m_pInventory; }
    cltMoneySystem* GetMoneySystem() { return m_pMoney; }

    static constexpr int MAX_TRADE_ITEMS = 8;

private:
    unsigned int m_account = 0;
    cltBaseInventory* m_pInventory = nullptr;
    cltMoneySystem* m_pMoney = nullptr;
    int m_changeMoney = 0;
    int m_tradeOK = 0;
    stTradeItem m_items[MAX_TRADE_ITEMS]{};
};

class cltTradeSystem {
public:
    using TradeCompletedFunc = void(__cdecl*)(unsigned int, int, cltBaseInventory*, std::uint8_t*, cltItemList*,
                                              unsigned int, int, cltBaseInventory*, std::uint8_t*, cltItemList*);
    using TradeFailedFunc = void(__cdecl*)(unsigned int, unsigned int, unsigned int);

    static void InitializeStaticVariable(TradeCompletedFunc onCompleted, TradeFailedFunc onFailed);

    cltTradeSystem();
    virtual ~cltTradeSystem();

    void Initialize(unsigned int myAccount, cltBaseInventory* myInventory, cltMoneySystem* myMoney,
                    unsigned int otherAccount, cltBaseInventory* otherInventory, cltMoneySystem* otherMoney);
    void Free();

    cltTradeBasket* AddTadeBasketByInventoryPos(unsigned int account, std::uint16_t invenPos, std::uint16_t qty);
    unsigned int AddTadeBasketByItemKind(unsigned int account, std::uint16_t itemKind, std::uint16_t qty, unsigned int time);
    int DeleteTadeBasket(unsigned int account, std::uint8_t slot, std::uint16_t qty);

    cltTradeBasket* CanAddTradeBasket(unsigned int account);
    cltTradeBasket* GetTradeBasket(unsigned int account);
    void GetAccounts(unsigned int* outMyAccount, unsigned int* outOtherAccount);

    cltTradeBasket* CanSetChangeMoney(unsigned int account);
    void SetChangeMoney(unsigned int account, int money);
    int GetChangeMoney(unsigned int account);
    void SuccedTradeMoney(unsigned int account);

    cltTradeBasket* GetTradeOK(unsigned int account);
    int CanSetTradeOK();
    unsigned int SetTradeOK(unsigned int account, int value);
    unsigned int SucceedTrade();

    std::uint16_t GetMyItemQuantityByInventoryPos(std::uint16_t invenPos);
    std::uint16_t GetMyTradeBasketItemKind(std::uint16_t slot);
    std::uint16_t GetMyTradeBasketItemQuantity(std::uint16_t slot);
    std::uint16_t GetOtherTradeBasketItemKind(std::uint16_t slot);
    std::uint16_t GetOtherTradeBasketItemQuantity(std::uint16_t slot);

    int GetMyTradeMoney();
    int GetOtherTradeMoney();
    int GetMyTradeOK();
    int GetOtherTradeOK();

    unsigned int IsUsingTradeSystem();
    void UpdateLastChangedTime();
    int IsOtherBasketEmpty();
    int IsMyBasketEmpty();

private:
    cltTradeBasket m_myBasket;
    cltTradeBasket m_otherBasket;
    unsigned int m_lastChangedTime = 0;

    static TradeCompletedFunc m_pOnTradeCompletedFuncPtr;
    static TradeFailedFunc m_pOnTradeFailedFuncPtr;
};

extern cltTradeSystem g_clTradeSystem;
