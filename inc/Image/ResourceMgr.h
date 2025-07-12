#pragma once

#include "cltGIResource.h"
#include "LoadingThread.h"

/// @class ResourceMgr
/// @brief 頂層的資源管理器，負責調度所有類型的資源。
///
/// 作為一個 Facade，它將請求轉發給內部的 cltGIResource 管理器陣列，
/// 並協調 LoadingThread 進行非同步資源載入。
class ResourceMgr {
public:
    // 定義資源類型枚舉，提高程式碼可讀性
    enum eResourceType {
        RES_CHARACTER = 0,
        RES_MONSTER,
        RES_MAPTILE,
        RES_ITEM,
        RES_NPC,
        RES_UI,
        RES_IMAGE,
        RES_EFFECT,
        RES_OBJECT,
        RES_MINIGAME,
        RES_MOVEMAPBG,
        RES_LOBBY,
        RES_SPIRIT,     // 原始碼中索引13在12之前初始化，這裡調整順序
        RES_MAPBG,
        RES_COUNT // 總類型數
    };

    static ResourceMgr* GetInstance();

    ~ResourceMgr();

    /// @brief 輪詢所有子管理器和載入執行緒。
    void Poll();

    /// @brief 獲取一個圖片資源。
    /// @param type 資源類型。
    /// @param resourceID 資源的唯一ID。
    /// @param bAsync 是否使用非同步載入（僅對特定類型有效）。
    /// @param priority 載入優先級。
    /// @return 成功返回資源指標，若非同步載入則立即返回 nullptr。
    ImageResourceListData* GetImageResource(eResourceType type, unsigned int resourceID, bool bAsync = false, int priority = 0);

    /// @brief 釋放對一個圖片資源的引用。
    void ReleaseImageResource(eResourceType type, unsigned int resourceID);
    
    /// @brief 在載入佇列中尋找指定的資源ID。
    bool FindInResLoadingList(unsigned int resourceID);
    
    /// @brief 從載入佇列中刪除指定的資源ID。
    void DelResInLoadingList(unsigned int resourceID);

    /// @brief 清空整個載入佇列。
    void ClearLoadingList();

private:
    // 讓 LoadingThread 可以回呼私有的 LoadingImageResource 函式
    friend class LoadingThread;

    /// @brief 實際執行資源載入的函式，由背景執行緒呼叫。
    void LoadingImageResource(unsigned int groupID, unsigned int resourceID, int a4, int a5);

private:
    // cltGIResource 管理器陣列，每個元素對應一種資源類型
    cltGIResource m_GIResources[RES_COUNT];

    // 背景載入執行緒管理器
    LoadingThread m_Loader;
    
    // 是否使用封裝檔的旗標
    bool m_bUsePackFile;

    static ResourceMgr* s_pInstance;
    ResourceMgr();
};