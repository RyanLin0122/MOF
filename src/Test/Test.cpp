#include "Test/Test.h"
#include "conf.h"

ImageDrawTest* g_pDrawTest = nullptr;
EffectSystemTest* g_pEffectTest = nullptr;
FontSystemTest* g_pFontTest = nullptr;

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

void dump_all_text_files() {
	printf("\n--- 開始傾印所有 TXT 檔案 ---\n");

	// 1. 建立 text_dump 資料夾
	const char* dir_name = "text_dump";
	if (CreateDirectoryA(dir_name, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("[1] 目標資料夾 '%s' 已準備就緒。\n", dir_name);
	}
	else {
		printf("[1] 錯誤：無法建立資料夾 '%s'。\n", dir_name);
		return; // 無法建立資料夾，直接返回
	}

	int dumped_files_count = 0;
	printf("[2] 開始掃描封裝檔中的所有檔案...\n");

	// 2. 遍歷所有可能的 Header (最多1024個)
	for (int i = 0; i < 1024; ++i) {
		Header* pHeader = g_clTextFileManager.GetHeaderByIndex(i);

		// 如果 Header 無效或不存在，就跳過
		if (pHeader == nullptr) {
			continue;
		}

		// 3. 檢查副檔名是否為 .txt (不分大小寫)
		const char* extension = strrchr(pHeader->filename, '.');
		if (extension && _stricmp(extension, ".txt") == 0) {

			printf("  > 找到 TXT 檔案: %s (大小: %u bytes)。正在解壓縮...\n", pHeader->filename, pHeader->original_size);

			// 4. 使用 cltTextFileManager::fopen 解壓縮檔案到暫存檔並取得 handle
			FILE* source_file = g_clTextFileManager.fopen(pHeader->filename);
			if (!source_file) {
				printf("    錯誤：g_clTextFileManager::fopen 無法開啟 '%s'。\n", pHeader->filename);
				continue;
			}

			// 5. 建立目標檔案路徑 (例如: text_dump/MoFTexts.txt)
			char dest_path[512];
			sprintf_s(dest_path, sizeof(dest_path), "%s\\%s", dir_name, pHeader->filename);

			// 6. 建立並寫入新的檔案
			FILE* dest_file;
			if (fopen_s(&dest_file, dest_path, "wb") == 0 && dest_file != nullptr) {
				// 分配緩衝區來讀寫檔案
				char buffer[4096];
				size_t bytes_read;

				// 從暫存檔讀取，寫入到目標檔案
				while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
					fwrite(buffer, 1, bytes_read, dest_file);
				}

				// 關閉目標檔案
				fclose(dest_file);
				dumped_files_count++;
			}
			else {
				printf("    錯誤：無法在 '%s' 建立檔案。\n", dest_path);
			}

			// 7. 關閉由 manager 開啟的暫存檔 (這會自動刪除 txt.tmp)
			g_clTextFileManager.fclose(source_file);
		}
	}

	if (dumped_files_count > 0) {
		printf("\n[3] 任務完成！總共傾印了 %d 個 TXT 檔案到 '%s' 資料夾。\n", dumped_files_count, dir_name);
	}
	else {
		printf("\n[3] 在封裝檔中未找到任何 .txt 檔案。\n");
	}

	printf("--- 傾印結束 ---\n\n");
}

