#pragma once
#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <d3d9.h>
#include "Text/cltTextFileManager.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------
// --- 全域變數定義 ---
extern int IsInMemory;
extern bool IsDialogBoxMode;
extern HWND             g_hWnd;

struct GameSystemInfo {
    int ScreenWidth;
    int ScreenHeight;
    float ScreenX;
	float ScreenY;
};


extern GameSystemInfo g_Game_System_Info;

extern bool DontDraw;
extern unsigned char NationCode;

extern bool g_bRenderStateLocked;

extern cltTextFileManager g_clTextFileManager;

extern DCTTextManager g_DCTTextManager;

extern MoFFont g_MoFFont;