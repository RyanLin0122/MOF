#include "global.h"

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------

extern int IsInMemory = 1;
extern bool IsDialogBoxMode = false;

extern GameSystemInfo g_Game_System_Info = { 1280, 720 , 0, 0};

extern bool DontDraw = false;
extern unsigned char NationCode = 4;

extern bool g_bRenderStateLocked = false;

extern cltTextFileManager g_clTextFileManager = *(new cltTextFileManager());