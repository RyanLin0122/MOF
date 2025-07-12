#pragma once

#include <windows.h> // for DWORD

// 前向宣告，讓此頭檔案可以獨立被引用，而不需要知道 ImageResourceListData 的完整定義
struct ImageResourceListData;

/// @class cltBaseResource
/// @brief 泛用的資源管理器基底類別。
///
/// 這個類別管理一個資源快取池，包含參考計數和基於時間的自動釋放機制。
/// 它定義了一個框架，而實際的資源載入與釋放由衍生類別 (如 cltGIResource)
/// 透過實現純虛擬函式來完成。
class cltBaseResource {
public:
    /// @brief 建構函式：初始化所有成員為預設值。
    cltBaseResource();

    /// @brief 虛擬解構函式：確保在刪除基底類別指標時，衍生類別的解構函式能被正確呼叫。
    virtual ~cltBaseResource();

    /// @brief 初始化資源管理器。
    /// @param capacity 快取池的最大容量。
    /// @param timeout 資源在無人引用後，自動被回收的逾時時間 (毫秒)。0 表示不回收。
    void Initialize(unsigned int capacity, unsigned int timeout);

    /// @brief 釋放所有資源並清理快取池。
    void Free();

    /// @brief 取得一個資源。如果資源不在快取中，會觸發載入流程。
    /// @param id 資源的唯一ID。
    /// @param a3 傳遞給載入函式的參數。
    /// @param a4 傳遞給載入函式的參數。
    /// @return 指向資源資料的指標 (void*)，失敗則返回 nullptr。
    void* Get(unsigned int id, int a3 = 0, int a4 = 0);

    /// @brief 僅取得已存在於快取中的資源，如果不存在則不會觸發載入。
    /// @param id 資源的唯一ID。
    /// @return 指向資源資料的指標 (void*)，若不存在則返回 nullptr。
    void* Get1(unsigned int id, int a3 = 0, unsigned char a4 = 0);

    /// @brief 釋放對一個資源的引用 (將參考計數減 1)。
    /// @param id 資源的ID。
    /// @return 成功返回 1，失敗返回 0。
    int Release(unsigned int id);

    /// @brief 強制從快取池中刪除一個資源，無論其參考計數為何。
    /// @param id 資源的ID。
    /// @return 成功返回 1，失敗返回 0。
    int Delete(unsigned int id);

    /// @brief 輪詢函式，用於檢查並回收超時且無人引用的資源。
    void Poll();

    /// @brief 強制刪除所有已載入的資源。
    void DeleteAllResource();

    /// @brief 取得指定資源目前的參考計數。
    /// @param id 資源的ID。
    /// @return 資源的參考計數，若資源不存在則返回 0。
    int GetRefCount(unsigned int id);

protected:
    /// @struct ResourceEntry
    /// @brief 在資源快取池中，用於描述單一資源狀態的結構。
    struct ResourceEntry {
        unsigned int id;        // 資源的唯一ID
        void* pData;     // 指向實際資源資料的指標 (例如 ImageResourceListData*)
        int          refCount;  // 參考計數
        DWORD        lastAccess;// 最後存取時間戳 (timeGetTime() 的結果)，用於自動回收
    };

    // --- 成員變數 ---
    ResourceEntry* m_pResourcePool;   // 指向資源快取池陣列的指標
    unsigned int   m_nCapacity;       // 快取池的最大容量
    unsigned int   m_nItemCount;      // 目前快取池中的項目數量
    unsigned int   m_nTimeout;        // 資源自動回收的逾時時間 (毫秒)
    bool           m_bUnkFlag;        // 未知的旗標 (在位移+20)

    /// @brief 內部函式，用於處理資源的新增邏輯，會呼叫虛擬載入函式。
    /// @return 狀態碼 (0:失敗, 1:已滿, 2:已存在, 3:成功加入)。
    int Add(unsigned int id, int a3, int a4);

    // --- 純虛擬函式 (Pure Virtual Functions) ---
    // 以下函式定義了衍生類別必須實現的介面。

    /// @brief 從獨立檔案載入資源的實作。
    virtual ImageResourceListData* LoadResource(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief 從封裝檔載入資源的實作。
    virtual ImageResourceListData* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief 釋放單一資源所佔用的記憶體和資源。
    virtual void FreeResource(void* pResourceData) = 0;
};