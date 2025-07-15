#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)

#include "nfs_test.h"
#include "cm_packing_integration_test.h"
#include "Test/CompTest.h"
#include "Sound/COgg.h"  // 您的 COgg 類別標頭檔
#include "CMOFPacking.h" // 您的 CMofPacking 類別標頭檔 (單例版本)
#include "Image/CDeviceResetManager.h"

#include <d3d9.h>

void create_vfs_archive() {
    const char* vfs_base_name = "mof"; // 最終會產生 mof.pak 和 mof.paki
    const char* source_directory = "D:\\VFS_Source\\"; // 您在步驟1中建立的來源目錄 (注意路徑結尾的斜線)

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
    const char* OGG_PATH_IN_VFS = "bg_beavers.ogg";

    printf("FMOD OGG 播放整合測試\n");
    printf("------------------------\n");

    create_vfs_archive();
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
LPDIRECT3D9             g_pD3D = NULL;               // 用於建立 D3D 裝置的 D3D 物件
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;         // 我們的渲染裝置
D3DPRESENT_PARAMETERS   g_d3dpp;                     // D3D 呈現參數
HWND                    g_hWnd = NULL;               // 我們的主視窗控制代碼
CDeviceResetManager* g_pDeviceResetManager = NULL;  // 裝置重設管理器

// 為了讓還原的類別可以找到裝置指標，我們定義這個全域變數
// 在 InitD3D 中會將它指向 g_pd3dDevice
LPDIRECT3DDEVICE9 Device = NULL;

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
    //ogg_play_test();
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
    //Debug Console
    CreateDebugConsole();
    test_func();
    // 註冊視窗類別 (使用 ANSI 版本)
    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassExA(&wc);

    // 建立應用程式視窗 (使用 ANSI 版本)
    g_hWnd = CreateWindowExA(0, "D3D Tutorial", "Direct3D 視窗程式",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
        NULL, NULL, wc.hInstance, NULL);

    // 初始化 Direct3D
    if (SUCCEEDED(InitD3D(g_hWnd)))
    {
        // 建立我們的裝置管理器
        g_pDeviceResetManager = CDeviceResetManager::GetInstance();

        // 顯示視窗
        ShowWindow(g_hWnd, nCmdShow);
        UpdateWindow(g_hWnd);

        // 進入訊息迴圈
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            // 使用 PeekMessage 來處理訊息，不會阻塞渲染迴圈
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // 如果沒有訊息，就進行渲染
                Render();
            }
        }
    }

    // 清理資源
    Cleanup();
    UnregisterClassA("D3D Tutorial", wc.hInstance);
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
    if (NULL == g_pd3dDevice)
        return;

    // 檢查裝置狀態，看是否遺失
    HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICENOTRESET)
    {
        // 如果裝置可以被重設，就呼叫我們的管理器來處理
        if (g_pDeviceResetManager)
        {
            g_pDeviceResetManager->ResetToDevice(hr);
        }
    }
    else if (hr == S_OK)
    {
        // 清除後台緩衝區和深度緩衝區
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

        // 開始繪製場景
        if (SUCCEEDED(g_pd3dDevice->BeginScene()))
        {
            //
            // --- 在這裡進行所有的繪圖操作 ---
            // 例如：g_pDeviceResetManager->GetSpriteObject()->Begin(...)
            //

            // 結束繪製場景
            g_pd3dDevice->EndScene();
        }

        // 將後台緩衝區的內容顯示到螢幕上
        g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
    }
}


//-----------------------------------------------------------------------------
// 程式結束時釋放資源
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if (g_pDeviceResetManager != NULL)
    {
        delete g_pDeviceResetManager;
        g_pDeviceResetManager = NULL;
    }
    if (g_pd3dDevice != NULL)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }
    if (g_pD3D != NULL)
    {
        g_pD3D->Release();
        g_pD3D = NULL;
    }
}