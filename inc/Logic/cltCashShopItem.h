#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "global.h"
#include "Info/cltItemKindInfo.h"

#pragma pack(push, 1)
struct stCashShopItem {
    std::uint16_t itemKind[5]{};   // D,W,M,Y,F order in file writes reverse
    std::uint16_t qty{};
    int dPrice{};
    int wPrice{};
    int mPrice{};
    int yPrice{};
    int fPrice{};
};

struct stCashShopItemList {
    int sellID{};                  // +0
    std::uint8_t saleFlag{};       // +4 (N=0,Y=1,C=2)
    std::uint8_t pad5{};           // +5
    std::uint16_t unk6{};          // +6
    std::uint8_t itemType{};       // +8
    std::uint8_t pad9{};           // +9
    std::uint16_t textCode{};      // +10
    std::uint16_t descCode{};      // +12
    std::uint16_t fashionType{};   // +14
    std::uint16_t levelLimit{};    // +16
    int classMask{};               // +18
    int periodLimit{};             // +22
    int packagePrice{};            // +26
    std::array<stCashShopItem*, 9> items{}; // +30
    std::uint8_t itemRows{1};      // +66
    std::uint8_t tailPad[5]{};     // pad to 72

    stCashShopItemList();
    ~stCashShopItemList();
};
#pragma pack(pop)

class cltCashShopItem {
public:
    static void InitializeStaticVariable(DCTTextManager* textMgr);

    cltCashShopItem();
    ~cltCashShopItem();

    int Initialize(char* fileName);
    void Free();
    int CheckPeriodByItemKind(stCashShopItem* item);
    int ParsingPrice(char* str, int* outPrice, int strictPositive);
    int CheckCopyItemExistInNonCopyItem();
    int ParsingItemType(stCashShopItemList* out, char* typeName);
    static std::uint16_t TranslateKindCode(char* code);
    stCashShopItemList* GetItemList(std::uint8_t category, std::uint16_t* outCount);
    unsigned int GetFashionType(unsigned int equipAtb);
    int FindCashItemListFileError(std::uint8_t category);
    stCashShopItemList* GetCashShopItemBySellID(int sellID);
    int GetTotalPrice(int* sellIDs, std::uint8_t count, std::uint8_t periodIndex);
    int GetCompositionItemData(char* out, int* sellIDs, std::uint8_t count, std::uint8_t periodIndex, int unused);
    stItemKindInfo* IsFashionItem(stCashShopItemList* itemList);
    BOOL IsPackageItem(std::uint8_t type);
    int IsInDeleteSellIDList(int sellID);
    int GetDelegatePeriod(stCashShopItemList* itemList);
    std::uint16_t GetDelegateItemKind(stCashShopItemList* itemList);
    int GetValidPeriodList(stCashShopItemList* itemList, int* const outList);
    stCashShopItemList* GetCashShopItemByIndex(std::uint16_t index);
    void DeleteSpecialItem();

private:
    std::vector<stCashShopItemList> items_;
    std::array<std::uint16_t, 27> counters_{}; // mirrors word slots [0..26], type counters at [2..26]
    std::vector<int> deleteSellIDs_;            // saleFlag 'N' list
    std::uint16_t totalRows_{};

    static DCTTextManager* m_pDCTTextManager;
};
