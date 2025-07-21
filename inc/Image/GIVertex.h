#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// 定義 GIVertex 類別
// 該類別用於表示圖形渲染中的一個頂點，包含位置、顏色和紋理座標等資訊。
class GIVertex {
public:
    // 頂點位置的 X 座標
    float position_x; // 從 GIVertex 構造函數中，*(_DWORD *)this 被初始化為 0
    // 頂點位置的 Y 座標
    float position_y; // 從 GIVertex 構造函數中，*((_DWORD *)this + 1) 被初始化為 0
    // 頂點位置的 Z 座標，或在 2D 渲染中作為 RHW (Reciprocal of Homogeneous W) 使用
    // 初始化值 1056964608 (0x3F000000) 對應浮點數 0.5f
    float position_z_or_rhw; 
    // 齊次座標的 W 分量倒數 (Reciprocal of Homogeneous W)。
    // 在 2D 渲染中，這通常是 1.0f，用於將頂點從齊次裁剪空間轉換到螢幕空間。
    // 初始化值 1065353216 (0x3F800000) 對應浮點數 1.0f
    float rhw_value;

    // 頂點顏色，通常是 ARGB (Alpha, Red, Green, Blue) 格式
    // 初始化值 -1 (0xFFFFFFFF) 代表完全不透明的白色
    unsigned int diffuse_color; 

    // 紋理座標 U (水平方向)
    // 從 GIVertex 構造函數中，*((_DWORD *)this + 5) 被初始化為 0
    float texture_u; 
    // 紋理座標 V (垂直方向)
    // 從 GIVertex 構造函數中，*((_DWORD *)this + 6) 被初始化為 0
    float texture_v; 

public:
    // 建構函式
    // 初始化頂點的各個屬性
    GIVertex();
    static const unsigned int FVF = D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW;
};