#include "Test/ImageTest.h"
#include "Image/GIVertex.h"
#include "Image/CDeviceManager.h"
#include "Image/ImageResource.h"
#include "Image/LoadingThread.h"
#include "Image/ResourceMgr.h"
#include "Image/BackgroundImage.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "CMOFPacking.h" // For integration test
#include <fstream>
#include <windows.h> // For Sleep

extern LPDIRECT3DDEVICE9 Device;

// 替換 ImageResource.cpp 中的 D3DXCreateTextureFromFileInMemoryEx
// 技巧：我們在這裡定義它，連結器會優先使用這個版本而不是 D3DX 函式庫中的版本。
// 這需要在編譯時特別設定，或者確保這個 obj 檔優先被連結。
// 一個更安全的作法是使用 function hooking 函式庫(如 MinHook)或修改原始碼來注入依賴。
// 為求簡單，我們先假設可以這樣覆蓋。
HRESULT D3DXCreateTextureFromFileInMemoryEx(
    LPDIRECT3DDEVICE9 pDevice,
    LPCVOID pSrcData,
    UINT SrcDataSize,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    DWORD Filter,
    DWORD MipFilter,
    D3DCOLOR ColorKey,
    D3DXIMAGE_INFO* pSrcInfo,
    PALETTEENTRY* pPalette,
    LPDIRECT3DTEXTURE9* ppTexture)
{
    // 將呼叫轉發給我們的 MockDevice
    MockD3DDevice* mockDevice = static_cast<MockD3DDevice*>(pDevice);
    return mockDevice->Mock_D3DXCreateTextureFromFileInMemoryEx(Width, Height, Format, ppTexture);
}

// 替換 LoadingThread.cpp 中的 ResourceMgr::GetInstance()->LoadingImageResource
// 這是比較棘手的部分，因為原始碼直接寫死了。理想情況下，原始碼應注入 ResourceMgr 的依賴。
// 在這裡，我們將在整合測試中提供一個特殊的 Mock ResourceMgr 來驗證行為。
// 對於單元測試，我們則假設可以替換其行為。

ImageSystemTester::ImageSystemTester() {
    m_mockDevice = new MockD3DDevice();
    // 備份原始的 Device 指標 (雖然在 main 中是 nullptr)
    m_originalDevicePtr = Device;
    // 將全域 Device 指標指向我們的 Mock 物件
    Device = m_mockDevice;
}

ImageSystemTester::~ImageSystemTester() {
    // 還原全域 Device 指標
    Device = m_originalDevicePtr;
    delete m_mockDevice;
    m_mockDevice = nullptr;
}

void ImageSystemTester::RunImageTests() {
    std::cout << "===== Running All Image System Tests =====" << std::endl;
    RunUnitTests();
    RunIntegrationTests();
    std::cout << "===== All Tests Completed =====" << std::endl;
}

void ImageSystemTester::Setup() {
    m_mockDevice->ResetCounters();
    // 清理可能存在的單例物件狀態
    if (CDeviceManager::GetInstance()) {
        // (無法直接刪除，因為建構函式是私有的。測試框架通常有 friend class 機制)
        // 這裡我們假設可以重設其內部狀態
    }
    MockResourceMgr::GetInstance()->Reset();
}

void ImageSystemTester::Teardown() {
    // 可以在此處刪除測試中建立的檔案
    remove("test_uncompressed.gi");
    remove("test_compressed.gi");
    remove("test_anim.gi");
}


// ----------------------------------------
// --- 單元測試 (UNIT TESTS) ---
// ----------------------------------------
void ImageSystemTester::RunUnitTests() {
    std::cout << "\n--- Running Unit Tests ---" << std::endl;

    // 原有的測試
    Setup(); RUN_TEST(Test_GIVertex_Constructor); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_Singleton); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_StateCaching); Teardown();
    Setup(); RUN_TEST(Test_CDeviceManager_ResetRenderState); Teardown();
    Setup(); RUN_TEST(Test_ImageResource_LoadGI_Success); Teardown();
    Setup(); RUN_TEST(Test_ImageResource_LoadGI_FileNotFound); Teardown();
    Setup(); RUN_TEST(Test_LoadingThread_QueueOperations); Teardown();

    Setup(); RUN_TEST(Test_cltImageManager_PoolExhaustionAndReuse); Teardown();
    Setup(); RUN_TEST(Test_GameImage_Transformation); Teardown();
    Setup(); RUN_TEST(Test_GameImage_ColorAndAlpha); Teardown();
    Setup(); RUN_TEST(Test_BackgroundImage_Scrolling); Teardown();
    // 注意: 引用計數測試需要一個真實的資源，因此放在整合測試部分


    std::cout << "--- Unit Tests Finished ---" << std::endl;
}

