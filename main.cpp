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
#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltLessonKindInfo.h"
#include "System/cltClassSystem.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltLessonSystem.h"
#include "Sound/GameSound.h"
#include "Logic/DirectInputManager.h"
#include "Character/cltMyCharData.h"
#include "Util/cltTimer.h"
#include "Effect/CEffectManager.h"
// 注意: CMoFNetwork / ClientCharacterManager 只用 forward declaration
// (來自 global.h)，避開 windows.h 已先載入後再 include winsock2.h 的衝突。
#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMini_Sword.h"
#include "MiniGame/cltMini_Sword_2.h"
#include "MiniGame/cltMini_Bow.h"
#include "MiniGame/cltMini_Bow_2.h"
#include "MiniGame/cltMini_Magic.h"
#include "MiniGame/cltMini_Magic_2.h"
#include "MiniGame/cltMini_Exorcist.h"
#include "MiniGame/cltMini_Exorcist_2.h"
#include "Test/Test.h"
#include "global.h"  // 包含全域變數定義
#include "conf.h"

// mofclient.c 還原之新增類別 — 提供 wWinMain 階段的 init 接點
#include "Util/CLog.h"
#include "Util/cltRegistry.h"
#include "Util/CCmdLine.h"
#include "Logic/CDeleteCashItemManager.h"
#include "UI/CUIManager.h"
#include "Effect/MainTitleBG.h"



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

// -----------------------------------------------------------------------------
// BootstrapMofClientGlobals — 對齊 mofclient.c 0x4307E0~0x433D40 的 Auth/Log
// 初始化序列：
//   1. CCmdLine::SetCmdLine(lpCmdLine)            ← 0x4FA0A0
//   2. cltRegistry::Open(<NationCode 對應 SubKey>) ← 0x519980
//   3. CUIManager::OpenLogWindow → CLog::CLog       ← 0x433CC0
// 由於本 codebase 走的是 test/debug harness（並非 mofclient.c 的 GameState
// 機制），把這三個 init 集中到單一 helper、在 Setup() 開頭呼叫。
// -----------------------------------------------------------------------------
static void BootstrapMofClientGlobals(const char* commandLine)
{
    // 1) CCmdLine — 把整個 lpCmdLine 拷進 g_CmdLine
    g_CmdLine.SetCmdLine(commandLine ? commandLine : "");

    // 2) cltRegistry — 依 NationCode 開對應 HKCU/HKLM Software 路徑
    //    對齊 mofclient.c 0x59E700：
    //      case 1/2/4: "Software\\BuddyGame\\MOF"
    //      case 3:     "Software\\Unalis\\MOF"
    //      case 5:     "Software\\URGAME\\MOF"
    const char* regPath = "Software\\BuddyGame\\MOF";
    switch (NationCode) {
        case 3: regPath = "Software\\Unalis\\MOF"; break;
        case 5: regPath = "Software\\URGAME\\MOF"; break;
        default: break; // 1/2/4 走預設
    }
    g_clRegistry.Open(regPath);

    // 3) CLog — 走 CUIManager::OpenLogWindow 的真實路徑
    //    （只在 MoFData/log.dat 存在時才實際輸出，否則 CLog 會以 kQuiet 模式建立）
    if (!g_UIMgr) {
        g_UIMgr = new CUIManager();
    }
    g_UIMgr->OpenLogWindow();

    // 4) CDeleteCashItemManager — 對齊 mofclient.c CUIManager::CreateLobbySelChannel
    //    (0x430A70) 開場時呼叫的 InitDeleteCashItem。沒有 lobby UI 也無妨：
    //    讓 owner 早期生效，AddDeleteCashitem 才有 vector 可用。
    g_DeleteCashItemManager.InitDeleteCashItem();

    if (logger) {
        logger->Log("MoFClient bootstrap complete (registry=%s)", regPath);
    }
}

