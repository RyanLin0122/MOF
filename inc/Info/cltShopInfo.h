#pragma once

#include <cstdint>

// ============================================================================
// 商店資訊類別 (cltShopInfo)
// 來源：mofclient.c::cltShopInfo (反編譯偏移 0x590940 系列)
// 解析的資料檔：
//   - shoplist.txt          普通商店清單
//   - packageshoplist.txt   패키지(套裝) 商店清單
// ============================================================================

#pragma pack(push, 1)

// ----------------------------------------------------------------------------
// stShopInfo — 一筆普通商店資料 (1012 bytes)
// 來自反編譯：cltShopInfo::Initialize_Shop 內 `operator new(1012 * count)`，
// 寫入點：
//   *(_WORD  *)(base+0)    ShopID         (TranslateKindCode)
//   *(_DWORD *)(base+4)    TaxRate        (atoi, 必須 >= 1000)
//   *(_WORD  *)(base+8..)  Items[500]     (TranslateKindCode；驗證 GetItemKindInfo)
//   *(_WORD  *)(base+1008) ItemCount      (有效商品數)
// ----------------------------------------------------------------------------
struct stShopInfo {
    // 韓文：상점 ID / 中文：商店 ID  // offset 0
    std::uint16_t ShopID;

    // 韓文：(정렬용 패딩) / 中文：對齊填充  // offset 2
    std::uint16_t pad0;

    // 韓文：세율(1000분율) / 中文：稅率(千分率，須 >= 1000)  // offset 4
    std::int32_t TaxRate;

    // 韓文：물품 코드 배열 / 中文：商品代碼陣列  // offset 8 ~ 1007
    // 每個元素為 cltShopInfo::TranslateKindCode 編碼後的 16-bit 物品代碼。
    // 解析時欄位若為 "0" 視為空 slot，pointer 仍前進；IsShopSellItem 在第一個 0 即停止搜尋。
    std::uint16_t Items[500];

    // 韓文：물품 갯수 / 中文：商品數量  // offset 1008
    // 紀錄此 row 內非 "0" 物品的個數（不含 sparse 0 slot 之間的空格）
    std::uint16_t ItemCount;

    // 韓文：(끝쪽 패딩) / 中文：尾端對齊填充  // offset 1010
    std::uint16_t pad1;
};
static_assert(sizeof(stShopInfo) == 1012, "stShopInfo must be 1012 bytes");

// ----------------------------------------------------------------------------
// strPackageShopInfo::Item — 單一套裝商品 (8 bytes)
// 寫入位置自反編譯：
//   *(WORD *)(v13-1)    ItemKind  (cltItemKindInfo::TranslateKindCode)
//   *(WORD *)(v13)      ItemQty   (atoi)
//   *(int  *)(v13+1)    Price     (atoi, 4 bytes)
//   v13 += 4            => 8 bytes / 條目
// ----------------------------------------------------------------------------
struct stPackageShopItem {
    // 韓文：물품 종류 / 中文：物品種類  // offset 0
    std::uint16_t ItemKind;

    // 韓文：물품 수량 / 中文：物品數量  // offset 2
    std::uint16_t ItemQty;

    // 韓文：물품 가격 / 中文：物品價格 (4 bytes int)  // offset 4
    std::int32_t Price;
};
static_assert(sizeof(stPackageShopItem) == 8, "stPackageShopItem must be 8 bytes");

// ----------------------------------------------------------------------------
// strPackageShopInfo — 一筆套裝商店資料 (404 bytes)
// 來自反編譯：cltShopInfo::Initialize_PackageShop 內 `operator new(404 * count)`，
// 並以 `v18 += 202` (WORD) 步進 = 404 bytes / row。
// 內部 v13 = v18 + 3 起點 (byte offset 6)，使第一個 ItemKind 落在 byte offset 4。
// ----------------------------------------------------------------------------
struct strPackageShopInfo {
    // 韓文：상점 ID / 中文：商店 ID  // offset 0
    std::uint16_t ShopID;

    // 韓文：(정렬용 패딩) / 中文：對齊填充  // offset 2
    std::uint16_t pad0;

