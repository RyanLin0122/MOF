#pragma once

#include <array>
#include <cstdint>

typedef int BOOL;
typedef unsigned char _BYTE;
typedef unsigned short _WORD;
typedef unsigned int _DWORD;

struct stItemKindInfo;

struct strInventoryItem {
    std::uint16_t itemKind = 0;
    std::uint16_t itemQty = 0;
    std::uint32_t value0 = 0;
    std::uint32_t value1 = 0;
};

struct strInventoryItemForSort {
    std::uint16_t sortKey = 0;
    std::uint16_t itemKind = 0;
    std::uint32_t value0 = 0;
    std::uint32_t value1 = 0;
    std::uint16_t itemPos = 0;
    std::uint16_t pad = 0;
};
class cltItemList;
class cltQuickSlotSystem;
class cltTitleSystem;
class cltPetSystem;
class cltPetInventorySystem;
class cltItemKindInfo;
class DCTTextManager;
class CMofMsg;

class cltBaseInventory {
public:
    static void InitializeStaticVariable(cltItemKindInfo* a1, DCTTextManager* a2, void (*a3)(char*, char*, unsigned int));

    cltBaseInventory();
    ~cltBaseInventory();

    void Initialize(cltQuickSlotSystem* a2, cltTitleSystem* a3, cltPetSystem* a4, int a5, cltItemList* a6);
    unsigned int CanAddInventoryItem(int a2, std::uint16_t a3, std::uint16_t a4);
    int CanAddInventoryItem(std::uint16_t a2, std::uint16_t a3);
    int CanAddInventoryItems(cltItemList* a2);
    int CanDelInventoryItems(cltItemList* a2);
    strInventoryItem* CanDelInventoryItem(std::uint16_t a2, std::uint16_t a3);
    strInventoryItem* CanDelInventoryItem(std::uint16_t a2, std::uint16_t a3, std::uint16_t a4);
    BOOL CanDelInventoryItemByKindNQty(int a2, std::uint16_t a3);
    int AddInventoryItem(cltItemList* a2, std::uint8_t* a3);
    int AddInventoryItem(strInventoryItem* a2, std::uint8_t* a3, std::uint16_t* a4);
    int AddInventoryItem(std::uint16_t a2, strInventoryItem* a3, std::uint8_t* a4);
    int DelInventoryItem(cltItemList* a2, std::uint8_t* a3);
    BOOL DelInventoryItemKind(std::uint16_t a2, std::uint16_t a3, cltItemList* a4, std::uint8_t* a5);
    int DelInventoryItem(std::uint16_t a2, std::uint16_t a3, std::uint8_t* a4);
    int DelInventoryItem(int a2, std::uint16_t a3, std::uint8_t* a4);
    void DelInventoryItem(char a2, std::uint8_t* a3);
    strInventoryItem* GetInventoryItem(std::uint16_t a2);
    int EmptyInventoryItem(std::uint16_t a2);
    void Free();
    int GetEmptyItemSlotPos(int a2);
    int GetEmptyItemSlotPos(std::uint16_t a2, int a3, int a4);
    int GetUseItemSlotPos(int a2);
    stItemKindInfo* GetUseItemSlotPos(int a2, int* a3, std::uint16_t* a4);
    strInventoryItem* IsThereInventoryItem(std::uint16_t a2, std::uint16_t a3);
    strInventoryItem* IsThereInventoryItem(std::uint16_t a2, std::uint16_t a3, std::uint16_t a4);
    int IsThereInventoryItem(cltItemList* a2);
    BOOL IsThereInventoryItemByKindNQty(int a2, std::uint16_t a3);
    int FindFirstItem(int a2);
    std::uint8_t GetGenericBagNum();
    std::uint8_t GetPileUpNum(std::uint16_t a2);
    std::int16_t GetInventoryItemQuantity(int a2);
    int CanMoveItem(std::uint16_t a2, std::uint16_t a3);
    int MoveItem(std::uint16_t a2, std::uint16_t a3);
    std::uint16_t* IsEquipTypeItem(std::uint16_t a2);
    BOOL GetItemSlotIndexRangeByItemKind(std::uint16_t a2, int* a3, int* a4);
    void GetItemSlotIndexRangeByItemType(int a2, int* a3, int* a4);
    int GetEmptyItemSlotNum(int a2);
    BOOL IsValidItemSlotIndex(int a2);
    BOOL IsValidItemSlotIndex(int a2, int a3);
    BOOL IsIdenticalItemTypeSlotIndex(int a2, int a3);
    void FillOutInventoryInfo(CMofMsg* a2);
    strInventoryItem* EquipedItem(std::uint16_t a2, std::int16_t a3, int a4, int a5);
    BOOL CanIncreaseGenericBagNum();
    void IncreaseGenericBagNum();
    int GetExpandExStorageItemQty();
    int DecreaseExpandExStorageItemQty();
    unsigned int GetTownPortalItemQty(int* a2);
    unsigned int GetPostItItemQty(int* a2);
    std::uint16_t GetTransportItem();
    int GetAllItemCount(std::uint16_t a2);
    int GetAllItemCount(int a2);
    int IsLock();
    int Lock(char* a2);
    int Unlock(int a2);
    int SortInventoryItems(int a2);
    BOOL ISGetItemID(unsigned int a2, unsigned int* a3, unsigned int* a4);
    cltPetInventorySystem* CanMoveItemToPetInventory(std::uint8_t a2, std::uint16_t a3);
    void MoveItemToPetInventory(char a2, int a3, std::uint8_t* a4, std::uint8_t* a5);
    cltPetInventorySystem* CanMoveItemFromPetInventory(std::uint8_t a2, std::uint16_t a3);
    void MoveItemFromPetInventory(std::uint8_t a2, std::uint16_t a3, std::uint8_t* a4, std::uint8_t* a5);
    int IsExistMoveServer();
    strInventoryItem* GetItemSealed(std::uint16_t a2);
    int SetItemSealed(std::uint16_t a2, int a3, int a4);
    int IsSaveChangeCoinItem(int a2);

    static cltItemKindInfo* m_pclItemKindInfo;
    static DCTTextManager* m_pclTextManager;
    static void (*m_pExternCriticalErrorFuncPtr)(char*, char*, unsigned int);

private:
    int m_lockToken = 0;
    int m_isLocked = 0;
    char m_lockReason[1024]{};
    std::uint32_t m_lockTick = 0;
    std::uint8_t m_genericBagNum = 1;
    std::uint8_t m_padding1041[3]{};
    cltQuickSlotSystem* m_pQuickSlotSystem = nullptr;
    cltTitleSystem* m_pTitleSystem = nullptr;
    cltPetSystem* m_pPetSystem = nullptr;
    std::array<strInventoryItem, 255> m_inventoryItems{};
};

int comp_arry_kind_up(const void* a1, const void* a2);
int comp_arry_kind_down(const void* a1, const void* a2);
int comp_arry_string_up(const void* a1, const void* a2);
int comp_arry_string_down(const void* a1, const void* a2);
int comp_arry_rare_up(const void* a1, const void* a2);
int comp_arry_rare_down(const void* a1, const void* a2);

