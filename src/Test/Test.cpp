#include "Test/Test.h"
#include "conf.h"

ImageDrawTest* g_pDrawTest = nullptr;
EffectSystemTest* g_pEffectTest = nullptr;
FontSystemTest* g_pFontTest = nullptr;
UIBasicTest* g_pUITest = nullptr;

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
	//ImageSystemTester tester;
	//tester.RunImageTests();
	//text_manager_test();
	//run_sound_system_test_suite();
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
	case UI_BASIC: // <--- 新增這個 case
		g_pUITest = new UIBasicTest();
		if (FAILED(g_pUITest->Initialize())) {
			//MessageBox(g_hWnd, L"UIBasicTest 初始化失敗", L"錯誤", MB_OK | MB_ICONERROR);
			return -1;
		}
		break;
	}
	return 0;
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
	if (G_CURRENT_TEST == UI_BASIC && g_pUITest)
	{
		g_pUITest->Update(fElapsedTime);
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
			g_pEffectTest->Render();
		}
	}
	else if (G_CURRENT_TEST == TEST_FONT) {
		if (g_pFontTest)
		{
			g_pFontTest->Render();
		}
	}
	else if (G_CURRENT_TEST == UI_BASIC) {
		if (g_pUITest)
		{
			g_pUITest->Render();
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
	if (g_pFontTest)
	{
		delete g_pFontTest;
		g_pFontTest = nullptr;
	}
	if (g_pUITest)
	{
		delete g_pUITest;
		g_pUITest = nullptr;
	}
}