    // 韓文：패키지 항목 / 中文：套裝商品條目 (上限 50；超過將寫入下一筆 row)  // offset 4
    stPackageShopItem Items[50];
};
static_assert(sizeof(strPackageShopInfo) == 404, "strPackageShopInfo must be 404 bytes");

#pragma pack(pop)


// ----------------------------------------------------------------------------
// cltShopInfo — 載入器與查詢器
// 反編譯偏移：
//   0x590910 ctor / 0x590940 Initialize / 0x590970 Initialize_Shop
//   0x590C80 Initialize_PackageShop / 0x590F30 Free
//   0x590F70 GetTotalShopNum / 0x590F80 GetShopInfoByIndex
//   0x590FB0 TranslateKindCode / 0x591000 GetShopInfoByID
//   0x5910B0 IsPackageShop / 0x5910F0 GetPackageItemPrice
//   0x591170 IsShopSellItem
// ----------------------------------------------------------------------------
class cltShopInfo {
public:
    cltShopInfo();
    ~cltShopInfo();

    // 主初始化：依序呼叫 Initialize_Shop(shopFile)；若成功再呼叫 Initialize_PackageShop(packageFile)。
    // 返回 1 = 雙方均成功，0 = 失敗。
    int Initialize(char* shopFile, char* packageShopFile);

    // 解析 shoplist.txt
    int Initialize_Shop(char* fileName);

    // 解析 packageshoplist.txt
    int Initialize_PackageShop(char* fileName);

    // 釋放兩個 array
    void Free();

    std::uint16_t GetTotalShopNum();

    // 依索引取得普通商店指標 (越界回傳 nullptr)
    stShopInfo* GetShopInfoByIndex(int index);

    // 將 "字+4位數字" 形式 (例如 "G0001") 轉為 16-bit 代碼
    // 公式：( (toupper(c) + 31) << 11 ) | atoi(s+1)，atoi 結果須 < 0x800
    static std::uint16_t TranslateKindCode(char* code);

    // 依 ShopID 查詢，先比對 普通 Shop list；若不在再比對 Package list
    // outShopInfo / outPackageInfo 中只有一個會被填入；返回 1 = 命中其一，0 = 找不到
    int GetShopInfoByID(std::uint16_t shopId, stShopInfo** outShopInfo, strPackageShopInfo** outPackageInfo);

    // 是否為套裝商店
    int IsPackageShop(std::uint16_t shopId);

    // 套裝商店中指定 (item, qty) 的價格；找不到回傳 0
    std::int64_t GetPackageItemPrice(std::uint16_t shopId, std::uint16_t itemKind, std::uint16_t itemQty);

    // 商店是否販售指定物品 (普通 shop 比對 itemKind；套裝 shop 比對 (itemKind, qty))
    int IsShopSellItem(stShopInfo* shop, std::uint16_t itemKind, std::uint16_t itemQty);

private:
    // GT 全域 ::IsDigit (mofclient.c:342909) 等價：空字串回傳 true；每次迭代允許吃掉 1 個 '+' / '-'。
    // 與 cltItemKindInfo.h 內的 inline IsDigit 語意不同 (那版拒空字串、拒符號)，
    // 因此這裡放一個與 GT 完全一致的私有版本，以避免 unqualified 名稱解析到全域版本。
    static bool IsDigit(const char* a1);

    // 反編譯佈局 (32-bit) 之欄位語意：
    //   *(DWORD *)(this+0)   m_pShopList         (普通商店陣列指標)
    //   *(WORD  *)(this+4)   m_wTotalShopNum     (普通商店總數)
    //   *(WORD  *)(this+6)   m_wCurItemCount     (解析過程暫存：當前 row 內已寫入物品數)
    //   *(DWORD *)(this+8)   m_pPackageShopList  (套裝商店陣列指標)
    //   *(DWORD *)(this+12)  m_dwTotalPackageShopNum (套裝商店總數)
    // 在 x64 重建中，指標自然為 8 bytes，不需保留 32-bit 偏移。
    stShopInfo*         m_pShopList;
    std::uint16_t       m_wTotalShopNum;
    std::uint16_t       m_wCurItemCount;
    strPackageShopInfo* m_pPackageShopList;
    std::int32_t        m_iTotalPackageShopNum;
};
