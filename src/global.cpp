#include "global.h"

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------

extern int IsInMemory = 1;
extern bool IsDialogBoxMode = false;
extern int g_LButtonUp = 1;

extern GameSystemInfo g_Game_System_Info = { 1280, 720 , 0, 0};

extern bool DontDraw = false;
extern unsigned char NationCode = 4;

extern bool g_bRenderStateLocked = false;

extern cltTextFileManager g_clTextFileManager = *(new cltTextFileManager());

extern DCTTextManager g_DCTTextManager = *(new DCTTextManager());

extern MoFFont g_MoFFont = *(new MoFFont());

extern DCTIMMList g_IMMList = *(new DCTIMMList());

extern HWND             g_hWnd = NULL;

extern short kGlyphIndexByStyleChar[] = { 51 }; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[] = { -1 }; //dword_6C6C54
extern int   kAdvanceByStyle[] = { 0 }; //dword_6C6C58
extern int   kAtlasIdByStyle[] = { 570425345 }; //dword_6C6C5C

extern unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
extern unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
extern unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

extern DWORD g_dwHeartBeatTime = 0;