void Setup() {
	BootstrapMofClientGlobals("");

	g_clClassKindInfo.Initialize((char*)"classkindinfo.txt");
	cltItemKindInfo::InitializeStaticVariable(&g_clClassKindInfo, nullptr);
	cltClassSystem::InitializeStaticVariable(&g_clClassKindInfo, &g_clItemKindInfo);
	cltEquipmentSystem::InitializeStaticVariable(&g_clItemKindInfo, &g_clClassKindInfo);
	g_clItemKindInfo.Initialize("itemkindinfo.txt", "item_instant.txt", "item_hunt.txt", "item_fashion.txt", "koreatext.txt", "TrainningCardInfo.txt");

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
// MiniGame Debug Mode — 終端機選單啟動單一小遊戲以便測試
//-----------------------------------------------------------------------------
static DirectInputManager g_DebugInputMgr{};
static cltMoF_BaseMiniGame* g_pDebugMiniGame = nullptr;
static int g_iDebugMiniGameChoice = 0;  // 1..8 對應 CreateDebugMiniGame 選單

static void InitMiniGameDebugSubsystems()
{
    // 0) 資源在 MoFData/nation_kor/ 下（目前僅此一國家資料），把 NationCode 切成 1
    //    讓 cltGIResource::LoadResourceInPack 的 fallback 去 Nation_kor/ 找得到資源。
    extern unsigned char NationCode;
    NationCode = 1;

    // 0.5) 計時器 — 小遊戲 Ready 倒數、遊戲時間皆透過 g_clTimerManager
    //      註冊每秒 callback，不初始化 + Poll 的話倒數永遠不減。
    cltTimerManager::InitializeStaticVariable();
    g_clTimerManager.Initialize(0x3E8u);

    // 1) 音效 + BGM
    // 開發環境下 mofdata/sound/*.wav 與 mofdata/music/*.ogg 通常是散檔，
    // 沒有被打包進 mof.nfs；若走 pack 路徑會讀回 size=0 導致 GameSound
    // 與 COgg 都 silent fail（PlaySoundA 的 CreateFromMemory 回 E_INVALIDARG，
    // COgg::OpenStreem 則 m_pStream = nullptr）。此處強迫走 direct-file。
    extern int g_bLoadOggFromMofPack;
    g_bLoadOggFromMofPack = 0;
    g_GameSoundManager.InitSound((char*)"SoundListInfo.txt");

    // 2) 課程種類表（用於小遊戲內讀取訓練卡資訊、倍率計算）
    g_clLessonKindInfo.Initialize((char*)"LessonKindInfo.txt");
    cltLessonSystem::InitializeStaticVariable(&g_clLessonKindInfo, &g_clItemKindInfo);

    // 3) DirectInput — 直接建立並取得鍵盤佈局，避開 hrInitDirectInput 於 x64
    //    的指標算術（ppvOut[135]）會寫到物件以外的已知問題。
    {
        HMODULE hModule = GetModuleHandleA(nullptr);
        DirectInput8Create(hModule, DIRECTINPUT_VERSION, IID_IDirectInput8A,
                           reinterpret_cast<LPVOID*>(&g_DebugInputMgr.m_pDirectInput),
                           nullptr);
        g_DebugInputMgr.m_hkl = GetKeyboardLayout(0);
        g_DebugInputMgr.hrInitKeyboard(g_hWnd);
        g_DebugInputMgr.hrInitMouse(g_hWnd);
    }

    // 4) 小遊戲基底的靜態管理器指標
    cltMoF_BaseMiniGame::InitializeStaticVariable(
        &g_clMyCharData,
        cltImageManager::GetInstance(),
        &g_Network,
        &g_ClientCharMgr,
        &g_DebugInputMgr,
        &g_GameSoundManager,
        &g_DCTTextManager);
}

static cltMoF_BaseMiniGame* CreateDebugMiniGame(int choice)
{
    switch (choice) {
    case 1: return new cltMini_Sword();
    case 2: return new cltMini_Sword_2();
    case 3: return new cltMini_Bow();
    case 4: return new cltMini_Bow_2();
    case 5: return new cltMini_Magic();
    case 6: return new cltMini_Magic_2();
    case 7: return new cltMini_Exorcist();
    case 8: return new cltMini_Exorcist_2();
    default: return nullptr;
    }
}

// 回傳新建立的小遊戲，或 nullptr 表示使用者選擇離開。
static cltMoF_BaseMiniGame* SelectAndCreateMiniGameFromConsole()
{
    while (true) {
        std::printf("\n================ MiniGame Debug Menu ================\n");
        std::printf("  1) Sword       (劍士 - 模式 A)\n");
        std::printf("  2) Sword_2     (劍士 - 模式 B)\n");
        std::printf("  3) Bow         (弓手 - 模式 A)\n");
        std::printf("  4) Bow_2       (弓手 - 模式 B)\n");
        std::printf("  5) Magic       (法師 - 模式 A)\n");
        std::printf("  6) Magic_2     (法師 - 模式 B)\n");
        std::printf("  7) Exorcist    (驅魔 - 模式 A)\n");
        std::printf("  8) Exorcist_2  (驅魔 - 模式 B)\n");
        std::printf("  0) Quit\n");
        std::printf("※ 難度 (Easy/Normal/Hard) 於遊戲內的 SelectDegree 畫面選擇\n");
        std::printf("Choice: ");
        std::fflush(stdout);

        int choice = 0;
        if (std::scanf("%d", &choice) != 1) {
            int ch; while ((ch = std::getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        if (choice == 0) return nullptr;

        cltMoF_BaseMiniGame* game = CreateDebugMiniGame(choice);
        if (game) {
            g_iDebugMiniGameChoice = choice;
            // 清空輸入狀態，避免上一輪殘留
            g_DebugInputMgr.FreeAllKey();
            // 把遊戲視窗帶到前景，避免 console 擋住
            if (g_hWnd) {
                ShowWindow(g_hWnd, SW_SHOW);
                SetForegroundWindow(g_hWnd);
                SetFocus(g_hWnd);
            }
            std::printf("[minigame] 已建立第 %d 個遊戲，切換到遊戲視窗。\n", choice);
            std::fflush(stdout);
            return game;
        }
    }
}

// mofclient.c FPS_Update (0x4FEDA0)：把邏輯 clamp 到 SETTING_FRAME*30 fps。
// 回傳 0 = 時間還沒到，略過這幀；1 = 正常推進；2 = 要追幀就略過 draw。
static int FPS_Update()
{
    static DWORD s_fpsStartTime = 0;
    static DWORD s_totalFrameCounter = 0;

    DWORD target;
    if (s_totalFrameCounter) {
        const double elapsed = (double)(timeGetTime() - s_fpsStartTime);
        const double frameMs = 1000.0 / ((double)SETTING_FRAME * 30.0);
        target = (DWORD)(elapsed / frameMs);
    } else {
        target = 1;
        s_fpsStartTime = timeGetTime();
    }
    if (target <= s_totalFrameCounter) return 0;
    const DWORD prev = s_totalFrameCounter;
    ++s_totalFrameCounter;
    return (target - (prev + 1) > 1) ? 2 : 1;
}

// 主迴圈每幀呼叫：驅動輸入、推進小遊戲邏輯；遊戲結束時回選單。
static void UpdateMiniGameDebug(float fElapsedTime)
{
    static DWORD s_lastTick = 0;
    static unsigned int s_frames = 0;
    ++s_frames;
    DWORD now = timeGetTime();
    if (s_lastTick == 0) s_lastTick = now;
    if (now - s_lastTick >= 1000) {
        const char* name = "(none)";
        unsigned state = 0;
        switch (g_iDebugMiniGameChoice) {
        case 1: name = "g_cGameSwordState";      state = g_cGameSwordState;      break;
        case 2: name = "g_cGameSword_2State";    state = g_cGameSword_2State;    break;
        case 3: name = "g_cGameBowState";        state = g_cGameBowState;        break;
        case 4: name = "g_cGameBow_2State";      state = g_cGameBow_2State;      break;
        case 5: name = "g_cGameMagicState";      state = g_cGameMagicState;      break;
        case 6: name = "g_cGameMagic_2State";    state = g_cGameMagic_2State;    break;
        case 7: name = "g_cGameExorcistState";   state = g_cGameExorcistState;   break;
        case 8: name = "g_cGameExorcist_2State"; state = g_cGameExorcist_2State; break;
        default: break;
        }
        std::printf("[minigame] %u fps, %s=%u\n", s_frames, name, state);
        std::fflush(stdout);
        s_frames = 0;
        s_lastTick = now;
    }

    // 每幀 tick 計時器 — 對齊 GT main loop 210289-210290。
    cltTimerManager::UpdateTime();
    g_clTimerManager.Poll();

    // 對齊 GT main loop 210272：推進特效管理器的生命週期，
    // 讓 Ani_Board::Play 加入的 CEffect_MiniGame_Fighter_Break 會被 tick 與清理。
    g_EffectManager_MiniGame.FrameProcess(fElapsedTime, false);

    g_DebugInputMgr.hrReadInput();
    g_DebugInputMgr.Update();

    // DirectInput 讀的是相對移動量，遊戲邏輯用的是絕對座標。
    // 直接從 Win32 取實際游標位置覆蓋上去，確保按鈕 hit-test 正確。
    POINT cursor;
    if (GetCursorPos(&cursor) && ScreenToClient(g_hWnd, &cursor)) {
        g_DebugInputMgr.m_mouseX = cursor.x;
        g_DebugInputMgr.m_mouseY = cursor.y;
    }
    // 同步 Win32 滑鼠按鈕狀態 (DISCL_FOREGROUND 下 DirectInput 取到的
    // 按鈕狀態不一定即時；這裡直接用 GetAsyncKeyState 當 fallback)。
    {
        static bool s_lPrev = false, s_rPrev = false;
        const bool lNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        const bool rNow = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (lNow && !s_lPrev) g_DebugInputMgr.SetLMButtonStatue(1);      // down
        else if (!lNow && s_lPrev) g_DebugInputMgr.SetLMButtonStatue(2); // up
        if (rNow && !s_rPrev) g_DebugInputMgr.SetRMButtonStatue(1);
        else if (!rNow && s_rPrev) g_DebugInputMgr.SetRMButtonStatue(2);
        s_lPrev = lNow;
        s_rPrev = rNow;
    }

    if (!g_pDebugMiniGame) return;

    // Debug mode 沒有真正的 server，但各 minigame 的 EndStage 需要 server ACK
    // 才能顯示結果 popup 並最終 Init_Wait 回主選單（見
    // cltMoF_MiniGame_Mgr::SetMiniGameResult 0x5BF4E0）。這裡在進入 state 6
    // 之後延遲約 300ms 模擬一次 server ACK，對齊 real play 的網路延遲感。
    {
        unsigned state = 0;
        switch (g_iDebugMiniGameChoice) {
        case 1: state = g_cGameSwordState;      break;
        case 2: state = g_cGameSword_2State;    break;
        case 3: state = g_cGameBowState;        break;
        case 4: state = g_cGameBow_2State;      break;
        case 5: state = g_cGameMagicState;      break;
        case 6: state = g_cGameMagic_2State;    break;
        case 7: state = g_cGameExorcistState;   break;
        case 8: state = g_cGameExorcist_2State; break;
        default: break;
        }

        static cltMoF_BaseMiniGame* s_lastGame = nullptr;
        static DWORD s_state6EnterTick = 0;
        if (s_lastGame != g_pDebugMiniGame) {
            s_lastGame = g_pDebugMiniGame;
            s_state6EnterTick = 0;
        }

        if (state == 6) {
            if (s_state6EnterTick == 0) {
                s_state6EnterTick = now;
            }
            if (now - s_state6EnterTick > 300 && !g_pDebugMiniGame->m_serverAck) {
                // 對齊 SetMiniGameResult：v3[140]=result, v3[522]=seed,
                // v3[139]=1, v3[141]=1
                g_pDebugMiniGame->m_serverResult = 1;
                g_pDebugMiniGame->m_dword522     = 0;
                g_pDebugMiniGame->m_serverAck    = 1;
                g_pDebugMiniGame->m_serverValid  = 1;
            }
        }
        else {
            s_state6EnterTick = 0;
        }
    }

    if (g_pDebugMiniGame->Poll()) {
        delete g_pDebugMiniGame;
        g_pDebugMiniGame = SelectAndCreateMiniGameFromConsole();
        if (!g_pDebugMiniGame) {
            PostQuitMessage(0);
        }
        return;
    }
    g_pDebugMiniGame->PrepareDrawing();

    // 對齊 GT main loop 210309：在 PrepareDrawing 之後更新特效位置，
    // 讓 Draw 取到當幀最新的變換資料。
    g_EffectManager_MiniGame.Process();

    // mofclient.c 210313：PrepareDrawing 之後、BeginScene 之前呼叫
    // ProcessAllGameImage，把每個 GameImage 的 m_bIsProcessed 設為 true；
    // GameImage::Draw() 會在此旗標為 false 時直接 return false。
    cltImageManager::GetInstance()->ProcessAllGameImage();
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

	//create_vfs_archive();
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
		case MINIGAME_DEBUG_MODE:
			Setup();
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

		if (CURRENT_MODE == MINIGAME_DEBUG_MODE) {
			InitMiniGameDebugSubsystems();
		}

		ShowWindow(g_hWnd, nCmdShow);
		UpdateWindow(g_hWnd);

		if (CURRENT_MODE == MINIGAME_DEBUG_MODE) {
			g_pDebugMiniGame = SelectAndCreateMiniGameFromConsole();
			if (!g_pDebugMiniGame) {
				Cleanup();
				return 0;
			}
		}

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		// 初始化時間
		lastTime = timeGetTime(); // 使用 timeGetTime() 取得當前時間
		// MiniGame 模式下 FPS_Update 會把邏輯 clamp 到 30fps，所以實際的
		// UpdateMiniGameDebug 兩次呼叫之間可能經過了很多個 Sleep(1) 迴圈。
		// 這裡累計期間經過的時間，讓特效管理器拿到「真實」的 dt，而不是
		// 每次 Sleep 後的 ~1ms 片段（否則動畫會被放慢 30 倍以上，導致
		// CCAEffect 永遠跑不完、舊特效卡在畫面上）。
		float fAccumulatedDt = 0.0f;
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
				fAccumulatedDt += fElapsedTime;

				// mofclient.c 對齊：MainTitleBG::FrameProcess、CCAEffect 等動畫實體
				// 都從 flt_21CB358 取「前一幀的秒數」。在主迴圈這裡一次更新。
				flt_21CB358 = fElapsedTime;

				// 測試 Update
				if (CURRENT_MODE == TEST_MODE) {
					UpdateTest(fElapsedTime);
					fAccumulatedDt = 0.0f;
				}
				else if (CURRENT_MODE == MINIGAME_DEBUG_MODE) {
					// 對齊 GT main loop 210260/210276：先 Sleep(1) 讓 OS
					// 有空排程，再用 FPS_Update 把邏輯 clamp 到 30fps。
					Sleep(1);
					const int fpsStep = FPS_Update();
					if (fpsStep == 0) {
						continue;   // 這一幀還沒到，跳過 Update/Render
					}
					UpdateMiniGameDebug(fAccumulatedDt);
					fAccumulatedDt = 0.0f;
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
		else if (CURRENT_MODE == MINIGAME_DEBUG_MODE) {
			if (g_pDebugMiniGame) g_pDebugMiniGame->Draw();
			// 對齊 GT GsGame_MiniGame_Draw 369186：小遊戲主畫面繪製完後，
			// 疊上 g_EffectManager_MiniGame 的所有特效（例：擊碎板子的碎片）。
			g_EffectManager_MiniGame.Draw();
		}
		Device->EndScene();
	}

	Device->Present(NULL, NULL, NULL, NULL);

	// mofclient.c 的 main loop：Present 後呼叫 ReleaseAllGameImage，把
	// PrepareDrawing 期間取出的 5000 格 pool slot 全部歸還。不做會溢出。
	if (CURRENT_MODE == MINIGAME_DEBUG_MODE) {
		cltImageManager::GetInstance()->ReleaseAllGameImage();
	}
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