void ImageSystemTester::Test_GIVertex_Constructor() {
    GIVertex v;
    assert(v.position_x == 0.0f);
    assert(v.position_y == 0.0f);
    assert(v.position_z_or_rhw == 0.5f);
    assert(v.rhw_value == 1.0f);
    assert(v.diffuse_color == 0xFFFFFFFF);
    assert(v.texture_u == 0.0f);
    assert(v.texture_v == 0.0f);
}

void ImageSystemTester::Test_CDeviceManager_Singleton() {
    CDeviceManager* p1 = CDeviceManager::GetInstance();
    CDeviceManager* p2 = CDeviceManager::GetInstance();
    assert(p1 != nullptr);
    assert(p1 == p2);
}

void ImageSystemTester::Test_CDeviceManager_StateCaching() {
    CDeviceManager* dm = CDeviceManager::GetInstance();

    // 測試 FVF
    dm->SetFVF(D3DFVF_XYZ);
    assert(m_mockDevice->SetFVF_count == 1);
    dm->SetFVF(D3DFVF_XYZ); // 重複呼叫
    assert(m_mockDevice->SetFVF_count == 1); // 計數器不應增加
    dm->SetFVF(D3DFVF_NORMAL);
    assert(m_mockDevice->SetFVF_count == 2); // 計數器應增加

    // 測試 Texture
    IDirect3DBaseTexture9* tex1 = reinterpret_cast<IDirect3DBaseTexture9*>(new char[1]);
    IDirect3DBaseTexture9* tex2 = reinterpret_cast<IDirect3DBaseTexture9*>(new char[1]);
    dm->SetTexture(0, tex1);
    assert(m_mockDevice->SetTexture_count == 1);
    dm->SetTexture(0, tex1);
    assert(m_mockDevice->SetTexture_count == 1);
    dm->SetTexture(0, tex2);
    assert(m_mockDevice->SetTexture_count == 2);
    delete[] tex1;
    delete[] tex2;

    // 測試 Render State
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    assert(m_mockDevice->SetRenderState_count == 1);
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    assert(m_mockDevice->SetRenderState_count == 1);
    dm->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    assert(m_mockDevice->SetRenderState_count == 2);
    assert(m_mockDevice->last_render_state_value == FALSE);

    // 測試無快取的 SetTextureStageState
    dm->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    dm->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    assert(m_mockDevice->SetTextureStageState_count == 2);
}

void ImageSystemTester::Test_CDeviceManager_ResetRenderState() {
    CDeviceManager* dm = CDeviceManager::GetInstance();
    m_mockDevice->ResetCounters();

    dm->ResetRenderState();
    // 應呼叫 3 次 SetRenderState
    assert(m_mockDevice->SetRenderState_count == 3);
}

void ImageSystemTester::Test_ImageResource_LoadGI_Success() {
    CreateDummyGIFile("test_uncompressed.gi", false, true);
    ImageResource res;
    bool success = res.LoadGI("test_uncompressed.gi", 0);

    assert(success == true);
    assert(res.m_version == 10);
    assert(res.m_width == 64);
    assert(res.m_height == 32);
    assert(res.m_d3dFormat == D3DFMT_A8R8G8B8);
    assert(res.m_animationFrameCount == 2);
    assert(res.m_pAnimationFrames != nullptr);
    assert(res.m_pImageData != nullptr);
    assert(res.m_imageDataSize == 64 * 32 * 4);

    // 檢查動畫影格資料是否正確讀取
    assert(res.m_pAnimationFrames[1].width == 123);
}

void ImageSystemTester::Test_ImageResource_LoadGI_FileNotFound() {
    ImageResource res;
    bool success = res.LoadGI("non_existent_file.gi", 0);
    assert(success == false);
}

