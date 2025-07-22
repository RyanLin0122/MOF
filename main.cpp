#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)

#include "nfs_test.h"
#include "cm_packing_integration_test.h"
#include "Test/CompTest.h"
#include "Test/ImageTest.h"
#include "Test/ImageDrawTest.h"
#include "Sound/COgg.h"  // 您的 COgg 類別標頭檔
#include "CMOFPacking.h" // 您的 CMofPacking 類別標頭檔 (單例版本)
#include "Image/CDeviceResetManager.h"

#include <d3d9.h>

void create_vfs_archive() {
    const char* vfs_base_name = "mof"; // 最終會產生 mof.pak 和 mof.paki
    const char* source_directory = "D:\\mof_module\\VFS\\mofdata\\"; // 您在步驟1中建立的來源目錄 (注意路徑結尾的斜線)

    std::cout << "正在建立 VFS 封裝檔: " << vfs_base_name << std::endl;
    std::cout << "來源目錄: " << source_directory << std::endl;

    // 取得 CMofPacking 單例
    CMofPacking* packer = CMofPacking::GetInstance();
    if (!packer) {
        std::cerr << "錯誤：無法取得 CMofPacking 實例。" << std::endl;
        return;
    }

    // 開啟 (建立) 一個新的 VFS 檔案
    // PackFileOpen 在檔案不存在時會建立新檔
    if (!packer->PackFileOpen(vfs_base_name)) {
        std::cerr << "錯誤：無法開啟/建立 VFS 檔案 " << vfs_base_name << std::endl;
        CMofPacking::DestroyInstance();
        return;
    }

    // 執行打包
    std::cout << "正在打包目錄內容..." << std::endl;
    int result = packer->DataPacking(source_directory);
    if (result == 0) {
        std::cerr << "警告：DataPacking 未找到任何檔案或目錄無效。" << std::endl;
    }
    else {
        std::cout << "打包完成。" << std::endl;
    }

    // 關閉 VFS 檔案並銷毀單例
    packer->PackFileClose();
    CMofPacking::DestroyInstance();

    std::cout << vfs_base_name << ".pak 和 " << vfs_base_name << ".paki 已成功建立。" << std::endl;
}

void ogg_play_test() {
    // --- 測試參數設定 ---
    // 請將此處的 VFS 名稱換成您實際的封裝檔基礎名稱 (不含 .pak/.paki)
    const char* VFS_ARCHIVE_NAME = "mof"; // 例如 "mof", "data" 等
    // 請將此處的路徑換成您封裝檔中實際的 OGG 檔案路徑
    const char* OGG_PATH_IN_VFS = "music/bg_beavers.ogg";

    printf("FMOD OGG 播放整合測試\n");
    printf("------------------------\n");

    // --- 1. 初始化並開啟 VFS ---
    // 這是應用程式層級的責任，必須在任何讀取操作之前完成。
    printf("正在初始化並開啟 VFS: %s.pak/.paki\n", VFS_ARCHIVE_NAME);
    CMofPacking* packer = CMofPacking::GetInstance();
    if (!packer) {
        printf("錯誤：無法取得 CMofPacking 實例。\n");
        return;
    }
    // 開啟虛擬檔案系統，這是使用前必須的步驟
    if (!packer->PackFileOpen(VFS_ARCHIVE_NAME)) {
        printf("錯誤：無法開啟 VFS 封裝檔案 '%s'。\n", VFS_ARCHIVE_NAME);
        printf("請確認 %s.pak 和 %s.paki 檔案是否存在於執行檔目錄下。\n", VFS_ARCHIVE_NAME, VFS_ARCHIVE_NAME);
        CMofPacking::DestroyInstance();
        return;
    }
    printf("VFS 開啟成功。\n\n");


    // --- 2. 初始化音效系統並播放音樂 ---
    // 現在 VFS 已經開啟，我們可以安全地使用 COgg 了。

    // 創建並初始化 COgg 播放器物件
    COgg oggPlayer;

    // 初始化 COgg。參數 1 表示啟用循環播放。
    printf("正在初始化 COgg 播放器 (啟用循環播放)...\n");
    oggPlayer.Initalize(1);

    printf("準備從 VFS 播放檔案: %s\n", OGG_PATH_IN_VFS);
    // 播放指定的 OGG 檔案，COgg 會透過 CMofPacking 從已開啟的 VFS 中讀取
    oggPlayer.Play(OGG_PATH_IN_VFS);


    // --- 3. 等待播放結束 ---
    printf("\n音樂應該正在播放 (如果檔案存在於 VFS 中且 FMOD 初始化成功)。\n");
    printf("按下任意鍵停止音樂並退出程式。\n\n");

    // 保持程式運行並定期更新 FMOD，直到使用者按下按鍵
    while (!_kbhit()) {
        FSOUND_Update(); // 定期呼叫 FSOUND_Update() 很重要，FMOD 依賴它來更新音訊串流等
        Sleep(50);       // 短暫休眠以降低 CPU 使用率
    }
    _getch(); // 讀取並清除按鍵緩衝


    // --- 4. 清理與關閉 ---
    printf("正在停止音樂...\n");
    oggPlayer.Stop(); // 明確停止音樂

    // oggPlayer 的解構函式 (~COgg) 會在函式結束時自動被呼叫，
    // 它會處理 FSOUND_Stream_Close 和 FSOUND_Close。

    // 關閉 VFS 並銷毀 CMofPacking 單例，這是應用程式層級的責任。
    printf("正在關閉 VFS 並銷毀 CMofPacking 實例...\n");
    packer->PackFileClose();
    CMofPacking::DestroyInstance();

    printf("測試程式結束。\n");
}

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------
// --- 全域變數定義 ---
extern int IsInMemory = 1;
extern bool IsDialogBoxMode = false;
extern int g_Game_System_Info = 1280;
extern int nHeight = 720;
extern bool DontDraw = false;
extern unsigned char NationCode = 0;

