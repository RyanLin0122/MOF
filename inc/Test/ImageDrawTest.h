#pragma once

#include <d3d9.h>

// 前向宣告，避免在標頭檔中引入過多不必要的內容
class cltImageManager;
class GameImage;

/// @class ImageDrawTest
/// @brief 封裝了載入、處理和渲染 GameImage 的所有測試邏輯。
class ImageDrawTest
{
public:
    ImageDrawTest();
    ~ImageDrawTest();

    /// @brief 初始化測試環境，包括開啟VFS、建立管理器和載入圖片資源。
    /// @return 成功返回 S_OK，失敗返回 E_FAIL。
    HRESULT Initialize();

    /// @brief 執行每一幀的渲染邏輯。
    void Render();

private:
    /// @brief 釋放所有資源。
    void Cleanup();

private:
    // --- 成員變數 ---
    cltImageManager* m_pImageManager; // 圖片物件池管理器
    GameImage* m_pItemImage1;   // 第一個物品圖片
    GameImage* m_pItemImage2;   // 第二個物品圖片
    GameImage* m_pMapBgImage;   // 地圖背景圖片
};