void ImageSystemTester::Test_ImageResource_LoadTexture_Success() {
    CreateDummyGIFile("test_uncompressed.gi", false, false);
    ImageResource res;
    res.LoadGI("test_uncompressed.gi", 0);

    m_mockDevice->force_texture_creation_failure = false;
    bool success = res.LoadTexture();

    assert(success == true);
    assert(m_mockDevice->CreateTexture_count == 1);
    assert(res.m_pTexture != nullptr);
    assert(res.m_pImageData == nullptr); // 記憶體應被釋放

    delete[] res.m_pTexture; // 清理 fake texture
}

void ImageSystemTester::Test_ImageResource_LoadTexture_Failure() {
    CreateDummyGIFile("test_uncompressed.gi", false, false);
    ImageResource res;
    res.LoadGI("test_uncompressed.gi", 0);

    m_mockDevice->force_texture_creation_failure = true; // 強制失敗
    bool success = res.LoadTexture();

    assert(success == false);
    assert(m_mockDevice->CreateTexture_count == 1);
    assert(res.m_pTexture == nullptr);
    assert(res.m_pImageData == nullptr); // 記憶體無論成功失敗都應釋放
}

void ImageSystemTester::Test_LoadingThread_QueueOperations() {
    LoadingThread lt;
    tResInfo res1 = { 101, 1, 0 };
    tResInfo res2 = { 102, 1, 0 };

    assert(lt.FindInResLoadingList(101) == false);

    lt.AddBackGroundLoadingRes(res1);
    lt.AddBackGroundLoadingRes(res2);
    assert(lt.FindInResLoadingList(101) == true);
    assert(lt.FindInResLoadingList(102) == true);

    lt.DelResInLoadingList(101);
    assert(lt.FindInResLoadingList(101) == false);
    assert(lt.FindInResLoadingList(102) == true);

    lt.ClearLodingList();
    assert(lt.FindInResLoadingList(102) == false);
}

void ImageSystemTester::Test_LoadingThread_ThreadProcessing() {
    LoadingThread lt;

    lt.AddBackGroundLoadingRes({184550388, 2, 1});
    lt.AddBackGroundLoadingRes({251658410, 2, 2});

    lt.Poll(); // 啟動執行緒

    // --- 可靠的等待 ---
    // 檢查執行緒控制代碼是否有效
    if (lt.m_hThread) {
        // 等待，直到由 m_hThread 所代表的執行緒結束。
        // INFINITE 表示無限期等待，直到它完成為止。
        WaitForSingleObject(lt.m_hThread, INFINITE);

        // 既然執行緒結束了，最好將其控制代碼關閉
        CloseHandle(lt.m_hThread);
        lt.m_hThread = NULL; // 避免重複關閉
    }

    // 現在我們可以 100% 確定執行緒已經執行完畢
    assert(lt.m_bIsRunning == false);
}

void ImageSystemTester::Test_cltImageManager_PoolExhaustionAndReuse() {
    // 這個測試驗證物件池的兩個核心功能：耗盡和回收再利用
    bool vfs_opened = CMofPacking::GetInstance()->PackFileOpen("mof");
    assert(vfs_opened == true); // 確保 .pak 檔案本身是存在的
    cltImageManager* mgr = cltImageManager::GetInstance();
    mgr->Initialize();

    // --- FIX START: 使用一個我們在主程式中已驗證過、確定存在的資源ID ---
    const unsigned int VALID_RESOURCE_ID = 201326853;
    const ResourceMgr::eResourceType VALID_RESOURCE_TYPE = ResourceMgr::RES_ITEM;
    // --- FIX END ---


    // 1. 測試物件池耗盡
    std::vector<GameImage*> acquired_images;
    acquired_images.reserve(cltImageManager::MAX_IMAGES);

    for (int i = 0; i < cltImageManager::MAX_IMAGES; ++i) {
        GameImage* img = mgr->GetGameImage(VALID_RESOURCE_TYPE, VALID_RESOURCE_ID);
        assert(img != nullptr); // 斷言：物件池本身返回了一個有效的物件指標

        // --- FIX START: 增加一個更嚴格的斷言，確保資源也成功綁定 ---
        // 這使得測試更加穩健，能抵抗因資源不存在而導致的假性成功。
        assert(img->IsInUse() == true);
        // --- FIX END ---

        acquired_images.push_back(img);
    }

    // 現在，物件池應該已經被 5000 個「真正使用中」的物件完全填滿。
    // 第 5001 次獲取，理應失敗並返回 nullptr。
    GameImage* extra_img = mgr->GetGameImage(VALID_RESOURCE_TYPE, VALID_RESOURCE_ID);
    assert(extra_img == nullptr); // 這個斷言現在應該會通過了

    // 2. 測試回收與再利用
    GameImage* first_img = acquired_images[0];
    mgr->ReleaseGameImage(first_img);

    GameImage* reused_img = mgr->GetGameImage(ResourceMgr::RES_UI, 201327578); // 用一個不同的ID，避免快取
    assert(reused_img != nullptr);
    assert(reused_img == first_img);

    // 清理
    mgr->ReleaseAllGameImage();

    delete mgr;
    mgr = nullptr;
}

