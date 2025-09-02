#pragma once
#include <iostream>
#include <stdio.h>     // 用於 printf
#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <d3d9.h>
#include "Text/cltTextFileManager.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Text/DCTIMMList.h"

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------
// --- 全域變數定義 ---
extern int IsInMemory;
extern bool IsDialogBoxMode;
extern int g_LButtonUp;
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

extern DCTIMMList g_IMMList;

extern short kGlyphIndexByStyleChar[]; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[]; //dword_6C6C54
extern int   kAdvanceByStyle[]; //dword_6C6C58
extern int   kAtlasIdByStyle[]; //dword_6C6C5C