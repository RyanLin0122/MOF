#include "global.h"
#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMapInfo.h"
#include "Character/ClientCharacterManager.h"
#include "System/cltQuestSystem.h"
#include "System/CMeritoriousSystem.h"
#include "Logic/cltBaseInventory.h"
#include "Info/cltCharKindInfo.h"
#include "Util/cltTimer.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/cltConfiguration.h"
#include "UI/CUITutorial.h"
#include "UI/CUIManager.h"
#include "Logic/cltNPCManager.h"
#include "Logic/CObjectManager.h"

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
extern cltClassKindInfo g_clClassKindInfo = *(new cltClassKindInfo());
extern cltItemKindInfo g_clItemKindInfo = *(new cltItemKindInfo());
extern cltMapInfo g_Map = *(new cltMapInfo());

extern HWND             g_hWnd = NULL;

extern short kGlyphIndexByStyleChar[] = { 51 }; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[] = { -1 }; //dword_6C6C54
extern int   kAdvanceByStyle[] = { 0 }; //dword_6C6C58
extern int   kAtlasIdByStyle[] = { 570425345 }; //dword_6C6C5C

extern unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
extern unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
extern unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

extern DWORD g_dwHeartBeatTime = 0;

extern unsigned int g_dwMyAccountID = 0;
extern ClientCharacterManager& g_ClientCharMgr = *ClientCharacterManager::GetInstance();

// Tutorial globals
extern int g_nTutorialState = 0;

cltQuestSystem       g_clQuestSystem;
CMeritoriousSystem   g_clMeritoriousSystem;
cltBaseInventory     g_clMyInventory;
cltTimerManager      g_clTimerManager;
cltFieldItemManager  g_clFieldItemMgr;
cltHelpMessage       g_clHelpMessage;
cltNPCManager        g_clNPCManager;
CObjectManager       g_ObjectManager;

extern int           g_nFieldItemPickupFlag = 0;
extern cltConfiguration* g_clConfig        = nullptr;
extern CUITutorial*  g_pUITutorial          = nullptr;
extern CUIManager*   g_UIMgr                = nullptr;
