#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/GIVertex.h" // GameImage 使用 GIVertex 格式
#include "Image/ImageResource.h"
#include "Image/VertexBufferDataMgr.h"
#include "Image/ImageResourceListDataMgr.h"


/// @class GameImage
/// @brief 一個複雜的遊戲圖像物件，支持縮放、旋轉、顏色混合等多種特效。
///
/// 此類別通常由 cltImageManager 物件池管理，用於高效地顯示大量動態圖像。
class GameImage {
public:
    // --- 核心生命週期與繪製函式 ---

    /// @brief 建構函式：初始化所有狀態為預設值。
    GameImage();

    /// @brief 解構函式：釋放持有的頂點緩衝區。
    virtual ~GameImage();

    /// @brief 為此圖像建立一個可容納8個頂點的緩衝區。
    void CreateVertexBuffer();

    /// @brief 重設圖像狀態並釋放所有相關資源（頂點緩衝區和圖片資源）。
    void ResetGI();

    /// @brief 從資源管理器獲取圖片資料並與此物件綁定。
    void GetGIData(unsigned int dwGroupID, unsigned int dwResourceID, int a4, int a5);

    /// @brief 釋放對圖片資料的引用，將其歸還給資源管理器。
    void ReleaseGIData();

    /// @brief 根據當前所有狀態（位置、縮放、旋轉等）計算最終的頂點資料。
    bool Process();

    /// @brief 將圖像繪製到螢幕上。
    bool Draw();

    // --- 其他功能函式 ---

    /// @brief 獲取當前影格在紋理上的像素區域。
    void GetBlockRect(RECT* pOutRect) const;

    /// @brief 使用外部提供的頂點資料覆蓋內部頂點，用於頂點動畫。
    void VertexAnimationCalculator(const GIVertex* pSourceVertices);

    void UpdateVertexBuffer();

    /// @brief 將顏色覆蓋值重設為預設(白色)。
    void SetDefaultTextureColor();

    /// @brief 設定用於覆蓋紋理的顏色。
    void SetOverWriteTextureColor(DWORD color);

    /// @brief 獲取目前設定的覆蓋顏色。
    void GetOverWriteTextureColor(float* pOutColor) const;

    /// @brief 設定是否啟用顏色覆蓋繪製模式。
    void SetOverWriteTextureColorDraw(bool bEnable);

    /// @brief 檢查是否啟用顏色覆蓋繪製模式。
    bool IsOverWriteTextureColorDraw() const;

    // --- 方便的 Setters / Getters ---

    void SetBlockID(unsigned short wBlockID) { m_wBlockID = wBlockID; }
    void SetPosition(float x, float y) { m_fPosX = x; m_fPosY = y; }
    void SetScale(int nScale) { m_nScale = nScale; }
    void SetScaleXY(float sx, float sy) { m_fScaleX = sx; m_fScaleY = sy; }
    void SetRotation(int nRotation) { m_nRotation = nRotation; }
    void SetFlipX(bool bFlip) { m_bFlipX = bFlip; }
    void SetFlipY(bool bFlip) { m_bFlipY = bFlip; }
    void SetAlpha(unsigned int dwAlpha) { m_dwAlpha = dwAlpha; }
    void SetColor(unsigned int dwColor) { m_dwColor = dwColor; }
    ImageResourceListData* GetGIDataPtr() const { return m_pGIData; }

    /// @brief 檢查此 GameImage 物件是否正在被使用。
    bool IsInUse() const { return m_pGIData != nullptr; }

    VertexBufferData* m_pVBData;       // 位移+4: 頂點緩衝區的管理節點
    ImageResourceListData* m_pGIData;    // 位移+8: 圖片資源的管理節點

    GIVertex m_Vertices[8];              // 位移+12: 儲存8個頂點的資料

    D3DXVECTOR3 m_baseVertices[4];       // 位移+236: 用於變換的基礎頂點座標
    D3DXVECTOR3 m_transformedVertices[4]; // 位移+284: 經過變換後的頂點座標

    float m_fPosX;                       // 位移+332: 繪製的 X 座標
    float m_fPosY;                       // 位移+336: 繪製的 Y 座標
    float m_fDrawWidth;                  // 位移+340: 繪製寬度
    float m_fDrawHeight;                 // 位移+344: 繪製高度
    float m_fAngleX;                     // 位移+348: X軸傾斜
    float m_fAngleY;                     // 位移+352: Y軸傾斜

    float m_fHotspotX;                   // 位移+356
    float m_fHotspotY;                   // 位移+360
    float m_fHotspotWidth;               // 位移+364
    float m_fHotspotHeight;              // 位移+368

    unsigned short m_wBlockID;           // 位移+372: 動畫影格/區塊ID

    int m_nScale;                        // 位移+376: 整體縮放比例 (e.g., 100)
    unsigned int m_dwAlpha;              // 位移+380: 透明度 (0-255)
    unsigned int m_dwColor;              // 位移+384: 顏色覆蓋 (0-255)
    int m_nRotation;                     // 位移+388: 旋轉角度 (0-359)

    bool m_bFlipX;                       // 位移+392: 水平翻轉
    bool m_bFlipY;                       // 位移+396: 垂直翻轉

    unsigned int m_dwColorOp;            // 位移+400: 顏色操作模式
    float m_fCenterX;                    // 位移+404: 水平翻轉的中心點

    unsigned int m_dwGroupID;            // 位移+408: 資源群組ID
    unsigned int m_dwResourceID;         // 位移+412: 資源ID

    float m_fScaleX;                     // 位移+416: 獨立的 X 軸縮放
    float m_fScaleY;                     // 位移+420: 獨立的 Y 軸縮放

    bool m_bDrawPart1;                   // 位移+432: 是否繪製第一個三角扇
    bool m_bDrawPart2;                   // 位移+436: 是否繪製第二個三角扇

    bool m_bIsProcessed;                 // 位移+440: 標記頂點資料是否已處理

    bool m_bVertexAnimation;             // 位移+444: 頂點動畫旗標
    bool m_bFlag_445;                    // 位移+445
    bool m_bFlag_446;
    bool m_bFlag_447;
    bool m_bFlag_448;
    bool m_bFlag_449;
    bool m_bFlag_450;
    bool m_bFlag_451;

    float m_OverwriteColor[4];           // 位移+456: RGBA顏色，用於覆蓋紋理顏色
    bool  m_bUseOverwriteColor;          // 位移+472: 是否啟用顏色覆蓋繪製
};