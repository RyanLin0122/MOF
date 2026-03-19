#include "global.h"
#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Logic/Map.h"
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
#include "Network/CMoFNetwork.h"

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
Map g_Map;

extern HWND             g_hWnd = NULL;

extern short kGlyphIndexByStyleChar[] = { 51 }; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[] = { -1 }; //dword_6C6C54
extern int   kAdvanceByStyle[] = { 0 }; //dword_6C6C58
extern int   kAtlasIdByStyle[] = { 570425345 }; //dword_6C6C5C

// Camera/scroll offset
int dword_A73088 = 0;
int dword_A7308C = 0;

extern unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
extern unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
extern unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

extern DWORD g_dwHeartBeatTime = 0;

extern unsigned int g_dwMyAccountID = 0;
extern ClientCharacterManager& g_ClientCharMgr = *ClientCharacterManager::GetInstance();
CMoFNetwork g_Network;

// Character-select slot data (3 slots × 96 bytes; filled by server packets)
extern char           byte_23158C8[288]  = {};  // name[32] × 3 slots
extern short          word_23158E8[144]  = {};  // charKind × 3 slots       (stride: 48 shorts)
extern char           byte_23158EA[288]  = {};  // nation × 3 slots
extern short          word_23158EC[144]  = {};  // charKindInit × 3 slots   (stride: 48 shorts)
extern char           byte_23158EE[288]  = {};  // sex × 3 slots
extern char           byte_23158EF[288]  = {};  // hair × 3 slots
extern int            dword_23158F0[72]  = {};  // mapKind × 3 slots        (stride: 24 ints)
extern char           byte_23158F4[288]  = {};  // classKind × 3 slots
extern unsigned short word_23158F8[144]  = {};  // equipKind1 × 3 slots     (stride: 48 shorts)
extern unsigned short word_231590E[144]  = {};  // equipKind2 × 3 slots     (stride: 48 shorts)

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
