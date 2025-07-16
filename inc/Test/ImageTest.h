#pragma once
#include <iostream>
#include <cassert>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>


// --- 前向宣告 ---
class ImageResource;

// --- 測試輔助宏 ---
#define RUN_TEST(test_func) \
    std::cout << "  [RUNNING] " << #test_func << "..." << std::endl; \
    test_func(); \
    std::cout << "  [PASSED]  " << #test_func << std::endl;

// --- Mock 物件 ---

/// @class MockD3DDevice
/// @brief 模擬 IDirect3DDevice9，用於追蹤 API 呼叫次數和參數，而不需真實的 GPU。
/// 注意：這是一個簡化版，僅實作測試所需的函式。
class MockD3DDevice : public IDirect3DDevice9 {
public:
    // 呼叫計數器 (維持不變)
    int SetFVF_count = 0;
    int SetStreamSource_count = 0;
    int SetTexture_count = 0;
    int SetRenderState_count = 0;
    int SetTextureStageState_count = 0;
    int CreateTexture_count = 0;
    int DrawPrimitiveUP_count = 0;

    // 儲存最後一次呼叫的參數 (維持不變)
    DWORD last_fvf = 0;
    IDirect3DVertexBuffer9* last_vertex_buffer = nullptr;
    UINT last_stride = 0;
    IDirect3DBaseTexture9* last_texture = nullptr;
    D3DRENDERSTATETYPE last_render_state_type = (D3DRENDERSTATETYPE)0;
    DWORD last_render_state_value = 0;

    // 控制模擬函式的行為 (維持不變)
    bool force_texture_creation_failure = false;

    // 重設狀態 (維持不變)
    void ResetCounters() {
        SetFVF_count = 0;
        SetStreamSource_count = 0;
        SetTexture_count = 0;
        SetRenderState_count = 0;
        SetTextureStageState_count = 0;
        CreateTexture_count = 0;
        DrawPrimitiveUP_count = 0;
        force_texture_creation_failure = false;
    }

    // --- 模擬 D3DX 函式 (維持不變) ---
    HRESULT Mock_D3DXCreateTextureFromFileInMemoryEx(UINT width, UINT height, D3DFORMAT format, IDirect3DTexture9** ppTexture) {
        CreateTexture_count++;
        if (force_texture_creation_failure) {
            *ppTexture = nullptr;
            return E_FAIL;
        }
        *ppTexture = reinterpret_cast<IDirect3DTexture9*>(new char[1]); // Fake object
        return S_OK;
    }

