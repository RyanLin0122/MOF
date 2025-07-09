#include "Image/GIVertex.h"

GIVertex::GIVertex() {
    position_x = 0.0f;
    position_y = 0.0f;         
    position_z_or_rhw = 0.5f;  
    rhw_value = 1.0f;          
    diffuse_color = 0xFFFFFFFF;
    texture_u = 0.0f;          
    texture_v = 0.0f;          
}

// 由於 FVF 通常是外部定義的常數，我們假設它是一個全域變數或靜態成員
// 根據 CDeviceResetManager.c，GIVertex::FVF 是一個 const unsigned int
// 這裡僅為模擬，實際應用中會引用 Direct3D FVF 的定義
// const unsigned int GIVertex::FVF = 0; // 實際值應為 D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 等組合