void ImageSystemTester::Test_GameImage_Transformation() {
    // 這個測試驗證 GameImage 的縮放、旋轉、翻轉等核心變換

    // --- FIX: 在堆積上建立 mgr 物件 ---
    cltImageManager* mgr = cltImageManager::GetInstance();
    mgr->Initialize();

    // 為了測試，我們需要一個確定存在的資源
    const unsigned int VALID_RESOURCE_ID = 201326853;
    const ResourceMgr::eResourceType VALID_RESOURCE_TYPE = ResourceMgr::RES_ITEM;
    CMofPacking::GetInstance()->PackFileOpen("mof"); // 確保VFS開啟

    GameImage* img = mgr->GetGameImage(VALID_RESOURCE_TYPE, VALID_RESOURCE_ID);
    assert(img != nullptr && img->IsInUse());

    img->SetBlockID(0);

    // 1. 測試翻轉 (檢查 UV 座標)
    img->Process(); // 先處理一次，獲取原始UV
    float u1_orig = img->m_Vertices[0].texture_u;
    float u2_orig = img->m_Vertices[1].texture_u;

    img->SetFlipX(true);
    img->Process();
    assert(img->m_Vertices[0].texture_u == u2_orig); // UV 座標應被交換
    assert(img->m_Vertices[1].texture_u == u1_orig);

    img->SetFlipX(false); // 恢復

    // 2. 測試縮放 (檢查頂點座標)
    img->Process();
    float width_orig = img->m_Vertices[1].position_x - img->m_Vertices[0].position_x;

    img->SetScaleXY(2.0f, 1.0f); // X 軸放大為 200%
    img->Process();
    float width_scaled = img->m_Vertices[1].position_x - img->m_Vertices[0].position_x;
    assert(abs(width_scaled - (width_orig * 2.0f)) < 0.001f);

    // 3. 測試旋轉 (檢查頂點座標)
    img->SetScaleXY(1.0f, 1.0f); // 恢復縮放
    img->SetRotation(90);
    img->Process();
    float rotated_width = img->m_Vertices[1].position_x - img->m_Vertices[0].position_x;
    float rotated_height = img->m_Vertices[1].position_y - img->m_Vertices[0].position_y;
    assert(abs(rotated_width) < abs(rotated_height));

    // --- 清理 ---
    mgr->ReleaseGameImage(img);
    CMofPacking::GetInstance()->PackFileClose(); // 關閉VFS
    delete mgr; // 釋放 mgr 物件
}

void ImageSystemTester::Test_GameImage_ColorAndAlpha() {
    // --- FIX: 在堆積上建立 mgr 物件 ---
    cltImageManager* mgr = cltImageManager::GetInstance();
    mgr->Initialize();

    const unsigned int VALID_RESOURCE_ID = 201326853;
    const ResourceMgr::eResourceType VALID_RESOURCE_TYPE = ResourceMgr::RES_ITEM;
    CMofPacking::GetInstance()->PackFileOpen("mof");

    GameImage* img = mgr->GetGameImage(VALID_RESOURCE_TYPE, VALID_RESOURCE_ID);
    assert(img != nullptr && img->IsInUse());

    // 1. 測試 Alpha
    img->SetAlpha(128);
    img->Process();
    // 檢查頂點顏色中的 Alpha 分量
    assert(((img->m_Vertices[0].diffuse_color >> 24) & 0xFF) == 128);

    // 2. 測試 Color (灰階)
    img->SetAlpha(255);
    img->SetColor(100);
    img->Process();
    DWORD color = img->m_Vertices[0].diffuse_color;
    assert(((color >> 24) & 0xFF) == 255); // A
    assert(((color >> 16) & 0xFF) == 100); // R
    assert(((color >> 8) & 0xFF) == 100);  // G
    assert((color & 0xFF) == 100);         // B

    // --- 清理 ---
    mgr->ReleaseGameImage(img);
    CMofPacking::GetInstance()->PackFileClose();
    delete mgr;
}

