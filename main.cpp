#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)

#include "nfs_test.h"
#include "cm_packing_integration_test.h"
#include "Sound/COgg.h"  // 您的 COgg 類別標頭檔
#include "CMOFPacking.h" // 您的 CMofPacking 類別標頭檔 (單例版本)

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

int main() {
	std::cout << "Starting Virtual File System Tests..." << std::endl;
	std::cout << "========================================" << std::endl;

	// 執行所有測試
	//run_all_tests(); //nfs unit test
	//print_test_result();
    //run_cmofpacking_tests();
    ogg_play_test();
	return 0;
}