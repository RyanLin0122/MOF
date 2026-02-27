#pragma once
#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <d3d9.h>

#include "FileSystem/CMOFPacking.h" // 您的 CMofPacking 類別標頭檔 (單例版本)
#include "Image/CDeviceResetManager.h"
#include "Image/CDeviceManager.h"
#include "Image/cltImageManager.h"
#include "Test/Test.h"
#include "global.h"  // 包含全域變數定義
#include "conf.h"


LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS   g_d3dpp;
LPDIRECT3DDEVICE9       Device = NULL;

extern cltTextFileManager g_clTextFileManager;

//-----------------------------------------------------------------------------
// 函式原型 (Forward Declarations)
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd);
VOID Render();
VOID Cleanup();

//-----------------------------------------------------------------------------
// 函式實作 (Function Implementation)
//-----------------------------------------------------------------------------
void create_vfs_archive() {
	const char* vfs_base_name = "mof"; // 最終會產生 mof.pak 和 mof.paki
	const char* source_directory = "mofdata\\"; // 您在步驟1中建立的來源目錄 (注意路徑結尾的斜線)

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
		// 設定控制台支援UTF-8顯示
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);

		// 或者使用寬字符模式 (二選一)
		// _setmode(_fileno(stdout), _O_U16TEXT);
	}
#endif
}

void Setup() {
	g_DCTTextManager.Initialize((char*)"MoFTexts.txt");
	CMoFFontTextureManager::GetInstance()->InitCMoFFontTextureManager(g_pd3dDevice);
	printf("  CMoFFontTextureManager 初始化完畢。\n");
	g_MoFFont.InitFontInfo("MofData/FontInfo.dat");
	g_MoFFont.CreateMoFFont(g_pd3dDevice, "CharacterName");
	g_IMMList.Create(40);

	// D3DX Initialize
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_CULLMODE, 1u);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_LIGHTING, 0);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_ZENABLE, 0);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_DITHERENABLE, 0);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_SPECULARENABLE, 0);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_FOGENABLE, 0);
	Device->SetSamplerState(0, D3DSAMP_ADDRESSU, 3);
	Device->SetSamplerState(0, D3DSAMP_ADDRESSV, 3);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_ALPHAFUNC, 6u);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_ALPHAREF, 0);
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, 5u);
	CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, 6u);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, 1);
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, 1);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);

	//dword_21C9C78 = 1023969417;
	CMoFFontTextureManager::GetInstance()->InitCMoFFontTextureManager(Device);
	g_MoFFont.CreateMoFFont(Device, "CharacterName");
	g_dwHeartBeatTime = timeGetTime();
	g_MoFFont.SetFont("CharacterName");

	// To Do
}

// EUC-KR(51949) 或 UHC(949) → UTF-16
static std::wstring KR_to_wide(const char* bytes) {
	if (!bytes) return L"";
	const UINT cps[] = { 51949, 949 }; // EUC-KR, CP949
	for (UINT cp : cps) {
		int n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, bytes, -1, nullptr, 0);
		if (n > 0) {
			std::wstring w(n, L'\0');
			MultiByteToWideChar(cp, 0, bytes, -1, &w[0], n);
			if (!w.empty() && w.back() == L'\0') w.pop_back();
			return w;
		}
	}
	// 都失敗：當成 ASCII
	return std::wstring(bytes, bytes + strlen(bytes));
}

static std::string wide_to_utf8(const std::wstring& w) {
	if (w.empty()) return {};
	int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
	std::string s(n, '\0');
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, nullptr, nullptr);
	return s;
}

void kr_printf(const char* euckr) {
	std::wstring w = KR_to_wide(euckr);

	// 1) 若在偵錯中，也丟到 Output 視窗（可選）
	if (IsDebuggerPresent()) {
		OutputDebugStringW(w.c_str());
		OutputDebugStringW(L"\r\n");
	}

	// 2) 判斷 stdout 是否真的是 Console
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD ft = GetFileType(hOut);

	if (ft == FILE_TYPE_CHAR) {
		// 可能是 Console 或 pseudo console
		DWORD mode;
		if (GetConsoleMode(hOut, &mode)) {
			// 是 Console：直接寫寬字元（不動碼頁、不動 _setmode）
			DWORD written = 0;
			WriteConsoleW(hOut, w.c_str(), (DWORD)w.size(), &written, nullptr);
			// 補換行
			WriteConsoleW(hOut, L"\r\n", 2, &written, nullptr);
			return;
		}
	}

	// 3) 若被重導到檔案/管線：輸出 UTF-8 位元組
	std::string u8 = wide_to_utf8(w);
	fwrite(u8.data(), 1, u8.size(), stdout);
	fwrite("\n", 1, 1, stdout);
	fflush(stdout);
}

//-----------------------------------------------------------------------------
// Windows 應用程式主進入點
//-----------------------------------------------------------------------------
int APIENTRY  wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	DWORD lastTime = 0;
	CreateDebugConsole();
	CMofPacking::GetInstance()->PackFileOpen("mof");
	g_clTextFileManager.Initialize((char*)("MoFData/Mof.dat"));


	kr_printf(g_DCTTextManager.GetText(4750));

	WNDCLASSEXW wc{ sizeof(WNDCLASSEXW) };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"D3D GameImage Test";

	RegisterClassEx(&wc);

	g_hWnd = CreateWindowExW(0, wc.lpszClassName, L"Game 渲染測試模式 (模組化版本)",
		WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
		NULL, NULL, wc.hInstance, NULL);

	if (SUCCEEDED(InitD3D(g_hWnd)))
	{
		switch (CURRENT_MODE)
		{
		case PACK_MODE:
			create_vfs_archive();
			break;
		case TEST_MODE:
			Setup();
			Run_Test();
			break;
		default:
			Setup();
			break;
		}
		cltImageManager::GetInstance()->Initialize();

		// 初始化測試物件
		if (CURRENT_MODE == TEST_MODE) {
			if (InitTest() != 0) {
				Cleanup();
				return -1;
			}
		}

		ShowWindow(g_hWnd, nCmdShow);
		UpdateWindow(g_hWnd);

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		// 初始化時間
		lastTime = timeGetTime(); // 使用 timeGetTime() 取得當前時間
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				// 累計 Frame 間隔
				DWORD currentTime = timeGetTime();
				float fElapsedTime = (currentTime - lastTime) / 1000.0f;
				lastTime = currentTime;

				// 測試 Update
				if (CURRENT_MODE == TEST_MODE) {
					UpdateTest(fElapsedTime);
				}

				// 執行 Render
				Render();
			}
		}
	}

	Cleanup();
	UnregisterClass(L"D3D GameImage Test", wc.hInstance);
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
	return DefWindowProc(hWnd, message, wParam, lParam);
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
		if (CURRENT_MODE == TEST_MODE) {
			Render_Test();
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
	CleanupTest();
	// 釋放 D3D 物件
	if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
	if (g_pD3D != NULL) g_pD3D->Release();

	// 關閉 VFS 單例
	printf("  [main] 正在關閉 VFS...\n");
	CMofPacking::GetInstance()->PackFileClose();
	CMofPacking::DestroyInstance();
	printf("--- [main] 清理完畢 ---\n");
}