void ImageSystemTester::Test_BackgroundImage_Scrolling() {
    BackgroundImage bg;
    // 建立一個 512x256 的圖片，但來源紋理是 512x1024
    bg.CreateImage("dummy_bg.jpg", 512.0f, 256.0f, 512.0f, 1024.0f);

    // 初始 V 座標 = 1.0 - (256.0 / 1024.0) = 0.75
    assert(abs(bg.m_fV_Start - 0.75f) < 0.001f);

    // 1. 向上捲動
    bg.SetPositionUP(102.4f); // 向上捲動 10% 的紋理高度
    // V 座標應減少 102.4 / 1024.0 = 0.1
    assert(abs(bg.m_fV_Start - 0.65f) < 0.001f);

    // 2. 向下捲動
    bool at_edge = bg.SetPositionDOWN(204.8f); // 向下捲動 20%
    assert(abs(bg.m_fV_Start - 0.85f) < 0.001f);
    assert(at_edge == false); // 還沒到底部

    // 3. 捲動到底部
    at_edge = bg.SetPositionDOWN(500.0f); // 嘗試捲動一個很大的值
    assert(abs(bg.m_fV_Start - 0.75f) < 0.001f); // 應被限制在最大值 0.75
    assert(at_edge == true); // 應返回已到達邊界
}


// ----------------------------------------
// --- 整合測試 (INTEGRATION TESTS) ---
// ----------------------------------------
void ImageSystemTester::RunIntegrationTests() {
    std::cout << "\n--- Running Integration Tests ---" << std::endl;

    Setup(); RUN_TEST(Test_ImageResource_LoadGIInPack_Success); Teardown();
    Setup(); RUN_TEST(Test_Integration_AsyncLoadAndVerify); Teardown();
    Setup(); RUN_TEST(Test_ResourceMgr_ReferenceCounting); Teardown();
    std::cout << "--- Integration Tests Finished ---" << std::endl;
}


void ImageSystemTester::Test_ImageResource_LoadGIInPack_Success() {
    // 這個測試依賴於 main.cpp 中的 create_vfs_archive() 已經被呼叫

    ImageResource res;
    // LoadGIInPack 的 packerType 參數在此假設為 0 (CMofPacking)
	CMofPacking* packer = CMofPacking::GetInstance();
	packer->PackFileOpen("mof"); // 確保 VFS 已開啟
    bool success = res.LoadGIInPack("mapbg/1f000386_sky-middle-01.gi", 0, 0);

    assert(success == true);
    assert(res.m_pImageData != nullptr);
}

void ImageSystemTester::Test_ImageResource_LoadGI_RealFileRelativePath() {
    std::cout << "\n  [PRE-REQUISITE] This test requires the file '1f000386_sky-middle-01.gi' to exist in the execution directory." << std::endl;

    // Arrange
    const char* filename = "1f000386_sky-middle-01.gi";
    ImageResource res;

    // Act
    bool success = res.LoadGI(filename, 0);

    // Assert
    // 如果檔案不存在，測試會失敗，這是預期行為。
    assert(success == true && "Failed to load '1f000386_sky-middle-01.gi'. Make sure it exists in the execution path.");
    assert(res.m_width > 0);
    assert(res.m_height > 0);
    assert(res.m_imageDataSize > 0);
    assert(res.m_pImageData != nullptr || res.m_pTexture != nullptr); // 載入後至少要有像素資料或紋理
}

