#pragma once

// 為了讓 cltImageManager 能夠編譯，我們需要 GameImage 的定義。
// 這裡我們包含一個根據 GameImage.c 推斷出的 GameImage.h 標頭檔。
#include "Image/GameImage.h" 
#include "Image/ResourceMgr.h"

/// @class cltImageManager
/// @brief GameImage 物件池管理器。
///
/// 負責管理一個固定大小的 GameImage 物件陣列，
/// 提供獲取和釋放 GameImage 物件的介面，以達到重複利用、提升效能的目的。
class cltImageManager {
public:
    static cltImageManager* GetInstance();

    /// @brief 解構函式。
    /// 會自動呼叫陣列中所有 GameImage 物件的解構函式。
    ~cltImageManager();

    /// @brief 初始化管理器。
    /// 為池中的每一個 GameImage 物件預先建立頂點緩衝區。
    void Initialize();

    /// @brief 釋放所有資源。
    /// 重設池中所有的 GameImage 物件。
    void Free();

    /// @brief 從池中獲取一個可用的 GameImage 物件。
    /// @param dwGroupID     資源的群組 ID (對應 a2)。
    /// @param dwResourceID  資源的唯一 ID (對應 a3)。
    /// @param a4            傳遞給資源管理器的參數。
    /// @param a5            傳遞給資源管理器的參數。
    /// @return 指向一個已設定好資源的 GameImage 物件的指標，如果池已滿則返回 nullptr。
    GameImage* GetGameImage(unsigned int dwGroupID, unsigned int dwResourceID, int a4 = 0, int a5 = 0);

    /// @brief 將一個 GameImage 物件歸還到池中。
    /// @param pImage 要釋放的 GameImage 物件指標。
    void ReleaseGameImage(GameImage* pImage);

    /// @brief 釋放所有正在使用的 GameImage 物件。
    void ReleaseAllGameImage();

    /// @brief 更新所有正在使用的 GameImage 物件的頂點資料。
    void ProcessAllGameImage();

    // 物件池的大小
    static const int MAX_IMAGES = 5000;

    // GameImage 物件池陣列
    GameImage m_Images[MAX_IMAGES];
private:
    cltImageManager();
    static cltImageManager* s_pInstance;
};