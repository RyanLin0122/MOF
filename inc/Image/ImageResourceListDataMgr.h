#pragma once

#include "ImageResource.h" // 假設 ImageResource.h 已被定義且可被包含

// 前向宣告
struct ImageResourceListData;

/// @class ImageResourceListDataMgr
/// @brief 管理 ImageResourceListData 節點的雙向鏈結串列。
///
/// 這個管理器負責建立、刪除和追蹤所有的圖片資源。
/// 它的行為是從反編譯的 C 程式碼中還原的。
class ImageResourceListDataMgr {
public:
    /// @brief 建構函式
    ImageResourceListDataMgr();

    /// @brief 解構函式
    ~ImageResourceListDataMgr();

    /// @brief 在鏈結串列的末尾新增一個新的資源節點。
    /// @return 指向新建立的 ImageResourceListData 節點的指標。
    ImageResourceListData* Add();

    /// @brief 從鏈結串列中刪除指定的資源節點。
    /// @param pNode 要刪除的節點指標。
    void Delete(ImageResourceListData* pNode);

    /// @brief 刪除鏈結串列中的所有節點。
    void DeleteAll();

    /// @brief 當 Direct3D 裝置遺失 (Device Lost) 時呼叫。
    /// 遍歷所有資源並釋放它們的紋理，但保留記憶體中的資料。
    void DeviceLostToRelease();

    /// @brief 當 Direct3D 裝置重設 (Reset) 後呼叫。
    /// 遍歷所有資源並根據保留的檔案資訊重新載入它們。
    void DeviceLostToReLoad();

private:
    ImageResourceListData* m_pHead; // 指向鏈結串列的第一個節點
    ImageResourceListData* m_pTail; // 指向鏈結串列的最後一個節點
    int m_nCount;                   // 鏈結串列中節點的總數
};

/// @struct ImageResourceListData
/// @brief 雙向鏈結串列中的節點，用於存放一個圖片資源及其元數據。
///
/// 結構是根據反編譯程式碼中的記憶體佈局和大小 (0x138 bytes) 推斷出來的。
struct ImageResourceListData {
    ImageResourceListData* pPrev;       // 指向前一個節點
    ImageResourceListData* pNext;       // 指向後一個節點
    ImageResource m_Resource;           // 嵌入的圖片資源物件

    // 從 Device_Reset_Manager::CreateImageResource 推斷出的成員
    char m_szFileName[255];             // 資源的檔案名稱
    char m_cFlag;                       // 未使用的旗標 (對應 a3)
    unsigned char m_ucPackerType;       // 封裝檔類型 (對應 a4)
    char m_padding[3];                  // 記憶體對齊的填充位元組

    /// @brief 節點的建構函式
    ImageResourceListData();

    /// @brief 節點的解構函式
    // 在原始碼中，有一個明確的解構函式被呼叫，
    // 主要目的是觸發 ImageResource 的解構。在 C++ 中，這是自動處理的。
    ~ImageResourceListData();
};