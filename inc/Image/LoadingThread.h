#pragma once
#include <windows.h>
#include <deque>

/// @struct tResInfo
/// @brief 描述一個資源載入請求的結構。
struct tResInfo {
    unsigned int dwResourceID;
    unsigned int dwGroupID;
    char cPriority; // 或其他旗標
};

/// @class LoadingThread
/// @brief 管理一個背景執行緒，用於非同步載入資源。
///
/// 使用一個 std::deque 作為請求佇列，並透過 Critical Section 確保執行緒安全。
class LoadingThread {
public:
    LoadingThread();
    ~LoadingThread();

    /// @brief 主迴圈呼叫的輪詢函式。
    /// 如果佇列中有請求且執行緒未執行，則啟動背景執行緒。
    void Poll();

    /// @brief 向佇列中新增一個背景載入請求。
    /// @param resInfo 包含資源ID和群組ID的請求資訊。
    /// @return 成功返回 true。
    bool AddBackGroundLoadingRes(const tResInfo& resInfo);

    /// @brief 在載入佇列中尋找指定的資源ID。
    /// @return 如果找到返回 true。
    bool FindInResLoadingList(unsigned int resourceID);

    /// @brief 從載入佇列中刪除指定的資源ID。
    void DelResInLoadingList(unsigned int resourceID);

    /// @brief 清空整個載入佇列。
    void ClearLodingList();

private:
    /// @brief 背景執行緒的進入點函式。
    /// @param pParam 指向 LoadingThread 物件的 this 指標。
    /// @return 執行緒結束碼。
    static unsigned int __stdcall OnBackgroundLoadUpdate(void* pParam);
    
    /// @brief 讓背景執行緒安全地退出。
    void ExitThread();

private:
    HANDLE m_hThread;                // 位移+0: 背景執行緒的控制代碼
    bool m_bIsRunning;               // 位移+4: 標記背景執行緒是否正在執行
    CRITICAL_SECTION m_cs;           // 位移+8: 用於保護佇列的臨界區
    
    // C++ STL 容器，用於儲存載入請求。
    // 反編譯程式碼中的大量指標操作都是在模擬這個 deque。
    std::deque<tResInfo> m_loadingQueue; // 位移+32 開始是 deque 的內部資料
};