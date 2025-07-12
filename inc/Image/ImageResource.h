#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <stdio.h>   // For FILE
#include <stdlib.h>  // For size_t
#include <stdint.h>  // For uintptr_t
#include <new>
#include "Image/comp.h"

/// @brief 儲存 GI 檔案中每個動畫影格的資訊。
/// 這個結構的大小 (52 bytes) 是根據 LoadGI/LoadGIInPack 中
/// 的記憶體分配和複製模式反向工程得出的。
struct AnimationFrameData {
    // 實際的成員未知，但總大小為 52 bytes
    DWORD unknown_data[13]; 
};


/// @class ImageResource
/// @brief 管理單一圖片資源，包括從檔案讀取、解壓縮、以及轉換為 Direct3D 紋理。
/// 這個類別處理兩種來源的圖片：獨立的 .gi 檔案，或從封裝檔 (pack file) 中讀取的資料。
class ImageResource {
public:
    /// @brief 建構函式：初始化所有成員變數為預設值。
    ImageResource();

    /// @brief 解構函式：釋放所有已分配的資源。
    ~ImageResource();

    /// @brief 從獨立的 .gi 檔案載入圖片資料。
    /// @param fileName 檔案路徑。
    /// @param a3 一個在原始碼中存在但未被使用的參數。
    /// @return 如果載入成功返回 true，否則返回 false。
    bool LoadGI(const char* fileName, unsigned char a3);

    /// @brief 從封裝檔 (pack file) 的記憶體中載入圖片資料。
    /// @param fileName 在封裝檔中的檔案名稱或識別符。
    /// @param packerType 標示使用哪種封裝檔系統 (0: g_clMofPacking, 1: g_clCharPacking)。
    /// @param a4 一個在原始碼中存在但未被使用的參數。
    /// @return 如果載入成功返回 true，否則返回 false。
    bool LoadGIInPack(char* fileNameInPack, int packerType, unsigned char a4);

    /// @brief 將載入到記憶體中的圖片資料轉換成 Direct3D 紋理。
    /// @return 如果紋理建立成功返回 true，否則返回 false。
    bool LoadTexture();

    /// @brief 重設所有 GI 相關的資料，釋放記憶體並將成員變數恢復到初始狀態。
    /// 主要用於裝置遺失 (Device Lost) 後的資源重建。
    void ResetGIData();

private:
    /// @brief 根據 D3DFORMAT 計算每個像素的字節數 (pixel depth)。
    /// @param format Direct3D 的紋理格式。
    /// @return 每個像素的字節數。
    unsigned char GetPixelDepth(D3DFORMAT format);

private:
    // 以下成員變數是根據反編譯程式碼中的記憶體位移 (offset) 還原的。

    int             m_version;              // 位移 +4:  檔案版本，20 代表 RLE 壓縮。
    unsigned short  m_width;                // 位移 +8:  圖片寬度。
    unsigned short  m_height;               // 位移 +10: 圖片高度。
    unsigned int    m_imageDataSize;        // 位移 +12: 圖片資料的總大小 (壓縮後或原始大小)。
    unsigned int    m_decompressedSize;     // 位移 +16: 壓縮前的原始圖片大小 (僅當版本為 20 時使用)。
    
    unsigned short  m_animationFrameCount;  // 位移 +20: 動畫影格的數量。
    AnimationFrameData* m_pAnimationFrames; // 位移 +24: 指向動畫影格資料陣列的指標。
    D3DFORMAT       m_d3dFormat;            // 位移 +28: 紋理的 D3D 格式。
    
    unsigned char   m_unknownFlag;          // 位移 +32: 一個用途不明的旗標。
    unsigned char* m_pImageData;           // 位移 +36: 指向像素資料的指標 (解壓縮後或原始資料)。
    
    IDirect3DTexture9* m_pTexture;        // 位移 +40: 指向已建立的 Direct3D 紋理的指標。
};