#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/VertexBufferDataMgr.h"     // 包含之前還原的 VertexBuffer 管理器
#include "Image/ImageResourceListDataMgr.h"// 包含 ImageResource 管理器
#include "Image/TextureListDataMgr.h"      // 包含之前還原的 Texture 管理器

/**
 * @class CDeviceResetManager
 * @brief 集中管理D3D資源，並處理裝置遺失(Lost)與重設(Reset)的核心邏輯。
 *
 * 這個類別使用單例模式(Singleton)，確保程式中只有一個實例。
 * 它封裝了對 Vertex Buffers、Image Resources 和 Textures 的生命週期管理。
 */
class CDeviceResetManager {
public:
    /// @brief 取得唯一的類別實例。
    static CDeviceResetManager* GetInstance();

    /// @brief 解構函式。
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
    /// @param flag 傳遞給 LoadGI/LoadGIInPack 的旗標。
    /// @param packerType 傳遞給 LoadGI/LoadGIInPack 的打包器類型。
    /// @param a5 傳遞給 LoadGIInPack 的參數(通常是封裝檔類型)。
    /// @return 指向新建立的 ImageResourceListData 節點的指標。
    ImageResourceListData* CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5);

    /// @brief 刪除一個圖片資源。
    /// @param pImageNode 要刪除的圖片資源節點。
    void DeleteImageResource(ImageResourceListData* pImageNode);

    /// @brief 從檔案建立一個紋理。
    /// @param pFileName 紋理檔案的路徑。
    /// @param flag 原始碼中的旗標。
    /// @return 指向新建立的 TextureListData 節點的指標。
    TextureListData* CreateTexture(const char* pFileName, unsigned char flag);

    /// @brief 刪除一個紋理。
    /// @param pTextureNode 要刪除的紋理節點。
    void DeleteTexture(TextureListData* pTextureNode);

    /// @brief 取得共享的 ID3DXSprite 物件，如果不存在則建立一個。
    /// @return 指向 ID3DXSprite 物件的指標。
    ID3DXSprite* GetSpriteObject();

    /// @brief 處理裝置重設。
    /// @param hresult 來自 Present() 或其他 D3D 呼叫的返回碼。
    /// @return 如果裝置狀態正常或已成功重設，返回 true。
    bool ResetToDevice(long hresult);

private:
    /// @brief 私有建構函式，防止外部直接建立。
    CDeviceResetManager();

    // 刪除拷貝建構函式和賦值運算子，確保單例的唯一性
    CDeviceResetManager(const CDeviceResetManager&) = delete;
    CDeviceResetManager& operator=(const CDeviceResetManager&) = delete;

private:
    // 指向唯一實例的靜態指標
    static CDeviceResetManager* s_pInstance;

    // 成員變數 (順序和大小與反編譯碼一致)
    VertexBufferDataMgr      m_vertexBufferMgr;      // 位移: +0
    ImageResourceListDataMgr m_imageResourceMgr;     // 位移: +12
    TextureListDataMgr       m_textureMgr;           // 位移: +24
    ID3DXSprite* m_pSprite;              // 位移: +36
};