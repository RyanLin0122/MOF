#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/CDeviceResetManager.h"

// 前向宣告，避免不必要的標頭檔引用
struct VertexBufferData;
struct TextureListData;

/// @struct ImageVertex
/// @brief 用於繪製2D圖片的頂點格式，對應 D3DFVF_XYZRHW | D3DFVF_TEX1。
struct ImageVertex {
    float x, y, z, rhw; // 位置 (Reciprocal of Homogeneous W)
    float u, v;         // 紋理座標
};

/// @class Image
/// @brief 負責管理和繪製單一 2D 圖片的基礎類別。
///
/// 這個類別處理一個由四個頂點組成的矩形，並將其對應的紋理區塊繪製到螢幕上。
class Image {
public:
    Image();
    virtual ~Image(); // 基底類別建議使用虛擬解構函式

    /// @brief 建立圖片資源，包括紋理和頂點緩衝區。
    /// @param szFilename 紋理檔案的路徑。
    /// @param texWidth 整個紋理的寬度。
    /// @param texHeight 整個紋理的高度。
    /// @param frameWidth 圖片中單一影格的寬度。
    /// @param frameHeight 圖片中單一影格的高度。
    /// @return 成功返回 true，失敗返回 false。
    bool CreateImage(const char* szFilename, int texWidth, int texHeight, int frameWidth, int frameHeight);

    /// @brief 釋放由 CreateImage 建立的資源。
    void Free();

    /// @brief 設定圖片的繪製位置和要顯示的影格。
    /// @param x 螢幕上的 X 座標。
    /// @param y 螢幕上的 Y 座標。
    /// @param frameID 要顯示的影格編號。
    void SetImage(float x, float y, unsigned short frameID);

    /// @brief 設定圖片的最終繪製位置。
    void SetPosition(float x, float y);

    /// @brief 設定圖片的旋轉角度（繞 Z 軸）。
    /// @param radians 旋轉的弧度。
    void SetAngle(float radians);

    /// @brief 左右翻轉圖片。
    void SetLRReverse();

    /// @brief 直接設定要顯示的紋理區塊（影格）。
    void SetBlockID(unsigned short blockID);

    /// @brief 將目前的頂點資料更新到頂點緩衝區中。
    void Process();

    /// @brief 使用自身的頂點緩衝區來繪製圖片。
    void DrawImage();

    /// @brief 使用外部提供的頂點緩衝區來繪製圖片。
    void DrawImage(IDirect3DVertexBuffer9* pVB);

    /// @brief 取得圖片的紋理與格線資訊。
    void GetVerTextInfo(int* gridX, float* width, float* height, float* gridWidth, float* gridHeight);

protected:
    // --- 成員變數 (根據反編譯程式碼的記憶體位移推斷) ---

    VertexBufferData* m_pVBData;      // 位移 0:   指向頂點緩衝區管理節點的指標
    TextureListData* m_pTexData;     // 位移 4:   指向紋理管理節點的指標
    int              m_nGridX;         // 位移 8:   紋理在 X 軸上可容納的影格數
    float            m_fFrameWidth;    // 位移 12:  單一影格的寬度
    float            m_fFrameHeight;   // 位移 16:  單一影格的高度
    float            m_fGridWidth;     // 位移 20:  單一影格寬度佔總寬度的比例 (U)
    float            m_fGridHeight;    // 位移 24:  單一影格高度占總高度的比例 (V)
    unsigned short   m_nWord_28;       // 位移 28:  未知用途的 WORD

    ImageVertex      m_vertices[4];    // 位移 32:  最終要繪製的4個頂點資料 (96 bytes)

    D3DXVECTOR3      m_baseVertices[4]; // 位移 128: 用於旋轉和定位的基礎頂點座標 (48 bytes)

    // 中間有未知用途的 padding

    bool             m_bIsCreated;     // 位移 176: 標記資源是否已建立
private:
    CDeviceResetManager* g_Device_Reset_Manager;
};