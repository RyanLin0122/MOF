#include "global.h"
#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltNPCInfo.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltPetKindInfo.h"
#include "Info/cltClientPetKindInfo.h"
#include "Info/cltMoFC_EffectKindInfo.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "System/cltQuestSystem.h"
#include "System/CMeritoriousSystem.h"
#include "System/CSpiritSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltClassSystem.h"
#include "System/cltPetSystem.h"
#include "System/cltMoneySystem.h"
#include "Util/cltTimer.h"
#include "Logic/Map.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltConfiguration.h"
#include "Logic/cltNPCManager.h"
#include "Logic/CObjectManager.h"
#include "Logic/cltMapTitle.h"
#include "Logic/cltMoF_MineManager.h"
#include "Logic/cltMatchManager.h"
#include "Logic/cltPKFlagManager.h"
#include "Logic/CSpiritSpeechMgr.h"
#include "Logic/SpiritSpeechParser.h"
#include "Logic/SpiritGossipParser.h"
#include "Info/cltQuestKindInfo.h"
#include "Logic/clTransportKindInfo.h"
#include "Logic/clClientTransportKindInfo.h"
#include "Logic/cltMyCharData.h"
#include "Logic/cltSystemMessage.h"
#include "UI/CUITutorial.h"
#include "UI/CUIManager.h"
#include "Object/CObjectResourceManager.h"
#include "Object/DrawObject_All.h"
#include "Network/CMoFNetwork.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"
#include "Logic/cltCashShopItem.h"
#include "Info/cltBasicAppearKindInfo.h"
#include "Sound/GameSound.h"


LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp = {};
LPDIRECT3DDEVICE9 Device = NULL;
HWND g_hWnd = NULL;

int SETTING_FRAME = 1;
int IsInMemory = 1;
bool IsDialogBoxMode = false;
int g_LButtonUp = 1;

GameSystemInfo g_Game_System_Info = { 1280, 720 , 0, 0};

bool DontDraw = false;
unsigned char NationCode = 4;
bool g_bRenderStateLocked = false;

short kGlyphIndexByStyleChar[] = { 51 };
int   kDotGlyphIndexByStyle[] = { -1 };
int   kAdvanceByStyle[] = { 0 };
int   kAtlasIdByStyle[] = { 570425345 };

int dword_A73088 = 0;
int dword_A7308C = 0;

unsigned int D3DRS_BLENDOP_TYPE[8] = { 1u, 1u, 1u, 5u, 4u, 1u, 2u, 1u };
unsigned int D3DRS_SRCBLEND_TYPE[8] = { 5u, 10u, 9u, 2u, 2u, 2u, 2u, 9u };
unsigned int D3DRS_DESTBLEND_TYPE[8] = { 6u, 2u, 1u, 2u, 2u, 2u, 2u, 5u };

DWORD g_dwHeartBeatTime = 0;
int g_nFieldItemPickupFlag = 0;
unsigned int g_dwMyAccountID = 0;
int g_nTutorialState = 0;

char           byte_23158C8[288]  = {};
short          word_23158E8[144]  = {};
char           byte_23158EA[288]  = {};
short          word_23158EC[144]  = {};
char           byte_23158EE[288]  = {};
char           byte_23158EF[288]  = {};
int            dword_23158F0[72]  = {};
char           byte_23158F4[288]  = {};
unsigned short word_23158F8[144]  = {};
unsigned short word_231590E[144]  = {};

cltTextFileManager g_clTextFileManager;
DCTTextManager g_DCTTextManager;
MoFFont g_MoFFont;
DCTIMMList g_IMMList;
Map g_Map;
CMoFNetwork g_Network;
cltClassKindInfo g_clClassKindInfo;
cltItemKindInfo g_clItemKindInfo;
cltNPCInfo g_clNPCInfo;
cltQuestKindInfo g_clQuestKindInfo;
CSpiritSystem g_clSpiritSystem;
cltQuestSystem g_clQuestSystem;
CMeritoriousSystem g_clMeritoriousSystem;
cltLevelSystem g_clLevelSystem;
cltClassSystem g_clClassSystem;
cltBaseInventory g_clMyInventory;
cltTimerManager g_clTimerManager;
cltFieldItemManager g_clFieldItemMgr;
cltHelpMessage g_clHelpMessage;
cltNPCManager g_clNPCManager;
ClientCharacterManager g_ClientCharMgr;
cltConfiguration* g_clConfig = nullptr;
CObjectManager g_ObjectManager;
CObjectResourceManager g_ObjectResourceManager;
DrawObject_All g_DrawObject_ALL;
CEffectManager g_EffectManager;
CUITutorial* g_pUITutorial = nullptr;
CUIManager* g_UIMgr = nullptr;

cltMapTitle g_clMapTitle;
cltMoF_MineManager g_clMineMgr;
cltMatchManager g_clMatchManager;
cltPKFlagManager g_clPKFlagManager;
CSpiritSpeechMgr g_clSpiritSpeechMgr;
CSpiritSpeechParser g_clSpiritSpeechParser;
CSpiritGossipParser g_clSpiritGossipParser;
clTransportKindInfo g_clTransportKindInfo;
clClientTransportKindInfo g_clClientTransportKindInfo;
CEffectManager* g_pEffectManager_Before_Chr = nullptr;
CEffectManager* g_pEffectManager_After_Chr = nullptr;

cltPetKindInfo g_clPetKindInfoBase;
cltClientPetKindInfo g_clClientPetKindInfo;
cltPetSystem g_clPetSystem;
cltMoneySystem g_clMoneySystem;
cltMoFC_EffectKindInfo g_clEffectKindInfo;
// cltMyCharData g_clMyCharData is defined in Logic/cltMyCharData.cpp
// cltSystemMessage g_clSysemMessage is defined in Logic/cltSystemMessage.cpp

cltCashShopItem g_clCashShopItem;
cltBasicAppearKindInfo g_clBasicAppearKindInfo;

GameSound g_GameSoundManager;
int dword_AFD34C = 0;
int dword_AFD344 = 0;

int dword_73D154 = 0;
int dword_B4BAB4 = 0;
char byte_21CB35D = 0;
cltFieldItem* unk_73D15C[1024] = {};
void* unk_813AA8[1024] = {};
void* unk_B4B924[1024] = {};
ClientCharacter unk_1409D80[1000] = {};
ClientCharacter* dword_1843F78[1000] = {};
size_t NumOfElements = 0;
