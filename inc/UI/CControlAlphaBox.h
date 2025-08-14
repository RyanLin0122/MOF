#ifndef CCONTROLALPHABOX_H
#define CCONTROLALPHABOX_H

#include "CControlBase.h"

// 由於我們沒有 DirectX 的完整標頭檔，在此模擬定義
// Forward declaration for Direct3D vertex buffer wrapper.
struct VertexBufferData; 

/**
 * @struct D3DVertex
 * @brief 描述一個帶有顏色資訊的 2D 頂點結構，用於 DirectX 繪製。
 * * FVF (Flexible Vertex Format) 應為 D3DFVF_XYZRHW | D3DFVF_DIFFUSE
 */
struct D3DVertex 
{
    float x, y, z, rhw; // 頂點的螢幕座標 (rhw 通常為 1.0)
    unsigned long color;    // 頂點的顏色 (AARRGGBB 格式)
};

/**
 * @class CControlAlphaBox
 * @brief 一個可繪製帶有顏色和透明度的矩形方塊的 UI 控制項。
 *
 * 繼承自 CControlBase，主要用於 UI 的背景、邊框或簡單的幾何圖形顯示。
 */
class CControlAlphaBox : public CControlBase
{
public:
    // 建構函式與解構函式
    CControlAlphaBox();
    virtual ~CControlAlphaBox();

    // --- 覆寫的虛擬函式 ---
    // 創建控制項並初始化 DirectX 資源
    virtual void Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, CControlBase* pParent);
    virtual void Create(CControlBase* pParent);
    void Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, float r, float g, float b, float a, CControlBase* pParent);

    // 準備繪製，計算頂點座標並更新到緩衝區
    virtual void PrepareDrawing() override;
    // 執行繪製命令
    virtual void Draw() override;
    
    // --- 顏色與透明度設定 ---
    // 設定方塊四個頂點的顏色
    void SetColor(float r, float g, float b, float a);
    // 分別設定方塊四個頂點的顏色，以實現漸層效果
    void SetColor(float r1, float g1, float b1, float a1, 
                  float r2, float g2, float b2, float a2,
                  float r3, float g3, float b3, float a3,
                  float r4, float g4, float b4, float a4);
    
    // 設定透明度
    void SetAlpha(unsigned char alpha);

    // --- 其他公用函式 ---
    // 設定方塊的屬性
    void SetAttr(int nPosX, int nPosY, short usWidth, short usHeight, float r, float g, float b, float a);
    // 讓方塊的大小和位置與父物件相同
    void SetRectInParent();

private:
    VertexBufferData* m_pVertexBuffer; // 指向 DirectX Vertex Buffer 的指標
    D3DVertex m_vertices[4];           // 儲存方塊四個頂點的資訊
    bool m_bIsCreated;                 // 標記 Create 函式是否已被呼叫
};

#endif // CCONTROLALPHABOX_H