    /*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) { return E_NOINTERFACE; }
    ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    ULONG STDMETHODCALLTYPE Release() { return 1; }

    /*** IDirect3DDevice9 methods ***/
    // 以下是為了讓這個類別變成「具體類別」而新增的最小實作
    HRESULT STDMETHODCALLTYPE TestCooperativeLevel() { return S_OK; }
    UINT STDMETHODCALLTYPE GetAvailableTextureMem() { return 1024 * 1024 * 128; } // Return a large value
    HRESULT STDMETHODCALLTYPE EvictManagedResources() { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D9** ppD3D9) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS9* pCaps) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) { return S_OK; }
    void STDMETHODCALLTYPE SetCursorPosition(int X, int Y, DWORD Flags) {}
    BOOL STDMETHODCALLTYPE ShowCursor(BOOL bShow) { return TRUE; }
    HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) { return S_OK; }
    UINT STDMETHODCALLTYPE GetNumberOfSwapChains() { return 1; }
    HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) { return S_OK; }
    HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetDialogBoxMode(BOOL bEnableDialogs) { return S_OK; }
    void STDMETHODCALLTYPE SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {}
    void STDMETHODCALLTYPE GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {}
    HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) { return S_OK; }
    HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) { return S_OK; }
    HRESULT STDMETHODCALLTYPE StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) { return S_OK; }
    HRESULT STDMETHODCALLTYPE ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) { return S_OK; }
    HRESULT STDMETHODCALLTYPE BeginScene() { return S_OK; }
    HRESULT STDMETHODCALLTYPE EndScene() { return S_OK; }
    HRESULT STDMETHODCALLTYPE Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) { return S_OK; }
    HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT9* pViewport) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT9* pViewport) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL9* pMaterial) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT9* pLight) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetLight(DWORD Index, D3DLIGHT9* pLight) { return S_OK; }
    HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index, BOOL* pEnable) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index, CONST float* pPlane) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index, float* pPlane) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
        SetRenderState_count++;
        last_render_state_type = State;
        last_render_state_value = Value;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) { return S_OK; }
    HRESULT STDMETHODCALLTYPE BeginStateBlock() { return S_OK; }
    HRESULT STDMETHODCALLTYPE EndStateBlock(IDirect3DStateBlock9** ppSB) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS9* pClipStatus) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
        SetTexture_count++;
        last_texture = pTexture;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        SetTextureStageState_count++;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) { return S_OK; }
    HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT PaletteNumber) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT* PaletteNumber) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetScissorRect(CONST RECT* pRect) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetScissorRect(RECT* pRect) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetSoftwareVertexProcessing(BOOL bSoftware) { return S_OK; }
    BOOL STDMETHODCALLTYPE GetSoftwareVertexProcessing() { return FALSE; }
    HRESULT STDMETHODCALLTYPE SetNPatchMode(float nSegments) { return S_OK; }
    float STDMETHODCALLTYPE GetNPatchMode() { return 0.0f; }
    HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
        DrawPrimitiveUP_count++;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) { return S_OK; }
    HRESULT STDMETHODCALLTYPE ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF) {
        SetFVF_count++;
        last_fvf = FVF;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetFVF(DWORD* pFVF) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexShader(IDirect3DVertexShader9* pShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetVertexShader(IDirect3DVertexShader9** ppShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {
        SetStreamSource_count++;
        last_vertex_buffer = pStreamData;
        last_stride = Stride;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetStreamSourceFreq(UINT StreamNumber, UINT Setting) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer9* pIndexData) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer9** ppIndexData) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetPixelShader(IDirect3DPixelShader9* pShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPixelShader(IDirect3DPixelShader9** ppShader) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) { return S_OK; }
    HRESULT STDMETHODCALLTYPE GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) { return NULL; }
    HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) { return S_OK; }
    HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) { return S_OK; }
    HRESULT STDMETHODCALLTYPE DeletePatch(UINT Handle) { return S_OK; }
    HRESULT STDMETHODCALLTYPE CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) { return S_OK; }
};


/// @class MockResourceMgr
/// @brief 模擬 ResourceMgr，用於測試 LoadingThread 是否正確呼叫其介面。
class MockResourceMgr {
public:
    struct CallInfo {
        unsigned int groupID;
        unsigned int resourceID;
        char priority;
    };

    std::vector<CallInfo> call_log;
    std::mutex log_mutex;

    static MockResourceMgr* GetInstance() {
        static MockResourceMgr instance;
        return &instance;
    }

    void LoadingImageResource(unsigned int groupID, unsigned int resourceID, int a3, char priority) {
        std::lock_guard<std::mutex> lock(log_mutex);
        call_log.push_back({ groupID, resourceID, priority });
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(log_mutex);
        call_log.clear();
    }
};

/// @class ImageSystemTester
/// @brief 整合所有測試案例的執行器類別。
class ImageSystemTester {
public:
    ImageSystemTester();
    ~ImageSystemTester();

    void RunImageTests();
    void RunUnitTests();
    void RunIntegrationTests();

private:
    void Setup();
    void Teardown();

    // --- 單元測試 ---
    void Test_GIVertex_Constructor();
    void Test_CDeviceManager_Singleton();
    void Test_CDeviceManager_StateCaching();
    void Test_CDeviceManager_ResetRenderState();
    void Test_ImageResource_LoadGI_Success();
    void Test_ImageResource_LoadGI_FileNotFound();
    void Test_ImageResource_LoadGIInPack_Success();
    void Test_ImageResource_LoadTexture_Success();
    void Test_ImageResource_LoadTexture_Failure();
    void Test_LoadingThread_QueueOperations();
    void Test_LoadingThread_ThreadProcessing();

    // --- 整合測試 ---
    void Test_Integration_AsyncLoadAndVerify();

    // --- 測試輔助函式 ---
    void CreateDummyGIFile(const std::string& filename, bool compressed, bool with_anim);

private:
    MockD3DDevice* m_mockDevice;
    // 全域的 Device 指標需要指向我們的 Mock 物件
    IDirect3DDevice9* m_originalDevicePtr;
};