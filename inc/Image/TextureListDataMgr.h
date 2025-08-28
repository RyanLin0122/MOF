#pragma once

#include <d3d9.h>
#include <d3dx9tex.h> // 使用D3DXCreateTextureFromFileExA需要此標頭
#include <cstdio>      // 用於 NULL

// 根據反編譯程式碼中的用法，定義TextureListData結構
// 大小為 0x10C (268位元組)
struct TextureListData {
    TextureListData* pPrev;          // 位移 +0: 指向前一個節點
    TextureListData* pNext;          // 位移 +4: 指向後一個節點
    IDirect3DTexture9* pTexture;     // 位移 +8: 指向D3D紋理物件
    char szFileName[255];            // 位移 +12: 儲存紋理的檔案路徑
    unsigned char flag;              // 位移 +267: 儲存的旗標

    // 節點的解構函式
    ~TextureListData() {
        // 在節點被銷毀時，自動釋放其擁有的D3D資源
        if (pTexture)
        {
            pTexture->Release();
            pTexture = nullptr;
        }
    }
};

/**
 * @class TextureListDataMgr
 * @brief 管理TextureListData節點的雙向鏈結串列。
 *
 * 這個管理器負責紋理的建立（通過外部邏輯）、刪除、並在D3D裝置遺失時處理所有紋理資源。
 */
class TextureListDataMgr {
public:
    /// @brief 建構函式
    TextureListDataMgr();

    /// @brief 解構函式
    ~TextureListDataMgr();

    /// @brief 新增一個節點到鏈結串列尾部並返回。
    /// @return 指向新建立的TextureListData節點的指標。
    TextureListData* Add();

    /// @brief 從鏈結串列中刪除指定的節點。
    /// @param pNode 要刪除的節點指標。
    void Delete(TextureListData* pNode);

    /// @brief 刪除並釋放管理器中的所有節點和資源。
    void DeleteAll();

    /// @brief 處理D3D裝置遺失事件。
    /// 此函式會釋放所有紋理，但保留節點結構以便後續重建。
    void DeviceLostToRelease();

    /// @brief 處理D3D裝置重設事件。
    /// 此函式會根據之前儲存的檔案路徑，重新載入所有紋理。
    void DeviceLostToReLoad();

private:
    TextureListData* m_pHead; // 鏈結串列的頭指標
    TextureListData* m_pTail; // 鏈結串列的尾指標
    int m_nCount;             // 鏈結串列中的節點數量
};