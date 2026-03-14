#pragma once

#include <array>
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "Logic/cltBaseInventory.h"
#include "Network/CMofMsg.h"
#include "Info/cltItemKindInfo.h"

class cltCashShopItem;

struct strBoughtCashshopItemInfo {
    std::int64_t id{};
    std::uint16_t itemKind{};
    std::uint16_t qty{};
    std::uint8_t selected{};
};

struct strVerifiedCharInfo {
    std::array<char, 64> name{};
    std::uint8_t age{};
    std::uint16_t unk{};
    std::uint8_t gender{};
    std::uint8_t nation{};
    std::array<char, 64> account{};
    int value0{};
    int value1{};
};

struct strBuyingCashItemsInfo {
    std::uint8_t shopType{};
    std::uint8_t buyCount{};
    std::array<int, 1000> itemIds{};
};

class cltCashshopSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltCashShopItem* cashShopItem);

    cltCashshopSystem();
    ~cltCashshopSystem() = default;

    int Initialize(cltBaseInventory* inventory, int cashMoney, CMofMsg* msg);
    void Free();

    cltBaseInventory* GetInventorySystem();
    int CanMoveBoughtCashItemToInventory(std::int64_t itemId);
    int CanMoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds);
    void MoveBoughtCashItemToInventory(std::int64_t itemId, unsigned int extraArg, int* outHasExtra, std::uint8_t* changedSlots);
    void MoveBoughtCashItemToInventory(int itemCount, std::int64_t* itemIds, unsigned int extraArg, std::uint8_t* changedSlots, char* buffer);

    void SetCashMoney(int money);
    int GetCashMoney();

    void SetVerifiedCharInfo(char* name, char age, std::uint16_t unk, char gender, char nation, char* account, int value0, int value1);
    void ResetVerifiedCharInfo();
    int GetVerifiedCharInfo(char* name, std::uint8_t* age, char* account, int* value0, int* value1);
    strVerifiedCharInfo* GetVerifiedCharInfo();
    int IsVerifiedCharInfo(char* name);
    int IsThereVerifiedChar();

    int SetBuyingCashItemsInfo(std::uint8_t shopType, std::uint8_t buyCount, int* ids);
    void ResetBuyingCashItemsInfo();
    int CanBuyCashItems();
    int GetBuyingItemAllPrices();
    void GetBuyingItemStringForWeb(char* out, int outSize);

    void BoughtCashItems(int cashMoney, int itemCount, std::uint16_t* itemKinds, std::uint16_t* itemQtys, std::int64_t* itemIds, int* outVerifiedValue, char* outVerifiedName);
    void BoughtCashItems(CMofMsg* msg);

    strBoughtCashshopItemInfo* GetBoughtCashItem(std::uint16_t* outCount);
    void SetSelectBoughtCashItem(std::uint16_t index);
    void SetSelectBoughtCashItem(std::uint16_t index, std::uint8_t onoff);
    void SetSelectAllBoughtCashItem();
    strBuyingCashItemsInfo* GetBuyingCashItemInfo();

private:
    cltBaseInventory* inventory_{};
    int cashMoney_{};
    strVerifiedCharInfo verified_{};
    strBuyingCashItemsInfo buying_{};
    std::array<strBoughtCashshopItemInfo, 1000> bought_{};
    std::uint16_t boughtCount_{};

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltCashShopItem* m_pclCashShopItem;
};