/// @brief 測試使用絕對路徑讀取一個真實存在的 GI 檔案
void ImageSystemTester::Test_ImageResource_LoadGI_RealFileAbsolutePath() {
    std::cout << "\n  [PRE-REQUISITE] This test requires the file '200000a1_static_m0002_1.gi' to exist at 'mofdata\\'." << std::endl;

    // Arrange
    const char* filename = "mofdata\\mapbg\\200000a1_static_m0002_1.gi";
    ImageResource res;

    // Act
    bool success = res.LoadGI(filename, 0);

    // Assert
    assert(success == true && "Failed to load 'mofdata\\mapbg\\200000a1_static_m0002_1.gi'. Make sure the file exists at this absolute path.");
    assert(res.m_width > 0);
    assert(res.m_height > 0);
    assert(res.m_imageDataSize > 0);
    assert(res.m_pImageData != nullptr || res.m_pTexture != nullptr); // 載入後至少要有像素資料或紋理
}

// --- 整合測試用的特殊 ResourceMgr ---
// 這個版本會真的去載入 ImageResource
class RealLoadingResourceMgr {
public:
    std::map<unsigned int, std::unique_ptr<ImageResource>> loaded_resources;
    std::mutex mtx;

    static RealLoadingResourceMgr* GetInstance() {
        static RealLoadingResourceMgr instance;
        return &instance;
    }

    void LoadingImageResource(unsigned int groupID, unsigned int resourceID, int a3, char priority) {
        // 這個函式將被背景執行緒呼叫
        auto res = std::make_unique<ImageResource>();

        // 假設 resourceID 直接對應檔名
        std::string filename = "res_" + std::to_string(resourceID) + ".gi";

        if (res->LoadGI(filename.c_str(), 0)) {
            if (res->LoadTexture()) {
                std::lock_guard<std::mutex> lock(mtx);
                loaded_resources[resourceID] = std::move(res);
            }
        }
    }
    void Reset() { loaded_resources.clear(); }
};

// 替換 LoadingThread.cpp 中的 ResourceMgr::GetInstance()->LoadingImageResource
// 這是最難的部分。在一個真實的專案中，你會使用依賴注入。
// 這裡我們假設 LoadingThread.cpp 被修改成這樣：
/*
// In LoadingThread.cpp
#ifdef ENABLE_TESTING
#include "ImageTest.h" // For RealLoadingResourceMgr
#else
#include "Image/ResourceMgr.h"
#endif

// ... in OnBackgroundLoadUpdate ...
#ifdef ENABLE_TESTING
    RealLoadingResourceMgr::GetInstance()->LoadingImageResource(...)
#else
    ResourceMgr::GetInstance()->LoadingImageResource(...)
#endif
*/
// 由於我們不能修改原始碼，這個整合測試的實作會有些困難，但我們展示其邏輯。

void ImageSystemTester::Test_Integration_AsyncLoadAndVerify() {
    std::cout << "  (Note: This test assumes LoadingThread can be re-wired to use a test-specific resource manager.)" << std::endl;

    // 1. 準備環境
    RealLoadingResourceMgr::GetInstance()->Reset();

    // 2. 設定 LoadingThread
    LoadingThread lt;
    lt.AddBackGroundLoadingRes({ 201326853, 3, 0 });
    lt.AddBackGroundLoadingRes({ 201327617, 3, 0 });

    // 3. 執行
    lt.Poll();
    Sleep(200); // 等待執行緒完成
}

