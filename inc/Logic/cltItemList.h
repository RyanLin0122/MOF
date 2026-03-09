#pragma once

#include <cstdint>

struct stShopInfo;
struct strPackageShopInfo;
struct stItemKindInfo;

class CMofMsg;
class cltBaseInventory;
class cltItemKindInfo;
class cltShopInfo;

class cltItemList {
public:
    static void InitializeStaticVariable(cltItemKindInfo* a1, cltShopInfo* a2);

    cltItemList();
    ~cltItemList();

    void Initialize();
    void Initialize(unsigned int a2);
    void Initialize(unsigned int a2, CMofMsg* a3, cltBaseInventory* a4);

    int AddItem(std::int16_t a2, std::int16_t a3, int a4, int a5, std::uint16_t a6, std::uint32_t* a7);
    int AddItemToBasket(std::uint16_t a2, std::uint16_t a3, std::uint16_t a4, int* a5);
    int DeleteItem(std::uint16_t a2, std::uint16_t a3, std::uint16_t a4);
    int DelLastItem();
    int GetItem(unsigned int a2, std::uint16_t* a3, std::uint16_t* a4, unsigned int* a5, std::uint16_t* a6, std::uint16_t* a7);
    int GetSealedStatus(unsigned int a2);
    unsigned int SetSealedStatus(unsigned int a2, int a3);
    std::uint16_t GetItemKind(unsigned int a2);
    std::uint16_t GetItemsNum();
    void FillOutItemListInfo(CMofMsg* a2);
    int GetAllItemsPrice(std::uint16_t a2, int a3);
    int GetAllItemGoldCoinPrice(stShopInfo* a2);
    int GetAllItemSilverCoinPrice(stShopInfo* a2);
    int GetAllItemBronzeCoinPrice(stShopInfo* a2);
    int GetAllItemsPriceForSell(int a2);
    int GetAllItemsBuyPVPPoint();
    void SetCaps(unsigned int a2);
    unsigned int GetCaps();
    void SetBaseInventory(cltBaseInventory* a2);
    void Normalize();
    int AddItemQty(unsigned int a2, std::uint16_t a3);
    int DeleteItem(unsigned int a2);
    void TranslateQueryArg(char* const Buffer);
    void GetItemInfo(std::uint16_t a2, std::uint16_t* a3, std::uint16_t* a4, std::uint16_t* a5);
    void AddTradeItemPos(std::uint16_t a2, std::uint16_t a3);

    static std::int64_t GetItemPrice(std::int64_t a1, int a2, int a3);

    static cltShopInfo* m_pclShopInfo;
    static cltItemKindInfo* m_pclItemKindInfo;

private:
    struct ItemEntry {
        std::uint16_t itemKind;
        std::uint16_t itemQty;
        std::uint16_t itemPos;
        std::uint16_t pad;
        std::uint32_t value0;
        std::uint32_t value1;
        std::uint16_t tradeItemPos;
        std::uint16_t pad2;
    };

    cltBaseInventory* m_pBaseInventory = nullptr;
    ItemEntry m_entries[500]{};
    std::uint16_t m_itemsNum = 0;
    std::uint16_t m_pad = 0;
    unsigned int m_caps = 0;
};