void text_manager_test() {
	printf("\n--- cltTextFileManager 測試開始 ---\n");

	// --- 測試案例 1: 使用 GetHeaderByFileName 獲取存在的檔案資訊 ---
	const char* existing_file = "achievement_castelist.txt"; // 假設這個檔案存在於 Mof.dat 中
	printf("\n[1] 正在測試 GetHeaderByFileName (尋找存在的檔案: %s)\n", existing_file);
	Header* pHeader = g_clTextFileManager.GetHeaderByFileName((char*)existing_file);

	if (pHeader) {
		printf("  成功找到檔案! 檔案資訊:\n");
		printf("  - 檔名: %s\n", pHeader->filename);
		printf("  - 偏移量: %u\n", pHeader->file_offset);
		printf("  - 壓縮後大小: %u bytes\n", pHeader->compressed_size);
		printf("  - 原始大小: %u bytes\n", pHeader->original_size);
	}
	else {
		printf("  錯誤：無法找到檔案 '%s' 的頭資訊。\n", existing_file);
	}

	// --- 測試案例 2: 獲取不存在的檔案資訊 ---
	const char* non_existing_file = "non_existent_file.txt";
	printf("\n[2] 正在測試 GetHeaderByFileName (尋找不存在的檔案: %s)\n", non_existing_file);
	pHeader = g_clTextFileManager.GetHeaderByFileName((char*)non_existing_file);
	if (pHeader == nullptr) {
		printf("  成功：函式正確地回傳了 nullptr，因為檔案不存在。\n");
	}
	else {
		printf("  錯誤：對於不存在的檔案，函式並未回傳 nullptr。\n");
	}

	// --- 測試案例 3: 使用 fopen 和 fclose 讀取檔案內容 ---
	printf("\n[3] 正在測試 fopen / fclose 流程 (讀取檔案: %s)\n", existing_file);
	FILE* fp = g_clTextFileManager.fopen((char*)existing_file);
	if (fp) {
		printf("  成功開啟檔案。正在讀取前 5 行內容：\n");
		printf("  ----------------------------------------\n");
		char buffer[512];
		for (int i = 0; i < 5; ++i) {
			if (fgets(buffer, sizeof(buffer), fp)) {
				// 移除 fgets 可能讀取到的換行符，讓輸出更整潔
				buffer[strcspn(buffer, "\r\n")] = 0;
				printf("  Line %d: %s\n", i + 1, buffer);
			}
			else {
				printf("  (已到達檔案結尾)\n");
				break;
			}
		}
		printf("  ----------------------------------------\n");

		printf("  正在關閉檔案...\n");
		g_clTextFileManager.fclose(fp);
		printf("  檔案已關閉，暫存檔 txt.tmp 應該已被刪除。\n");
	}
	else {
		printf("  錯誤：fopen 無法開啟檔案 '%s'。\n", existing_file);
	}

	// --- 測試案例 4: 嘗試用 fopen 開啟不存在的檔案 ---
	printf("\n[4] 正在測試 fopen (開啟不存在的檔案: %s)\n", non_existing_file);
	fp = g_clTextFileManager.fopen((char*)non_existing_file);
	if (fp == nullptr) {
		printf("  成功：函式正確地回傳了 nullptr，因為檔案在封裝檔中不存在。\n");
	}
	else {
		printf("  錯誤：對於不存在的檔案，fopen 並未回傳 nullptr。\n");
		g_clTextFileManager.fclose(fp); // 如果意外成功，還是要清理
	}

	dump_all_text_files();
	printf("\n--- cltTextFileManager 測試結束 ---\n\n");
}

int Run_Test() {
	std::cout << "Starting Virtual File System Tests..." << std::endl;
	std::cout << "========================================" << std::endl;

	// 執行所有測試
	//run_all_nfs_tests();
	//print_nfs_test_result();
	//run_cmofpacking_tests();
	//run_comp_test();
	//create_vfs_archive();
	//ImageSystemTester tester;
	//tester.RunImageTests();
	//ogg_play_test();
	//text_manager_test();
	return 0;
}

int InitTest() {
	switch (G_CURRENT_TEST) {
	case TEST_IMAGE:
		g_pDrawTest = new ImageDrawTest();
		if (FAILED(g_pDrawTest->Initialize()))
		{
			//MessageBox(g_hWnd, L"無法初始化或載入測試圖片，請檢查 .pak 檔案和資源 ID 是否正確。", L"測試初始化失敗", MB_OK | MB_ICONERROR);
			return -1;
		}
		break;
	case TEST_EFFECT:
		g_pEffectTest = new EffectSystemTest();
		if (FAILED(g_pEffectTest->Initialize()))
		{
			// 2. 修改錯誤訊息
			//MessageBox(g_hWnd, L"無法初始化特效系統測試，請檢查 .pak 檔案和資源是否存在。", L"測試初始化失敗", MB_OK | MB_ICONERROR);
			return -1;
		}
		break;
	case TEST_FONT:
		g_pFontTest = new FontSystemTest();
		if (FAILED(g_pFontTest->Initialize())) {
			//MessageBox(g_hWnd, L"FontSystemTest 初始化失敗", L"錯誤", MB_OK | MB_ICONERROR);
			return -1;
		}
		break;
	}
}

void UpdateTest(float fElapsedTime) {
	if (G_CURRENT_TEST == TEST_EFFECT && g_pEffectTest)
	{
		g_pEffectTest->Update(fElapsedTime);
	}
	if (G_CURRENT_TEST == TEST_FONT && g_pFontTest)
	{
		g_pFontTest->Update(fElapsedTime);
	}
}

void Render_Test() {
	if (G_CURRENT_TEST == TEST_IMAGE) {
		if (g_pDrawTest)
		{
			g_pDrawTest->Render();
		}
	}
	else if (G_CURRENT_TEST == TEST_EFFECT) {
		if (g_pEffectTest)
		{
			g_pEffectTest->Render(); // 呼叫新類別的 Render
		}
	}
	else if (G_CURRENT_TEST == TEST_FONT) {
		if (g_pFontTest)
		{
			g_pFontTest->Render();
		}
	}
}

void CleanupTest() {
	if (g_pDrawTest)
	{
		delete g_pDrawTest;
		g_pDrawTest = nullptr;
	}
	if (g_pEffectTest)
	{
		delete g_pEffectTest;
		g_pEffectTest = nullptr;
	}
}