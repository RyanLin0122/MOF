#pragma once

#include "Image.h"      // 為了 ImageVertex 結構
#include "GIVertex.h"   // 為了 GIVertex 結構
#include "Image/ImageResource.h"
#include "Image/VertexBufferDataMgr.h"
#include "Image/ImageResourceListDataMgr.h"

/// @class BackgroundImage
/// @brief 負責管理和繪製背景，可以是滾動的圖片，也可以是純黑色的矩形。
class BackgroundImage {
public:
    BackgroundImage();
    ~BackgroundImage();

    /// @brief 建立可滾動的圖片背景。
    /// @param szFilename 圖片檔案路徑。
    /// @param imgWidth 要在螢幕上顯示的圖片寬度。
    /// @param imgHeight 要在螢幕上顯示的圖片高度。
    /// @param texWidth 來源紋理的完整寬度。
    /// @param texHeight 來源紋理的完整高度。
    void CreateImage(const char* szFilename, float imgWidth, float imgHeight, float texWidth, float texHeight);

    /// @brief 建立純黑色的矩形背景。
    /// @param x 矩形左上角的 X 座標。
    /// @param y 矩形左上角的 Y 座標。
    /// @param width 矩形的寬度。
    /// @param height 矩形的高度。
    void CreateBlackBG(float x, float y, float width, float height);

    /// @brief 重設所有狀態並釋放資源。
    void Reset();

    /// @brief 設定圖片的繪製位置。
    void SetPosition(float x, float y);

    /// @brief 向上滾動背景。
    /// @param delta 向上滾動的距離。
    /// @return 如果滾動到頂部邊界，返回 true。
    bool SetPositionUP(float delta);

    /// @brief 向下滾動背景。
    /// @param delta 向下滾動的距離。
    /// @return 如果滾動到底部邊界，返回 true。
    bool SetPositionDOWN(float delta);

    /// @brief 更新頂點緩衝區的資料。
    void Process();

    /// @brief 繪製圖片背景。
    void Render();

    /// @brief 繪製純黑色背景。
    void RenderBlackBG();

//private:
    // 這兩個指標被 CreateImage 和 CreateBlackBG 共用
    VertexBufferData* m_pVBData;      // 位移 0:   頂點緩衝區
    TextureListData* m_pTexData;     // 位移 4:   紋理

    // 圖片模式下的屬性
    float m_fTexWidth;                  // 位移 8:   紋理完整寬度
    float m_fTexHeight;                 // 位移 12:  紋理完整高度
    float m_fImgWidth;                  // 位移 16:  顯示區域寬度
    float m_fImgHeight;                 // 位移 20:  顯示區域高度
    float m_fU_End;                     // 位移 24:  U 座標終點 (顯示區域寬/紋理寬)
    float m_fV_End;                     // 位移 28:  V 座標終點 (顯示區域高/紋理高)
    float m_fPosX;                      // 位移 32:  繪製位置 X
    float m_fPosY;                      // 位移 36:  繪製位置 Y
    float m_fU_Start;                   // 位移 40:  U 座標起點 (用於水平滾動)
    float m_fV_Start;                   // 位移 44:  V 座標起點 (用於垂直滾動)
    
    // 用於更新 GPU 的本地頂點快取
    ImageVertex m_imageVertices[4];     // 位移 48:  圖片模式的頂點資料
    char m_padding[48];                 // 位移 144: 未知的填充區域
    GIVertex m_giVertices[4];           // 位移 192: 黑色背景模式的頂點資料
};