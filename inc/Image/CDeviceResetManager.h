#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/ImageResourceListDataMgr.h" // 包含我們之前還原的 ImageResource 管理器
#include <d3dx9tex.h>

//-- 輔助類別和結構的簡化定義 (Stub) -------------------------------------------
// 這些是根據 CDeviceResetManager.cpp 的用法推斷出的簡化版定義

// 假設的 VertexBufferData 結構
struct VertexBufferData {
    VertexBufferData* pPrev;
    VertexBufferData* pNext;
    IDirect3DVertexBuffer9* pVertexBuffer; // 從CreateVertexBuffer推斷，偏移量為+8
    unsigned short capacity;               // 偏移量為+12
    unsigned char type;                    // 偏移量為+14
};

// 假設的 VertexBufferDataMgr 類別
class VertexBufferDataMgr {
public:
    VertexBufferData* Add() { /* 實作細節省略 */ return nullptr; }
    void Delete(VertexBufferData* pNode) { /* 實作細節省略 */ }
};

// 假設的 TextureListData 結構
struct TextureListData {
    TextureListData* pPrev;
    TextureListData* pNext;
    IDirect3DTexture9* pTexture;           // 從CreateTexture推斷，偏移量為+8
    char szFileName[256];                  // 儲存檔案路徑
    unsigned char flag;                    // 儲存旗標
};

// 假設的 TextureListDataMgr 類別
class TextureListDataMgr {
public:
    TextureListData* Add() { /* 實作細節省略 */ return nullptr; }
    void Delete(TextureListData* pNode) { /* 實作細節省略 */ }
};

//--------------------------------------------------------------------------------

/// @class CDeviceResetManager
/// @brief 集中管理因 Direct3D 裝置重設 (Device Reset) 而需要重新建立的資源。
///
/// 這個類別封裝了對 Vertex Buffers、圖片資源 (ImageResource) 和紋理 (Texture) 的管理，
/// 並處理裝置遺失 (Device Lost) 和重設時的相關邏輯。
class CDeviceResetManager {
public:
    static CDeviceResetManager* GetInstance();
    /// @brief 解構函式
    ~CDeviceResetManager();

    /// @brief 建立一個指定類型的頂點緩衝區。
    /// @param capacity 頂點緩衝區能容納的頂點數量。
    /// @param type 頂點類型 (0/3: GIVertex, 1: AlphaBoxVertex, 2: ImageVertex)。
    /// @return 指向新建立的 VertexBufferData 節點的指標。
    VertexBufferData* CreateVertexBuffer(unsigned short capacity, unsigned char type);

    /// @brief 刪除一個頂點緩衝區。
    /// @param pBufferData 要刪除的頂點緩衝區節點。
    void DeleteVertexBuffer(VertexBufferData* pBufferData);

    /// @brief 建立一個圖片資源。
    /// @param pFileName 圖片檔案的路徑或在封裝檔中的名稱。
    /// @param flag 傳遞給 LoadGI/LoadGIInPack 的旗標 (原始碼中的 a3)。
    /// @param packerType 傳遞給 LoadGI/LoadGIInPack 的旗標 (原始碼中的 a4)。
    /// @param a5 傳遞給 LoadGIInPack 的參數 (原始碼中的 a5)，通常是封裝檔類型。
    /// @return 指向新建立的 ImageResourceListData 節點的指標。
    ImageResourceListData* CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5);

    /// @brief 刪除一個圖片資源。
    /// @param pImageNode 要刪除的圖片資源節點。
    void DeleteImageResource(ImageResourceListData* pImageNode);

    /// @brief 從檔案建立一個紋理。
    /// @param pFileName 紋理檔案的路徑。
    /// @param flag 原始碼中的旗標 (a3)。
    /// @return 指向新建立的 TextureListData 節點的指標。
    TextureListData* CreateTexture(const char* pFileName, unsigned char flag);

    /// @brief 刪除一個紋理。
    /// @param pTextureNode 要刪除的紋理節點。
    void DeleteTexture(TextureListData* pTextureNode);

    /// @brief 取得共享的 ID3DXSprite 物件，如果不存在則建立一個。
    /// @return 指向 ID3DXSprite 物件的指標。
    ID3DXSprite* GetSpriteObject();

    /// @brief 處理裝置重設。
    /// 當主迴圈偵測到裝置需要重設時呼叫此函式。
    /// @param hresult 來自 Present() 或其他 D3D 呼叫的返回碼。
    /// @return 如果裝置狀態正常或已成功重設，返回 true。如果重設失敗，返回 false。
    bool ResetToDevice(long hresult);

private:
    // 私有建構函式
    CDeviceResetManager();

    // 指向唯一實例的靜態指標
    static CDeviceResetManager* s_pInstance;
    // 根據反編譯程式碼的建構函式和成員位移，還原出以下成員
    VertexBufferDataMgr      m_vertexBufferMgr;      // 偏移量: 0
    ImageResourceListDataMgr m_imageResourceMgr;     // 偏移量: 12
    TextureListDataMgr       m_textureMgr;           // 偏移量: 24
    ID3DXSprite* m_pSprite;              // 偏移量: 36
};