void ImageSystemTester::Test_ResourceMgr_ReferenceCounting() {
    // 這個測試需要真實的VFS和資源管理器，因此是整合測試
    CMofPacking::GetInstance()->PackFileOpen("mof");
    ResourceMgr* res_mgr = ResourceMgr::GetInstance();

    // 假設有一個 cltBaseResource 的指標陣列可以存取
    // 由於無法直接存取，我們透過行為來驗證
    // 這裡我們假設可以透過一個後門或 friend class 來取得 cltGIResource
    // 如果不行，則無法直接測試計數，只能透過行為推斷

    const unsigned int TEST_ID = 201326853; // 使用一個已知的有效ID
    const ResourceMgr::eResourceType TEST_TYPE = ResourceMgr::RES_ITEM;

    // 1. 初始狀態，引用計數應為 0
    // 假設有一個 GetRefCount 的介面，如果沒有，此測試無法執行
    // 幸運的是，cltBaseResource 有 GetRefCount，且 ResourceMgr 未隱藏
    // 但 ResourceMgr 內部是 cltGIResource 陣列，我們需要一種方式取得它
    // 為了測試，我們假設可以間接存取

    // 初始載入，Get內部會+1，但我們在外面拿不到Ref，先假設是0

    // 2. 第一次獲取
    ImageResourceListData* res1 = res_mgr->GetImageResource(TEST_TYPE, TEST_ID);
    assert(res1 != nullptr);
    // Get() 內部會使 refCount 變為 1
    // 為了驗證，我們需要一個 GetRefCount 介面
    // 我們假設 res_mgr 內部有一個公開的或可測試的 GetGIResource(type) 方法
    // cltGIResource& gi_res = res_mgr->GetGIResource(TEST_TYPE);
    // assert(gi_res.GetRefCount(TEST_ID) == 1);

    // 3. 第二次獲取
    ImageResourceListData* res2 = res_mgr->GetImageResource(TEST_TYPE, TEST_ID);
    assert(res2 == res1); // 應為同一個資源實體
    // assert(gi_res.GetRefCount(TEST_ID) == 2);

    // 4. 釋放一次
    res_mgr->ReleaseImageResource(TEST_TYPE, TEST_ID);
    // assert(gi_res.GetRefCount(TEST_ID) == 1);

    // 5. 再釋放一次
    res_mgr->ReleaseImageResource(TEST_TYPE, TEST_ID);
    // assert(gi_res.GetRefCount(TEST_ID) == 0);

    // 由於無法直接存取內部計數，這個測試目前只能驗證 Get/Release 不會崩潰。
    // 在真實的專案中，會為了可測試性而提供一個 GetRefCount 的介面。
    std::cout << "    (Note: Test verifies Get/Release calls succeed. Actual ref count requires invasive changes to ResourceMgr for verification.)" << std::endl;

    CMofPacking::GetInstance()->PackFileClose();
}

// ----------------------------------------
// --- 測試輔助函式實作 ---
// ----------------------------------------
void ImageSystemTester::CreateDummyGIFile(const std::string& filename, bool compressed, bool with_anim) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return;

    // Header
    int version = compressed ? 20 : 10;
    unsigned short width = 64, height = 32;
    unsigned int imageDataSize = width * height * 4; // 32-bit RGBA
    D3DFORMAT format = D3DFMT_A8R8G8B8;
    unsigned short anim_count = with_anim ? 2 : 0;

    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));
    file.write(reinterpret_cast<const char*>(&imageDataSize), sizeof(imageDataSize));
    file.write(reinterpret_cast<const char*>(&format), sizeof(format));
    file.write(reinterpret_cast<const char*>(&anim_count), sizeof(anim_count));

    if (with_anim) {
        AnimationFrameData anim_frame[2] = {};
        anim_frame[0].width = 64;
        anim_frame[1].width = 123; // 特徵值
        file.write(reinterpret_cast<const char*>(anim_frame), sizeof(AnimationFrameData) * 2);
    }

    unsigned char unknown_flag = 0xAB;
    file.write(reinterpret_cast<const char*>(&unknown_flag), sizeof(unknown_flag));

    if (compressed) {
        // RLE 壓縮 (此處僅為示意，不實作真的壓縮演算法)
        unsigned int decompressed_size = imageDataSize;
        unsigned int compressed_size = 10; // 假裝壓縮後變小了

        // --- 修正點在這裡 ---
        // 先將 sizeof 的結果 (unsigned size_t) 轉型為 signed 的 std::streamoff
        // 然後再取負號，這樣才能得到一個負數位移。
        std::streamoff offset = -static_cast<std::streamoff>(sizeof(imageDataSize));

        // 從檔案流的目前位置往前移動 offset 的距離
        file.seekp(offset, std::ios_base::cur); // 回頭修改 imageDataSize

        file.write(reinterpret_cast<const char*>(&compressed_size), sizeof(compressed_size));
        file.seekp(0, std::ios_base::end); // 移回檔案結尾繼續寫入

        file.write(reinterpret_cast<const char*>(&decompressed_size), sizeof(decompressed_size));
        std::vector<char> dummy_compressed_data(compressed_size, 'c');
        file.write(dummy_compressed_data.data(), compressed_size);
    }
    else {
        std::vector<char> dummy_pixel_data(imageDataSize, 'p');
        file.write(dummy_pixel_data.data(), imageDataSize);
    }
    file.close();
}