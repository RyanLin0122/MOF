#include <iostream>
#include <stdio.h>     // �Ω� printf
#include <windows.h>   // �Ω� HWND, GetConsoleWindow, Sleep
#include <conio.h>     // �Ω� _kbhit, _getch (�ˬd��L��J)
#include <d3d9.h>

//-----------------------------------------------------------------------------
// �����ܼ�
//-----------------------------------------------------------------------------
// --- �����ܼƩw�q ---
extern int IsInMemory;
extern bool IsDialogBoxMode;

struct GameSystemInfo {
    int ScreenWidth;
    int ScreenHeight;
};

extern GameSystemInfo g_Game_System_Info;

extern bool DontDraw;
extern unsigned char NationCode;