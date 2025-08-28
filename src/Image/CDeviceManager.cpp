#include "Image/CDeviceManager.h" // 包含我們定義的標頭檔

// 初始化靜態成員指標
CDeviceManager* CDeviceManager::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
CDeviceManager* CDeviceManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CDeviceManager();
    }
    return s_pInstance;
}

// CDeviceManager 的建構函式
CDeviceManager::CDeviceManager() {
    // 初始化 current_fvf 為 -1 (0xFFFFFFFF)，表示沒有設定任何 FVF
    current_fvf = -1;
    // 初始化 current_vertex_buffer 為 nullptr，表示沒有設定任何頂點緩衝區
    current_vertex_buffer = nullptr;
    // 初始化 current_vertex_stride 為 -1 (0xFFFFFFFF)，表示沒有設定任何步長
    current_vertex_stride = -1;
    // 初始化用於快取渲染狀態的陣列為 0xFFFFFFFF
    // 0x348u (840) 位元組，每個 unsigned int 佔 4 位元組，所以是 210 個 unsigned int
    memset(render_state_cache, 0xFFu, 0x348u);
    // 注意：原始碼中 current_texture_stage0 (偏移 12) 沒有在這裡直接初始化
    // 但在 SetTexture 函式中，它會被檢查。
    // 通常也應初始化為 nullptr。
    current_texture_stage0 = nullptr;
}

// 設定 Flexible Vertex Format (FVF)
// 如果請求的 FVF 與當前已設定的 FVF 相同，則直接返回，避免不必要的 API 呼叫。
// 這是一種性能優化。
HRESULT CDeviceManager::SetFVF(unsigned int fvf_value) {
    // 檢查是否與當前 FVF 相同
    if (current_fvf == fvf_value)
        return S_OK; // 相同則返回成功

    // 更新快取中的 FVF 值
    current_fvf = fvf_value;
    // 呼叫 Direct3D 裝置的 SetFVF 方法來設定 FVF
    return Device->SetFVF(fvf_value);
}

// 設定頂點串流來源
// 檢查頂點緩衝區和步長是否與當前已設定的相同，如果相同則直接返回。
// stream_number 參數在原始碼中似乎總是 0，所以此處只使用一個硬編碼值。
HRESULT CDeviceManager::SetStreamSource(unsigned int stream_number, IDirect3DVertexBuffer9* vertex_buffer, unsigned int offset_in_bytes, unsigned int stride) {
    // 檢查頂點緩衝區和步長是否與快取中的相同
    if (current_vertex_buffer == vertex_buffer && current_vertex_stride == stride)
        return S_OK; // 相同則返回成功

    // 更新快取中的頂點緩衝區和步長值
    current_vertex_buffer = vertex_buffer;
    current_vertex_stride = stride;
    // 呼叫 Direct3D 裝置的 SetStreamSource 方法
    // 原始碼中 stream_number 和 offset_in_bytes 都為 0
    return Device->SetStreamSource(0, vertex_buffer, 0, stride);
}

// 設定紋理
// 檢查紋理是否與當前已設定的相同 (針對 Stage 0)，如果相同則直接返回。
// stage 參數在原始碼中似乎總是 0。
HRESULT CDeviceManager::SetTexture(unsigned int stage, IDirect3DBaseTexture9* texture) {
    // 檢查紋理 Stage 0 是否與快取中的相同
    if (current_texture_stage0 == texture)
        return S_OK; // 相同則返回成功

    // 更新快取中的紋理 Stage 0 值
    current_texture_stage0 = texture;
    // 呼叫 Direct3D 裝置的 SetTexture 方法
    // 原始碼中 stage 為 0
    return Device->SetTexture(0, texture);
}

// 設定渲染狀態
// 檢查請求的渲染狀態值是否與快取中的相同，如果相同則直接返回。
HRESULT CDeviceManager::SetRenderState(D3DRENDERSTATETYPE state, unsigned int value) {
    // 檢查渲染狀態是否與快取中的相同
    // state 是一個 D3DRENDERSTATETYPE 枚舉值，直接用作陣列索引
    // 原始程式碼的偏移量是 (state + 4) * sizeof(DWORD)，因為前四個 DWORD 用於其他成員
    if (render_state_cache[state] == value)
        return S_OK; // 相同則返回成功

    // 更新快取中的渲染狀態值
    render_state_cache[state] = value;
    // 呼叫 Direct3D 裝置的 SetRenderState 方法
    return Device->SetRenderState(state, value);
}

// 重設常見的渲染狀態為預設值
// 該函式呼叫 SetRenderState 多次，以設定 Direct3D 裝置的渲染狀態。
// 這些設定通常是 Direct3D 渲染管線的常見預設或優化值。
HRESULT CDeviceManager::ResetRenderState() {
    // 設定混合操作為 D3DBLENDOP_ADD (加法混合)
    SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    // 設定源混合因子為 D3DBLEND_SRCALPHA (使用源顏色的 Alpha 分量)
    SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    // 設定目標混合因子為 D3DBLEND_INVSRCALPHA (使用 1 - 源顏色的 Alpha 分量)
    return SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

// 設定紋理 Stage 狀態
// 該函式直接呼叫 Direct3D 裝置的 SetTextureStageState。
// 原始程式碼中沒有對這些狀態進行快取，因此每次呼叫都會轉發到 Direct3D API。
HRESULT CDeviceManager::SetTextureStageState(unsigned int stage, D3DTEXTURESTAGESTATETYPE type, unsigned int value) {
    // 直接呼叫 Direct3D 裝置的 SetTextureStageState 方法
    return Device->SetTextureStageState(stage, type, value);
}

HRESULT CDeviceManager::SetSamplerState(unsigned int stage, D3DSAMPLERSTATETYPE type, unsigned int value) {
    return Device->SetSamplerState(stage, type, value);
}