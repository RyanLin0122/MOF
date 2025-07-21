#pragma once

#include <windows.h> // 使用 timeGetTime, MessageBoxA 等
#include <cstdio>    // 使用 NULL

/**
 * @struct ResourceInfo
 * @brief 儲存單一資源的中繼資料。
 */
struct ResourceInfo {
    unsigned int id;         // 資源的唯一ID
    void* pData;      // 指向實際資源資料的指標
    int          refCount;   // 引用計數
    DWORD        timestamp;  // 最後一次被釋放的時間戳
};

/**
 * @class cltBaseResource
 * @brief 資源管理的抽象基底類別。
 *
 * 定義了一套通用的資源管理框架，包括載入、釋放、引用計數和自動回收。
 * 具體的資源載入與釋放邏輯由衍生類別透過實作純虛擬函式來完成。
 */
class cltBaseResource {
public:
    /// @brief 建構函式
    cltBaseResource();

    /// @brief 虛擬解構函式
    virtual ~cltBaseResource();

    /// @brief 初始化資源管理器。
    /// @param capacity 資源陣列的最大容量。
    /// @param timeout 閒置資源被自動回收的超時時間(毫秒)。0表示不回收。
    void Initialize(unsigned int capacity, unsigned int timeout);

    /// @brief 釋放所有已分配的內部記憶體。
    void Free();

    /// @brief 取得資源。如果資源不存在，會嘗試載入。
    /// @return 成功則返回指向資源資料的指標，否則返回 nullptr。
    void* Get(unsigned int id, int a3, int a4);

    /// @brief 取得資源，但如果不存在則不載入。
    /// @return 成功則返回指向資源資料的指標，否則返回 nullptr。
    void* Get1(unsigned int id, int a3, unsigned char a4);

    /// @brief 釋放對一個資源的引用。
    /// @return 成功返回1，資源不存在返回0。
    int Release(unsigned int id);

    /// @brief 取得指定資源的引用計數。
    /// @return 返回引用計數值，若資源不存在則返回0。
    int GetRefCount(unsigned int id);

    /// @brief 輪詢並清理閒置的資源。
    virtual void Poll();

    /// @brief 刪除所有已載入的資源。
    void DeleteAllResource();

protected:
    /// @brief 嘗試新增一個資源到管理器中（通常由Get內部呼叫）。
    /// @return 1:錯誤(Buffer滿), 2:已存在, 3:新增成功
    virtual int Add(unsigned int id, int a3, int a4);

    /// @brief 從管理器中刪除一個資源。
    /// @return 成功返回1，失敗返回0。
    virtual int Delete(unsigned int id);

    // --- 純虛擬函式 (由衍生類別實作) ---

    /// @brief 從檔案載入資源。
    virtual void* LoadResource(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief 從封裝檔載入資源。
    virtual void* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) = 0;

    /// @brief 釋放資源資料。
    virtual void FreeResource(void* pResourceData) = 0;

protected:
    ResourceInfo* m_pResourceArray;           // 資源資訊陣列
    unsigned int  m_uResourceArrayCapacity;   // 陣列容量
    unsigned int  m_uResourceCount;           // 目前資源數量
    DWORD         m_dwTimeout;                // 閒置超時時間(ms)
    bool          m_bInitialized;             // 是否已初始化
};