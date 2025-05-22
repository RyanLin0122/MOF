#include <iostream>
#include "nfs_test.h"
#include "nfs_integration_test.h"
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)

#include "Sound/COgg.h"  // 您的 COgg 類別標頭檔
#include "CMOFPacking.h" // 您的 CMofPacking 類別標頭檔 (單例版本)

void ogg_play_test() {
    // 測試音樂
    HWND hWnd = GetConsoleWindow();
    if (hWnd == nullptr) {
        printf("警告：無法獲取主控台視窗句柄。FSOUND_SetHWND 將使用 nullptr。\n");
        // 在某些情況下，FMOD 可能仍然可以運作，或者需要一個有效的視窗。
        // 對於簡單測試，這可能不是主要問題。
    }
    printf("FMOD OGG 播放測試\n");
    printf("------------------------\n");

    // 創建並初始化 COgg 播放器物件
    COgg oggPlayer;

    // 初始化 COgg。參數 1 表示啟用循環播放，0 表示不循環。
    printf("正在初始化 COgg 播放器 (啟用循環播放)...\n");
    oggPlayer.Initalize(1);

    const char* oggFilePath = "D:\\MOF\\mofdata\\music\\bg_beavers.ogg";
    printf("準備播放檔案: %s\n", oggFilePath);

    // 播放指定的 OGG 檔案
    oggPlayer.Play(oggFilePath);

    printf("\n音樂應該正在播放 (如果檔案存在且 FMOD 初始化成功)。\n");
    printf("按下任意鍵停止音樂並退出程式。\n\n");

    // 保持程式運行並定期更新 FMOD，直到使用者按下按鍵
    while (!_kbhit()) { // 當沒有鍵盤按下時循環
        FSOUND_Update(); // 定期呼叫 FSOUND_Update() 很重要，FMOD 依賴它來更新音訊串流等
        Sleep(50);       // 短暫休眠以降低 CPU 使用率
    }
    _getch(); // 讀取並清除按鍵緩衝

    printf("正在停止音樂...\n");
    oggPlayer.Stop(); // 明確停止音樂

    // COgg 的解構函式 (~COgg) 應會呼叫 FSOUND_Close() 來關閉 FMOD 系統。
    // 在程式結束前，銷毀 CMofPacking 的單例實例
    printf("正在銷毀 CMofPacking 實例...\n");
    CMofPacking::DestroyInstance();

    printf("測試程式結束。\n");
}

int main() {
	std::cout << "Starting Virtual File System Tests..." << std::endl;
	std::cout << "========================================" << std::endl;

	// 執行所有測試
	run_all_tests();
	print_test_result();
	/*
	test_create_write_read_small();
	test_cross_block_write_read();
	test_multi_channel_isolation();
	test_channel_truncate();
	printf("ALL INTEGRATION TESTS PASSED\n");
	*/
    ogg_play_test();
	return 0;
}