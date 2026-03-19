#pragma once
#include <iostream>
#include <stdio.h>     // 用於 printf

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <mmsystem.h>  // 用於 timeGetTime
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <d3d9.h>
#include "Text/cltTextFileManager.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Text/DCTIMMList.h"

class cltClassKindInfo;
class cltItemKindInfo;
class cltMapInfo;
class Map;
class ClientCharacterManager;
class cltQuestSystem;
class CMeritoriousSystem;
class cltBaseInventory;
class cltCharKindInfo;
#include "Util/cltTimer.h"
class cltFieldItemManager;
class cltHelpMessage;
class cltConfiguration;
class CUITutorial;
class CUIManager;
class cltNPCManager;
class CObjectManager;
class CObjectResourceManager;
class CMoFNetwork;

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------
// --- 全域變數定義 ---
extern int IsInMemory;
extern bool IsDialogBoxMode;
extern int g_LButtonUp;
extern HWND g_hWnd;
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern D3DPRESENT_PARAMETERS g_d3dpp;
extern LPDIRECT3DDEVICE9 Device;
extern int SETTING_FRAME;

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
extern cltClassKindInfo g_clClassKindInfo;
extern cltItemKindInfo g_clItemKindInfo;
extern Map g_Map;
extern CObjectManager       g_ObjectManager;
extern CObjectResourceManager g_ObjectResourceManager;

extern short kGlyphIndexByStyleChar[]; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[]; //dword_6C6C54
extern int   kAdvanceByStyle[]; //dword_6C6C58
extern int   kAtlasIdByStyle[]; //dword_6C6C5C

// Camera/scroll offset (world coordinate of the top-left screen corner)
extern int dword_A73088;  // camera scroll X
extern int dword_A7308C;  // camera scroll Y

extern unsigned int D3DRS_BLENDOP_TYPE[8];
extern unsigned int D3DRS_SRCBLEND_TYPE[8];
extern unsigned int D3DRS_DESTBLEND_TYPE[8];

extern DWORD g_dwHeartBeatTime;

extern unsigned int g_dwMyAccountID;
extern ClientCharacterManager& g_ClientCharMgr;
extern CMoFNetwork g_Network;

// Character-select slot data (filled from server packets; 3 slots × 96 bytes).
// Slot layout (byte offset from slot base = 96 * slotIndex):
//   0x00  char[32]   name           (byte_23158C8)
//   0x20  short      charKind       (word_23158E8)  — AddCharacter
//   0x22  char       nation         (byte_23158EA)
//   0x24  short      charKindInit   (word_23158EC)  — cltMyCharData::Initialize
//   0x26  char       sex            (byte_23158EE)
//   0x27  char       hair           (byte_23158EF)
//   0x28  int        mapKind        (dword_23158F0)
//   0x2C  char       classKind      (byte_23158F4)
//   0x30  uint16[11] equipKind1     (&word_23158F8)
//   0x46  uint16[11] equipKind2     (&word_231590E)
extern char           byte_23158C8[];   // name × 3 slots
extern short          word_23158E8[];   // charKind × 3 slots       (stride: 48 shorts)
extern char           byte_23158EA[];   // nation × 3 slots
extern short          word_23158EC[];   // charKindInit × 3 slots   (stride: 48 shorts)
extern char           byte_23158EE[];   // sex × 3 slots
extern char           byte_23158EF[];   // hair × 3 slots
extern int            dword_23158F0[];  // mapKind × 3 slots        (stride: 24 ints)
extern char           byte_23158F4[];   // classKind × 3 slots
extern unsigned short word_23158F8[];   // equipKind1 array start   (base of 3×11 shorts, stride 96 bytes)
extern unsigned short word_231590E[];   // equipKind2 array start   (base of 3×11 shorts, stride 96 bytes)

// Tutorial globals
// (full class definitions are in their respective headers)
extern int g_nTutorialState;
extern int g_nFieldItemPickupFlag;
extern cltConfiguration* g_clConfig;
extern CUITutorial* g_pUITutorial;
extern CUIManager* g_UIMgr;

enum Direction : uint8_t {
    DirUp = 0,        // 氣球箭頭朝上（置中）
    DirLeft = 1,        // 箭頭朝左（垂直置中）
    DirRight = 2,        // 箭頭朝右（垂直置中）
    DirDown = 3         // 箭頭朝下（置中）== (DirLeft | DirRight)
};
