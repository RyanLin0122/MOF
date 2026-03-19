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
#include "Object/CObjectResourceManager.h"
#include "Network/CMoFNetwork.h"

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp = {};
LPDIRECT3DDEVICE9 Device = NULL;
int SETTING_FRAME = 1;

int IsInMemory = 1;
bool IsDialogBoxMode = false;
int g_LButtonUp = 1;

GameSystemInfo g_Game_System_Info = { 1280, 720 , 0, 0};

bool DontDraw = false;
unsigned char NationCode = 4;

bool g_bRenderStateLocked = false;

cltTextFileManager g_clTextFileManager;

DCTTextManager g_DCTTextManager;

MoFFont g_MoFFont;

DCTIMMList g_IMMList;
cltClassKindInfo g_clClassKindInfo;
cltItemKindInfo g_clItemKindInfo;
Map g_Map;

HWND g_hWnd = NULL;

short kGlyphIndexByStyleChar[] = { 51 }; //word_6C6B6C
int   kDotGlyphIndexByStyle[] = { -1 }; //dword_6C6C54
int   kAdvanceByStyle[] = { 0 }; //dword_6C6C58
int   kAtlasIdByStyle[] = { 570425345 }; //dword_6C6C5C

// Camera/scroll offset
int dword_A73088 = 0;
int dword_A7308C = 0;

unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

DWORD g_dwHeartBeatTime = 0;

unsigned int g_dwMyAccountID = 0;
ClientCharacterManager& g_ClientCharMgr = *ClientCharacterManager::GetInstance();
CMoFNetwork g_Network;

// Character-select slot data (3 slots × 96 bytes; filled by server packets)
char           byte_23158C8[288]  = {};  // name[32] × 3 slots
short          word_23158E8[144]  = {};  // charKind × 3 slots       (stride: 48 shorts)
char           byte_23158EA[288]  = {};  // nation × 3 slots
short          word_23158EC[144]  = {};  // charKindInit × 3 slots   (stride: 48 shorts)
char           byte_23158EE[288]  = {};  // sex × 3 slots
char           byte_23158EF[288]  = {};  // hair × 3 slots
int            dword_23158F0[72]  = {};  // mapKind × 3 slots        (stride: 24 ints)
char           byte_23158F4[288]  = {};  // classKind × 3 slots
unsigned short word_23158F8[144]  = {};  // equipKind1 × 3 slots     (stride: 48 shorts)
unsigned short word_231590E[144]  = {};  // equipKind2 × 3 slots     (stride: 48 shorts)

// Tutorial globals
int g_nTutorialState = 0;

cltQuestSystem       g_clQuestSystem;
CMeritoriousSystem   g_clMeritoriousSystem;
cltBaseInventory     g_clMyInventory;
cltTimerManager      g_clTimerManager;
cltFieldItemManager  g_clFieldItemMgr;
cltHelpMessage       g_clHelpMessage;
cltNPCManager        g_clNPCManager;
CObjectManager       g_ObjectManager;
CObjectResourceManager g_ObjectResourceManager;

int g_nFieldItemPickupFlag = 0;
cltConfiguration* g_clConfig = nullptr;
CUITutorial* g_pUITutorial = nullptr;
CUIManager* g_UIMgr = nullptr;
