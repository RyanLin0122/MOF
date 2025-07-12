#pragma once

#include <d3d9.h>   // 包含 Direct3D 9 相關的定義，例如 IDirect3DDevice9、D3DRENDERSTATETYPE 等
#include <vector>   // 可能需要 vector 來模擬動態的渲染狀態快取，儘管原始是固定大小
#include <cstring>  // 包含 memset 所需的頭文件

// 前向宣告 Direct3D 裝置全域變數，因為它在 CDeviceManager 中被廣泛使用
// 原始反編譯程式碼中 Device 是一個 extern LPDIRECT3DDEVICE9 變數
extern LPDIRECT3DDEVICE9 Device;

// CDeviceManager 類別
// 負責管理 Direct3D 裝置的狀態，避免重複設定相同的狀態，提高渲染效率。
class CDeviceManager {
public:
    // 當前設定的 Flexible Vertex Format (FVF)
    // 偏移: 0
    unsigned int current_fvf;

    // 當前設定的頂點緩衝區指標
    // 偏移: 4
    IDirect3DVertexBuffer9* current_vertex_buffer;

    // 當前設定的頂點緩衝區的步長 (stride)
    // 偏移: 8
    unsigned int current_vertex_stride;

    // 當前設定到紋理 Stage 0 的紋理指標
    // 偏移: 12 (3 * sizeof(DWORD))
    IDirect3DBaseTexture9* current_texture_stage0; 
    // 注意：原始碼只顯示了 stage 0，如果有多個紋理 stage，可能需要一個陣列

    // 用於快取 Direct3D 渲染狀態的陣列
    // 偏移: 16 (4 * sizeof(DWORD))
    // 原始程式碼中使用 memset 將 0x348 (840) 位元組設定為 0xFF，
    // 這表示它是一個大小為 840 / 4 = 210 個 DWORD 的陣列，
    // 對應 D3DRENDERSTATETYPE 枚舉中的索引。
    unsigned int render_state_cache[210]; // D3DRENDERSTATETYPE 的最大值 D3DRS_BLENDOPALPHA 是 209 (截至 DX9)

public:
    static CDeviceManager* GetInstance();
    // 設定 Flexible Vertex Format (FVF)
    // 如果 FVF 與當前設定的相同，則不執行任何操作以優化性能。
    // fvf_value: 要設定的 FVF 值
    HRESULT SetFVF(unsigned int fvf_value);

    // 設定頂點串流來源
    // 如果頂點緩衝區和步長與當前設定的相同，則不執行任何操作。
    // stream_number: Stream Number (通常為 0)
    // vertex_buffer: 要設定的頂點緩衝區指標
    // offset_in_bytes: OffsetInBytes (通常為 0)
    // stride: Stride (頂點結構的大小)
    HRESULT SetStreamSource(unsigned int stream_number, IDirect3DVertexBuffer9* vertex_buffer, unsigned int offset_in_bytes, unsigned int stride);

    // 設定紋理
    // 如果紋理與當前設定的相同，則不執行任何操作。
    // stage: Stage 編號 (通常為 0)
    // texture: 要設定的紋理指標
    HRESULT SetTexture(unsigned int stage, IDirect3DBaseTexture9* texture);

    // 設定渲染狀態
    // 如果渲染狀態值與當前快取中的相同，則不執行任何操作。
    // state: 要設定的渲染狀態類型 (D3DRENDERSTATETYPE)
    // value: 要設定的渲染狀態值
    HRESULT SetRenderState(D3DRENDERSTATETYPE state, unsigned int value);

    // 重設常見的渲染狀態為預設值
    // 通常在裝置重置後或需要確保特定渲染行為時呼叫。
    HRESULT ResetRenderState();

    // 設定紋理 Stage 狀態
    // stage: Stage 編號
    // type: 要設定的紋理 Stage 狀態類型 (D3DTEXTURESTAGESTATETYPE)
    // value: 要設定的紋理 Stage 狀態值
    HRESULT SetTextureStageState(unsigned int stage, D3DTEXTURESTAGESTATETYPE type, unsigned int value);

private:
    CDeviceManager();
    static CDeviceManager* s_pInstance;
};