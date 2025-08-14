#pragma once
#include "CControlBase.h"
#include <d3d9.h>

// 前向宣告：由裝置重設管理器建立/刪除的 VB 包裝
struct VertexBufferData;

/**
 * @brief 2D 頂點（螢幕空間）+ Diffuse 顏色
 * FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE (0x0044)
 */
struct D3DVertex
{
    float x;
    float y;
    float z;
    float rhw;
    DWORD diffuse;
};

class CControlAlphaBox : public CControlBase
{
public:
    CControlAlphaBox();
    virtual ~CControlAlphaBox();

    // Create 多載（與基準對齊）
    void Create(int x, int y, unsigned short w, unsigned short h, CControlBase* pParent);
    void Create(int x, int y, unsigned short w, unsigned short h,
        float r, float g, float b, float a, CControlBase* pParent);
    void Create(CControlBase* pParent);

    // 生命週期：準備繪製 / 實際繪製
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

    // 顏色（單一顏色或四角獨立顏色）
    void SetColor(float r, float g, float b, float a);
    void SetColor(float r1, float g1, float b1, float a1,
        float r2, float g2, float b2, float a2,
        float r3, float g3, float b3, float a3,
        float r4, float g4, float b4, float a4);

    // 設定透明度（0~255）
    void SetAlpha(unsigned char a);

    // 其他工具
    void SetAttr(int x, int y, unsigned short w, unsigned short h,
        float r, float g, float b, float a);
    void SetRectInParent();

private:
    static constexpr DWORD kFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

    // 將 float(0~1) 轉 packed ARGB
    static DWORD PackColor(float r, float g, float b, float a);
    static unsigned char ToByte(float v);

    // 依目前絕對座標與尺寸，更新四顆頂點（含 half-pixel 對齊）
    void UpdateVerticesFromRect();

    VertexBufferData* m_pVBData = nullptr; // ResetManager 建立/刪除
    D3DVertex         m_vtx[4] = {};      // 左上、右上、右下、左下（TriangleFan）
    bool              m_bCreated = false;  // 防重複 Create
};