// --- D3D 和測試模式所需的全域變數 ---
LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS   g_d3dpp;
HWND                    g_hWnd = NULL;
LPDIRECT3DDEVICE9       Device = NULL;

// <<< NEW CODE: 建立一個指向我們測試類別實體的指標 >>>
ImageDrawTest* g_pDrawTest = nullptr;

//-----------------------------------------------------------------------------
// 函式原型 (Forward Declarations)
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd);
VOID Render();
VOID Cleanup();

int test_func() {
    std::cout << "Starting Virtual File System Tests..." << std::endl;
    std::cout << "========================================" << std::endl;

    // 執行所有測試
    run_all_tests(); //nfs unit test
    print_test_result();
    run_cmofpacking_tests();
    run_comp_test();
    //create_vfs_archive();
    ImageSystemTester tester;
    tester.RunImageTests();
    ogg_play_test();
    return 0;
}

void CreateDebugConsole()
{
    // 使用 _DEBUG 預處理器，確保這段程式碼只在「偵錯」模式下編譯
#ifdef _DEBUG
    if (AllocConsole())
    {
        FILE* pFile;
        // 重新導向標準輸出到新的主控台
        freopen_s(&pFile, "CONOUT$", "w", stdout);

        // 重新導向標準錯誤到新的主控台
        freopen_s(&pFile, "CONOUT$", "w", stderr);

        // 重新導向標準輸入到新的主控台
        freopen_s(&pFile, "CONIN$", "r", stdin);

        // 也讓 C++ 的 iostream 可以正常運作
        std::cout.clear();
        std::wcout.clear();
        std::cerr.clear();
        std::wcerr.clear();
        std::cin.clear();
        std::wcin.clear();
    }
#endif
}

//-----------------------------------------------------------------------------
// Windows 應用程式主進入點
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CreateDebugConsole();
    test_func();

    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D GameImage Test", NULL };
    RegisterClassExA(&wc);

    g_hWnd = CreateWindowExA(0, "D3D GameImage Test", "GameImage 渲染測試模式 (模組化版本)",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
        NULL, NULL, wc.hInstance, NULL);

    if (SUCCEEDED(InitD3D(g_hWnd)))
    {
        // <<< NEW CODE START: 初始化我們的測試類別 >>>
        g_pDrawTest = new ImageDrawTest();
        if (FAILED(g_pDrawTest->Initialize()))
        {
            MessageBoxA(g_hWnd, "無法初始化或載入測試圖片，請檢查 .pak 檔案和資源 ID 是否正確。", "測試初始化失敗", MB_OK | MB_ICONERROR);
            Cleanup();
            return -1;
        }
        // <<< NEW CODE END >>>

        ShowWindow(g_hWnd, nCmdShow);
        UpdateWindow(g_hWnd);

        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                Render();
            }
        }
    }

    Cleanup();
    UnregisterClassA("D3D GameImage Test", wc.hInstance);
    return 0;
}


//-----------------------------------------------------------------------------
// 視窗訊息處理函式 (Window Procedure)
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    // 其他訊息交給預設的處理函式 (使用 ANSI 版本)
    return DefWindowProcA(hWnd, message, wParam, lParam);
}


//-----------------------------------------------------------------------------
// 初始化 Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
    // 建立 D3D 物件
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return E_FAIL;

    // 設定 D3D 呈現參數
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // 自動偵測格式
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.hDeviceWindow = hWnd;

    // 建立 D3D 裝置
    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &g_d3dpp, &g_pd3dDevice)))
    {
        return E_FAIL;
    }

    // *** 關鍵步驟 ***
    // 將我們建立的裝置指標，賦值給我們還原的類別所依賴的全域 Device 指標
    Device = g_pd3dDevice;

    return S_OK;
}


//-----------------------------------------------------------------------------
// 每一幀的渲染函式
//-----------------------------------------------------------------------------
VOID Render()
{
    if (NULL == Device) return;

    HRESULT hr = Device->TestCooperativeLevel();
    if (hr != S_OK)
    {
        if (hr == D3DERR_DEVICENOTRESET)
        {
            CDeviceResetManager::GetInstance()->ResetToDevice(hr);
        }
        return;
    }

    Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(20, 20, 60), 1.0f, 0);

    if (SUCCEEDED(Device->BeginScene()))
    {
        // <<< NEW CODE: 將渲染呼叫委派給我們的測試類別 >>>
        if (g_pDrawTest)
        {
            g_pDrawTest->Render();
        }

        Device->EndScene();
    }

    Device->Present(NULL, NULL, NULL, NULL);
}


//-----------------------------------------------------------------------------
// 程式結束時釋放資源
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    printf("--- [main] 清理資源 ---\n");

    // <<< NEW CODE: 刪除測試類別的實體 >>>
    // 這將觸發 ImageDrawTest 的解構函式，進而呼叫其內部的 Cleanup 函式
    if (g_pDrawTest)
    {
        delete g_pDrawTest;
        g_pDrawTest = nullptr;
    }

    // 釋放 D3D 物件
    if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
    if (g_pD3D != NULL) g_pD3D->Release();

    // 關閉 VFS 單例
    printf("  [main] 正在關閉 VFS...\n");
    CMofPacking::GetInstance()->PackFileClose();
    CMofPacking::DestroyInstance();
    printf("--- [main] 清理完畢 ---\n");
}